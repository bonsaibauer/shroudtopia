param(
    [string]$BuildNumber = $(if ($env:SHROUDTOPIA_BUILD_NUMBER) { $env:SHROUDTOPIA_BUILD_NUMBER } else { 'dev' }),
    [string]$RefName,
    [ValidateSet('branch', 'tag')]
    [string]$RefType = 'branch'
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$version = (Get-Content -LiteralPath (Join-Path $root 'VERSION') -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$') { throw "VERSION must use MAJOR.MINOR.PATCH: $version" }

if ($RefName) {
    $expected = if ($RefType -eq 'tag') { "v$version" } else { $version }
    if ($RefName -ne $expected) { throw "Version mismatch: $RefType '$RefName', expected '$expected'." }
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer was not found.' }
$visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) { throw 'Visual Studio C++ build tools were not found.' }
$msbuild = Join-Path $visualStudio 'MSBuild\Current\Bin\MSBuild.exe'
$vsDevCmd = Join-Path $visualStudio 'Common7\Tools\VsDevCmd.bat'

$output = Join-Path $root 'build\x64'
New-Item -ItemType Directory -Force -Path $output | Out-Null
& $msbuild (Join-Path $root 'shroudtopia.sln') /m /t:Build /p:Configuration=Release /p:Platform=x64 /p:OutDir="$output\" /p:ShroudtopiaVersion=$version /p:ShroudtopiaBuildNumber=$BuildNumber
if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE." }

$engineRoot = Join-Path $root 'src\assets'
& cargo test --manifest-path (Join-Path $engineRoot 'Cargo.toml') --release -p shroudtopia --lib
if ($LASTEXITCODE -ne 0) { throw 'Shroudtopia asset engine tests failed.' }
& cargo build --manifest-path (Join-Path $engineRoot 'Cargo.toml') --release -p shroudtopia
if ($LASTEXITCODE -ne 0) { throw "Shroudtopia asset engine build failed with exit code $LASTEXITCODE." }
$engineBinary = Join-Path $engineRoot 'target\release\shroudtopia.dll'
if (-not (Test-Path -LiteralPath $engineBinary)) { throw "Shroudtopia asset engine binary missing: $engineBinary" }
Copy-Item -LiteralPath $engineBinary -Destination (Join-Path $output 'shroudtopia.dll') -Force

# ShroudEdit is maintained as its own repository and pinned here as a submodule.
# Build and test that exact commit, then place its DLL beside the bundled mod
# binaries so the existing validation and packaging stages treat it uniformly.
$shroudEditSource = Join-Path $root 'mods\shroudedit'
$shroudEditProject = Join-Path $shroudEditSource 'CMakeLists.txt'
if (-not (Test-Path -LiteralPath $shroudEditProject)) {
    throw 'ShroudEdit submodule is missing. Run: git submodule update --init --recursive'
}
$shroudEditBuild = Join-Path $root 'build\shroudedit'
& cmake -S $shroudEditSource -B $shroudEditBuild -A x64 "-DSHROUDTOPIA_API_DIR=$root"
if ($LASTEXITCODE -ne 0) { throw "ShroudEdit configure failed with exit code $LASTEXITCODE." }
& cmake --build $shroudEditBuild --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "ShroudEdit build failed with exit code $LASTEXITCODE." }
& ctest --test-dir $shroudEditBuild -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "ShroudEdit tests failed with exit code $LASTEXITCODE." }
$shroudEditBinary = Join-Path $shroudEditBuild 'Release\mod.shroudedit.dll'
if (-not (Test-Path -LiteralPath $shroudEditBinary)) { throw "ShroudEdit binary missing: $shroudEditBinary" }
Copy-Item -LiteralPath $shroudEditBinary -Destination (Join-Path $output 'mod.shroudedit.dll') -Force

$smokeSource = Join-Path $root 'tools\platform-api-smoke.cpp'
$smokeExecutable = Join-Path $output 'platform-api-smoke.exe'
$nativeModSmokeSource = Join-Path $root 'tools\native-mod-smoke.cpp'
$nativeModSmokeExecutable = Join-Path $output 'native-mod-smoke.exe'
$docsExampleSource = Join-Path $root 'docs\api\examples\reference.cpp'
$runtimePatchSmokeSource = Join-Path $root 'tools\runtime-patches-smoke.cpp'
$runtimePatchSmokeExecutable = Join-Path $output 'runtime-patches-smoke.exe'
$compileCommand = Join-Path $output 'compile-smoke.cmd'
$includeDirectory = Join-Path $root 'api'
$loaderDirectory = Join-Path $root 'src\loader'
$jsonDirectory = Join-Path $root 'third-party\nlohmann-json\include'
Set-Content -LiteralPath $compileCommand -Encoding Ascii -Value @"
@echo off
call "$vsDevCmd" -arch=x64 -host_arch=x64 >nul 2>&1
if errorlevel 1 exit /b %errorlevel%
cl.exe /nologo /std:c++20 /EHsc /W4 /I"$includeDirectory" "$smokeSource" /Fo:"$output\platform-api-smoke.obj" /Fe:"$smokeExecutable"
if errorlevel 1 exit /b %errorlevel%
cl.exe /nologo /std:c++20 /EHsc /W4 /I"$includeDirectory" "$nativeModSmokeSource" /Fo:"$output\native-mod-smoke.obj" /Fe:"$nativeModSmokeExecutable"
if errorlevel 1 exit /b %errorlevel%
cl.exe /nologo /std:c++20 /EHsc /W4 /c /I"$includeDirectory" "$docsExampleSource" /Fo:"$output\api-docs-examples.obj"
if errorlevel 1 exit /b %errorlevel%
cl.exe /nologo /std:c++20 /EHsc /W4 /Y- /I"$includeDirectory" /I"$loaderDirectory" /I"$jsonDirectory" "$runtimePatchSmokeSource" "$loaderDirectory\runtime_patches.cpp" /Fo:"$output\\" /Fe:"$runtimePatchSmokeExecutable"
if errorlevel 1 exit /b %errorlevel%
cl.exe /nologo /std:c++20 /EHsc /W4 /Y- /I"$includeDirectory" /I"$loaderDirectory" /I"$jsonDirectory" "$root\tools\log-reader-smoke.cpp" "$loaderDirectory\utils.cpp" "$loaderDirectory\ui_text.cpp" /Fo:"$output\\" /Fe:"$output\log-reader-smoke.exe" /link user32.lib gdi32.lib
exit /b %errorlevel%
"@

try {
    & $compileCommand
    if ($LASTEXITCODE -ne 0) { throw "Smoke-test compilation failed with exit code $LASTEXITCODE." }
}
finally {
    Remove-Item -LiteralPath $compileCommand -ErrorAction SilentlyContinue
}

function Copy-BundledMods([string]$Destination) {
    Get-ChildItem -LiteralPath (Join-Path $root 'mods') -Directory | ForEach-Object {
        $manifestPath = Join-Path $_.FullName 'mod.json'
        if (-not (Test-Path -LiteralPath $manifestPath)) { return }
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        $binary = [string]$manifest.shroudtopia.binary
        if (-not $manifest.id -or -not $binary) { throw "Invalid bundled mod manifest: $manifestPath" }
        if ([string]$manifest.shroudtopia.target -notin @('client', 'server', 'both')) {
            throw "Invalid shroudtopia.target in bundled mod manifest: $manifestPath"
        }
        $sourceBinary = Join-Path $output $binary
        if (-not (Test-Path -LiteralPath $sourceBinary)) { throw "Bundled mod binary missing: $sourceBinary" }
        $target = Join-Path $Destination ([string]$manifest.id)
        New-Item -ItemType Directory -Force -Path $target | Out-Null
        Copy-Item -LiteralPath $sourceBinary,$manifestPath -Destination $target -Force
        foreach ($documentation in @('README.md','LICENSE','VALIDATED-BUILD.md')) {
            $documentationPath = Join-Path $_.FullName $documentation
            if (Test-Path -LiteralPath $documentationPath) {
                Copy-Item -LiteralPath $documentationPath -Destination $target -Force
            }
        }
    }
}

$runtimeMods = Join-Path $output 'mods'
Remove-Item -LiteralPath $runtimeMods -Recurse -Force -ErrorAction SilentlyContinue
Copy-BundledMods $runtimeMods
Push-Location $output
try {
    & $runtimePatchSmokeExecutable
    if ($LASTEXITCODE -ne 0) { throw "Runtime patch smoke test failed with exit code $LASTEXITCODE." }
    & (Join-Path $output 'log-reader-smoke.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Log reader / native UI smoke test failed.' }
    Get-ChildItem -LiteralPath (Join-Path $root 'mods') -Directory | ForEach-Object {
        $manifestPath = Join-Path $_.FullName 'mod.json'
        if (-not (Test-Path -LiteralPath $manifestPath)) { return }
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        # ShroudEdit has its own CMake lifecycle/command test suite above. The
        # generic harness below models only the built-in patch and utility mods.
        if ([string]$manifest.id -eq 'mod.shroudedit') { return }
        & $nativeModSmokeExecutable (Join-Path $output ([string]$manifest.shroudtopia.binary)) ([string]$manifest.id)
        if ($LASTEXITCODE -ne 0) { throw "Native mod smoke test failed for $($manifest.id)." }
        if ([string]$manifest.id -ne 'mod.commands') {
            & $nativeModSmokeExecutable (Join-Path $output ([string]$manifest.shroudtopia.binary)) ([string]$manifest.id) missing
            if ($LASTEXITCODE -ne 0) { throw "Missing-signature smoke test failed for $($manifest.id)." }
        }
    }
    & $smokeExecutable
    if ($LASTEXITCODE -ne 0) { throw "Smoke test failed with exit code $LASTEXITCODE." }

    foreach ($harnessName in @('enshrouded.exe', 'enshrouded_server.exe')) {
        $targetHarness = Join-Path $output $harnessName
        Copy-Item -LiteralPath $smokeExecutable -Destination $targetHarness -Force
        try {
            & $targetHarness
            if ($LASTEXITCODE -ne 0) { throw "$harnessName target smoke test failed with exit code $LASTEXITCODE." }
        }
        finally {
            Remove-Item -LiteralPath $targetHarness -Force -ErrorAction SilentlyContinue
        }
    }
}
finally {
    Pop-Location
    Remove-Item -LiteralPath $runtimeMods -Recurse -Force -ErrorAction SilentlyContinue
}

$package = Join-Path $root 'build\package'
$archive = Join-Path $root "build\shroudtopia-$version-$BuildNumber.zip"
Remove-Item -LiteralPath $package -Recurse -Force -ErrorAction SilentlyContinue
# Keep build/ deterministic: one current package instead of an accumulating archive history.
Get-ChildItem -LiteralPath (Join-Path $root 'build') -Filter "shroudtopia-$version-*.zip" -File -ErrorAction SilentlyContinue |
    Remove-Item -Force
New-Item -ItemType Directory -Force -Path (Join-Path $package 'game'),(Join-Path $package 'licenses') | Out-Null
Copy-Item -LiteralPath (Join-Path $output 'winmm.dll'),(Join-Path $output 'shroudtopia.dll') -Destination (Join-Path $package 'game')
Copy-BundledMods (Join-Path $package 'game\mods')
Copy-Item -LiteralPath (Join-Path $root 'LICENSE') -Destination (Join-Path $package 'licenses\Shroudtopia.txt')
Copy-Item -LiteralPath (Join-Path $root 'src\assets\NOTICE.md') -Destination (Join-Path $package 'licenses\NOTICE.md')
Copy-Item -LiteralPath (Join-Path $root 'third-party\kfc-parser\LICENSE') -Destination (Join-Path $package 'licenses\kfc-parser.txt')
Compress-Archive -Path "$package\*" -DestinationPath $archive
Remove-Item -LiteralPath $package -Recurse -Force

Write-Host "Shroudtopia $version-$BuildNumber built, tested and packaged: $archive"
