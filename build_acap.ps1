# ACAP Build Script
# This script builds the Spotify Connect ACAP for both armv7hf and aarch64
# and moves the results to the builds/ directory.

$Version = "1.0.0"
$Archs = @("armv7hf", "aarch64")

foreach ($Arch in $Archs) {
    Write-Host "--- Building for $Arch ---" -ForegroundColor Cyan
    
    # Build the docker image
    docker build --build-arg ARCH=$Arch -t spotify-connect-builder-$Arch .
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed for $Arch"
        continue
    }

    # Create a temporary container to extract the .eap file
    $ContainerId = docker create spotify-connect-builder-$Arch
    
    # Identify the .eap file in the container
    $EapFile = "spotifyconnect_$($Version)_$($Arch).eap"
    
    # Copy the file out
    Write-Host "Extracting $EapFile..."
    docker cp "$($ContainerId):/opt/app/$EapFile" "./builds/Spotify_Connect_$($Version)_$($Arch).eap"
    
    # Cleanup
    docker rm $ContainerId
    Write-Host "Successfully built and moved to builds/Spotify_Connect_$($Version)_$($Arch).eap" -ForegroundColor Green
}

Write-Host "Build process complete!" -ForegroundColor Green
