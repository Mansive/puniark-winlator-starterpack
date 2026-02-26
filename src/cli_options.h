#pragma once

#include "types.h"

const wchar_t *GetHelpText();

[[nodiscard]]
Options ParseArgs(int argc, wchar_t **argv);
