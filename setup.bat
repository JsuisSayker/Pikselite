@echo off
setlocal enabledelayedexpansion

title PikseliteEngine Setup

:: -------------------------------------------------
:: CONFIGURATION
:: -------------------------------------------------
set "PROJECT_NAME=PikseliteEngine"
set "BUILD_DIR=build"
set "TARGET_TRIPLET=x64-windows"
set "VCPKG_HASH_FILE=%USERPROFILE%\.vcpkg_hash"
set "VCPKG_PATHS=%USERPROFILE%\vcpkg C:\vcpkg"
set "VCPKG_ROOT="
set "CMAKE_EXE=%USERPROFILE%\cmake\cmake-3.30.0-windows-x86_64\bin\cmake.exe"

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
    set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
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

:: -------------------------------------------------
:: CHECK CMAKE BINARY
:: -------------------------------------------------
if not exist "!CMAKE_EXE!" (
    echo CMake not found at "!CMAKE_EXE!"!
    exit /b 1
)
echo Found CMake at "!CMAKE_EXE!"

:: -------------------------------------------------
:: DETERMINE IF DEPENDENCIES NEED INSTALLATION
:: -------------------------------------------------
set "INSTALL_DEPS=1"
if exist "%VCPKG_HASH_FILE%" (
    certutil -hashfile vcpkg.json SHA256 | find /i " " > "%TEMP%\temp_hash.txt"
    set /p CURRENT_HASH=<"%TEMP%\temp_hash.txt"
    set /p LAST_HASH=<"%VCPKG_HASH_FILE%"
    del "%TEMP%\temp_hash.txt"
    if "!CURRENT_HASH!"=="!LAST_HASH!" (
        set "INSTALL_DEPS=0"
        echo vcpkg.json unchanged, skipping dependency installation.
    )
)

:: -------------------------------------------------
:: INSTALL DEPENDENCIES (if needed)
:: -------------------------------------------------
if "!INSTALL_DEPS!"=="1" (
    echo === Installing dependencies via vcpkg ===

    :: Setup VS 2022 x64 environment
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

    :: Force vcpkg to use the correct toolset and triplet
    set VCPKG_PLATFORM_TOOLSET=v143
    "!VCPKG_ROOT!\vcpkg.exe" integrate install --triplet !TARGET_TRIPLET!
    if %ERRORLEVEL% neq 0 (
        echo Failed to install dependencies.
        exit /b 1
    )

    echo Dependencies installed successfully.

    :: Save current hash
    certutil -hashfile vcpkg.json SHA256 | find /i " " > "%VCPKG_HASH_FILE%"
)


:: -------------------------------------------------
:: CONFIGURE PROJECT
:: -------------------------------------------------
echo === Setting up build directory ===
echo === Configuring project ===
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
"!CMAKE_EXE!" -B "!BUILD_DIR!" -S . -DCMAKE_TOOLCHAIN_FILE="!VCPKG_ROOT!\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=!TARGET_TRIPLET!
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: -------------------------------------------------
:: BUILD PROJECT
:: -------------------------------------------------
echo === Building project ===
"!CMAKE_EXE!" --build "!BUILD_DIR!" --config Release
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
