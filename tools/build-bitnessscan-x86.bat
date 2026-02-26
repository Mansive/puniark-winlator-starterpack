@echo off
setlocal
set "EXIT_CODE=1"

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR="
if exist "%SCRIPT_DIR%..\meson.build" set "ROOT_DIR=%SCRIPT_DIR%.."
if not defined ROOT_DIR if exist "%SCRIPT_DIR%..\..\meson.build" set "ROOT_DIR=%SCRIPT_DIR%..\.."

if not defined ROOT_DIR (
  echo Could not locate project root from "%SCRIPT_DIR%"
  set "EXIT_CODE=1"
  goto :end
)

for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-local"
set "OUTPUT=%ROOT_DIR%\build\bitnessscan.exe"
set "PYTHON_VERSION=3.14.3"
set "MESON_VERSION=1.10.1"

where uv >nul 2>nul
if errorlevel 1 (
  echo Could not find uv in PATH.
  echo Install uv and try again.
  set "EXIT_CODE=1"
  goto :end
)

pushd "%ROOT_DIR%" >nul

if exist "%BUILD_DIR%\build.ninja" (
  echo Reconfiguring Meson build directory: %BUILD_DIR%
  uv tool run --python "%PYTHON_VERSION%" --from "meson==%MESON_VERSION%" --with ninja meson setup --reconfigure "%BUILD_DIR%"
  if errorlevel 1 (
    popd >nul
    echo Reconfigure failed.
    set "EXIT_CODE=1"
    goto :end
  )
) else (
  echo Configuring Meson build directory: %BUILD_DIR%
  uv tool run --python "%PYTHON_VERSION%" --from "meson==%MESON_VERSION%" --with ninja meson setup "%BUILD_DIR%" --buildtype release
  if errorlevel 1 (
    popd >nul
    echo Configure failed.
    set "EXIT_CODE=1"
    goto :end
  )
)

echo Building with Meson %MESON_VERSION%
uv tool run --python "%PYTHON_VERSION%" --from "meson==%MESON_VERSION%" --with ninja meson compile -C "%BUILD_DIR%"
if errorlevel 1 (
  popd >nul
  echo Build failed.
  set "EXIT_CODE=1"
  goto :end
)

popd >nul

if not exist "%OUTPUT%" (
  echo Build completed but output is missing: "%OUTPUT%"
  set "EXIT_CODE=1"
  goto :end
)

echo Built %OUTPUT%
set "EXIT_CODE=0"

:end
if not defined NO_PAUSE if not defined CI (
  echo.
  pause
)
exit /b %EXIT_CODE%
