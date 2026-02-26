cmake_minimum_required(VERSION 3.25)

project(bitnessscan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bitnessscan
  src/main.cpp
  src/app.cpp
  src/asset_deploy.cpp
  src/cli_options.cpp
  src/folder_picker.cpp
  src/output.cpp
  src/path_utils.cpp
  src/pe_bitness.cpp
  src/scan.cpp
  src/win32_utils.cpp
)

target_link_libraries(bitnessscan PRIVATE shell32 ole32 version)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_AR)
    set(CMAKE_AR "${CMAKE_CXX_COMPILER_AR}")
  endif ()
  if (CMAKE_CXX_COMPILER_RANLIB)
    set(CMAKE_RANLIB "${CMAKE_CXX_COMPILER_RANLIB}")
  endif ()

  set(IS_SIZE_OPT_BUILD "$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>")
  set(GNU_COMPILE_OPTIONS
    -Os
    -municode
    "$<${IS_SIZE_OPT_BUILD}:-ffunction-sections>"
    "$<${IS_SIZE_OPT_BUILD}:-fdata-sections>"
    "$<${IS_SIZE_OPT_BUILD}:-flto>"
  )
  set(GNU_LINK_OPTIONS
    -municode
    -static
    -static-libgcc
    -static-libstdc++
    "$<${IS_SIZE_OPT_BUILD}:-Wl,-s>"
    "$<${IS_SIZE_OPT_BUILD}:-Wl,--gc-sections>"
    "$<${IS_SIZE_OPT_BUILD}:-flto>"
  )
  target_compile_options(
    bitnessscan
    PRIVATE
      ${GNU_COMPILE_OPTIONS}
  )
  target_link_options(
    bitnessscan
    PRIVATE
      ${GNU_LINK_OPTIONS}
  )
endif ()

set(BITNESSSCAN_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/build")
set(UAL_DOWNLOAD_ROOT "${PROJECT_SOURCE_DIR}/downloads/ultimate-asi-loader")
set(UAL_DOWNLOAD_STAMP "${UAL_DOWNLOAD_ROOT}/.download-complete")
set(FRIDA_VERSION "17.7.3" CACHE STRING "Frida Gadget release version")
set(FRIDA_DOWNLOAD_ROOT "${PROJECT_SOURCE_DIR}/downloads/frida")
set(FRIDA_DOWNLOAD_STAMP "${FRIDA_DOWNLOAD_ROOT}/.download-complete-${FRIDA_VERSION}")

add_custom_command(
  OUTPUT "${UAL_DOWNLOAD_STAMP}"
  COMMAND ${CMAKE_COMMAND}
          -DOUTPUT_ROOT=${UAL_DOWNLOAD_ROOT}
          -P "${PROJECT_SOURCE_DIR}/cmake/download_ultimate_asi_loader.cmake"
  COMMAND ${CMAKE_COMMAND} -E touch "${UAL_DOWNLOAD_STAMP}"
  COMMENT "Downloading Ultimate ASI Loader assets"
  VERBATIM
)

add_custom_command(
  OUTPUT "${FRIDA_DOWNLOAD_STAMP}"
  COMMAND ${CMAKE_COMMAND}
          -DOUTPUT_ROOT=${FRIDA_DOWNLOAD_ROOT}
          -DFRIDA_VERSION=${FRIDA_VERSION}
          -P "${PROJECT_SOURCE_DIR}/cmake/download_frida_gadget.cmake"
  COMMAND ${CMAKE_COMMAND} -E touch "${FRIDA_DOWNLOAD_STAMP}"
  COMMENT "Downloading Frida Gadget assets"
  VERBATIM
)

add_custom_target(prepare_ultimate_asi_loader DEPENDS "${UAL_DOWNLOAD_STAMP}")
add_custom_target(prepare_frida_gadget DEPENDS "${FRIDA_DOWNLOAD_STAMP}")
add_dependencies(bitnessscan prepare_ultimate_asi_loader prepare_frida_gadget)

set_target_properties(bitnessscan PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BITNESSSCAN_OUTPUT_DIR}")

if (CMAKE_CONFIGURATION_TYPES)
  foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(
      bitnessscan
      PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${BITNESSSCAN_OUTPUT_DIR}"
    )
  endforeach()
endif()

add_custom_command(
  TARGET bitnessscan
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${BITNESSSCAN_OUTPUT_DIR}"
  COMMAND ${CMAKE_COMMAND}
          -DASSETS_ROOT=${UAL_DOWNLOAD_ROOT}
          -DDEST_ROOT=${BITNESSSCAN_OUTPUT_DIR}
          -P "${PROJECT_SOURCE_DIR}/cmake/copy_ultimate_asi_loader_dlls.cmake"
  COMMAND ${CMAKE_COMMAND}
          -DASSETS_ROOT=${FRIDA_DOWNLOAD_ROOT}
          -DDEST_ROOT=${BITNESSSCAN_OUTPUT_DIR}
          -DFRIDA_VERSION=${FRIDA_VERSION}
          -DCONFIG_TEMPLATE=${PROJECT_SOURCE_DIR}/src/frida/frida-gadget.config
          -P "${PROJECT_SOURCE_DIR}/cmake/copy_frida_gadget_assets.cmake"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${PROJECT_SOURCE_DIR}/tools/AUTOMATIC.bat"
          "${BITNESSSCAN_OUTPUT_DIR}/AUTOMATIC.bat"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${PROJECT_SOURCE_DIR}/tools/PICKER.bat"
          "${BITNESSSCAN_OUTPUT_DIR}/PICKER.bat"
  COMMAND ${CMAKE_COMMAND} -E rm -f "${BITNESSSCAN_OUTPUT_DIR}/build-bitnessscan-x86.bat"
  VERBATIM
)
