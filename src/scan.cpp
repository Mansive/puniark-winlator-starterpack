#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "scan.h"

#include "path_utils.h"
#include "win32_utils.h"

#include <unordered_set>

namespace {

constexpr wchar_t kDefaultScanDirectory[] = L"A:\\";
constexpr wchar_t kFallbackScanDirectory[] = L"X:\\";

bool HasExeExtension(const std::wstring &filename) {
  const size_t dot = filename.find_last_of(L'.');
  if (dot == std::wstring::npos) {
    return false;
  }
  return ToLower(filename.substr(dot)) == L".exe";
}

}  // namespace

std::vector<std::wstring> CollectScanDirectories(const std::vector<std::wstring> &raw_directories) {
  if (raw_directories.empty()) {
    if (!DirectoryExists(kDefaultScanDirectory)) {
      return {kFallbackScanDirectory};
    }
    return {kDefaultScanDirectory};
  }

  std::unordered_set<std::wstring> seen;
  std::vector<std::wstring> directories;
  directories.reserve(raw_directories.size());

  for (const std::wstring &raw : raw_directories) {
    std::wstring normalized = NormalizePath(raw);
    std::wstring key = ToLower(normalized);
    if (seen.find(key) != seen.end()) {
      continue;
    }
    seen.insert(key);
    directories.push_back(normalized);
  }

  return directories;
}

bool CollectExecutableTargetsFromDirectory(const std::wstring &directory,
                                           std::vector<std::wstring> *out_paths,
                                           std::wstring *out_error) {
  const DWORD attributes = GetFileAttributesW(directory.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    const DWORD err = GetLastError();
    if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND) {
      *out_error = L"Directory not found: " + directory;
    } else {
      *out_error = L"Could not access directory " + directory + L": " + FormatWin32Error(err);
    }
    return false;
  }

  if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    *out_error = L"Not a directory: " + directory;
    return false;
  }

  std::wstring pattern = JoinPath(directory, L"*");
  WIN32_FIND_DATAW data;
  HANDLE find_handle = FindFirstFileW(pattern.c_str(), &data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    *out_error = L"Could not list directory " + directory + L": " +
                 FormatWin32Error(GetLastError());
    return false;
  }

  do {
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      continue;
    }

    std::wstring filename(data.cFileName);
    if (HasExeExtension(filename)) {
      out_paths->push_back(JoinPath(directory, filename));
    }
  } while (FindNextFileW(find_handle, &data));

  FindClose(find_handle);
  SortPaths(out_paths);
  return true;
}

std::wstring DetermineGameArchitecture(int bit32_count, int bit64_count, int unknown_count,
                                       int error_count) {
  if (bit64_count > 0) {
    return L"64-bit";
  }

  if (bit32_count > 0) {
    return L"32-bit";
  }

  if (unknown_count > 0) {
    return L"Unknown (unrecognized executable format)";
  }

  if (error_count > 0) {
    return L"Unknown (could not read executable files)";
  }

  return L"Unknown (no executables found)";
}
