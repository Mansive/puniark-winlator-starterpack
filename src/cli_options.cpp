#include "cli_options.h"

#include "path_utils.h"

namespace {

constexpr wchar_t kHelpTextValue[] =
    L"Usage: bitnessscan.exe [--debug] [--help] [--pick] [directory ...]";

bool IsDebugArg(const std::wstring &lower_arg) {
  return lower_arg == L"--debug" || lower_arg == L"/debug" || lower_arg == L"debug" ||
         lower_arg.rfind(L"/debug:", 0) == 0 || lower_arg.rfind(L"--debug=", 0) == 0;
}

bool IsHelpArg(const std::wstring &lower_arg) {
  return lower_arg == L"--help" || lower_arg == L"/help" || lower_arg == L"-h" ||
         lower_arg == L"/?";
}

bool IsPickArg(const std::wstring &lower_arg) {
  return lower_arg == L"--pick" || lower_arg == L"/pick" || lower_arg == L"pick";
}

}  // namespace

const wchar_t *GetHelpText() {
  return kHelpTextValue;
}

Options ParseArgs(int argc, wchar_t **argv) {
  Options options;

  for (int i = 1; i < argc; ++i) {
    std::wstring raw = argv[i] ? argv[i] : L"";
    std::wstring lower = ToLower(raw);

    if (IsDebugArg(lower)) {
      options.debug = true;
      continue;
    }
    if (IsHelpArg(lower)) {
      options.help = true;
      continue;
    }
    if (IsPickArg(lower)) {
      options.pick = true;
      continue;
    }

    options.directories.push_back(raw);
  }

  return options;
}
