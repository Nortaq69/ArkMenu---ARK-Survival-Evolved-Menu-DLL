# ArkMenu Automated Setup Script
# This script will automatically set up the entire development environment

param(
    [switch]$SkipPrerequisites,
    [switch]$SkipDependencies,
    [switch]$SkipBuild,
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

Write-Host "=== ArkMenu Automated Setup ===" -ForegroundColor Cyan
Write-Host "This script will set up the complete development environment" -ForegroundColor Yellow
Write-Host ""

# Function to check if a command exists
function Test-Command($cmdname) {
    return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
}

# Function to download file
function Download-File($url, $output) {
    Write-Host "Downloading $url..." -ForegroundColor Green
    try {
        Invoke-WebRequest -Uri $url -OutFile $output -UseBasicParsing
        return $true
    }
    catch {
        Write-Host "Failed to download $url" -ForegroundColor Red
        return $false
    }
}

# Function to install Chocolatey if not present
function Install-Chocolatey {
    if (Test-Command choco) {
        Write-Host "Chocolatey is already installed" -ForegroundColor Green
        return $true
    }
    
    Write-Host "Installing Chocolatey package manager..." -ForegroundColor Yellow
    try {
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
        return $true
    }
    catch {
        Write-Host "Failed to install Chocolatey" -ForegroundColor Red
        return $false
    }
}

# Function to install Visual Studio Build Tools
function Install-VisualStudio {
    if (Test-Command cl) {
        Write-Host "Visual Studio Build Tools are already installed" -ForegroundColor Green
        return $true
    }
    
    Write-Host "Installing Visual Studio Build Tools..." -ForegroundColor Yellow
    try {
        # Download Visual Studio Build Tools installer
        $vsInstallerUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe"
        $vsInstallerPath = "$env:TEMP\vs_buildtools.exe"
        
        if (Download-File $vsInstallerUrl $vsInstallerPath) {
            # Install with C++ workload
            $args = @(
                "--quiet",
                "--wait",
                "--norestart",
                "--nocache",
                "--installPath", "C:\BuildTools",
                "--add", "Microsoft.VisualStudio.Workload.VCTools",
                "--add", "Microsoft.VisualStudio.Component.Windows10SDK.19041"
            )
            
            Start-Process -FilePath $vsInstallerPath -ArgumentList $args -Wait
            Remove-Item $vsInstallerPath -Force
            
            # Add to PATH
            $env:PATH += ";C:\BuildTools\VC\Tools\MSVC\*\bin\Hostx64\x64"
            return $true
        }
        return $false
    }
    catch {
        Write-Host "Failed to install Visual Studio Build Tools" -ForegroundColor Red
        return $false
    }
}

# Function to install CMake
function Install-CMake {
    if (Test-Command cmake) {
        Write-Host "CMake is already installed" -ForegroundColor Green
        return $true
    }
    
    Write-Host "Installing CMake..." -ForegroundColor Yellow
    try {
        choco install cmake -y
        refreshenv
        return $true
    }
    catch {
        Write-Host "Failed to install CMake" -ForegroundColor Red
        return $false
    }
}

# Function to install Git
function Install-Git {
    if (Test-Command git) {
        Write-Host "Git is already installed" -ForegroundColor Green
        return $true
    }
    
    Write-Host "Installing Git..." -ForegroundColor Yellow
    try {
        choco install git -y
        refreshenv
        return $true
    }
    catch {
        Write-Host "Failed to install Git" -ForegroundColor Red
        return $false
    }
}

# Function to setup dependencies
function Setup-Dependencies {
    Write-Host "Setting up dependencies..." -ForegroundColor Yellow
    
    # Create external directory if it doesn't exist
    if (!(Test-Path "external")) {
        New-Item -ItemType Directory -Path "external" | Out-Null
    }
    
    # Change to external directory
    Push-Location "external"
    
    # Clone ImGui
    if (!(Test-Path "imgui")) {
        Write-Host "Cloning ImGui..." -ForegroundColor Green
        git clone https://github.com/ocornut/imgui.git
    } else {
        Write-Host "ImGui already exists, updating..." -ForegroundColor Green
        Push-Location "imgui"
        git pull
        Pop-Location
    }
    
    # Clone MinHook
    if (!(Test-Path "minhook")) {
        Write-Host "Cloning MinHook..." -ForegroundColor Green
        git clone https://github.com/TsudaKageyu/minhook.git
    } else {
        Write-Host "MinHook already exists, updating..." -ForegroundColor Green
        Push-Location "minhook"
        git pull
        Pop-Location
    }
    
    # Clone JsonCpp
    if (!(Test-Path "jsoncpp")) {
        Write-Host "Cloning JsonCpp..." -ForegroundColor Green
        git clone https://github.com/open-source-parsers/jsoncpp.git
    } else {
        Write-Host "JsonCpp already exists, updating..." -ForegroundColor Green
        Push-Location "jsoncpp"
        git pull
        Pop-Location
    }
    
    Pop-Location
    Write-Host "Dependencies setup complete!" -ForegroundColor Green
}

# Function to build the project
function Build-Project {
    Write-Host "Building ArkMenu..." -ForegroundColor Yellow
    
    # Create build directory
    if (!(Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }
    
    Push-Location "build"
    
    try {
        # Configure with CMake
        Write-Host "Configuring project with CMake..." -ForegroundColor Green
        cmake .. -G "Visual Studio 17 2022" -A x64
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "CMake configuration failed, trying Visual Studio 16 2019..." -ForegroundColor Yellow
            cmake .. -G "Visual Studio 16 2019" -A x64
        }
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        # Build the project
        Write-Host "Building project..." -ForegroundColor Green
        cmake --build . --config Release
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "Build completed successfully!" -ForegroundColor Green
        
        # Build the launcher
        Write-Host "Building ArkMenuLauncher..." -ForegroundColor Green
        Push-Location "..\tools"
        .\build_launcher.bat
        Pop-Location
        
        # Build the injector
        Write-Host "Building Injector..." -ForegroundColor Green
        Push-Location "..\tools"
        .\build_injector.bat
        Pop-Location
        
    }
    catch {
        Write-Host "Build failed: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
    finally {
        Pop-Location
    }
    
    return $true
}

# Function to create desktop shortcuts
function Create-Shortcuts {
    Write-Host "Creating desktop shortcuts..." -ForegroundColor Yellow
    
    $desktop = [Environment]::GetFolderPath("Desktop")
    $currentDir = Get-Location
    
    # Create launcher shortcut
    $launcherShortcut = "$desktop\ArkMenu Launcher.lnk"
    $WshShell = New-Object -comObject WScript.Shell
    $Shortcut = $WshShell.CreateShortcut($launcherShortcut)
    $Shortcut.TargetPath = "$currentDir\tools\ArkMenuLauncher.exe"
    $Shortcut.WorkingDirectory = "$currentDir"
    $Shortcut.Description = "Launch ARK and inject ArkMenu automatically"
    $Shortcut.Save()
    
    # Create injector shortcut
    $injectorShortcut = "$desktop\ArkMenu Injector.lnk"
    $Shortcut = $WshShell.CreateShortcut($injectorShortcut)
    $Shortcut.TargetPath = "$currentDir\tools\injector.exe"
    $Shortcut.WorkingDirectory = "$currentDir"
    $Shortcut.Description = "Inject ArkMenu into running ARK process"
    $Shortcut.Save()
    
    Write-Host "Desktop shortcuts created!" -ForegroundColor Green
}

# Main setup process
function Main {
    Write-Host "Starting automated setup..." -ForegroundColor Cyan
    
    # Check if running as administrator
    if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
        Write-Host "This script requires administrator privileges. Please run as administrator." -ForegroundColor Red
        return
    }
    
    # Step 1: Install prerequisites
    if (!$SkipPrerequisites) {
        Write-Host "`n=== Step 1: Installing Prerequisites ===" -ForegroundColor Cyan
        
        if (!(Install-Chocolatey)) {
            Write-Host "Failed to install Chocolatey. Setup cannot continue." -ForegroundColor Red
            return
        }
        
        if (!(Install-VisualStudio)) {
            Write-Host "Failed to install Visual Studio Build Tools. Setup cannot continue." -ForegroundColor Red
            return
        }
        
        if (!(Install-CMake)) {
            Write-Host "Failed to install CMake. Setup cannot continue." -ForegroundColor Red
            return
        }
        
        if (!(Install-Git)) {
            Write-Host "Failed to install Git. Setup cannot continue." -ForegroundColor Red
            return
        }
        
        Write-Host "Prerequisites installation complete!" -ForegroundColor Green
    }
    
    # Step 2: Setup dependencies
    if (!$SkipDependencies) {
        Write-Host "`n=== Step 2: Setting up Dependencies ===" -ForegroundColor Cyan
        Setup-Dependencies
    }
    
    # Step 3: Build project
    if (!$SkipBuild) {
        Write-Host "`n=== Step 3: Building Project ===" -ForegroundColor Cyan
        if (!(Build-Project)) {
            Write-Host "Build failed. Please check the error messages above." -ForegroundColor Red
            return
        }
    }
    
    # Step 4: Create shortcuts
    Write-Host "`n=== Step 4: Creating Desktop Shortcuts ===" -ForegroundColor Cyan
    Create-Shortcuts
    
    # Final success message
    Write-Host "`n=== Setup Complete! ===" -ForegroundColor Green
    Write-Host "ArkMenu has been successfully set up!" -ForegroundColor Green
    Write-Host ""
    Write-Host "What's next:" -ForegroundColor Yellow
    Write-Host "1. Use 'ArkMenu Launcher' to start ARK and auto-inject the cheat" -ForegroundColor White
    Write-Host "2. Use 'ArkMenu Injector' to inject into a running ARK process" -ForegroundColor White
    Write-Host "3. Press Delete in-game to open the menu" -ForegroundColor White
    Write-Host ""
    Write-Host "Files created:" -ForegroundColor Yellow
    Write-Host "- build\bin\Release\ArkMenu.dll (Main cheat DLL)" -ForegroundColor White
    Write-Host "- tools\ArkMenuLauncher.exe (Auto-launch and inject)" -ForegroundColor White
    Write-Host "- tools\injector.exe (Manual injector)" -ForegroundColor White
    Write-Host ""
    Write-Host "Press any key to exit..." -ForegroundColor Cyan
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}

# Run the main setup
Main 