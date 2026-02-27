project(
  'bitnessscan',
  'cpp',
  meson_version: '>=1.10.1',
  default_options: ['cpp_std=c++23'],
)

cpp = meson.get_compiler('cpp')

bitnessscan_sources = files(
  'src/main.cpp',
  'src/app.cpp',
  'src/asset_deploy.cpp',
  'src/cli_options.cpp',
  'src/folder_picker.cpp',
  'src/output.cpp',
  'src/path_utils.cpp',
  'src/pe_bitness.cpp',
  'src/scan.cpp',
  'src/win32_utils.cpp',
)

compile_args = []
link_args = []

if cpp.get_id() == 'gcc'
  compile_args += [
    '-Os',
    '-municode',
  ]
  link_args += [
    '-municode',
    '-static',
    '-static-libgcc',
    '-static-libstdc++',
  ]

  buildtype = get_option('buildtype')
  if buildtype == 'release' or buildtype == 'minsize'
    compile_args += [
      '-ffunction-sections',
      '-fdata-sections',
      '-flto',
    ]
    link_args += [
      '-Wl,-s',
      '-Wl,--gc-sections',
      '-flto',
    ]
  endif
endif

win32_libraries = [
  cpp.find_library('shell32', required: true),
  cpp.find_library('ole32', required: true),
  cpp.find_library('version', required: true),
]

bitnessscan_exe = executable(
  'bitnessscan',
  bitnessscan_sources,
  dependencies: win32_libraries,
  cpp_args: compile_args,
  link_args: link_args,
)

python = import('python').find_installation(required: true)

frida_version = get_option('frida_version')
download_runtime_assets = get_option('download_runtime_assets')
source_root = meson.project_source_root()
output_root = source_root / 'build'
frida_config_template = files('src/frida/frida-gadget.config')[0]
run_auto_script = files('tools/launchers/RUN_AUTO.bat')[0]
run_picker_script = files('tools/launchers/RUN_PICKER.bat')[0]
prepare_assets_script = files('tools/prepare_runtime_assets.py')[0]

bundle_command = [
  python,
  prepare_assets_script,
  '--source-root',
  source_root,
  '--output-root',
  output_root,
  '--exe-path',
  bitnessscan_exe,
  '--frida-version',
  frida_version,
  '--config-template',
  frida_config_template,
  '--run-auto',
  run_auto_script,
  '--run-picker',
  run_picker_script,
]

if not download_runtime_assets
  bundle_command += ['--skip-downloads']
endif

run_target(
  'bundle',
  command: bundle_command,
  depends: [bitnessscan_exe],
)
