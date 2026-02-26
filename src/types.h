#pragma once

#include <string>
#include <vector>

enum class Bitness {
  Unknown,
  Bit32,
  Bit64,
};

struct Options {
  bool debug = false;
  bool help = false;
  bool pick = false;
  std::vector<std::wstring> directories;
};

enum class PickResult {
  Success,
  Canceled,
  Error,
};
