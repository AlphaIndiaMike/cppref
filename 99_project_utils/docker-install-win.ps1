# docker-install-win.ps1 - Docker Desktop installation script for Windows
# Run this script as Administrator

#Requires -RunAsAdministrator

Write-Host "========================================" -ForegroundColor Green
Write-Host "Docker Desktop Installation for Windows" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# Check Windows version
$osInfo = Get-CimInstance -ClassName Win32_OperatingSystem
$osVersion = [System.Environment]::OSVersion.Version

Write-Host "Windows Version: $($osInfo.Caption) (Build $($osVersion.Build))" -ForegroundColor Yellow
Write-Host ""

# Check if Windows 10/11 Pro, Enterprise, or Education (for Hyper-V)
$windowsEdition = (Get-WindowsEdition -Online).Edition

if ($windowsEdition -match "Home") {
    Write-Host "WARNING: Windows Home edition detected." -ForegroundColor Yellow
    Write-Host "Docker Desktop will use WSL 2 backend (Hyper-V not available)." -ForegroundColor Yellow
    Write-Host ""
}

# Check if WSL 2 is installed
Write-Host "Checking WSL 2..." -ForegroundColor Cyan
$wslVersion = wsl --status 2>&1

if ($LASTEXITCODE -ne 0) {
    Write-Host "WSL not found. Installing WSL 2..." -ForegroundColor Yellow
    
    # Enable WSL
    dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
    
    # Enable Virtual Machine Platform
    dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
    
    Write-Host ""
    Write-Host "WSL features enabled. A restart is required." -ForegroundColor Yellow
    $restart = Read-Host "Restart now? (Y/N)"
    
    if ($restart -eq "Y" -or $restart -eq "y") {
        Write-Host "Restarting... Please run this script again after restart." -ForegroundColor Yellow
        Restart-Computer
        exit
    } else {
        Write-Host "Please restart manually and run this script again." -ForegroundColor Yellow
        exit
    }
} else {
    Write-Host "WSL 2 is installed ✓" -ForegroundColor Green
}

# Set WSL 2 as default
wsl --set-default-version 2 2>&1 | Out-Null

# Check if Docker is already installed
$dockerInstalled = Test-Path "C:\Program Files\Docker\Docker\Docker Desktop.exe"

if ($dockerInstalled) {
    Write-Host ""
    Write-Host "Docker Desktop is already installed." -ForegroundColor Yellow
    $reinstall = Read-Host "Do you want to reinstall? (Y/N)"
    
    if ($reinstall -ne "Y" -and $reinstall -ne "y") {
        Write-Host "Installation cancelled." -ForegroundColor Green
        exit
    }
    
    Write-Host "Uninstalling existing Docker Desktop..." -ForegroundColor Yellow
    Start-Process "C:\Program Files\Docker\Docker\Docker Desktop Installer.exe" -ArgumentList "uninstall" -Wait -NoNewWindow
}

# Download Docker Desktop installer
$dockerUrl = "https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe"
$installerPath = "$env:TEMP\DockerDesktopInstaller.exe"

Write-Host ""
Write-Host "Downloading Docker Desktop..." -ForegroundColor Cyan
try {
    Invoke-WebRequest -Uri $dockerUrl -OutFile $installerPath -UseBasicParsing
    Write-Host "Download complete ✓" -ForegroundColor Green
} catch {
    Write-Host "Failed to download Docker Desktop." -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}

# Install Docker Desktop
Write-Host ""
Write-Host "Installing Docker Desktop..." -ForegroundColor Cyan
Write-Host "This may take several minutes..." -ForegroundColor Yellow

try {
    Start-Process -FilePath $installerPath -ArgumentList "install", "--quiet", "--accept-license" -Wait -NoNewWindow
    Write-Host "Installation complete ✓" -ForegroundColor Green
} catch {
    Write-Host "Installation failed." -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}

# Clean up installer
Remove-Item $installerPath -ErrorAction SilentlyContinue

# Start Docker Desktop
Write-Host ""
Write-Host "Starting Docker Desktop..." -ForegroundColor Cyan
Start-Process "C:\Program Files\Docker\Docker\Docker Desktop.exe"

Write-Host ""
Write-Host "Waiting for Docker to start (this may take 1-2 minutes)..." -ForegroundColor Yellow

# Wait for Docker to be ready
$maxAttempts = 60
$attempt = 0
$dockerReady = $false

while ($attempt -lt $maxAttempts) {
    Start-Sleep -Seconds 2
    $attempt++
    
    try {
        $dockerInfo = docker info 2>&1
        if ($LASTEXITCODE -eq 0) {
            $dockerReady = $true
            break
        }
    } catch {
        # Continue waiting
    }
    
    Write-Host "." -NoNewline
}

Write-Host ""

if ($dockerReady) {
    Write-Host "Docker is running! ✓" -ForegroundColor Green
    
    # Verify installation
    Write-Host ""
    Write-Host "Verifying Docker installation..." -ForegroundColor Cyan
    docker --version
    docker compose version
} else {
    Write-Host "Docker did not start within expected time." -ForegroundColor Yellow
    Write-Host "Please start Docker Desktop manually from the Start menu." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Docker Desktop installed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Docker Desktop is running in the system tray"
Write-Host "2. Test: docker run hello-world"
Write-Host "3. Configure settings via Docker Desktop icon"
Write-Host ""
Write-Host "Tips:" -ForegroundColor Yellow
Write-Host "- Enable 'Start Docker Desktop when you log in' in Settings"
Write-Host "- Allocate resources (CPU/Memory) in Settings → Resources"
Write-Host "- Use WSL 2 integration for better performance"
Write-Host ""
