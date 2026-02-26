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

where act >nul 2>nul
if errorlevel 1 (
  echo Could not find act in PATH.
  echo Install act and try again.
  exit /b 1
)

where docker >nul 2>nul
if errorlevel 1 (
  echo Could not find docker in PATH.
  echo Install Docker Desktop and try again.
  exit /b 1
)

pushd "%ROOT_DIR%" >nul

echo Running workflow job "linux-cross-win32" with act...
act -j "linux-cross-win32" %*
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul

if not "%EXIT_CODE%"=="0" (
  echo act run failed with exit code %EXIT_CODE%.
  exit /b %EXIT_CODE%
)

echo act run completed successfully.
exit /b 0
