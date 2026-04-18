@echo off

set "BUILD_DIR=build"
set "ENABLE_COVERAGE=ON"
set "ENABLE_PROFILING=ON"
set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
set "TARGET_TRIPLET=x64-windows"
set "VCPKG_OVERLAY_PORTS=%USERPROFILE%\Desktop\Pikselite-Engine\external\overlay-ports"
set "VC_VARS_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

@REM echo %VCPKG_OVERLAY_PORTS%
@REM echo %VCPKG_ROOT%
@REM echo "%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"

:: Get the shell second parameter and set CONFIG_TYPE to it if it exists, otherwise set it to Release
if "%~1" neq "" (
    set "CONFIG_TYPE=%~1"
) else (
    set "CONFIG_TYPE=Release"
)

echo Build configuration: %CONFIG_TYPE%
:: -------------------------------------------------
:: CLEAN VCPKG ARCHIVES IF BUILD DIR MISSING
:: -------------------------------------------------
@REM echo === Checking build directory ===
@REM if not exist "%BUILD_DIR%" (
@REM     echo Build directory missing, cleaning vcpkg archives...
@REM     rmdir /s /q "%LOCALAPPDATA%\vcpkg\archives"
@REM )

:: -------------------------------------------------
:: Force vcpkg to use the correct toolset and triplet
:: -------------------------------------------------
call "%VC_VARS_PATH%" x64
set VCPKG_PLATFORM_TOOLSET=v143
echo %TARGET_TRIPLET%
vcpkg integrate install --triplet %TARGET_TRIPLET%
if %ERRORLEVEL% neq 0 (
    echo Failed to install dependencies.
    exit /b 1
)

echo Dependencies installed successfully.

:: -------------------------------------------------
:: CONFIGURE PROJECT
:: -------------------------------------------------
echo === Configuring project ===
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
@REM -DVCPKG_OVERLAY_PORTS="%VCPKG_OVERLAY_PORTS%" ^
cmake -B "%BUILD_DIR%" -S . ^
-DCMAKE_BUILD_TYPE="%CONFIG_TYPE%" ^
-DENABLE_COVERAGE="%ENABLE_COVERAGE%" ^
-DVCPKG_OVERLAY_PORTS=external/overlay-ports ^
-DPIKSELITE_ENABLE_PROFILING="%ENABLE_PROFILING%" ^
-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
-DVCPKG_TARGET_TRIPLET=%TARGET_TRIPLET%

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: -------------------------------------------------
:: BUILD PROJECT
:: -------------------------------------------------
echo === Building project ===
cmake --build "%BUILD_DIR%" --config "%CONFIG_TYPE%"
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)
echo Build completed successfully.