@echo off
setlocal EnableDelayedExpansion

set "ROOT_DIR=%~dp0"
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"

where uv >nul 2>nul
if errorlevel 1 (
  echo Could not find uv in PATH.
  echo Install uv and try again.
  exit /b 1
)

set "CROSS_MODE=0"
set "PASSTHROUGH_ARGS="

:parse_args
if "%~1"=="" goto after_parse
if /I "%~1"=="--cross" (
  set "CROSS_MODE=1"
) else (
  set "PASSTHROUGH_ARGS=!PASSTHROUGH_ARGS! "%~1""
)
shift
goto parse_args

:after_parse
set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-local"
set "CROSS_ARGS="

if "%CROSS_MODE%"=="1" (
  set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-linux"
  set "CROSS_ARGS=--cross-file meson/cross/mingw32.ini"
)

pushd "%ROOT_DIR%" >nul

if exist "%BUILD_DIR%\build.ninja" (
  echo Reconfiguring Meson build directory: %BUILD_DIR%
  uv run --group build meson setup --reconfigure "%BUILD_DIR%" %CROSS_ARGS% %PASSTHROUGH_ARGS%
) else (
  echo Configuring Meson build directory: %BUILD_DIR%
  uv run --group build meson setup "%BUILD_DIR%" --buildtype release %CROSS_ARGS% %PASSTHROUGH_ARGS%
)

set "EXIT_CODE=%ERRORLEVEL%"
popd >nul

if not "%EXIT_CODE%"=="0" (
  echo Meson setup failed.
)

exit /b %EXIT_CODE%
