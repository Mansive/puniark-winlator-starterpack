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

set "TARGET="
set "CROSS_MODE=0"
set "REMAINING_ARGS="

:parse_args
if "%~1"=="" goto after_parse

if /I "%~1"=="--cross" (
  set "CROSS_MODE=1"
) else if not defined TARGET if /I "%~1"=="all" (
  set "TARGET=bundle"
) else if not defined TARGET if /I "%~1"=="bundle" (
  set "TARGET=bundle"
) else if not defined TARGET if /I "%~1"=="compile" (
  set "TARGET=compile"
) else if not defined TARGET if /I "%~1"=="configure" (
  set "TARGET=configure"
) else if not defined TARGET if /I "%~1"=="clean" (
  set "TARGET=clean"
) else if not defined TARGET if /I "%~1"=="test" (
  set "TARGET=test"
) else if not defined TARGET if /I "%~1"=="distclean" (
  set "TARGET=distclean"
) else (
  set "REMAINING_ARGS=!REMAINING_ARGS! "%~1""
)

shift
goto parse_args

:after_parse
if not defined TARGET set "TARGET=bundle"

set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-local"
set "CROSS_ARG="
if "%CROSS_MODE%"=="1" (
  set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-linux"
  set "CROSS_ARG=--cross"
)

set "MESON=uv run --group build meson"

if /I "%TARGET%"=="distclean" goto do_distclean
if /I "%TARGET%"=="configure" goto do_configure

call "%ROOT_DIR%\configure.bat" %CROSS_ARG% %REMAINING_ARGS%
if errorlevel 1 exit /b 1

pushd "%ROOT_DIR%" >nul

if /I "%TARGET%"=="compile" goto do_compile
if /I "%TARGET%"=="bundle" goto do_bundle
if /I "%TARGET%"=="clean" goto do_clean
if /I "%TARGET%"=="test" goto do_test

popd >nul
echo Unknown target: %TARGET%
exit /b 1

:do_compile
%MESON% compile -C "%BUILD_DIR%" %REMAINING_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"
popd >nul
exit /b %EXIT_CODE%

:do_bundle
%MESON% compile -C "%BUILD_DIR%" bundle %REMAINING_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"
popd >nul
exit /b %EXIT_CODE%

:do_clean
if exist "%BUILD_DIR%\build.ninja" (
  %MESON% compile -C "%BUILD_DIR%" --clean %REMAINING_ARGS%
  set "EXIT_CODE=%ERRORLEVEL%"
) else (
  echo Nothing to clean in "%BUILD_DIR%"
  set "EXIT_CODE=0"
)
popd >nul
exit /b %EXIT_CODE%

:do_test
%MESON% test -C "%BUILD_DIR%" %REMAINING_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"
popd >nul
exit /b %EXIT_CODE%

:do_configure
call "%ROOT_DIR%\configure.bat" %CROSS_ARG% %REMAINING_ARGS%
exit /b %ERRORLEVEL%

:do_distclean
if exist "%ROOT_DIR%\.meson\build\mingw-win32-local" rmdir /s /q "%ROOT_DIR%\.meson\build\mingw-win32-local"
if exist "%ROOT_DIR%\.meson\build\mingw-win32-linux" rmdir /s /q "%ROOT_DIR%\.meson\build\mingw-win32-linux"
if exist "%ROOT_DIR%\build" rmdir /s /q "%ROOT_DIR%\build"
if exist "%ROOT_DIR%\dist" rmdir /s /q "%ROOT_DIR%\dist"
exit /b 0
