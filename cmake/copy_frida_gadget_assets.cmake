cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ASSETS_ROOT OR ASSETS_ROOT STREQUAL "")
  message(FATAL_ERROR "ASSETS_ROOT is required")
endif()

if(NOT DEFINED DEST_ROOT OR DEST_ROOT STREQUAL "")
  message(FATAL_ERROR "DEST_ROOT is required")
endif()

if(NOT DEFINED FRIDA_VERSION OR FRIDA_VERSION STREQUAL "")
  message(FATAL_ERROR "FRIDA_VERSION is required")
endif()

if(NOT DEFINED CONFIG_TEMPLATE OR CONFIG_TEMPLATE STREQUAL "")
  message(FATAL_ERROR "CONFIG_TEMPLATE is required")
endif()

if(NOT EXISTS "${CONFIG_TEMPLATE}")
  message(FATAL_ERROR "Missing Frida config template: ${CONFIG_TEMPLATE}")
endif()

function(copy_frida_gadget arch suffix)
  set(base_name "frida-gadget-${FRIDA_VERSION}-windows-${suffix}")
  set(source_dll "${ASSETS_ROOT}/${arch}/${base_name}.dll")
  set(dest_dir "${DEST_ROOT}/${arch}/scripts")
  set(dest_asi "${dest_dir}/${base_name}.asi")
  set(dest_config "${dest_dir}/${base_name}.config")

  if(NOT EXISTS "${source_dll}")
    message(FATAL_ERROR "Missing Frida DLL: ${source_dll}")
  endif()

  file(MAKE_DIRECTORY "${dest_dir}")

  file(GLOB stale_asi "${dest_dir}/frida-gadget-*-windows-${suffix}.asi")
  file(GLOB stale_config "${dest_dir}/frida-gadget-*-windows-${suffix}.config")
  if(stale_asi)
    file(REMOVE ${stale_asi})
  endif()
  if(stale_config)
    file(REMOVE ${stale_config})
  endif()

  file(COPY_FILE "${source_dll}" "${dest_asi}" ONLY_IF_DIFFERENT)
  file(COPY_FILE "${CONFIG_TEMPLATE}" "${dest_config}" ONLY_IF_DIFFERENT)
endfunction()

copy_frida_gadget("win32" "x86")
copy_frida_gadget("win64" "x86_64")
