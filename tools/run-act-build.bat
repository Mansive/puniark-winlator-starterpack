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

where act >nul 2>nul
if errorlevel 1 (
  echo Could not find act in PATH.
  echo Install act and try again.
  set "EXIT_CODE=1"
  goto :end
)

where docker >nul 2>nul
if errorlevel 1 (
  echo Could not find docker in PATH.
  echo Install Docker Desktop and try again.
  set "EXIT_CODE=1"
  goto :end
)

pushd "%ROOT_DIR%" >nul

echo Running workflow job "linux-cross-win32" with act...
act -j "linux-cross-win32" %*
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul

if not "%EXIT_CODE%"=="0" (
  echo act run failed with exit code %EXIT_CODE%.
  goto :end
)

echo act run completed successfully.
set "EXIT_CODE=0"

:end
if not defined NO_PAUSE if not defined CI (
  echo.
  pause
)
exit /b %EXIT_CODE%
