@echo off
echo ArkMenu Automated Setup Launcher
echo ================================
echo.
echo This will automatically set up the complete ArkMenu development environment.
echo.
echo Requirements:
echo - Windows 10/11
echo - Internet connection
echo - Administrator privileges
echo.
echo The setup will install:
echo - Visual Studio Build Tools 2022
echo - CMake
echo - Git
echo - All dependencies (ImGui, MinHook, JsonCpp)
echo - Build the project
echo - Create desktop shortcuts
echo.
pause

echo.
echo Starting automated setup...
echo.

REM Check if setup.ps1 exists
if not exist "%~dp0setup.ps1" (
    echo ERROR: setup.ps1 not found in the current directory!
    echo Current directory: %CD%
    echo Script location: %~dp0
    echo.
    pause
    exit /b 1
)

echo Found setup.ps1 at: %~dp0setup.ps1
echo.

REM Set execution policy and run PowerShell script
powershell -ExecutionPolicy Bypass -File "%~dp0setup.ps1"

echo.
echo Setup complete! Check the output above for any errors.
echo.
pause 