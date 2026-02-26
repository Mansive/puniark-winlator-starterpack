#pragma once

#include "types.h"

#include <string>

[[nodiscard]]
bool DeployArchitectureAssets(const std::wstring &target_directory, Bitness architecture, bool debug,
                              std::wstring *out_error);
