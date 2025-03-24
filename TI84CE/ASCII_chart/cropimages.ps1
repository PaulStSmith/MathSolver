# PowerShell script to crop images in a folder from vertical pixels 96 to 176
# and stitch them together vertically using a different approach
# Requires PowerShell and .NET Framework

Clear-Host

# Set the folder path containing the images
# Use the current directory by default, change this if your images are elsewhere
$folderPath = Join-Path $(Get-Location) "images"
Write-Host "Looking for images in: $folderPath" -ForegroundColor Cyan

# Check if the folder exists, if not create it
if (-not (Test-Path -Path $folderPath)) {
    Write-Host "Images folder doesn't exist, creating it..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $folderPath | Out-Null
}

# Create an output folder for the cropped images if it doesn't exist
$outputFolder = "$folderPath\cropped"
if (-not (Test-Path -Path $outputFolder)) {
    New-Item -ItemType Directory -Path $outputFolder | Out-Null
    Write-Host "Created output folder: $outputFolder"
}

# Get all image files in the folder (common image formats)
# Using -Recurse:$false to stay in the current folder and -Force to show hidden files
$imageFiles = Get-ChildItem -Path "$folderPath\*" -File -Include "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif" -Recurse:$false -Force

# Add debug info to check what's happening
Write-Host "Image folder path: $folderPath" -ForegroundColor Cyan
Write-Host "Initial file count: $($imageFiles.Count)" -ForegroundColor Cyan

# Check if any files were found
if ($imageFiles.Count -eq 0) {
    Write-Host "No image files found in $folderPath" -ForegroundColor Yellow
    Write-Host "Files in the current directory:" -ForegroundColor Yellow
    Get-ChildItem -Path $folderPath -File | ForEach-Object { Write-Host "  - $($_.Name)" }
    
    $continue = Read-Host "No images found. Would you like to try again with a different path? (Y/N)"
    if ($continue -eq "Y" -or $continue -eq "y") {
        $newPath = Read-Host "Enter the full path to your images folder"
        if (Test-Path -Path $newPath) {
            $folderPath = $newPath
            $imageFiles = Get-ChildItem -Path $newPath -File -Include "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif" -Recurse:$false -Force
        } else {
            Write-Host "The path $newPath does not exist. Exiting script." -ForegroundColor Red
            exit
        }
    } else {
        exit
    }
}

# Load the System.Drawing assembly for image manipulation
Add-Type -AssemblyName System.Drawing

# Sort the image files using natural sort order to handle filenames like "image (1).png" correctly
if ($null -ne $imageFiles -and $imageFiles.Count -gt 0) {
    Write-Host "Sorting $($imageFiles.Count) image files..." -ForegroundColor Cyan
    
    # Alternative sorting approach that doesn't use a script block
    $sortedFiles = @()
    $temp = @{}
    
    # Process each file
    foreach ($file in $imageFiles) {
        $sortKey = $file.Name
        # Extract numbers in parentheses and pad them
        if ($sortKey -match '\((\d+)\)') {
            $num = $matches[1]
            $paddedNum = $num.PadLeft(10, '0')
            $sortKey = $sortKey -replace "\($num\)", "($paddedNum)"
        }
        $temp[$sortKey] = $file
    }
    
    # Get sorted keys
    $sortedKeys = $temp.Keys | Sort-Object
    
    # Create sorted array
    foreach ($key in $sortedKeys) {
        $sortedFiles += $temp[$key]
    }
    
    # Replace original array with sorted one
    $imageFiles = $sortedFiles
    
    Write-Host "Sorted file list:" -ForegroundColor Cyan
    $imageFiles | ForEach-Object { Write-Host "  - $($_.Name)" }
} else {
    Write-Host "No files to sort." -ForegroundColor Yellow
}

Write-Host "Found $($imageFiles.Count) images to process" -ForegroundColor Cyan

# Prepare a list to store cropped image information
$croppedImagePaths = @()

# First, crop all images and save them
foreach ($imageFile in $imageFiles) {
    try {
        # Skip processing files that are already in the output folder
        if ($imageFile.DirectoryName -eq $outputFolder) {
            Write-Host "Skipping file in output folder: $($imageFile.Name)" -ForegroundColor Yellow
            continue
        }

        Write-Host "Processing: $($imageFile.Name)"
        
        # Load the image
        $image = [System.Drawing.Image]::FromFile($imageFile.FullName)
        
        # Create a new bitmap for the cropped section
        # Keep the full width, but only vertical pixels from 96 to 176 (height = 81 pixels)
        $croppedImage = New-Object System.Drawing.Bitmap $image.Width, 81
        
        # Create graphics object for the new bitmap
        $graphics = [System.Drawing.Graphics]::FromImage($croppedImage)
        
        # Draw the cropped section onto the new bitmap
        $graphics.DrawImage(
            $image, 
            # Destination rectangle (where to place the image in the new bitmap)
            [System.Drawing.Rectangle]::new(0, 0, $image.Width, 81),
            # Source rectangle (which part of the original to take)
            [System.Drawing.Rectangle]::new(0, 96, $image.Width, 81),
            [System.Drawing.GraphicsUnit]::Pixel
        )
        
        # Save the cropped image to the output folder
        $outputPath = Join-Path -Path $outputFolder -ChildPath $imageFile.Name
        $croppedImage.Save($outputPath, $image.RawFormat)
        
        # Add the cropped image path to our array for stitching
        $croppedImagePaths += $outputPath
        
        # Clean up resources
        $graphics.Dispose()
        $croppedImage.Dispose()
        $image.Dispose()
        
        Write-Host "Saved cropped image to: $outputPath" -ForegroundColor Green
    }
    catch {
        Write-Host "Error processing $($imageFile.Name): $_" -ForegroundColor Red
    }
}

# Alternative approach for stitching: use ImageMagick CLI if available
try {
    Write-Host "Attempting to stitch images using alternative method..." -ForegroundColor Cyan
    
    # Option 1: Try using the .NET approach with smaller batches if there are many images
    if ($croppedImagePaths.Count -gt 0) {
        # Define a function to stitch images in batches and then combine the batches
        function Join-ImageBatch {
            param (
                [string[]]$ImagePaths,
                [string]$OutputPath,
                [int]$BatchSize = 5
            )
            
            # If we have just a few images, stitch them directly
            if ($ImagePaths.Count -le $BatchSize) {
                Write-Host "Stitching batch of $($ImagePaths.Count) images..." -ForegroundColor Yellow
                
                # Check dimensions of all images in the batch
                $maxWidth = 0
                $totalHeight = 0
                $imageObjects = @()
                
                foreach ($path in $ImagePaths) {
                    $img = [System.Drawing.Image]::FromFile($path)
                    $imageObjects += @{
                        "Image" = $img
                        "Width" = $img.Width
                        "Height" = $img.Height
                    }
                    
                    if ($img.Width -gt $maxWidth) { $maxWidth = $img.Width }
                    $totalHeight += $img.Height
                }
                
                # Create the canvas for this batch
                try {
                    Write-Host "Creating batch image with dimensions: $maxWidth x $totalHeight" -ForegroundColor Yellow
                    
                    # Create a blank image
                    $batchImage = New-Object System.Drawing.Bitmap $maxWidth, $totalHeight
                    $graphics = [System.Drawing.Graphics]::FromImage($batchImage)
                    $graphics.Clear([System.Drawing.Color]::White)
                    
                    # Place each image
                    $currentY = 0
                    foreach ($img in $imageObjects) {
                        # Center horizontally
                        $x = [Math]::Floor(($maxWidth - $img.Width) / 2)
                        
                        # Draw image
                        $graphics.DrawImage(
                            $img.Image,
                            [System.Drawing.Rectangle]::new($x, $currentY, $img.Width, $img.Height)
                        )
                        
                        # Move down for next image
                        $currentY += $img.Height
                    }
                    
                    # Save the stitched batch
                    $batchImage.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
                    
                    # Clean up
                    $graphics.Dispose()
                    $batchImage.Dispose()
                    foreach ($img in $imageObjects) {
                        $img.Image.Dispose()
                    }
                    
                    return $OutputPath
                }
                catch {
                    Write-Host "Error stitching batch: $_" -ForegroundColor Red
                    throw
                }
            }
            else {
                # Too many images, split into batches
                Write-Host "Splitting $($ImagePaths.Count) images into batches of $BatchSize..." -ForegroundColor Yellow
                
                # Create temp directory for batch outputs if it doesn't exist
                $batchDir = Join-Path $outputFolder "temp_batches"
                if (-not (Test-Path -Path $batchDir)) {
                    New-Item -ItemType Directory -Path $batchDir | Out-Null
                }
                
                # Process each batch
                $batchOutputs = @()
                for ($i = 0; $i -lt $ImagePaths.Count; $i += $BatchSize) {
                    $end = [Math]::Min($i + $BatchSize - 1, $ImagePaths.Count - 1)
                    $batchPaths = $ImagePaths[$i..$end]
                    $batchOutputPath = Join-Path $batchDir "batch_$i.png"
                    
                    # Stitch this batch
                    $batchOutput = Join-ImageBatch -ImagePaths $batchPaths -OutputPath $batchOutputPath -BatchSize $BatchSize
                    $batchOutputs += $batchOutput
                }
                
                # Now stitch the batches together
                $finalOutput = Join-ImageBatch -ImagePaths $batchOutputs -OutputPath $OutputPath -BatchSize $BatchSize
                
                # Clean up batch files if desired
                # Remove-Item -Path $batchDir -Recurse -Force
                
                return $finalOutput
            }
        }
        
        # Use the function with a sensible batch size
        $stitchedPath = Join-Path -Path $outputFolder -ChildPath "stitched_character_set.png"
        Join-ImageBatch -ImagePaths $croppedImagePaths -OutputPath $stitchedPath -BatchSize 4
        
        if (Test-Path $stitchedPath) {
            Write-Host "Successfully created stitched image at: $stitchedPath" -ForegroundColor Green
        }
    }
}
catch {
    Write-Host "Error during stitching: $_" -ForegroundColor Red
    
    # Fallback: Try to use system commands if available
    try {
        Write-Host "Trying alternate system method for stitching..." -ForegroundColor Yellow
        
        # Create a text file listing all images
        $imageListPath = Join-Path $outputFolder "image_list.txt"
        $croppedImagePaths | ForEach-Object { $_ } | Out-File -FilePath $imageListPath
        
        # Try using PowerShell to call ImageMagick if installed
        $stitchedPath = Join-Path -Path $outputFolder -ChildPath "stitched_character_set.png"
        
        # Check if ImageMagick is installed
        $magickCommand = Get-Command magick -ErrorAction SilentlyContinue
        
        if ($magickCommand) {
            Write-Host "Using ImageMagick to stitch images..." -ForegroundColor Yellow
            & magick convert -append $croppedImagePaths $stitchedPath
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "Successfully stitched images using ImageMagick." -ForegroundColor Green
            }
            else {
                Write-Host "ImageMagick command failed." -ForegroundColor Red
            }
        }
        else {
            Write-Host "ImageMagick not found. Please install it or try another method." -ForegroundColor Red
            Write-Host "You can manually stitch the cropped images found in: $outputFolder" -ForegroundColor Yellow
        }
    }
    catch {
        Write-Host "All stitching methods failed. You can find the individual cropped images in: $outputFolder" -ForegroundColor Red
    }
}

Write-Host "Image processing complete!" -ForegroundColor Cyan