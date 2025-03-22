Remove-Item .\bin\* -Recurse -Force
Remove-Item .\obj\* -Recurse -Force
Clear-Host
Write-Host "Building project..."
Write-Host "Calling CreateHeaders.ps1"
# Search for all .c files in the src directory and call CreateHeaders.ps1 on each one
Get-ChildItem .\src\*.c | ForEach-Object { .\CreateHeaders.ps1 -SourceFile $_.FullName }
write-host "Calling make"
make

