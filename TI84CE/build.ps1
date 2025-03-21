Clear-Host
Remove-Item .\bin\* -Recurse -Force
Remove-Item .\obj\* -Recurse -Force
Clear-Host
Write-Host "Building project..."
write-host "Calling make"
make

