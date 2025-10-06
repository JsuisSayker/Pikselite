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
set "CMAKE_LOCATION=%USERPROFILE%\cmake\cmake-3.30.0-windows-x86_64\bin\"

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

:: =====================================================
:: CHECK / INSTALL CMAKE (Portable)
:: =====================================================
echo === Checking for CMake ===

setlocal enabledelayedexpansion
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
:: do a loop that look for cmake.exe in the CMAKE_LOCATION folder
for /r "%CMAKE_LOCATION%" %%i in (cmake.exe) do (
    if exist "%%i" (
        set "CMAKE_EXE=%%i"
        goto :found_cmake
    )
)

:: If still not found, install portable version
echo CMake not found. Installing portable version...
if not exist "%CMAKE_DIR%" mkdir "%CMAKE_DIR%"
pushd "%CMAKE_DIR%"
curl -L -o cmake.zip https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-windows-x86_64.zip

if exist cmake.zip (
    powershell -Command "Expand-Archive -Path 'cmake.zip' -DestinationPath . -Force"
    del cmake.zip

    :: Find the cmake.exe in the extracted subfolder
    for /r "%CMAKE_LOCATION%" %%f in (cmake.exe) do (
        set "CMAKE_EXE=%%f"
        goto :cmake_found_after_extract
    )

    :cmake_found_after_extract
    if not defined CMAKE_EXE (
        echo Failed to extract CMake executable!
        popd
        exit /b 1
    )
    echo CMake installed locally at "%CMAKE_EXE%"
)
popd

:found_cmake
echo Found CMake at "%CMAKE_EXE%"

:: Add only the directory to PATH
for %%i in ("%CMAKE_EXE%") do set "CMAKE_BIN=%%~dpi"
set "PATH=%CMAKE_BIN%;%PATH%"

cmake --version


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

if "%INSTALL_DEPS%" equ "1" (
    echo === Installing dependencies via vcpkg ===

    set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

    :: -------------------------------------------------
    :: Check if VS Build Tools are installed
    :: -------------------------------------------------
    echo !VC_VARS_PATH!
    if not exist "!VC_VARS_PATH!" (
        echo Visual Studio Build Tools not found.

        :: Create temp folder
        set "TEMP_DIR=%TEMP%\vs_buildtools_installer"
        if not exist "!TEMP_DIR!" mkdir "!TEMP_DIR!"
        pushd "!TEMP_DIR!"

        :: Download Visual Studio Build Tools installer
        curl -L -o vs_buildtools.exe https://aka.ms/vs/17/release/vs_buildtools.exe

        if exist vs_buildtools.exe (
            echo Running silent installation...
            start /wait vs_buildtools.exe ^
                --quiet --wait --norestart --nocache ^
                --add Microsoft.VisualStudio.Workload.VCTools ^
                --includeRecommended
            set "VC_VARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
        ) else (
            echo Failed to download Visual Studio Build Tools installer!
            popd
            exit /b 1
        )

        popd

        :: Verify installation success
        if not exist "!VC_VARS_PATH!" (
            echo Visual Studio Build Tools installation failed!
            exit /b 1
        )
        echo Visual Studio Build Tools installed successfully.
    )

    :: -------------------------------------------------
    :: Setup VS 2022 x64 environment
    :: -------------------------------------------------
    echo Setting up Visual Studio environment...
    call "!VC_VARS_PATH!" x64

    :: -------------------------------------------------
    :: Force vcpkg to use the correct toolset and triplet
    :: -------------------------------------------------
    set VCPKG_PLATFORM_TOOLSET=v143
    "%VCPKG_ROOT%\vcpkg.exe" integrate install --triplet !TARGET_TRIPLET!
    if %ERRORLEVEL% neq 0 (
        echo Failed to install dependencies.
        exit /b 1
    )

    echo Dependencies installed successfully.

    :: -------------------------------------------------
    :: Save current hash
    :: -------------------------------------------------
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
