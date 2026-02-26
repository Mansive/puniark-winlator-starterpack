#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winver.h>

#include "asset_deploy.h"

#include "output.h"
#include "path_utils.h"
#include "win32_utils.h"

#include <cwchar>
#include <vector>

namespace {

struct VersionStrings {
  std::wstring file_description;
  std::wstring original_filename;
  std::wstring product_name;
};

struct Translation {
  WORD language;
  WORD code_page;
};

constexpr wchar_t kUalFileDescription[] = L"ultimate asi loader";
constexpr wchar_t kUalNameNeedle[] = L"ultimate-asi-loader";

bool FileExists(const std::wstring &path) {
  const DWORD attributes = GetFileAttributesW(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool EnsureDirectory(const std::wstring &directory, std::wstring *out_error) {
  if (DirectoryExists(directory)) {
    return true;
  }

  if (CreateDirectoryW(directory.c_str(), nullptr)) {
    return true;
  }

  const DWORD err = GetLastError();
  if (err == ERROR_ALREADY_EXISTS && DirectoryExists(directory)) {
    return true;
  }

  *out_error = L"Could not create directory " + directory + L": " + FormatWin32Error(err);
  return false;
}

bool GetExecutableDirectory(std::wstring *out_directory, std::wstring *out_error) {
  std::vector<wchar_t> buffer(MAX_PATH, L'\0');

  while (true) {
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) {
      *out_error = L"Could not resolve executable path: " + FormatWin32Error(GetLastError());
      return false;
    }

    if (length < buffer.size()) {
      const std::wstring executable_path(buffer.data(), length);
      const size_t slash = executable_path.find_last_of(L"\\/");
      if (slash == std::wstring::npos) {
        *out_error = L"Could not resolve executable directory for: " + executable_path;
        return false;
      }

      *out_directory = executable_path.substr(0, slash);
      return true;
    }

    if (buffer.size() >= 32768) {
      *out_error = L"Executable path is too long to resolve.";
      return false;
    }

    buffer.resize(buffer.size() * 2, L'\0');
  }
}

bool CollectDllPaths(const std::wstring &directory, std::vector<std::wstring> *out_paths,
                     std::wstring *out_error) {
  out_paths->clear();

  WIN32_FIND_DATAW data{};
  const std::wstring pattern = JoinPath(directory, L"*.dll");
  HANDLE find_handle = FindFirstFileW(pattern.c_str(), &data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    const DWORD err = GetLastError();
    if (err == ERROR_FILE_NOT_FOUND) {
      return true;
    }

    *out_error = L"Could not list DLL assets in " + directory + L": " + FormatWin32Error(err);
    return false;
  }

  do {
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      continue;
    }

    out_paths->push_back(JoinPath(directory, data.cFileName));
  } while (FindNextFileW(find_handle, &data));

  const DWORD final_error = GetLastError();
  FindClose(find_handle);
  if (final_error != ERROR_NO_MORE_FILES) {
    *out_error = L"Could not list DLL assets in " + directory + L": " +
                 FormatWin32Error(final_error);
    return false;
  }

  SortPaths(out_paths);
  return true;
}

bool CopyDirectoryContentsRecursive(const std::wstring &source_dir, const std::wstring &dest_dir,
                                    bool debug, std::wstring *out_error) {
  if (!EnsureDirectory(dest_dir, out_error)) {
    return false;
  }

  WIN32_FIND_DATAW data{};
  const std::wstring pattern = JoinPath(source_dir, L"*");
  HANDLE find_handle = FindFirstFileW(pattern.c_str(), &data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    *out_error = L"Could not list directory " + source_dir + L": " +
                 FormatWin32Error(GetLastError());
    return false;
  }

  do {
    const std::wstring entry_name(data.cFileName);
    if (entry_name == L"." || entry_name == L"..") {
      continue;
    }

    const std::wstring source_path = JoinPath(source_dir, entry_name);
    const std::wstring dest_path = JoinPath(dest_dir, entry_name);
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      if (!CopyDirectoryContentsRecursive(source_path, dest_path, debug, out_error)) {
        FindClose(find_handle);
        return false;
      }
      continue;
    }

    if (!CopyFileW(source_path.c_str(), dest_path.c_str(), FALSE)) {
      *out_error = L"Could not copy " + source_path + L" to " + dest_path + L": " +
                   FormatWin32Error(GetLastError());
      FindClose(find_handle);
      return false;
    }

    DebugLine(debug, L"Copied script asset: " + source_path + L" -> " + dest_path);
  } while (FindNextFileW(find_handle, &data));

  const DWORD final_error = GetLastError();
  FindClose(find_handle);
  if (final_error != ERROR_NO_MORE_FILES) {
    *out_error = L"Could not list directory " + source_dir + L": " +
                 FormatWin32Error(final_error);
    return false;
  }

  return true;
}

std::wstring BuildHookedFileName(const std::wstring &dll_name) {
  const size_t dot = dll_name.find_last_of(L'.');
  if (dot == std::wstring::npos) {
    return dll_name + L"Hooked.dll";
  }

  return dll_name.substr(0, dot) + L"Hooked.dll";
}

bool ContainsCaseInsensitive(const std::wstring &haystack, const std::wstring &needle) {
  if (haystack.empty() || needle.empty()) {
    return false;
  }

  return ToLower(haystack).find(ToLower(needle)) != std::wstring::npos;
}

bool QueryVersionString(void *version_block, const std::vector<Translation> &translations,
                        const wchar_t *key, std::wstring *out_value) {
  wchar_t query_block[128];
  for (const Translation &translation : translations) {
    const int written = swprintf(query_block, sizeof(query_block) / sizeof(query_block[0]),
                                 L"\\StringFileInfo\\%04x%04x\\%ls", translation.language,
                                 translation.code_page, key);
    if (written <= 0) {
      continue;
    }

    LPVOID value = nullptr;
    UINT value_size = 0;
    if (VerQueryValueW(version_block, query_block, &value, &value_size) && value != nullptr &&
        value_size > 1) {
      *out_value = std::wstring(static_cast<const wchar_t *>(value));
      return true;
    }
  }

  return false;
}

bool ReadVersionStrings(const std::wstring &dll_path, VersionStrings *out_strings) {
  DWORD handle = 0;
  const DWORD version_size = GetFileVersionInfoSizeW(dll_path.c_str(), &handle);
  if (version_size == 0) {
    return false;
  }

  std::vector<BYTE> version_blob(version_size);
  if (!GetFileVersionInfoW(dll_path.c_str(), 0, version_size, version_blob.data())) {
    return false;
  }

  std::vector<Translation> translations;
  LPVOID translation_ptr = nullptr;
  UINT translation_size = 0;
  if (VerQueryValueW(version_blob.data(), L"\\VarFileInfo\\Translation", &translation_ptr,
                     &translation_size) &&
      translation_ptr != nullptr && translation_size >= sizeof(Translation)) {
    const auto *items = static_cast<const Translation *>(translation_ptr);
    const size_t count = translation_size / sizeof(Translation);
    translations.assign(items, items + count);
  }

  if (translations.empty()) {
    translations.push_back({0x0409, 0x04B0});
    translations.push_back({0x0409, 0x04E4});
  }

  QueryVersionString(version_blob.data(), translations, L"FileDescription",
                     &out_strings->file_description);
  QueryVersionString(version_blob.data(), translations, L"OriginalFilename",
                     &out_strings->original_filename);
  QueryVersionString(version_blob.data(), translations, L"ProductName", &out_strings->product_name);

  return !out_strings->file_description.empty() || !out_strings->original_filename.empty() ||
         !out_strings->product_name.empty();
}

bool IsUltimateAsiLoaderDll(const std::wstring &dll_path) {
  VersionStrings strings;
  if (!ReadVersionStrings(dll_path, &strings)) {
    return false;
  }

  if (ToLower(strings.file_description) == kUalFileDescription) {
    return true;
  }

  return ContainsCaseInsensitive(strings.original_filename, kUalNameNeedle) ||
         ContainsCaseInsensitive(strings.product_name, kUalNameNeedle);
}

bool CopyDllWithBackupRule(const std::wstring &source_dll, const std::wstring &target_directory,
                           bool debug, std::wstring *out_error) {
  const std::wstring dll_name = GetBaseName(source_dll);
  const std::wstring target_dll = JoinPath(target_directory, dll_name);

  if (FileExists(target_dll)) {
    if (IsUltimateAsiLoaderDll(target_dll)) {
      DebugLine(debug, L"Existing UAL DLL detected, replacing in place: " + target_dll);
    } else {
      const std::wstring hooked_path = JoinPath(target_directory, BuildHookedFileName(dll_name));
      if (GetFileAttributesW(hooked_path.c_str()) != INVALID_FILE_ATTRIBUTES) {
        *out_error = L"Cannot backup existing DLL because " + hooked_path +
                     L" already exists. Remove or rename that file and retry.";
        return false;
      }

      if (!MoveFileW(target_dll.c_str(), hooked_path.c_str())) {
        *out_error = L"Could not rename existing DLL " + target_dll + L" to " + hooked_path + L": " +
                     FormatWin32Error(GetLastError());
        return false;
      }

      DebugLine(debug, L"Renamed existing DLL: " + target_dll + L" -> " + hooked_path);
    }
  }

  if (!CopyFileW(source_dll.c_str(), target_dll.c_str(), FALSE)) {
    *out_error = L"Could not copy " + source_dll + L" to " + target_dll + L": " +
                 FormatWin32Error(GetLastError());
    return false;
  }

  DebugLine(debug, L"Copied DLL: " + source_dll + L" -> " + target_dll);
  return true;
}

}  // namespace

bool DeployArchitectureAssets(const std::wstring &target_directory, Bitness architecture, bool debug,
                              std::wstring *out_error) {
  std::wstring ignored_error;
  if (out_error == nullptr) {
    out_error = &ignored_error;
  }
  out_error->clear();

  const DWORD target_attributes = GetFileAttributesW(target_directory.c_str());
  if (target_attributes == INVALID_FILE_ATTRIBUTES) {
    const DWORD err = GetLastError();
    if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND) {
      *out_error = L"Target directory not found: " + target_directory;
    } else {
      *out_error = L"Could not access target directory " + target_directory + L": " +
                   FormatWin32Error(err);
    }
    return false;
  }

  if ((target_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    *out_error = L"Target is not a directory: " + target_directory;
    return false;
  }

  std::wstring architecture_folder;
  if (architecture == Bitness::Bit32) {
    architecture_folder = L"win32";
  } else if (architecture == Bitness::Bit64) {
    architecture_folder = L"win64";
  } else {
    *out_error = L"Unknown architecture; no deployable asset set is available.";
    return false;
  }

  std::wstring executable_directory;
  if (!GetExecutableDirectory(&executable_directory, out_error)) {
    return false;
  }

  const std::wstring source_root = JoinPath(executable_directory, architecture_folder);
  if (!DirectoryExists(source_root)) {
    *out_error = L"Missing asset directory: " + source_root;
    return false;
  }

  DebugLine(debug, L"Deploying " + architecture_folder + L" assets from " + source_root + L" to " +
                       target_directory);

  std::vector<std::wstring> source_dlls;
  if (!CollectDllPaths(source_root, &source_dlls, out_error)) {
    return false;
  }

  if (source_dlls.empty()) {
    *out_error = L"No DLL assets were found in: " + source_root;
    return false;
  }

  for (const std::wstring &source_dll : source_dlls) {
    if (!CopyDllWithBackupRule(source_dll, target_directory, debug, out_error)) {
      return false;
    }
  }

  const std::wstring source_scripts = JoinPath(source_root, L"scripts");
  if (!DirectoryExists(source_scripts)) {
    *out_error = L"Missing scripts directory in asset source: " + source_scripts;
    return false;
  }

  const std::wstring target_scripts = JoinPath(target_directory, L"scripts");
  if (!CopyDirectoryContentsRecursive(source_scripts, target_scripts, debug, out_error)) {
    return false;
  }

  DebugLine(debug, L"Finished deploying assets to " + target_directory);
  return true;
}
