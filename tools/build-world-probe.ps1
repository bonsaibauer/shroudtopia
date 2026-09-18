$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$dev = Join-Path $vs 'Common7\Tools\VsDevCmd.bat'
$out = Join-Path $root 'build\world-probe'
New-Item -ItemType Directory -Force -Path $out | Out-Null
Push-Location $out
try {
    cmd /d /c "`"$dev`" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++20 /EHsc /W4 /I`"$root\api`" `"$PSScriptRoot\world-cursor-sample.cpp`" /Fe:world-cursor-sample.exe"
    if ($LASTEXITCODE -ne 0) { throw 'World probe build failed.' }
    cmd /d /c "`"$dev`" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++20 /EHsc /W4 /I`"$root\api`" `"$PSScriptRoot\world-layout-smoke.cpp`" /Fe:world-layout-smoke.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Native layout test build failed.' }
    & .\world-layout-smoke.exe
    if ($LASTEXITCODE -ne 0) { throw 'Native layout tests failed.' }
    cmd /d /c "`"$dev`" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++20 /EHsc /W4 /I`"$root\api`" `"$PSScriptRoot\world-terrain-engine-probe.cpp`" /Fe:world-terrain-engine-probe.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Native terrain probe build failed.' }
    cmd /d /c "`"$dev`" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++20 /EHsc /W4 /I`"$root\api`" `"$PSScriptRoot\building-menu-layout-smoke.cpp`" /Fe:building-menu-layout-smoke.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Building menu layout test build failed.' }
    & .\building-menu-layout-smoke.exe
    if ($LASTEXITCODE -ne 0) { throw 'Building menu layout tests failed.' }
    cmd /d /c "`"$dev`" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++20 /EHsc /W4 /I`"$root\api`" `"$PSScriptRoot\world-diagnostic-smoke.cpp`" /Fe:world-diagnostic-smoke.exe"
    if ($LASTEXITCODE -ne 0) { throw 'World diagnostic build failed.' }
    & .\world-diagnostic-smoke.exe
    if ($LASTEXITCODE -ne 0) { throw 'World diagnostic tests failed.' }
} finally { Pop-Location }
