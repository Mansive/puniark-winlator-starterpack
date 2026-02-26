# Build bitnessscan (Dev)

The project uses Meson + Ninja, wrapped by helper scripts.
Pinned versions are defined in `pyproject.toml` (`[dependency-groups].build`) and locked in `uv.lock`.

Prerequisites:

- `uv`
- Windows local build: MinGW (`g++`) in `PATH`
- Linux cross-build: `mingw-w64` and `make`

## Quick start

- Windows local build: `tools\build-bitnessscan-x86.bat`
- Linux cross-build (Win32): `make CROSS=1`

## Build wrappers

- `configure` / `configure.bat`: configure or reconfigure Meson build directories
- `make` / `make.bat`: build commands (defaults to `bundle`)
  - `bundle`: build executable and stage runtime assets to `build/`
  - `compile`: build executable only
  - `clean`: clean current Meson build directory
  - `distclean`: remove local build and dist outputs

On Linux, `make CROSS=1` runs cross-configure internally, so you do not need a separate `configure --cross` step.

Direct uv usage (without wrappers):

- `uv run --group build meson setup ...`
- `uv run --group build meson compile -C <build-dir> bundle`

Examples:

- Local Windows configure + bundle:
  - `configure.bat`
  - `make.bat bundle`
- Linux cross-build with custom Frida version:
  - `make CROSS=1 CONFIGURE_ARGS='-Dfrida_version=17.7.3'`
- Linux cross-build using cached assets only (offline-friendly):
  - `make CROSS=1 CONFIGURE_ARGS='-Ddownload_runtime_assets=false'`

## Output

`bundle` stages files to `build/`:

- `build/bitnessscan.exe`
- `build/win32/`
- `build/win64/`
- `build/RUN_AUTO.bat`
- `build/RUN_PICKER.bat`

## Local CI (Dev)

To run the GitHub Actions build job locally with `act`, use:

- `tools\run-act-build.bat`

This wrapper runs the `linux-cross-win32` job from `.github/workflows/build.yml`.
