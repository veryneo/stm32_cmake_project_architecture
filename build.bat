@echo off
setlocal enabledelayedexpansion

echo ========================================
echo STM32 CMake Build Script
echo ========================================

:: Check if we're in the right directory (should contain CMakeLists.txt)
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found in current directory!
    echo Please run this script from the project root directory.
    pause
    exit /b 1
)

:: Check and clean build directory
echo Checking build directory...
if exist "build" (
    echo Build directory exists. Removing it...
    rmdir /s /q build
    if !ERRORLEVEL! NEQ 0 (
        echo Failed to remove existing build directory!
        echo Please close any applications that might be using files in the build directory.
        pause
        exit /b 1
    )
    echo Build directory removed successfully.
)

:: Create fresh build directory
echo Creating fresh build directory...
mkdir build
if !ERRORLEVEL! NEQ 0 (
    echo Failed to create build directory!
    pause
    exit /b 1
)

:: Change to build directory
echo Changing to build directory...
cd build
if !ERRORLEVEL! NEQ 0 (
    echo Failed to change to build directory!
    pause
    exit /b 1
)

:: Run CMake
echo Running CMake configuration...
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="../custom_gcc_win.cmake"
if !ERRORLEVEL! NEQ 0 (
    echo CMake configuration failed!
    echo Please check your CMake configuration and toolchain file.
    pause
    exit /b 1
)

:: Run Ninja build
echo Running Ninja build...
ninja
if !ERRORLEVEL! NEQ 0 (
    echo Build failed!
    echo Please check the error messages above.
    pause
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================
pause