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
$fileBaseName = [System.IO.Path]::GetFileNameWithoutExtension($SourceFile)

if (-not $OutputFile) {
    $OutputFile = Join-Path $headersDir "$($fileBaseName)_public.h"
}

if (-not $PrivateOutputFile) {
    $PrivateOutputFile = Join-Path $headersDir "$($fileBaseName)_private.h"
}

# Check if a base header file exists (myfile.h)
$baseHeaderFile = Join-Path $headersDir "$($fileBaseName).h"
$baseHeaderExists = Test-Path -Path $baseHeaderFile
$baseHeaderFileName = "$($fileBaseName).h"

# Read the source file
$sourceContent = Get-Content -Path $SourceFile -Raw

# Extract the filename without extension for the include guard
$publicGuard = [System.IO.Path]::GetFileNameWithoutExtension($OutputFile).ToUpper() + "_H"
$privateGuard = [System.IO.Path]::GetFileNameWithoutExtension($PrivateOutputFile).ToUpper() + "_H"

# First, check if we need to regenerate the header files by comparing timestamps
$sourceLastModified = (Get-Item -Path $SourceFile).LastWriteTime

# Flag to track if we need to regenerate
$needsRegeneration = $true

# Check public header timestamp if it exists
if (Test-Path -Path $OutputFile) {
    $publicLastModified = (Get-Item -Path $OutputFile).LastWriteTime
    if ($publicLastModified -gt $sourceLastModified) {
        $needsRegeneration = $false
    }
}

# Check private header timestamp if it exists
if (-not $needsRegeneration -and (Test-Path -Path $PrivateOutputFile)) {
    $privateLastModified = (Get-Item -Path $PrivateOutputFile).LastWriteTime
    if ($privateLastModified -lt $sourceLastModified) {
        $needsRegeneration = $true
    }
}

# If source file hasn't been modified since the header files were last generated, we can skip
if (-not $needsRegeneration) {
    Write-Host "Header files for $($fileBaseName).c are up to date. Skipping generation."
    exit 0
}

# Extract only the last documentation comment before each function
# This pattern looks for function declarations
$functionDeclarationPattern = '(?:static\s+)?(?:\w+(?:\s+\w+)*(?:\s*\*+\s*|\s+))(\w+)\s*\([^{]*\)\s*(?:{|[\r\n]\s*{)'
$functionMatches = [regex]::Matches($sourceContent, $functionDeclarationPattern)

# Create dictionary to store documentation for each function
$functionDocs = @{}

foreach ($match in $functionMatches) {
    $functionName = $match.Groups[1].Value
    $functionPos = $match.Index
    
    # Get all content before this function
    $contentBeforeFunction = $sourceContent.Substring(0, $functionPos)
    
    # Find the last comment block before this function
    $lastCommentPattern = '/\*\*[\s\S]*?\*/'
    $lastCommentMatches = [regex]::Matches($contentBeforeFunction, $lastCommentPattern)
    
    if ($lastCommentMatches.Count -gt 0) {
        # Get only the last comment block
        $lastComment = $lastCommentMatches[$lastCommentMatches.Count - 1].Value
        
        # Check if this comment is directly before the function (no other code between)
        $commentEnd = $lastCommentMatches[$lastCommentMatches.Count - 1].Index + $lastCommentMatches[$lastCommentMatches.Count - 1].Length
        $textBetween = $contentBeforeFunction.Substring($commentEnd).Trim()
        
        # If there's only whitespace, preprocessor directives, or nothing between the comment and function
        if ($textBetween -match '^\s*(?:#.*?[\r\n]+\s*)*$') {
            $functionDocs[$functionName] = $lastComment
        }
    }
}

# First, strip comments to avoid false positives when extracting function declarations
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
    $prototype = ""
    
    # Add doc comment if it exists for this function
    if ($functionDocs.ContainsKey($functionName)) {
        $prototype += "$($functionDocs[$functionName])`n"
    }
    
    $prototype += "static $returnType $functionName($parameters);"
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
    $prototype = ""
    
    # Add doc comment if it exists for this function
    if ($functionDocs.ContainsKey($functionName)) {
        $prototype += "$($functionDocs[$functionName])`n"
    }
    
    $prototype += "$returnType $functionName($parameters);"
    if (-not ($prototype -match 'static')) {
        $publicPrototypes += $prototype
    }
}

# Determine system headers based on whether base header exists
$systemHeaders = @"
// Include necessary system headers
#include <tice.h>
#include <ti/real.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
"@

# Determine what to include in the public header
$publicIncludes = if ($baseHeaderExists) {
    "// Include base header file
#include `"$baseHeaderFileName`""
} else {
    $systemHeaders
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
 * Documentation comments from source are preserved
 */

#ifndef $publicGuard
#define $publicGuard

#ifdef __cplusplus
extern "C" {
#endif

$publicIncludes

// Public Function Prototypes
$($publicPrototypes -join "`n`n")

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
 * Documentation comments from source are preserved
 */

#ifndef $privateGuard
#define $privateGuard

// Include the public header first to ensure all types are available
#include "$(Split-Path $OutputFile -Leaf)"

// Static Function Prototypes - only include this header in the corresponding .c file
$($staticPrototypes -join "`n`n")

#endif // $privateGuard
"@

# Write to the output files
try {
    # Create headers directory if it doesn't exist
    if (-not (Test-Path -Path $headersDir)) {
        New-Item -ItemType Directory -Path $headersDir -Force | Out-Null
    }

    # Check if there are any public or static functions
    $hasPublic = $publicPrototypes.Count -gt 0
    $hasStatic = $staticPrototypes.Count -gt 0

    # Handle all possible combinations
    if (-not $hasPublic -and -not $hasStatic) {
        Write-Host "No functions found in the source file. No header files generated."
        exit 0
    }
    
    # Generate public header if there are public functions
    if ($hasPublic) {
        Set-Content -Path $OutputFile -Value $publicHeaderContent -ErrorAction Stop
        Write-Host "Public header generated: $OutputFile ($($publicPrototypes.Count) prototypes)"
        if ($baseHeaderExists) {
            Write-Host "  Base header found and referenced: $baseHeaderFile"
        }
    } else {
        Write-Host "No public functions found. Public header not generated."
    }
    
    # Generate private header if there are static functions
    if ($hasStatic) {
        # If we have static functions but no public functions, we need to modify the private header
        if (-not $hasPublic) {
            # Create a modified private header that doesn't include the public header
            $privateHeaderContent = @"

/*
 * This is an automatically generated private header file from the source file: $SourceFile
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: $(Get-Date)
 * Source file: $SourceFile
 * Contains static function prototypes: $($staticPrototypes.Count)
 * Documentation comments from source are preserved
 */

#ifndef $privateGuard
#define $privateGuard

// Include system headers or base header
$($baseHeaderExists ? "// Include base header file`n#include `"$baseHeaderFileName`"" : $systemHeaders)

// Static Function Prototypes - only include this header in the corresponding .c file
$($staticPrototypes -join "`n`n")

#endif // $privateGuard
"@
        }
        
        Set-Content -Path $PrivateOutputFile -Value $privateHeaderContent -ErrorAction Stop
        Write-Host "Private header generated: $PrivateOutputFile ($($staticPrototypes.Count) prototypes)"
    } else {
        Write-Host "No static functions found. Private header not generated."
    }
} catch {
    Write-Error "Failed to create header files: $_"
    exit 1
}