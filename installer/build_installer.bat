@echo off
setlocal enabledelayedexpansion

title PikseliteEngine - Build Installer

:: -------------------------------------------------
:: CONFIGURATION
:: -------------------------------------------------
set "PROJECT_ROOT=%~dp0.."
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "CONFIG_TYPE=Release"
set "VCPKG_PATHS=%USERPROFILE%\vcpkg C:\vcpkg"
set "VCPKG_ROOT="
set "CI=false"

:: -------------------------------------------------
:: VERSION (single source of truth is vcpkg.json)
:: Accept as first arg, else read from vcpkg.json
:: -------------------------------------------------
set "APP_VERSION=%~1"
if "!APP_VERSION!"=="" (
    for /f "delims=" %%v in ('powershell -NoProfile -Command "(Get-Content '%PROJECT_ROOT%\vcpkg.json' -Raw | ConvertFrom-Json).version"') do set "APP_VERSION=%%v"
)
if "!APP_VERSION!"=="" (
    echo ERROR: Could not determine version. Pass it as the first argument or ensure vcpkg.json has a version.
    exit /b 1
)
echo Using version !APP_VERSION!

:: -------------------------------------------------
:: STEP 1: FIND VCPKG
:: -------------------------------------------------
echo === Searching for vcpkg ===
for %%p in (%VCPKG_PATHS%) do (
    if exist "%%p\vcpkg.exe" (
        set "VCPKG_ROOT=%%p"
    )
)
if "!VCPKG_ROOT!"=="" (
    echo ERROR: vcpkg not found. Please run setup.bat first.
    exit /b 1
)
echo Using vcpkg at "!VCPKG_ROOT!"

:: -------------------------------------------------
:: STEP 2: FIND CMake
:: -------------------------------------------------
echo === Searching for CMake ===
set "CMAKE_EXE="
for /f "delims=" %%i in ('where cmake 2^>nul') do (
    set "CMAKE_EXE=%%i"
    goto :found_cmake
)
if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles%\CMake\bin\cmake.exe"
    goto :found_cmake
)
if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles(x86)%\CMake\bin\cmake.exe"
    goto :found_cmake
)
if exist "%USERPROFILE%\cmake\cmake-3.30.0-windows-x86_64\bin\cmake.exe" (
    set "CMAKE_EXE=%USERPROFILE%\cmake\cmake-3.30.0-windows-x86_64\bin\cmake.exe"
    goto :found_cmake
)

echo ERROR: CMake not found. Please run setup.bat first.
exit /b 1

:found_cmake
echo Found CMake at "!CMAKE_EXE!"

:: Add cmake to PATH
for %%i in ("!CMAKE_EXE!") do set "CMAKE_BIN=%%~dpi"
set "PATH=!CMAKE_BIN!;%PATH%"

:: -------------------------------------------------
:: STEP 3: BUILD RELEASE
:: -------------------------------------------------
echo === Building Release configuration ===

:: Setup MSVC environment
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to set up MSVC environment.
    echo Please ensure Visual Studio Build Tools 2022 is installed.
    exit /b 1
)

:: Configure cmake (only if needed)
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo Configuring CMake...
    "!CMAKE_EXE!" -B "!BUILD_DIR!" -S "%PROJECT_ROOT%" ^
        -DCMAKE_TOOLCHAIN_FILE="!VCPKG_ROOT!\scripts\buildsystems\vcpkg.cmake" ^
        -G "Visual Studio 17 2022" -A x64 -T ClangCL
    if !ERRORLEVEL! neq 0 (
        echo CMake configuration failed!
        exit /b 1
    )
)

:: Build
echo Building PikseliteEngine...
"!CMAKE_EXE!" --build "!BUILD_DIR!" --config "!CONFIG_TYPE!" --target PikseliteEngine
if %ERRORLEVEL% neq 0 (
    echo Release build failed!
    exit /b 1
)

:: -------------------------------------------------
:: STEP 4: VERIFY BUILD ARTIFACTS
:: -------------------------------------------------
echo === Verifying build artifacts ===
set "RELEASE_DIR=%BUILD_DIR%\%CONFIG_TYPE%"
if not exist "%RELEASE_DIR%\PikseliteEngine.exe" (
    echo ERROR: PikseliteEngine.exe not found in "%RELEASE_DIR%"
    dir "%BUILD_DIR%" /s /b PikseliteEngine.exe 2>nul
    exit /b 1
)

echo Build artifacts found in "%RELEASE_DIR%"

:: -------------------------------------------------
:: STEP 5: FIND INNO SETUP
:: -------------------------------------------------
echo === Searching for Inno Setup ===
set "ISCC_EXE="
for /f "delims=" %%i in ('where iscc 2^>nul') do (
    set "ISCC_EXE=%%i"
    goto :found_iscc
)
if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" (
    set "ISCC_EXE=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
    goto :found_iscc
)
if exist "%ProgramFiles%\Inno Setup 6\ISCC.exe" (
    set "ISCC_EXE=%ProgramFiles%\Inno Setup 6\ISCC.exe"
    goto :found_iscc
)
if exist "%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" (
    set "ISCC_EXE=%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe"
    goto :found_iscc
)

echo ERROR: Inno Setup not found.
echo Please install Inno Setup 6 from: https://jrsoftware.org/isdl.php
echo.
echo After installing, run this script again.
exit /b 1

:found_iscc
echo Found Inno Setup at "!ISCC_EXE!"

:: -------------------------------------------------
:: STEP 6: BUILD INSTALLER
:: -------------------------------------------------
echo === Building installer ===
cd /d "%PROJECT_ROOT%\installer"

if not exist output mkdir output

"!ISCC_EXE!" setup.iss ^
    /dSourcePath="%RELEASE_DIR%" ^
    /dProjectRoot="%PROJECT_ROOT%" ^
    /dMyAppVersion="!APP_VERSION!"

if %ERRORLEVEL% neq 0 (
    echo Installer build failed!
    exit /b 1
)

echo.
echo ============================================
echo Installer built successfully!
echo Output: %PROJECT_ROOT%\installer\output\PikseliteEngine-Setup-!APP_VERSION!.exe
echo ============================================

:: -------------------------------------------------
:: STEP 7: LAUNCH INSTALLER (optional)
:: -------------------------------------------------
if "%CI%"=="true" (
    set /p LAUNCH=Launch installer now? (Y/N):
    if /i "!LAUNCH!"=="Y" (
        start "" "%PROJECT_ROOT%\installer\output\PikseliteEngine-Setup-!APP_VERSION!.exe"
    )
)
endlocal