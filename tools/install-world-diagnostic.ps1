#requires -Version 7.0
param(
    [string]$GameDirectory = 'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded',
    [string]$RestoreBackup
)
$ErrorActionPreference = 'Stop'
if (Get-Process enshrouded,enshrouded_server -ErrorAction SilentlyContinue) { throw 'Close game/server before deployment.' }
$game = (Resolve-Path -LiteralPath $GameDirectory).Path
$exe = Join-Path $game 'enshrouded.exe'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne 'AF2F5A1227911D8AA06B3908D6BD0211838211CAE14EA91099CB57D0DF990781') { throw 'Unsupported game executable.' }
$names = @('winmm.dll','shroudtopia.json')
$backupRoot = Join-Path $game '.shroudtopia-world-diagnostic-backups'
function Restore([string]$Backup) {
    $resolved = (Resolve-Path -LiteralPath $Backup).Path
    if (-not $resolved.StartsWith($backupRoot+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Backup outside diagnostic backup directory.' }
    $records = @(Get-Content -LiteralPath (Join-Path $resolved 'files.json') -Raw | ConvertFrom-Json)
    if ($records.Count -ne 2) { throw 'Incomplete inventory.' }
    foreach ($name in $names) {
        $record = @($records | Where-Object name -CEQ $name)
        if ($record.Count -ne 1 -or (Get-FileHash -LiteralPath (Join-Path $resolved $name)).Hash -ne $record[0].sha256) { throw 'Invalid backup inventory or hash.' }
    }
    if (Get-Process enshrouded,enshrouded_server -ErrorAction SilentlyContinue) { throw 'Game started; close it before restoring.' }
    foreach ($name in $names) { Copy-Item -LiteralPath (Join-Path $resolved $name) -Destination (Join-Path $game $name) -Force }
    Write-Output "Restored loader/settings from $resolved; backup retained."
}
if ($RestoreBackup) { Restore $RestoreBackup; return }
$root = Split-Path -Parent $PSScriptRoot
$binary = Join-Path $root 'build\x64\winmm.dll'
if (-not (Test-Path -LiteralPath $binary -PathType Leaf)) { throw 'Build loader first.' }
$config = Get-Content -LiteralPath (Join-Path $game 'shroudtopia.json') -Raw | ConvertFrom-Json -AsHashtable
$config.worldDiagnostic = $true
$backup = Join-Path $backupRoot ([DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffffff'))
New-Item -ItemType Directory -Path $backup | Out-Null
$records = foreach ($name in $names) {
    $source = Join-Path $game $name
    Copy-Item -LiteralPath $source -Destination (Join-Path $backup $name)
    $hash = (Get-FileHash -LiteralPath $source).Hash
    if ((Get-FileHash -LiteralPath (Join-Path $backup $name)).Hash -ne $hash) { throw 'Backup verification failed.' }
    @{name=$name; sha256=$hash}
}
$records | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $backup 'files.json') -Encoding utf8
# Prepare configuration before touching either installed file.
$staged = Join-Path $backup 'diagnostic-settings.json'
$config | ConvertTo-Json -Depth 100 | Set-Content -LiteralPath $staged -Encoding utf8
if (Get-Process enshrouded,enshrouded_server -ErrorAction SilentlyContinue) { throw 'Game started; installation cancelled.' }
try {
    Copy-Item -LiteralPath $binary -Destination (Join-Path $game 'winmm.dll') -Force
    Copy-Item -LiteralPath $staged -Destination (Join-Path $game 'shroudtopia.json') -Force
    if ((Get-FileHash -LiteralPath $binary).Hash -ne (Get-FileHash -LiteralPath (Join-Path $game 'winmm.dll')).Hash) { throw 'Installed loader hash mismatch.' }
    Write-Output "Installed bounded world diagnostic. Backup: $backup. Assets, mods and savegames unchanged."
} catch {
    if (Get-Process enshrouded,enshrouded_server -ErrorAction SilentlyContinue) { Write-Warning "Close the game, then restore $backup" }
    else { Restore $backup }
    throw
}
