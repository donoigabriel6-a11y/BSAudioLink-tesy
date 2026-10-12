Param(
    [Parameter(Mandatory=$false)]
    [Switch]$clean,
    [Parameter(Mandatory=$false)]
    [Switch]$release
)

# Restore the binary Unity asset from its text-safe repository representation.
$assetBase64 = Join-Path $PSScriptRoot "../assets/Bundle.base64"
$assetBinary = Join-Path $PSScriptRoot "../assets/Bundle"
if ((Test-Path $assetBase64) -and (-not (Test-Path $assetBinary))) {
    $encoded = (Get-Content -Path $assetBase64 -Raw).Trim()
    [System.IO.File]::WriteAllBytes($assetBinary, [System.Convert]::FromBase64String($encoded))
}
if ($clean.IsPresent -and (Test-Path -Path "build")) {
    Remove-Item -Path "build" -Recurse -Force
}
if (($clean.IsPresent) -or (-not (Test-Path -Path "build"))) {
    New-Item -Path "build" -ItemType Directory | Out-Null
}
& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build ./build
exit $LASTEXITCODE
