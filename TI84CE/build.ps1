Remove-Item .\bin\* -Recurse -Force
Remove-Item .\obj\* -Recurse -Force
Clear-Host
Write-Host "Building project..."
Write-Host "Calling CreateHeaders.ps1"
.\CreateHeaders.ps1 -SourceFile .\src\ui.c          -OutputFile .\src\headers\ui_functions.h
.\CreateHeaders.ps1 -SourceFile .\src\mathsolver.c  -OutputFile .\src\headers\mathsolver_functions.h
write-host "Calling make"
make

