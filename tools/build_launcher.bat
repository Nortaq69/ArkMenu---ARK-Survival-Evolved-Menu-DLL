@echo off
echo Building ArkMenu Launcher...

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: Visual Studio not found. Please install Visual Studio with C++ tools.
    pause
    exit /b 1
)

REM Build the launcher
echo Compiling ArkMenuLauncher...
cl /EHsc /std:c++17 ArkMenuLauncher.cpp /Fe:ArkMenuLauncher.exe

if %errorlevel% neq 0 (
    echo Error: Build failed.
    pause
    exit /b 1
)

echo.
echo ArkMenuLauncher built successfully!
echo Location: tools\ArkMenuLauncher.exe
echo.
echo Usage:
echo 1. Run ArkMenuLauncher.exe as Administrator

echo This will launch ARK and auto-inject ArkMenu.dll when ready.
echo.
pause 