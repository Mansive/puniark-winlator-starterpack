#include "app.h"

#include "asset_deploy.h"
#include "cli_options.h"
#include "folder_picker.h"
#include "output.h"
#include "path_utils.h"
#include "pe_bitness.h"
#include "scan.h"

namespace {

class OutputScope {
 public:
  OutputScope() {
    InitOutput();
  }

  ~OutputScope() {
    CloseOutput();
  }

  OutputScope(const OutputScope &) = delete;
  OutputScope &operator=(const OutputScope &) = delete;
};

}  // namespace

int RunApp(int argc, wchar_t **argv) {
  [[maybe_unused]] const OutputScope output_scope;

  const Options options = ParseArgs(argc, argv);
  if (options.help) {
    OutLine(GetHelpText());
    return 0;
  }

  if (options.pick && !options.directories.empty()) {
    ErrLine(L"Error (--pick cannot be combined with directory arguments.)");
    return 1;
  }

  std::vector<std::wstring> scan_directories;
  if (options.pick) {
    std::wstring picked_directory;
    std::wstring pick_error;
    const PickResult pick_result = PickFolder(&picked_directory, &pick_error);
    if (pick_result == PickResult::Canceled) {
      OutLine(L"Folder selection canceled.");
      return 0;
    }

    if (pick_result == PickResult::Error) {
      ErrLine(L"Error (" + pick_error + L")");
      return 1;
    }

    scan_directories.push_back(NormalizePath(picked_directory));
    DebugLine(options.debug, L"Picked folder: " + scan_directories.front());
  } else {
    scan_directories = CollectScanDirectories(options.directories);
  }

  bool has_directory_errors = false;

  for (const std::wstring &directory : scan_directories) {
    std::vector<std::wstring> executables;
    std::wstring error;
    if (!CollectExecutableTargetsFromDirectory(directory, &executables, &error)) {
      has_directory_errors = true;
      ErrLine(L"Error (" + error + L")");
      continue;
    }

    DebugLine(options.debug,
              L"Scanned " + directory + L": found " + std::to_wstring(executables.size()) +
                  L" executable(s)");

    int bit32_count = 0;
    int bit64_count = 0;
    int unknown_count = 0;
    int error_count = 0;

    for (const std::wstring &exe_path : executables) {
      Bitness bitness = Bitness::Unknown;
      std::wstring detect_error;
      if (!DetectBitness(exe_path, &bitness, &detect_error)) {
        ++error_count;
        DebugLine(options.debug, L"Failed to analyze " + exe_path + L": " + detect_error);
        continue;
      }

      DebugLine(options.debug,
                L"Detected " + GetBaseName(exe_path) + L": " + BitnessToString(bitness));

      if (bitness == Bitness::Bit32) {
        ++bit32_count;
      } else if (bitness == Bitness::Bit64) {
        ++bit64_count;
      } else {
        ++unknown_count;
      }
    }

    DebugLine(options.debug,
              L"Summary for " + directory + L": 32-bit=" + std::to_wstring(bit32_count) +
                  L", 64-bit=" + std::to_wstring(bit64_count) + L", unknown=" +
                  std::to_wstring(unknown_count) + L", errors=" +
                  std::to_wstring(error_count));

    const std::wstring architecture_label =
        DetermineGameArchitecture(bit32_count, bit64_count, unknown_count, error_count);
    OutLine(directory + L" -> " + architecture_label);

    Bitness deploy_architecture = Bitness::Unknown;
    if (bit64_count > 0) {
      deploy_architecture = Bitness::Bit64;
    } else if (bit32_count > 0) {
      deploy_architecture = Bitness::Bit32;
    }

    if (deploy_architecture != Bitness::Unknown) {
      std::wstring deploy_error;
      if (!DeployArchitectureAssets(directory, deploy_architecture, options.debug, &deploy_error)) {
        has_directory_errors = true;
        ErrLine(L"Error (" + deploy_error + L")");
      }
    }
  }

  return has_directory_errors ? 1 : 0;
}
