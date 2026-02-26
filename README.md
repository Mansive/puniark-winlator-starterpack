# PuniArk Winlator Starter Pack

This repo contains resources and instructions on making Winlator and Winlator-based apps (GameNative, GameHub, GameHub Lite) compatible with PuniArk. It's not convenient, but at least you only need to perform simple copy-pastes once per game.

## Guide

> [!NOTE]
> The following guide is meant for GameNative, but its steps can be adapted for the other apps.

1. On your Android device, download the [starter pack](https://github.com/Mansive/puniark-winlator-starterpack/releases/latest) and extract its contents into the `Downloads` folder
2. Open up GameNative and select your game, but don't launch the game yet
3. Click on the three vertical dots in the top-right corner then `Open container`
4. Click on the `D:` drive on the left side menu, then double-click on `puniark-winlator-starterpack` or the folder you extracted the contents into
5. Select the `scripts` folder, click `Copy` near the top-left corner, navigate to the `A:` drive which contains the game, then click `Paste`
    - If you do this right, you should see the `scripts` folder in the `A:` drive
6. Navigate back to the `D:` drive to copy-paste the `.dll` files from either the `win32` or `win64` folders into the `A:` drive
    - How do you know if you should copy-paste from the `win32` or `win64` folders?
        - If your game is 32-bit, copy-paste from `win32`
            - Old games and visual novels tend to be 32-bit
        - If your game is 64-bit, copy-paste from `win64`
            - Newer games and those made with Unity Engine or Unreal Engine tend to be 64-bit
        - If you have no idea, try with the 64-bit DLLs first. If PuniArk says it can't connect, then delete the 64-bit DLLs and try with the 32-bit ones.
    - There are multiple DLL files in `win32` and `win64`. If you know what you're doing, you can just copy-paste the DLL you need. If not, copy-paste *all* the DLL files of a specific `win` folder.
7. Without exiting the container yet, launch the game from the `A:` drive and see if PuniArk can connect. If not, try swapping the 32-bit DLLs for 64-bit or vice versa.
8. After confirming PuniArk can connect, exit the container. You can start the game with the normal play button from now on.

> [!TIP]
> To make it easier to copy-paste from the starter pack folder, in GameNative:
> 1. Click on your profile picture in the top-right corner
> 2. Navigate `Settings` -> `Modify Default Config` -> `Drives`
> 3. Click on the `+` icon at the bottom
> 4. Select any letter that isn't `A`, for example `F`
> 5. Select the starter pack folder
>
> This will make the starter pack folder appear as a selectable drive in the side menu in the container, reducing the amount of clicks needed to navigate to the folder.

## PE Bitness Helper (Dev)

The local `test.js` helper script uses `executables/pebitness.exe` to detect whether an `.exe` is 32-bit or 64-bit.

- Source: `executables/pebitness.c`
- Build (x86): run `executables\build-pebitness-x86.bat`
- Exit codes: `32` for 32-bit, `64` for 64-bit, `1` for unknown format

The helper binary is built locally and ignored by git (`executables/pebitness.exe`).

Examples:

- Scan `.exe` files in the same folder as `test.js`:
  - `cscript //nologo test.js`
- Check specific files:
  - `cscript //nologo test.js "A:\Atelier Sophie DX.exe" "A:\Game.exe"`
- Enable debug logs:
  - `cscript //nologo test.js --debug`

This avoids relying on Wine XMLHTTP binary response behavior, which can return inconsistent results for some files.

## Build bitnessscan (Dev)

The project uses Meson + Ninja, wrapped by helper scripts.
Pinned versions are defined in `pyproject.toml` (`[dependency-groups].build`) and locked in `uv.lock`.

Prerequisites:

- `uv`
- Windows local build: MinGW (`g++`) in `PATH`
- Linux cross-build: `mingw-w64` and `make`

### Quick start

- Windows local build: `tools\build-bitnessscan-x86.bat`
- Linux cross-build (Win32): `make CROSS=1`

### Build wrappers

- `configure` / `configure.bat`: configure or reconfigure Meson build directories
- `make` / `make.bat`: build commands (defaults to `bundle`)
  - `bundle`: build executable and stage runtime assets to `build/`
  - `compile`: build executable only
  - `clean`: clean current Meson build directory
  - `distclean`: remove local build and dist outputs

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

### Output

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
