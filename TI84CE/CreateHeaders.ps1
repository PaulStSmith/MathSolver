# Generate-Header.ps1
# Script to automatically generate C header files from C implementation files
# This version creates both public and private header files

param(
    [Parameter(Mandatory=$true)]
    [string]$SourceFile,
    
    [Parameter(Mandatory=$false)]
    [string]$OutputFile = $null,
    
    [Parameter(Mandatory=$false)]
    [string]$PrivateOutputFile = $null
)

# If no output files are specified, use the source filename with .h and _private.h extensions
# Determine the directory of the source file
$sourceDir = [System.IO.Path]::GetDirectoryName($SourceFile)
$headersDir = Join-Path $sourceDir "headers"

if (-not $OutputFile) {
    $fileBaseName = [System.IO.Path]::GetFileNameWithoutExtension($SourceFile)
    $OutputFile = Join-Path $headersDir "$($fileBaseName)_public.h"
}

if (-not $PrivateOutputFile) {
    $fileBaseName = [System.IO.Path]::GetFileNameWithoutExtension($SourceFile)
    $PrivateOutputFile = Join-Path $headersDir "$($fileBaseName)_private.h"
}

# Read the source file
$sourceContent = Get-Content -Path $SourceFile -Raw

# Extract the filename without extension for the include guard
$publicGuard = [System.IO.Path]::GetFileNameWithoutExtension($OutputFile).ToUpper() + "_H"
$privateGuard = [System.IO.Path]::GetFileNameWithoutExtension($PrivateOutputFile).ToUpper() + "_H"

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

# Improved regex for static function definitions
$staticFunctionPattern = '(?<!\bif|\bwhile|\bfor|\bswitch|\belse|\bdo)' + 
                         '\bstatic\s+' +                                 # static keyword
                         '(\b\w+(?:\s+\w+)*(?:\s*\*+\s*|\s+))' +         # Return type with possible pointers
                         '(\w+)\s*\(' +                                  # Function name
                         '([\s\w,.*[\]\(\)]*?)' +                        # Parameter list
                         '\)\s*(?:{|[\r\n]\s*{)'                         # Opening brace

# Regex for public function definitions (not marked static)
$publicFunctionPattern = '(?<!\bif|\bwhile|\bfor|\bswitch|\belse|\bdo)' + 
                         '(?<!static\s+)' +                               # Not preceded by static
                         '(\b\w+(?:\s+\w+)*(?:\s*\*+\s*|\s+))' +          # Return type with possible pointers
                         '(\w+)\s*\(' +                                   # Function name
                         '([\s\w,.*[\]\(\)]*?)' +                         # Parameter list
                         '\)\s*(?:{|[\r\n]\s*{)'                          # Opening brace

# Match static and public functions
$staticBlocks = [regex]::Matches($preprocessed, $staticFunctionPattern)
$publicBlocks = [regex]::Matches($preprocessed, $publicFunctionPattern)

$staticPrototypes = @()
$publicPrototypes = @()

# Process static functions
foreach ($block in $staticBlocks) {
    $returnType = $block.Groups[1].Value.Trim()
    $functionName = $block.Groups[2].Value.Trim()
    $parameters = $block.Groups[3].Value.Trim()
    
    # Skip functions with empty names (likely false positives)
    if ([string]::IsNullOrWhiteSpace($functionName)) {
        continue
    }
    
    # Format the prototype, preserving pointer spacing as in original
    $prototype = "static $returnType $functionName($parameters);"
    $staticPrototypes += $prototype
}

# Process public functions
foreach ($block in $publicBlocks) {
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
    if (-not ($prototype -match 'static')) {
        $publicPrototypes += $prototype
    }
}

# Generate the public header file content
$publicHeaderContent = @"

/*
 * This is an automatically generated public header file from the source file: $SourceFile
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: $(Get-Date)
 * Source file: $SourceFile
 * Contains public function prototypes: $($publicPrototypes.Count)
 */

#ifndef $publicGuard
#define $publicGuard

#ifdef __cplusplus
extern "C" {
#endif

// Include necessary system headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Public Function Prototypes
$($publicPrototypes -join "`n")

#ifdef __cplusplus
}
#endif

#endif // $publicGuard
"@

# Generate the private header file content
$privateHeaderContent = @"

/*
 * This is an automatically generated private header file from the source file: $SourceFile
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: $(Get-Date)
 * Source file: $SourceFile
 * Contains static function prototypes: $($staticPrototypes.Count)
 */

#ifndef $privateGuard
#define $privateGuard

// Include the public header first to ensure all types are available
#include "$(Split-Path $OutputFile -Leaf)"

// Static Function Prototypes - only include this header in the corresponding .c file
$($staticPrototypes -join "`n")

#endif // $privateGuard
"@

# Write to the output files
try {
    # Create headers directory if it doesn't exist
    if (-not (Test-Path -Path $headersDir)) {
        New-Item -ItemType Directory -Path $headersDir -Force | Out-Null
    }

    # Write public header
    Set-Content -Path $OutputFile -Value $publicHeaderContent -ErrorAction Stop
    
    # Only write private header if there are static functions to include
    if ($staticPrototypes.Count -gt 0) {
        Set-Content -Path $PrivateOutputFile -Value $privateHeaderContent -ErrorAction Stop
        Write-Host "Header files generated successfully:"
        Write-Host "  Public: $OutputFile ($($publicPrototypes.Count) prototypes)"
        Write-Host "  Private: $PrivateOutputFile ($($staticPrototypes.Count) prototypes)"
    } else {
        Write-Host "No static functions found. Only public header generated:"
        Write-Host "  Public: $OutputFile ($($publicPrototypes.Count) prototypes)"
    }
} catch {
    Write-Error "Failed to create header files: $_"
    exit 1
}