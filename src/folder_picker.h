#pragma once

#include "types.h"

#include <string>

[[nodiscard]]
PickResult PickFolder(std::wstring *out_directory, std::wstring *out_error);
