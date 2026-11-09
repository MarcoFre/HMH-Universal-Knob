# HMH Universal Knob v0.1.3 -- minimal Windows update.
# Only firmware source files changed since v0.1.2 (no HA script/PNG changes).
# Pass -HaHost explicitly; no private IP addresses are stored in this public tool.
param(
    [Parameter(Mandatory=$true)][string]$HaHost,
    [int]$SshPort = 22,
    [string]$SshUser = 'root',
    [switch]$IncludeIcons
)
$ErrorActionPreference = 'Stop'
$Project = Split-Path -Parent $PSScriptRoot
$Root = Join-Path $Project 'esphome'
$Remote = "${SshUser}@${HaHost}"
$Yaml = Join-Path $Root 'hmh_universal_knob.yaml'
$Header = Join-Path $Root 'hmh_universal_knob.h'
if (!(Test-Path -LiteralPath $Yaml) -or !(Test-Path -LiteralPath $Header)) {
    throw 'Mancano i file ESPHome nel pacchetto.'
}
Write-Host 'Carico la v0.1.3 su ESPHome (solo YAML e C++)...' -ForegroundColor Cyan
& scp -P $SshPort $Yaml $Header "${Remote}:/config/esphome/"
if ($LASTEXITCODE -ne 0) { throw 'Trasferimento firmware fallito.' }
if ($IncludeIcons) {
    & scp -r -P $SshPort (Join-Path $Root 'icons') "${Remote}:/config/esphome/"
    if ($LASTEXITCODE -ne 0) { throw 'Trasferimento icone fallito.' }
}
Write-Host 'Sorgenti caricati. Nessun flash, nessun cambiamento agli script HA.' -ForegroundColor Green
Write-Host 'Ora: ESPHome Device Builder > HMH Universal Knob > Validate > Install.' -ForegroundColor Cyan
