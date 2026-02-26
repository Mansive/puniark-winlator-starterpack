#pragma once

#include "types.h"

#include <string>

[[nodiscard]]
std::wstring BitnessToString(Bitness bitness);

[[nodiscard]]
bool DetectBitness(const std::wstring &path, Bitness *out_bitness, std::wstring *out_error);
