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
set "OUTPUT=%ROOT_DIR%\build\bitnessscan.exe"

if not exist "%ROOT_DIR%\make.bat" (
  echo Missing build wrapper: "%ROOT_DIR%\make.bat"
  set "EXIT_CODE=1"
  goto :end
)

call "%ROOT_DIR%\make.bat" bundle
if errorlevel 1 (
  echo Build failed.
  set "EXIT_CODE=1"
  goto :end
)

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
