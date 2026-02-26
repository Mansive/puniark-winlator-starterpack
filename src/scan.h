#pragma once

#include <string>
#include <vector>

[[nodiscard]]
std::vector<std::wstring> CollectScanDirectories(const std::vector<std::wstring> &raw_directories);

[[nodiscard]]
bool CollectExecutableTargetsFromDirectory(const std::wstring &directory,
                                           std::vector<std::wstring> *out_paths,
                                           std::wstring *out_error);

[[nodiscard]]
std::wstring DetermineGameArchitecture(int bit32_count, int bit64_count, int unknown_count,
                                       int error_count);
