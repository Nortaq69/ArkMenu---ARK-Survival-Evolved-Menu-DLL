# Test script to verify setup.ps1 exists and can be executed
Write-Host "=== Setup Script Test ===" -ForegroundColor Cyan
Write-Host ""

$scriptPath = Join-Path $PSScriptRoot "setup.ps1"
Write-Host "Looking for setup script at: $scriptPath" -ForegroundColor Yellow

if (Test-Path $scriptPath) {
    Write-Host "✓ setup.ps1 found!" -ForegroundColor Green
    Write-Host "File size: $((Get-Item $scriptPath).Length) bytes" -ForegroundColor Green
    
    # Test if we can read the script
    try {
        $content = Get-Content $scriptPath -Raw
        Write-Host "✓ Script can be read successfully" -ForegroundColor Green
        Write-Host "First line: $($content.Split("`n")[0])" -ForegroundColor Gray
    }
    catch {
        Write-Host "✗ Error reading script: $($_.Exception.Message)" -ForegroundColor Red
    }
    
    # Test if we can execute the script (syntax check)
    try {
        $null = [System.Management.Automation.PSParser]::Tokenize($content, [ref]$null)
        Write-Host "✓ Script syntax is valid" -ForegroundColor Green
    }
    catch {
        Write-Host "✗ Script syntax error: $($_.Exception.Message)" -ForegroundColor Red
    }
}
else {
    Write-Host "✗ setup.ps1 not found!" -ForegroundColor Red
    Write-Host "Current directory: $PWD" -ForegroundColor Yellow
    Write-Host "Available files:" -ForegroundColor Yellow
    Get-ChildItem -Name "*.ps1" | ForEach-Object { Write-Host "  - $_" -ForegroundColor Gray }
}

Write-Host ""
Write-Host "Test complete!" -ForegroundColor Cyan 