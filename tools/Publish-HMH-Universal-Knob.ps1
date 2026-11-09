# HackMyHome Universal Knob v0.1.3
# Pubblica l'intero progetto in un repository GitHub GIA' ESISTENTE e VUOTO.
# Non sovrascrive branch, commit, o file remoti.
param(
    [string]$Owner = 'MarcoFre',
    [string]$Repository = 'HMH-Universal-Knob'
)
$ErrorActionPreference = 'Stop'
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$RepoFullName = "$Owner/$Repository"
$Remote = "https://github.com/$RepoFullName.git"

foreach ($cmd in @('git','gh')) {
    if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) {
        throw "Manca '$cmd'. Installa Git e GitHub CLI, poi riprova."
    }
}

& gh auth status *> $null
if ($LASTEXITCODE -ne 0) { throw 'GitHub CLI non autenticato: esegui gh auth login.' }

foreach ($required in @('README.md','README_IT.md','esphome/hmh_universal_knob.yaml','esphome/hmh_universal_knob.h','ha/hmh_knob_scripts.yaml','docs/INSTALLAZIONE.md')) {
    if (-not (Test-Path (Join-Path $ProjectRoot $required))) { throw "File mancante: $required" }
}
if (Test-Path (Join-Path $ProjectRoot '.git')) {
    throw 'Cartella Git gia inizializzata: per sicurezza non modifico il repository locale.'
}

# Repository deve esistere e appartenere all'utente autenticato.
$found = & gh repo view $RepoFullName --json nameWithOwner --jq .nameWithOwner 2>$null
if ($LASTEXITCODE -ne 0 -or $found -ne $RepoFullName) {
    throw "Non trovo $RepoFullName oppure non hai accesso. Controlla l'account con gh auth status."
}

# Configura il Git credential helper di GitHub CLI e verifica che non esistano branch remoti.
& gh auth setup-git
if ($LASTEXITCODE -ne 0) { throw 'Configurazione autenticazione Git non riuscita.' }
$heads = & git ls-remote --heads $Remote
if ($LASTEXITCODE -ne 0) { throw 'Non riesco a controllare i branch remoti.' }
if ($heads) {
    throw "Il repository $RepoFullName contiene gia branch/commit: non pubblico per evitare sovrascritture."
}

Push-Location $ProjectRoot
try {
    & git init -b main
    if ($LASTEXITCODE -ne 0) { throw 'git init non riuscito.' }
    & git add -A
    if ($LASTEXITCODE -ne 0) { throw 'git add non riuscito.' }

    $staged = & git diff --cached --name-only
    if (@($staged).Count -lt 20) {
        throw "File insufficienti (@($staged).Count): verifica di avere estratto l'intero ZIP."
    }
    if ($staged -match '(^|/)(secrets\.yaml|\.env|id_rsa|.*\.pem)$') {
        throw 'Rilevati possibili segreti fra i file da pubblicare. Pubblicazione annullata.'
    }

    Write-Host "`nRepository di destinazione: https://github.com/$RepoFullName" -ForegroundColor Cyan
    Write-Host 'Questi sono i file che stai per pubblicare:' -ForegroundColor Cyan
    & git status --short
    $confirm = Read-Host "`nScrivi PUBBLICA per caricare i file nel repository"
    if ($confirm -cne 'PUBBLICA') { throw 'Operazione annullata.' }

    & git commit -m 'Initial release: HMH Universal Knob v0.1.3'
    if ($LASTEXITCODE -ne 0) {
        throw 'Commit non riuscito. Imposta git config --global user.name e user.email, poi riprova.'
    }
    & git remote add origin $Remote
    if ($LASTEXITCODE -ne 0) { throw 'Impossibile configurare origin.' }
    # Secondo controllo remoto immediatamente prima del push.
    $headsNow = & git ls-remote --heads $Remote
    if ($LASTEXITCODE -ne 0 -or $headsNow) { throw 'Il repository remoto non e piu vuoto: pubblicazione interrotta.' }
    & git push -u origin main
    if ($LASTEXITCODE -ne 0) { throw 'Push GitHub non riuscito: verifica rete e permessi.' }
    Write-Host "`nPUBBLICAZIONE COMPLETATA: https://github.com/$RepoFullName" -ForegroundColor Green
} finally {
    Pop-Location
}
