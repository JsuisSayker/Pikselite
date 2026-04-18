@echo off
setlocal EnableDelayedExpansion

:: -------------------------------------------------
:: CONFIG
:: -------------------------------------------------
set "BUILD_DIR=build"
set "CONFIG_TYPE=Release"

:: Use CI-provided env if available, fallback to local
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=%CD%\vcpkg"
if "%VCPKG_OVERLAY_PORTS%"=="" set "VCPKG_OVERLAY_PORTS=%CD%\external\overlay-ports"
if "%VCPKG_DEFAULT_TRIPLET%"=="" set "VCPKG_DEFAULT_TRIPLET=x64-windows"

:: -------------------------------------------------
:: DEBUG INFO (very useful for CI)
:: -------------------------------------------------
echo === Environment ===
echo CD=%CD%
echo VCPKG_ROOT=%VCPKG_ROOT%
echo VCPKG_OVERLAY_PORTS=%VCPKG_OVERLAY_PORTS%
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
cmake -B "%BUILD_DIR%" -S . ^
-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
-DVCPKG_OVERLAY_PORTS="%VCPKG_OVERLAY_PORTS%" ^
-DVCPKG_TARGET_TRIPLET=%VCPKG_DEFAULT_TRIPLET% ^
-DCMAKE_BUILD_TYPE=%CONFIG_TYPE%

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