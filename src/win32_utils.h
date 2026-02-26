#pragma once

#include <string>

[[nodiscard]]
std::wstring FormatWin32Error(unsigned long code);

[[nodiscard]]
bool DirectoryExists(const std::wstring &directory);
