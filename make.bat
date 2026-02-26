@echo off
setlocal

set "ROOT_DIR=%~dp0"
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"

where uv >nul 2>nul
if errorlevel 1 (
  echo Could not find uv in PATH.
  echo Install uv and try again.
  exit /b 1
)

set "TARGET=bundle"
set "CROSS_MODE=0"

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="--cross" (
  set "CROSS_MODE=1"
) else if /I "%~1"=="all" (
  set "TARGET=bundle"
) else if /I "%~1"=="bundle" (
  set "TARGET=bundle"
) else if /I "%~1"=="compile" (
  set "TARGET=compile"
) else if /I "%~1"=="configure" (
  set "TARGET=configure"
) else if /I "%~1"=="clean" (
  set "TARGET=clean"
) else if /I "%~1"=="test" (
  set "TARGET=test"
) else if /I "%~1"=="distclean" (
  set "TARGET=distclean"
) else (
  echo Unknown argument: %~1
  exit /b 1
)
shift
goto parse_args

:args_done
set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-local"
set "CROSS_ARGS="
if "%CROSS_MODE%"=="1" (
  set "BUILD_DIR=%ROOT_DIR%\.meson\build\mingw-win32-linux"
  set "CROSS_ARGS=--cross-file meson/cross/mingw32.ini"
)

set "MESON=uv run --group build meson"
set "EXIT_CODE=0"

pushd "%ROOT_DIR%" >nul

if /I "%TARGET%"=="distclean" goto do_distclean
if /I "%TARGET%"=="clean" goto do_clean

call :configure
if errorlevel 1 (
  set "EXIT_CODE=%ERRORLEVEL%"
  goto done
)

if /I "%TARGET%"=="configure" goto done
if /I "%TARGET%"=="compile" goto do_compile
if /I "%TARGET%"=="bundle" goto do_bundle
if /I "%TARGET%"=="test" goto do_test

echo Unknown target: %TARGET%
set "EXIT_CODE=1"
goto done

:configure
if exist "%BUILD_DIR%\build.ninja" (
  echo Reconfiguring Meson build directory: %BUILD_DIR%
  %MESON% setup --reconfigure "%BUILD_DIR%" %CROSS_ARGS% %CONFIGURE_ARGS%
) else (
  echo Configuring Meson build directory: %BUILD_DIR%
  %MESON% setup "%BUILD_DIR%" --buildtype release %CROSS_ARGS% %CONFIGURE_ARGS%
)
exit /b %ERRORLEVEL%

:do_compile
%MESON% compile -C "%BUILD_DIR%"
set "EXIT_CODE=%ERRORLEVEL%"
goto done

:do_bundle
%MESON% compile -C "%BUILD_DIR%" bundle
set "EXIT_CODE=%ERRORLEVEL%"
goto done

:do_test
%MESON% test -C "%BUILD_DIR%"
set "EXIT_CODE=%ERRORLEVEL%"
goto done

:do_clean
if exist "%BUILD_DIR%\build.ninja" (
  %MESON% compile -C "%BUILD_DIR%" --clean
  set "EXIT_CODE=%ERRORLEVEL%"
) else (
  echo Nothing to clean in "%BUILD_DIR%"
  set "EXIT_CODE=0"
)
goto done

:do_distclean
if exist "%ROOT_DIR%\.meson\build\mingw-win32-local" rmdir /s /q "%ROOT_DIR%\.meson\build\mingw-win32-local"
if exist "%ROOT_DIR%\.meson\build\mingw-win32-linux" rmdir /s /q "%ROOT_DIR%\.meson\build\mingw-win32-linux"
if exist "%ROOT_DIR%\build" rmdir /s /q "%ROOT_DIR%\build"
if exist "%ROOT_DIR%\dist" rmdir /s /q "%ROOT_DIR%\dist"
set "EXIT_CODE=0"

:done
popd >nul
exit /b %EXIT_CODE%
