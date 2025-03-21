Remove-Item .\bin\* -Recurse -Force
Remove-Item .\obj\* -Recurse -Force
Clear-Host
Write-Host "Building project..."
Write-Host "Calling CreateHeaders.ps1"
.\CreateHeaders.ps1 -SourceFile .\src\log.c
.\CreateHeaders.ps1 -SourceFile .\src\ui.c
.\CreateHeaders.ps1 -SourceFile .\src\mathsolver.c
write-host "Calling make"
make

