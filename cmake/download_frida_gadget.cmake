cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OUTPUT_ROOT OR OUTPUT_ROOT STREQUAL "")
  message(FATAL_ERROR "OUTPUT_ROOT is required")
endif()

if(NOT DEFINED FRIDA_VERSION OR FRIDA_VERSION STREQUAL "")
  message(FATAL_ERROR "FRIDA_VERSION is required")
endif()

set(BASE_URL "https://github.com/frida/frida/releases/download")
set(EXTRACT_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/extract_xz.py")

if(NOT EXISTS "${EXTRACT_SCRIPT}")
  message(FATAL_ERROR "Missing helper script: ${EXTRACT_SCRIPT}")
endif()

find_program(PYTHON_EXECUTABLE NAMES python3 python)
if(NOT PYTHON_EXECUTABLE)
  message(FATAL_ERROR
          "Python is required to extract Frida .xz assets, but no interpreter was found in PATH. "
          "Install Python 3 and ensure `python3` or `python` is available in PATH.")
endif()

message(STATUS "Using Python extractor: ${PYTHON_EXECUTABLE}")

function(download_and_extract arch suffix)
  set(arch_root "${OUTPUT_ROOT}/${arch}")
  set(archive_root "${arch_root}/archives")
  set(asset_name "frida-gadget-${FRIDA_VERSION}-windows-${suffix}.dll.xz")
  set(dll_name "frida-gadget-${FRIDA_VERSION}-windows-${suffix}.dll")
  set(archive_path "${archive_root}/${asset_name}")
  set(dll_path "${arch_root}/${dll_name}")
  set(url "${BASE_URL}/${FRIDA_VERSION}/${asset_name}")

  file(MAKE_DIRECTORY "${archive_root}")

  if(NOT EXISTS "${archive_path}")
    message(STATUS "Downloading ${url}")
    file(
      DOWNLOAD
      "${url}"
      "${archive_path}"
      STATUS download_status
      SHOW_PROGRESS
      TLS_VERIFY ON
      TIMEOUT 120
      INACTIVITY_TIMEOUT 60
    )

    list(GET download_status 0 download_code)
    list(GET download_status 1 download_message)
    if(NOT download_code EQUAL 0)
      file(REMOVE "${archive_path}")
      message(FATAL_ERROR "Failed downloading ${url}: ${download_message}")
    endif()
  else()
    message(STATUS "Using cached ${archive_path}")
  endif()

  if(NOT EXISTS "${dll_path}")
    message(STATUS "Extracting ${archive_path}")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" "${EXTRACT_SCRIPT}" "${archive_path}" "${dll_path}"
      RESULT_VARIABLE extract_code
      OUTPUT_VARIABLE extract_stdout
      ERROR_VARIABLE extract_stderr
    )

    if(NOT extract_code EQUAL 0)
      file(REMOVE "${dll_path}")
      message(FATAL_ERROR
              "Failed extracting ${archive_path} with ${PYTHON_EXECUTABLE}: "
              "${extract_stderr}${extract_stdout}")
    endif()
  else()
    message(STATUS "Using cached ${dll_path}")
  endif()
endfunction()

download_and_extract("win32" "x86")
download_and_extract("win64" "x86_64")
