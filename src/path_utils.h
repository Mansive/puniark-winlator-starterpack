#pragma once

#include <string>
#include <vector>

[[nodiscard]]
std::wstring ToLower(const std::wstring &text);

[[nodiscard]]
std::string WideToUtf8(const std::wstring &text);

[[nodiscard]]
std::wstring JoinPath(const std::wstring &left, const std::wstring &right);

[[nodiscard]]
std::wstring NormalizePath(const std::wstring &path);

[[nodiscard]]
std::wstring GetBaseName(const std::wstring &path);

void SortPaths(std::vector<std::wstring> *paths);
