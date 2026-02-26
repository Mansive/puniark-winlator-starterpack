cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ASSETS_ROOT OR ASSETS_ROOT STREQUAL "")
  message(FATAL_ERROR "ASSETS_ROOT is required")
endif()

if(NOT DEFINED DEST_ROOT OR DEST_ROOT STREQUAL "")
  message(FATAL_ERROR "DEST_ROOT is required")
endif()

function(copy_arch_dlls arch)
  set(source_dir "${ASSETS_ROOT}/${arch}")
  set(dest_dir "${DEST_ROOT}/${arch}")

  if(NOT IS_DIRECTORY "${source_dir}")
    message(FATAL_ERROR "Missing assets directory: ${source_dir}")
  endif()

  if(EXISTS "${dest_dir}")
    file(REMOVE_RECURSE "${dest_dir}")
  endif()

  file(MAKE_DIRECTORY "${dest_dir}")

  file(GLOB dll_files "${source_dir}/*.dll")
  if(NOT dll_files)
    message(FATAL_ERROR "No DLL files found in ${source_dir}")
  endif()

  foreach(dll_path IN LISTS dll_files)
    get_filename_component(dll_name "${dll_path}" NAME)
    file(COPY_FILE "${dll_path}" "${dest_dir}/${dll_name}" ONLY_IF_DIFFERENT)
  endforeach()
endfunction()

copy_arch_dlls("win32")
copy_arch_dlls("win64")
