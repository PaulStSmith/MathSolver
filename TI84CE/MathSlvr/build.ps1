param(
    [Parameter(Mandatory=$false)]
    [string]$BuildType = "debug"  # Default to debug build
)

# Clean up previous build artifacts
Remove-Item .\bin\* -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\obj\* -Recurse -Force -ErrorAction SilentlyContinue

# Clear the console and show build info
Clear-Host
Write-Host "Building project in $BuildType mode..."

# Call CreateHeaders.ps1 for each .c file in the src directory
Write-Host "Calling CreateHeaders.ps1"
Get-ChildItem .\src\*.c | ForEach-Object { .\CreateHeaders.ps1 -SourceFile $_.FullName }

# Call make with the appropriate build target
Write-Host "Calling make $BuildType"
if ($BuildType -ne "debug") {
    make DEBUG=0
} else {
    # Default to debug build
    make debug
}

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build completed successfully in $BuildType mode!" -ForegroundColor Green
} else {
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
}