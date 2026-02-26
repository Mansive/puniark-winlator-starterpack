@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "EXE_FILE=%SCRIPT_DIR%bitnessscan.exe"
set "EXIT_CODE=0"

if not exist "%EXE_FILE%" (
  echo Error: bitnessscan.exe is missing at "%EXE_FILE%"
  set "EXIT_CODE=1"
  goto :end
)

"%EXE_FILE%" --pick %*
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
pause
exit /b %EXIT_CODE%
