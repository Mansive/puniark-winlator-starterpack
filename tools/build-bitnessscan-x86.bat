@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR="
if exist "%SCRIPT_DIR%..\CMakeLists.txt" set "ROOT_DIR=%SCRIPT_DIR%.."
if not defined ROOT_DIR if exist "%SCRIPT_DIR%..\..\CMakeLists.txt" set "ROOT_DIR=%SCRIPT_DIR%..\.."

if not defined ROOT_DIR (
  echo Could not locate project root from "%SCRIPT_DIR%"
  exit /b 1
)

for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "PRESET=mingw-win32-local"
set "OUTPUT=%ROOT_DIR%\build\bitnessscan.exe"

where cmake >nul 2>nul
if errorlevel 1 (
  echo Could not find cmake in PATH.
  echo Install CMake and try again.
  exit /b 1
)

pushd "%ROOT_DIR%" >nul

echo Configuring with CMake preset: %PRESET%
cmake --preset "%PRESET%"
if errorlevel 1 (
  popd >nul
  echo Configure failed.
  exit /b 1
)

echo Building with CMake preset: %PRESET%
cmake --build --preset "%PRESET%"
if errorlevel 1 (
  popd >nul
  echo Build failed.
  exit /b 1
)

popd >nul

if not exist "%OUTPUT%" (
  echo Build completed but output is missing: "%OUTPUT%"
  exit /b 1
)

echo Built %OUTPUT%
exit /b 0
