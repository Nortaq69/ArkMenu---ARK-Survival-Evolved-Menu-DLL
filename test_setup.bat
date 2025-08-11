@echo off
echo Testing Setup Script
echo ===================
echo.

echo Current directory: %CD%
echo Script directory: %~dp0
echo.

echo Testing PowerShell script execution...
powershell -ExecutionPolicy Bypass -File "%~dp0test_setup.ps1"

echo.
echo Test complete!
pause 