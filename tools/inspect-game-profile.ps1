param([string]$GameDirectory = 'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$game = (Resolve-Path -LiteralPath $GameDirectory).Path
$modules = @()
foreach ($process in @(Get-Process enshrouded -ErrorAction SilentlyContinue)) {
    foreach ($module in $process.Modules) {
        if ($module.FileName.StartsWith($game + '\', [StringComparison]::OrdinalIgnoreCase)) {
            $modules += [ordered]@{ processId=$process.Id; name=$module.ModuleName; path=$module.FileName; base=('0x{0:X}' -f $module.BaseAddress.ToInt64()); size=$module.ModuleMemorySize; diskSha256=(Get-FileHash -LiteralPath $module.FileName -Algorithm SHA256).Hash }
        }
    }
}
$files = @()
foreach ($name in @('enshrouded.exe','shroudtopia.dll','shroudtopia-assets.dll')) {
    foreach ($path in @((Join-Path $game $name),(Join-Path "$root\build\x64" $name))) {
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            $files += [ordered]@{ path=$path; sha256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash }
        }
    }
}
$directory = Join-Path $root 'build\game-profiles'
New-Item -ItemType Directory -Path $directory -Force | Out-Null
$report = Join-Path $directory ('profile-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffffff') + '.json')
[ordered]@{ capturedUtc=[DateTime]::UtcNow.ToString('o'); note='Disk hashes identify files, not modified in-memory code. This tool does not attach a debugger or modify the game.'; files=$files; modules=$modules } | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $report -Encoding utf8
Write-Output $report
