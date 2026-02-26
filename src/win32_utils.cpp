#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32_utils.h"

std::wstring FormatWin32Error(unsigned long code) {
  wchar_t *buffer = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD length =
      FormatMessageW(flags, nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

  if (!buffer || length == 0) {
    return L"Win32 error " + std::to_wstring(code);
  }

  std::wstring message(buffer, length);
  LocalFree(buffer);

  while (!message.empty() &&
         (message.back() == L'\r' || message.back() == L'\n' || message.back() == L' ' ||
          message.back() == L'\t')) {
    message.pop_back();
  }

  return message;
}

bool DirectoryExists(const std::wstring &directory) {
  const DWORD attributes = GetFileAttributesW(directory.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
