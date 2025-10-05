@echo off
setlocal enabledelayedexpansion

title PikseliteEngine Setup

:: -------------------------------------------------
:: CONFIGURATION
:: -------------------------------------------------
set "PROJECT_NAME=PikseliteEngine"
set "BUILD_DIR=build"
set "TARGET_TRIPLET=x64-windows"
set "CMAKE_DIR=%USERPROFILE%\cmake"

:: -------------------------------------------------
:: DEFAULT VCPKG PATHS TO CHECK
:: -------------------------------------------------
set "VCPKG_PATHS=%USERPROFILE%\vcpkg C:\vcpkg"
set "VCPKG_ROOT="

:: -------------------------------------------------
:: SEARCH FOR EXISTING VCPKG
:: -------------------------------------------------
echo === Searching for vcpkg installation ===
for %%p in (%VCPKG_PATHS%) do (
    if exist "%%p\vcpkg.exe" (
        set "VCPKG_ROOT=%%p"
    )
)

:: -------------------------------------------------
:: INSTALL VCPKG IF NOT FOUND
:: -------------------------------------------------
echo === VCPKG Setup ===
if "!VCPKG_ROOT!" equ "" (
    echo vcpkg not found on this system.
    if not defined VCPKG_ROOT set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
    echo Installing vcpkg to "!VCPKG_ROOT!"...

    git clone https://github.com/microsoft/vcpkg "!VCPKG_ROOT!"
    if %ERRORLEVEL% neq 0 (
        echo Failed to clone vcpkg!
        exit /b 1
    )
    pushd "!VCPKG_ROOT!"
    call bootstrap-vcpkg.bat
    popd
    echo vcpkg installed successfully.
) else (
    echo Using existing vcpkg at "!VCPKG_ROOT!"
)

:: =====================================================
:: CHECK / INSTALL CMAKE (Portable)
:: =====================================================
echo === Checking for CMake ===

set "CMAKE_EXE="
set "CMAKE_DIR=%USERPROFILE%\cmake"

:: Try finding cmake from PATH
for /f "delims=" %%i in ('where cmake 2^>nul') do (
    set "CMAKE_EXE=%%i"
    goto :found_cmake
)

:: Try common installation paths
if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles%\CMake\bin\cmake.exe"
    goto :found_cmake
)
if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles(x86)%\CMake\bin\cmake.exe"
    goto :found_cmake
)

:: Try local portable version
for /r "!CMAKE_DIR!" %%i in (cmake.exe) do (
    set "CMAKE_EXE=%%i"
    goto :found_cmake
)

:: If still not found, install portable version
echo CMake not found. Installing portable version...
if not exist "!CMAKE_DIR!" mkdir "!CMAKE_DIR!"
pushd "!CMAKE_DIR!"
curl -L -o cmake.zip https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-windows-x86_64.zip
if exist cmake.zip (
    powershell -Command "Expand-Archive -Path 'cmake.zip' -DestinationPath . -Force"
    del cmake.zip

    for /d %%d in (*) do (
        if exist "%%d\bin\cmake.exe" (
            set "CMAKE_EXE=!CMAKE_DIR!\%%d\bin\cmake.exe"
        )
    )

    if not defined CMAKE_EXE (
        echo Failed to extract CMake executable!
        popd
        exit /b 1
    )
    echo CMake installed locally at "!CMAKE_EXE!"
) else (
    echo Failed to download portable CMake!
    popd
    exit /b 1
)
popd
goto :found_cmake

:found_cmake
echo Found CMake at "!CMAKE_EXE!"
set "PATH=%PATH%;%~dp0;%~dp0\bin;!CMAKE_DIR!;%~dp0;!CMAKE_EXE!"


:: -------------------------------------------------
:: INSTALL DEPENDENCIES
:: -------------------------------------------------
echo === Installing dependencies via vcpkg ===
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
"!VCPKG_ROOT!\vcpkg.exe" integrate install
if %ERRORLEVEL% neq 0 (
    echo Failed to install dependencies.
    exit /b 1
)
echo Dependencies installed successfully.

:: -------------------------------------------------
:: CONFIGURE PROJECT
:: -------------------------------------------------
echo === Setting up build directory ===
echo === Configuring project ===
cmake -B "!BUILD_DIR!" -S . -DCMAKE_TOOLCHAIN_FILE="!VCPKG_ROOT!\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=!TARGET_TRIPLET!
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: -------------------------------------------------
:: BUILD PROJECT
:: -------------------------------------------------
echo === Building project ===
cmake --build "!BUILD_DIR!" --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)
echo Build completed successfully.

:: -------------------------------------------------
:: RUN PROJECT
:: -------------------------------------------------
echo === Running project ===
if exist "!BUILD_DIR!\Release\!PROJECT_NAME!.exe" (
    "!BUILD_DIR!\Release\!PROJECT_NAME!.exe"
) else if exist "!BUILD_DIR!\!PROJECT_NAME!.exe" (
    "!BUILD_DIR!\!PROJECT_NAME!.exe"
) else (
    echo Executable not found!
)

echo === PikseliteEngine setup complete ===
