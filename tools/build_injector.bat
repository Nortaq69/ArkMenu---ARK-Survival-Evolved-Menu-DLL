@echo off
echo Building ArkMenu Injector...

REM Check if Visual Studio is available
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: Visual Studio not found. Please install Visual Studio with C++ tools.
    pause
    exit /b 1
)

REM Create tools directory if it doesn't exist
if not exist tools mkdir tools

REM Build the injector
echo Compiling injector...
cl /EHsc /std:c++17 Injector.cpp /Fe:injector.exe

if %errorlevel% neq 0 (
    echo Error: Build failed.
    pause
    exit /b 1
)

echo.
echo Injector built successfully!
echo Location: tools\injector.exe
echo.
echo Usage:
echo 1. Start ARK: Survival Evolved
echo 2. Run injector.exe as Administrator
echo 3. The injector will automatically find and inject the DLL
echo.
pause 