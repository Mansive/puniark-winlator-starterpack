# Build bitnessscan (Dev)

The project uses Meson + Ninja, wrapped by helper scripts.
Pinned versions are defined in `pyproject.toml` (`[dependency-groups].build`) and locked in `uv.lock`.

Prerequisites:

- [uv](https://docs.astral.sh/uv/getting-started/installation/)
- Windows: [MSYS2](https://www.msys2.org/)
- Linux cross-build: `mingw-w64` and `make`

## Quick start

- Windows local build: `tools\build-bitnessscan-x86.bat`
- Linux cross-build (Win32): `make CROSS=1`

## Build commands

`make` and `make.bat` wrap Meson + Ninja.

Targets:

- `bundle` (default): build executable and stage runtime assets to `build/`
- `compile`: build executable only
- `configure`: configure or reconfigure the Meson build directory
- `clean`: clean current Meson build directory
- `test`: run Meson tests
- `distclean`: remove local build and dist outputs

Cross build mode:

- Linux/CI: add `CROSS=1` (for example `make CROSS=1 bundle`)

Pass Meson setup options through `CONFIGURE_ARGS`:

- Linux: `make CROSS=1 CONFIGURE_ARGS='-Dfrida_version=17.7.3'`
- Linux offline-friendly: `make CROSS=1 CONFIGURE_ARGS='-Ddownload_runtime_assets=false'`
- Windows (cmd): `set CONFIGURE_ARGS=-Ddownload_runtime_assets=false && make.bat bundle`

Direct uv usage (without wrappers):

- Local: `uv run --group build meson setup .meson/build/mingw-win32-local --buildtype release`
- Cross: `uv run --group build meson setup .meson/build/mingw-win32-linux --buildtype release --cross-file meson/cross/mingw32.ini`
- Build bundle: `uv run --group build meson compile -C .meson/build/mingw-win32-linux bundle`

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
