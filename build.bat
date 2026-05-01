@echo off
setlocal EnableDelayedExpansion

:: -------------------------------------------------
:: CONFIG
:: -------------------------------------------------
set "BUILD_DIR=build"
set "CONFIG_TYPE=Release"
set "CACHE_ENABLED=0"
set "BUILD_COVERAGE=OFF"
set "CACHE_DIR=%LOCALAPPDATA%\temp\vcpkg-cache"
if "%VCPKG_CACHE_KEY%"=="" set "VCPKG_CACHE_KEY=local-default"
set "CACHE_ZIP=%CACHE_DIR%\%VCPKG_CACHE_KEY%.zip"

if "%~1" neq "" if /I not "%~1"=="Cache" (
    set "CONFIG_TYPE=%~1"
    set "BUILD_COVERAGE=ON"
) else if "%~1"=="Cache" (
    set "CONFIG_TYPE=Release"
    set "CACHE_ENABLED=1"
) else (
    set "CONFIG_TYPE=Release"
)


:: Use CI-provided env if available, fallback to local
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=%CD%\vcpkg"
if "%VCPKG_OVERLAY_PORTS%"=="" set "VCPKG_OVERLAY_PORTS=%CD%\external\overlay-ports"
if "%VCPKG_DEFAULT_TRIPLET%"=="" set "VCPKG_DEFAULT_TRIPLET=x64-windows"

:: -------------------------------------------------
:: CACHE: Restore from cache
:: -------------------------------------------------
if "%CACHE_ENABLED%"=="1" (
    set "VCPKG_INSTALLED=%BUILD_DIR%\vcpkg_installed"
    if exist "%CACHE_ZIP%" (
        echo === Restoring vcpkg from cache ===
        if exist "%VCPKG_INSTALLED%" rmdir /s /q "%VCPKG_INSTALLED%"
        powershell -Command "Expand-Archive -Path '%CACHE_ZIP%' -DestinationPath '%BUILD_DIR%\vcpkg_installed' -Force"
        echo === Cache restored ===
    ) else (
        echo === No cached vcpkg found, will build from scratch ===
    )
)

:: -------------------------------------------------
:: DEBUG INFO (very useful for CI)
:: -------------------------------------------------
echo === Environment ===
echo CD=%CD%
echo VCPKG_ROOT=%VCPKG_ROOT%
echo VCPKG_OVERLAY_PORTS=%VCPKG_OVERLAY_PORTS%
echo CACHE_KEY=%VCPKG_CACHE_KEY%
echo.

:: -------------------------------------------------
:: VALIDATE PATHS
:: -------------------------------------------------
if not exist "%VCPKG_OVERLAY_PORTS%" (
    echo ERROR: Overlay ports directory not found!
    exit /b 1
)

:: -------------------------------------------------
:: SETUP MSVC
:: -------------------------------------------------
echo === Setup MSVC ===
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if %ERRORLEVEL% neq 0 exit /b 1

:: -------------------------------------------------
:: SETUP VCPKG (local, reproducible)
:: -------------------------------------------------
echo === Setup vcpkg ===
if not exist "%VCPKG_ROOT%" (
    git clone https://github.com/microsoft/vcpkg "%VCPKG_ROOT%"
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat"
    if %ERRORLEVEL% neq 0 exit /b 1
)

:: -------------------------------------------------
:: CONFIGURE
:: -------------------------------------------------
echo === Configuring project ===
set VCPKG_OVERLAY_PORTS=%VCPKG_OVERLAY_PORTS%
cmake -B "%BUILD_DIR%" -S . ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
  -DVCPKG_TARGET_TRIPLET=%VCPKG_DEFAULT_TRIPLET% ^
  -DCMAKE_BUILD_TYPE=%CONFIG_TYPE% ^
  -DENABLE_COVERAGE=%BUILD_COVERAGE% ^
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: -------------------------------------------------
:: BUILD
:: -------------------------------------------------
echo === Building project ===
cmake --build "%BUILD_DIR%" --config %CONFIG_TYPE%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo === Build completed successfully ===

:: -------------------------------------------------
:: CACHE: Save to cache
:: -------------------------------------------------*
if "%CACHE_ENABLED%"=="1" (
    echo %LOCALAPPDATA%
    echo %TEMP%
    echo %CACHE_ZIP%
    set "VCPKG_INSTALLED=%BUILD_DIR%\vcpkg_installed"
    if exist "%VCPKG_INSTALLED%" (
        if not exist "%CACHE_DIR%" mkdir "%CACHE_DIR%"
        if exist "%CACHE_ZIP%" del /f /q "%CACHE_ZIP%"
        echo === Saving vcpkg cache ===
        powershell -Command "Compress-Archive -Path '%VCPKG_INSTALLED%\*' -DestinationPath '%CACHE_ZIP%' -Force"
        echo === Cache saved to %CACHE_ZIP% ===
    ) else (
        echo === No vcpkg installed, skipping cache save ===
    )
)