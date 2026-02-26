#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "pe_bitness.h"

#include "win32_utils.h"

#include <array>

#ifndef SCS_64BIT_BINARY
#define SCS_64BIT_BINARY 6
#endif

namespace {

class HandleGuard {
 public:
  explicit HandleGuard(HANDLE handle) : handle_(handle) {}

  ~HandleGuard() {
    if (handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(handle_);
    }
  }

  HANDLE get() const {
    return handle_;
  }

  HandleGuard(const HandleGuard &) = delete;
  HandleGuard &operator=(const HandleGuard &) = delete;

 private:
  HANDLE handle_;
};

bool DetectWithGetBinaryType(const std::wstring &path, Bitness *out_bitness) {
  DWORD binary_type = 0;
  if (!GetBinaryTypeW(path.c_str(), &binary_type)) {
    return false;
  }

  switch (binary_type) {
    case SCS_32BIT_BINARY:
      *out_bitness = Bitness::Bit32;
      return true;
    case SCS_64BIT_BINARY:
      *out_bitness = Bitness::Bit64;
      return true;
    default:
      *out_bitness = Bitness::Unknown;
      return true;
  }
}

bool ReadExact(HANDLE file, void *buffer, DWORD size, DWORD *out_error) {
  DWORD read = 0;
  if (!ReadFile(file, buffer, size, &read, nullptr)) {
    *out_error = GetLastError();
    return false;
  }

  if (read != size) {
    *out_error = ERROR_HANDLE_EOF;
    return false;
  }

  return true;
}

bool DetectWithPEHeader(const std::wstring &path, Bitness *out_bitness, std::wstring *out_error) {
  HandleGuard file(CreateFileW(path.c_str(), GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (file.get() == INVALID_HANDLE_VALUE) {
    *out_error = L"Could not read file " + path + L": " + FormatWin32Error(GetLastError());
    return false;
  }

  std::array<BYTE, 64> dos{};
  DWORD read_error = ERROR_SUCCESS;
  if (!ReadExact(file.get(), dos.data(), static_cast<DWORD>(dos.size()), &read_error)) {
    *out_error = L"Could not read file " + path + L": " + FormatWin32Error(read_error);
    return false;
  }

  if (dos[0] != 'M' || dos[1] != 'Z') {
    *out_bitness = Bitness::Unknown;
    return true;
  }

  const DWORD pe_offset = static_cast<DWORD>(dos[0x3C]) |
                          (static_cast<DWORD>(dos[0x3D]) << 8) |
                          (static_cast<DWORD>(dos[0x3E]) << 16) |
                          (static_cast<DWORD>(dos[0x3F]) << 24);

  LARGE_INTEGER cursor;
  cursor.QuadPart = pe_offset;
  if (!SetFilePointerEx(file.get(), cursor, nullptr, FILE_BEGIN)) {
    *out_error = L"Could not read file " + path + L": " + FormatWin32Error(GetLastError());
    return false;
  }

  std::array<BYTE, 26> pe_header{};
  if (!ReadExact(file.get(), pe_header.data(), static_cast<DWORD>(pe_header.size()),
                 &read_error)) {
    *out_error = L"Could not read file " + path + L": " + FormatWin32Error(read_error);
    return false;
  }

  if (pe_header[0] != 'P' || pe_header[1] != 'E' || pe_header[2] != 0 || pe_header[3] != 0) {
    *out_bitness = Bitness::Unknown;
    return true;
  }

  const WORD optional_magic = static_cast<WORD>(pe_header[24]) |
                              (static_cast<WORD>(pe_header[25]) << 8);
  if (optional_magic == 0x10B) {
    *out_bitness = Bitness::Bit32;
  } else if (optional_magic == 0x20B) {
    *out_bitness = Bitness::Bit64;
  } else {
    *out_bitness = Bitness::Unknown;
  }

  return true;
}

}  // namespace

std::wstring BitnessToString(Bitness bitness) {
  switch (bitness) {
    case Bitness::Bit32:
      return L"32-bit";
    case Bitness::Bit64:
      return L"64-bit";
    default:
      return L"Unknown (unrecognized PE format)";
  }
}

bool DetectBitness(const std::wstring &path, Bitness *out_bitness, std::wstring *out_error) {
  const DWORD attributes = GetFileAttributesW(path.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    const DWORD err = GetLastError();
    if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND) {
      *out_error = L"Target not found: " + path;
    } else {
      *out_error = L"Could not access file " + path + L": " + FormatWin32Error(err);
    }
    return false;
  }

  if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    *out_error = L"Target is a directory: " + path;
    return false;
  }

  Bitness bitness = Bitness::Unknown;
  if (DetectWithGetBinaryType(path, &bitness) && bitness != Bitness::Unknown) {
    *out_bitness = bitness;
    return true;
  }

  std::wstring read_error;
  if (!DetectWithPEHeader(path, &bitness, &read_error)) {
    *out_error = read_error;
    return false;
  }

  *out_bitness = bitness;
  return true;
}
