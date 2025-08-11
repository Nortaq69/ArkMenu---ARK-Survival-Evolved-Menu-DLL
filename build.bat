@echo off
echo Building ArkMenu...

REM Check if CMake is available
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: CMake not found. Please install CMake and add it to PATH.
    pause
    exit /b 1
)

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: Visual Studio not found. Please install Visual Studio with C++ tools.
    pause
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring project...
cmake .. -G "Visual Studio 16 2019" -A x64
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed.
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Error: Build failed.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo DLL location: build\bin\Release\ArkMenu.dll
echo.
echo To install:
echo 1. Copy ArkMenu.dll to your ARK installation directory
echo 2. For Microsoft Store version: C:\Program Files\WindowsApps\Microsoft.FlightSimulator_*\ARK\ShooterGame\Binaries\WinGDK\
echo 3. Inject the DLL using your preferred injector
echo.
pause 