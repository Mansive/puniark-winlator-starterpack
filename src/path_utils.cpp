#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "path_utils.h"

#include <algorithm>
#include <cwctype>

std::wstring ToLower(const std::wstring &text) {
  std::wstring lowered = text;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towlower(ch));
  });
  return lowered;
}

std::string WideToUtf8(const std::wstring &text) {
  if (text.empty()) {
    return std::string();
  }

  const int needed =
      WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0,
                          nullptr, nullptr);
  if (needed <= 0) {
    return std::string();
  }

  std::string out(static_cast<size_t>(needed), '\0');
  const int written = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                          &out[0], needed, nullptr, nullptr);
  if (written <= 0) {
    return std::string();
  }

  out.resize(static_cast<size_t>(written));
  return out;
}

std::wstring JoinPath(const std::wstring &left, const std::wstring &right) {
  if (left.empty()) {
    return right;
  }

  if (left.back() == L'\\' || left.back() == L'/') {
    return left + right;
  }

  return left + L"\\" + right;
}

std::wstring NormalizePath(const std::wstring &path) {
  wchar_t buffer[MAX_PATH];
  const DWORD count = GetFullPathNameW(path.c_str(), MAX_PATH, buffer, nullptr);
  if (count == 0 || count >= MAX_PATH) {
    return path;
  }
  return std::wstring(buffer);
}

std::wstring GetBaseName(const std::wstring &path) {
  const size_t slash = path.find_last_of(L"\\/");
  if (slash == std::wstring::npos) {
    return path;
  }
  if (slash + 1 >= path.size()) {
    return path;
  }
  return path.substr(slash + 1);
}

namespace {

bool PathLess(const std::wstring &a, const std::wstring &b) {
  const std::wstring a_lower = ToLower(a);
  const std::wstring b_lower = ToLower(b);
  if (a_lower < b_lower) {
    return true;
  }
  if (a_lower > b_lower) {
    return false;
  }
  return a < b;
}

}  // namespace

void SortPaths(std::vector<std::wstring> *paths) {
  std::sort(paths->begin(), paths->end(), PathLess);
}
