param(
    [Parameter(Mandatory=$false)]
    [string]$BuildType = "debug"  # Default to debug build
)

function Write-Line {
    param(
        [Parameter(Mandatory=$false)]
        [string]$Character = "-",
        
        [Parameter(Mandatory=$false)]
        [System.ConsoleColor]$Color = [System.ConsoleColor]::White
    )
    
    $width = $Host.UI.RawUI.WindowSize.Width
    
    # Calculate how many characters to repeat
    # If the character string is longer than 1 character, ensure we don't exceed the window width
    $repeatCount = [Math]::Floor($width / $Character.Length)
    
    # Create the line by repeating the character
    $line = $Character * $repeatCount
    
    # Trim if needed to ensure we don't exceed window width
    if ($line.Length -gt $width) {
        $line = $line.Substring(0, $width)
    }
    
    # Write the line with the specified color
    Write-Host $line -ForegroundColor $Color
}

$makefileExist = Test-Path -Path .\makefile
If (-not $makefileExist) {
    Write-Host "Makefile not found in the current directory." -ForegroundColor Red
    exit 1
}

$currDir = Get-Location
$myPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$createHeaders = Join-Path $myPath CreateHeaders.ps1
$bin = Join-Path $currDir bin "*"
$obj = Join-Path $currDir obj "*"

Clear-Host
Write-Host "<#" -ForegroundColor DarkGray
Write-Host " #  ___                        _               ___      _ _    _ " -ForegroundColor DarkGray
Write-Host " # | _ \_ _ ___ _ __  __ _ _ _(_)_ _  __ _    | _ )_  _(_) |__| | " -ForegroundColor DarkGray
Write-Host " # |  _/ '_/ -_) '_ \/ _`` | '_| | ' \/ _`` |   | _ \ || | | / _`` |_ _ _ " -ForegroundColor DarkGray
Write-Host " # |_| |_| \___| .__/\__,_|_| |_|_||_\__, |   |___/\_,_|_|_\__,_(_)_)_)" -ForegroundColor DarkGray
Write-Host " #             |_|                   |___/ " -ForegroundColor DarkGray
Write-Host " #>" -ForegroundColor DarkGray

# Clear the console and show build info
Write-Host "Building project in $BuildType mode..." -ForegroundColor Green
Write-Host "Build started at $(Get-Date)" -ForegroundColor DarkGray
Write-Line

Write-Host "<#" -ForegroundColor DarkGray
Write-Host " #   ___ _               _ " -ForegroundColor DarkGray
Write-Host " #  / __| |___ __ _ _ _ (_)_ _  __ _ " -ForegroundColor DarkGray
Write-Host " # | (__| / -_) _`` | ' \| | ' \/ _`` |_ _ _ " -ForegroundColor DarkGray
Write-Host " #  \___|_\___\__,_|_||_|_|_||_\__, (_)_)_)" -ForegroundColor DarkGray
Write-Host " #                             |___/ " -ForegroundColor DarkGray
Write-Host " #>" -ForegroundColor DarkGray

# Clean up previous build artifacts
Write-Host "Cleaning up previous build artifacts..." -ForegroundColor Gray
Remove-Item $bin -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item $obj -Recurse -Force -ErrorAction SilentlyContinue
Write-Line

Write-Host "<#" -ForegroundColor DarkGray
Write-Host " #   ___              _   _               _  _             _ " -ForegroundColor DarkGray
Write-Host " #  / __|_ _ ___ __ _| |_(_)_ _  __ _    | || |___ __ _ __| |___ _ _ ___" -ForegroundColor DarkGray
Write-Host " # | (__| '_/ -_) _`` |  _| | ' \/ _`` |   | __ / -_) _`` / _`` / -_) '_(_-<" -ForegroundColor DarkGray
Write-Host " #  \___|_| \___\__,_|\__|_|_||_\__, |   |_||_\___\__,_\__,_\___|_| /__/" -ForegroundColor DarkGray
Write-Host " #                              |___/ " -ForegroundColor DarkGray
Write-Host " #>" -ForegroundColor DarkGray

# Call CreateHeaders.ps1 for each .c file in the src directory
Write-Host "Creating headers..."
if (-not (Test-Path $createHeaders)) {
    Write-Host "CreateHeaders.ps1 not found in the current directory." -ForegroundColor Red
    exit 1
}
Get-ChildItem .\src\*.c | ForEach-Object { 
    & $CreateHeaders -SourceFile $_.FullName 
    Write-Line " - "
}

Write-Host "<#" -ForegroundColor DarkGray
Write-Host " #  ___      _ _    _ _ " -ForegroundColor DarkGray
Write-Host " # | _ )_  _(_) |__| (_)_ _  __ _ " -ForegroundColor DarkGray
Write-Host " # | _ \ || | | / _`` | | ' \/ _`` |_ _ _ " -ForegroundColor DarkGray
Write-Host " # |___/\_,_|_|_\__,_|_|_||_\__, (_)_)_)" -ForegroundColor DarkGray
Write-Host " #                          |___/ " -ForegroundColor DarkGray
Write-Host " #>" -ForegroundColor DarkGray
# Call make with the appropriate build target
Write-Host "Calling 'make $BuildType'" -ForegroundColor Yellow
if ($BuildType -ne "debug") {
    make DEBUG=0
} else {
    # Default to debug build
    make debug
}
Write-Line

Write-Host "<#" -ForegroundColor DarkGray
Write-Host " #  ___ " -ForegroundColor DarkGray
Write-Host " # |   \ ___ _ _  ___ " -ForegroundColor DarkGray
Write-Host " # | |) / _ \ ' \/ -_)" -ForegroundColor DarkGray
Write-Host " # |___/\___/_||_\___|" -ForegroundColor DarkGray
Write-Host " # " -ForegroundColor DarkGray
Write-Host " #>" -ForegroundColor DarkGray

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build completed successfully in $BuildType mode!" -ForegroundColor Green
} else {
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
}