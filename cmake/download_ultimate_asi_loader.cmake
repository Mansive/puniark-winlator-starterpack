cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OUTPUT_ROOT OR OUTPUT_ROOT STREQUAL "")
  message(FATAL_ERROR "OUTPUT_ROOT is required")
endif()

set(BASE_URL "https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download")

function(download_and_extract arch tag asset)
  set(arch_root "${OUTPUT_ROOT}/${arch}")
  set(zip_root "${arch_root}/zips")
  set(zip_path "${zip_root}/${asset}")
  set(url "${BASE_URL}/${tag}/${asset}")

  file(MAKE_DIRECTORY "${zip_root}")

  if(NOT EXISTS "${zip_path}")
    message(STATUS "Downloading ${url}")
    file(
      DOWNLOAD
      "${url}"
      "${zip_path}"
      STATUS download_status
      SHOW_PROGRESS
      TLS_VERIFY ON
      TIMEOUT 120
      INACTIVITY_TIMEOUT 60
    )

    list(GET download_status 0 download_code)
    list(GET download_status 1 download_message)
    if(NOT download_code EQUAL 0)
      file(REMOVE "${zip_path}")
      message(FATAL_ERROR "Failed downloading ${url}: ${download_message}")
    endif()
  else()
    message(STATUS "Using cached ${zip_path}")
  endif()

  message(STATUS "Extracting ${zip_path}")
  file(ARCHIVE_EXTRACT INPUT "${zip_path}" DESTINATION "${arch_root}")
endfunction()

download_and_extract("win32" "Win32-latest" "d3d9-Win32.zip")
download_and_extract("win32" "Win32-latest" "d3d11-Win32.zip")
download_and_extract("win32" "Win32-latest" "d3d12-Win32.zip")
download_and_extract("win32" "Win32-latest" "version-Win32.zip")

download_and_extract("win64" "x64-latest" "d3d9-x64.zip")
download_and_extract("win64" "x64-latest" "d3d11-x64.zip")
download_and_extract("win64" "x64-latest" "d3d12-x64.zip")
download_and_extract("win64" "x64-latest" "version-x64.zip")
