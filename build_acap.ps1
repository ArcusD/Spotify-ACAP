# ACAP Build Script
# Builds the Spotify Connect ACAP for every chipset family used in Axis
# network speakers and puts each package in builds/<chipset>/.
#
# One build serves several chipsets: the package only depends on the CPU
# architecture and the SDK version. The SDK version follows the newest
# AXIS OS the speakers can run: the i.MX 6ULL models stop at AXIS OS 11.11
# LTS, so they get a build from ACAP Native SDK 1.15 (AXIS OS 11.11);
# everything else gets SDK 12.1 (AXIS OS 12.1 and later).

$Version = "1.0.0"
$VersionTag = $Version.Replace('.', '_')

$Builds = @(
    @{ Arch = "armv7hf"; Sdk = "12.1.0"; Folders = @("imx6sx") },
    @{ Arch = "armv7hf"; Sdk = "1.15.4"; Folders = @("imx6ull") },
    @{ Arch = "aarch64"; Sdk = "12.1.0"; Folders = @("imx8m-nano", "imx8m-mini") }
)

foreach ($Build in $Builds) {
    $Arch = $Build.Arch
    $Sdk = $Build.Sdk
    $Image = "spotify-connect-builder-$Arch-sdk$Sdk"
    Write-Host "--- Building for $Arch (SDK $Sdk) ---" -ForegroundColor Cyan

    docker build --build-arg ARCH=$Arch --build-arg SDK_VERSION=$Sdk -t $Image .
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed for $Arch (SDK $Sdk)"
        continue
    }

    # acap-build names the package after the manifest version and architecture.
    $EapFile = "Spotify_Connect_$($VersionTag)_$($Arch).eap"
    $ContainerId = docker create $Image
    foreach ($Folder in $Build.Folders) {
        New-Item -ItemType Directory -Force "./builds/$Folder" | Out-Null
        docker cp "$($ContainerId):/opt/app/$EapFile" "./builds/$Folder/$EapFile"
        Write-Host "Copied to builds/$Folder/$EapFile" -ForegroundColor Green
    }
    docker rm $ContainerId | Out-Null
}

Write-Host "Build process complete!" -ForegroundColor Green
