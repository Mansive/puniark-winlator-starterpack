#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "output.h"

#include "path_utils.h"

#include <array>

namespace {

HANDLE g_stdout_handle = INVALID_HANDLE_VALUE;
HANDLE g_stderr_handle = INVALID_HANDLE_VALUE;
HANDLE g_conout_handle = INVALID_HANDLE_VALUE;
std::wstring g_fallback_log_path;

void AppendFallbackLog(const std::wstring &line) {
  if (g_fallback_log_path.empty()) {
    return;
  }

  HANDLE file = CreateFileW(g_fallback_log_path.c_str(), FILE_APPEND_DATA,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return;
  }

  std::string utf8 = WideToUtf8(line + L"\r\n");
  if (!utf8.empty()) {
    DWORD written = 0;
    WriteFile(file, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr);
  }

  CloseHandle(file);
}

bool WriteWideLine(HANDLE handle, const std::wstring &line) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD console_mode = 0;
  if (GetConsoleMode(handle, &console_mode)) {
    DWORD written = 0;
    if (!WriteConsoleW(handle, line.c_str(), static_cast<DWORD>(line.size()), &written, nullptr)) {
      return false;
    }

    if (!WriteConsoleW(handle, L"\r\n", 2, &written, nullptr)) {
      return false;
    }

    return true;
  }

  std::string utf8 = WideToUtf8(line + L"\r\n");
  if (utf8.empty()) {
    return false;
  }

  DWORD written = 0;
  return WriteFile(handle, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr) &&
         written == static_cast<DWORD>(utf8.size());
}

}  // namespace

void OutLine(const std::wstring &line) {
  if (WriteWideLine(g_stdout_handle, line)) {
    return;
  }
  if (g_conout_handle != INVALID_HANDLE_VALUE && WriteWideLine(g_conout_handle, line)) {
    return;
  }
  AppendFallbackLog(line);
}

void ErrLine(const std::wstring &line) {
  if (WriteWideLine(g_stderr_handle, line)) {
    return;
  }
  if (g_conout_handle != INVALID_HANDLE_VALUE && WriteWideLine(g_conout_handle, line)) {
    return;
  }
  AppendFallbackLog(line);
}

void DebugLine(bool enabled, const std::wstring &line) {
  if (!enabled) {
    return;
  }
  OutLine(L"[debug] " + line);
}

void InitOutput() {
  g_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  g_stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

  std::array<wchar_t, MAX_PATH> temp_path{};
  const DWORD temp_len = GetTempPathW(static_cast<DWORD>(temp_path.size()), temp_path.data());
  if (temp_len > 0 && temp_len < MAX_PATH) {
    g_fallback_log_path = JoinPath(std::wstring(temp_path.data()), L"bitnessscan.log");
  } else {
    std::array<wchar_t, MAX_PATH> exe_path{};
    const DWORD exe_len =
        GetModuleFileNameW(nullptr, exe_path.data(), static_cast<DWORD>(exe_path.size()));
    if (exe_len > 0 && exe_len < MAX_PATH) {
      std::wstring exe_dir(exe_path.data());
      const size_t slash = exe_dir.find_last_of(L"\\/");
      if (slash != std::wstring::npos) {
        exe_dir.resize(slash);
      }
      g_fallback_log_path = JoinPath(exe_dir, L"bitnessscan.log");
    } else {
      g_fallback_log_path = L"bitnessscan.log";
    }
  }

  g_conout_handle = CreateFileW(L"CONOUT$", GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void CloseOutput() {
  if (g_conout_handle != INVALID_HANDLE_VALUE) {
    CloseHandle(g_conout_handle);
    g_conout_handle = INVALID_HANDLE_VALUE;
  }
}
