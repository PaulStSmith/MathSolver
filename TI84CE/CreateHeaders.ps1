# Generate-Header.ps1
# Script to automatically generate C header files from C implementation files

param(
    [Parameter(Mandatory=$true)]
    [string]$SourceFile,
    
    [Parameter(Mandatory=$false)]
    [string]$OutputFile = $null
)

# If no output file is specified, use the source filename with .h extension
if (-not $OutputFile) {
    $OutputFile = [System.IO.Path]::ChangeExtension($SourceFile, ".h")
}

# Read the source file
$sourceContent = Get-Content -Path $SourceFile -Raw

# Extract the filename without extension for the include guard
$fileBaseName = [System.IO.Path]::GetFileNameWithoutExtension($OutputFile)
$includeGuard = $fileBaseName.ToUpper() + "_H"

# First, strip comments to avoid false positives
$noComments = $sourceContent
# Remove single-line comments
$noComments = [regex]::Replace($noComments, "//.*?$", "", "Multiline")
# Remove multi-line comments
$noComments = [regex]::Replace($noComments, "/\*[\s\S]*?\*/", "", "Singleline")

# Preprocess to handle preprocessor directives, macros, etc.
$lines = $noComments -split "`n"
$preprocessedLines = New-Object System.Collections.ArrayList
foreach ($line in $lines) {
    # Skip preprocessor directives
    if (-not ($line -match '^\s*#')) {
        $null = $preprocessedLines.Add($line)
    }
}
$preprocessed = $preprocessedLines -join "`n"

# Improved regex for function definitions that handles pointers correctly
# This pattern captures:
# 1. Return type, which may include pointers (*) and type qualifiers
# 2. Function name (identifier)
# 3. Parameter list, which may contain complex types
# 4. Followed by opening brace (either on same line or next line)
$functionPattern = '(?<!\bif|\bwhile|\bfor|\bswitch|\belse|\bdo)' + 
                  '(\b\w+(?:\s+\w+)*(?:\s*\*+\s*|\s+))' +  # Return type with possible pointers
                  '(\w+)\s*\(' +                            # Function name
                  '([\s\w,.*[\]\(\)]*?)' +                 # Parameter list
                  '\)\s*(?:{|[\r\n]\s*{)'                   # Opening brace

$blocks = [regex]::Matches($preprocessed, $functionPattern)

$functionPrototypes = @()

foreach ($block in $blocks) {
    $returnType = $block.Groups[1].Value.Trim()
    $functionName = $block.Groups[2].Value.Trim()
    $parameters = $block.Groups[3].Value.Trim()
    
    # Skip main function
    if ($functionName -eq "main") {
        continue
    }
    
    # Skip if return type contains control keywords
    if ($returnType -match '\b(if|while|for|switch|else|do)\b') {
        continue
    }
    
    # Skip functions with empty names (likely false positives)
    if ([string]::IsNullOrWhiteSpace($functionName)) {
        continue
    }
    
    # Format the prototype, preserving pointer spacing as in original
    $prototype = "$returnType $functionName($parameters);"
    $functionPrototypes += $prototype
}

# Generate the header file content
$headerContent = @"

/*
 * This is an automatically generated header file from the source file: $SourceFile
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: $(Get-Date)
 * Source file size: $(($sourceContent.Length / 1KB).ToString("F2")) KB
 * Source file lines: $($sourceContent.Split("`n").Count)
 * Function prototypes found: $($functionPrototypes.Count)
 */

#ifndef $includeGuard
#define $includeGuard

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes
$($functionPrototypes -join "`n")

#ifdef __cplusplus
}
#endif

#endif // $includeGuard
"@

# Write to the output file
try {
    Set-Content -Path $OutputFile -Value $headerContent -ErrorAction Stop
    Write-Host "Header file generated successfully: $OutputFile"
    Write-Host "Found $($functionPrototypes.Count) function prototypes"
} catch {
    Write-Error "Failed to create header file: $_"
    exit 1
}