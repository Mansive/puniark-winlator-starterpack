#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <objbase.h>
#include <shlobj.h>

#include "folder_picker.h"

#include "win32_utils.h"

#include <array>

PickResult PickFolder(std::wstring *out_directory, std::wstring *out_error) {
  const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  const bool com_initialized = SUCCEEDED(hr);

  if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
    *out_error = L"Could not initialize folder picker (HRESULT " +
                 std::to_wstring(static_cast<long>(hr)) + L")";
    return PickResult::Error;
  }

  BROWSEINFOW browse_info = {};
  browse_info.hwndOwner = nullptr;
  browse_info.lpszTitle = L"Select the game folder";
  browse_info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE |
                        BIF_NONEWFOLDERBUTTON;

  PIDLIST_ABSOLUTE item_id = SHBrowseForFolderW(&browse_info);
  if (!item_id) {
    if (com_initialized) {
      CoUninitialize();
    }
    return PickResult::Canceled;
  }

  std::array<wchar_t, MAX_PATH> selected_path{};
  if (!SHGetPathFromIDListW(item_id, selected_path.data())) {
    CoTaskMemFree(item_id);
    if (com_initialized) {
      CoUninitialize();
    }
    *out_error = L"Selected item is not a valid filesystem folder.";
    return PickResult::Error;
  }

  CoTaskMemFree(item_id);

  std::wstring selected_directory(selected_path.data());
  if (!DirectoryExists(selected_directory)) {
    if (com_initialized) {
      CoUninitialize();
    }
    *out_error = L"Selected folder does not exist: " + selected_directory;
    return PickResult::Error;
  }

  if (com_initialized) {
    CoUninitialize();
  }

  *out_directory = selected_directory;
  return PickResult::Success;
}
