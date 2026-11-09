# Pubblicazione sul repository GitHub gia creato

Repository: https://github.com/MarcoFre/HMH-Universal-Knob

Il repository deve essere **vuoto**, senza README iniziale e senza commit. Lo script **non sovrascrive** repository gia popolati.

1. Installa [Git](https://git-scm.com/download/win) e [GitHub CLI](https://cli.github.com/) su Windows, se non sono gia installati.
2. Scarica lo ZIP aggiornato e **estrai l’intera cartella** `HMH-Universal-Knob` (non eseguire lo script dentro lo ZIP).
3. Apri PowerShell nella cartella estratta `HMH-Universal-Knob`.
4. Se necessario, autentica la CLI con `gh auth login`.
5. Esegui `./tools/Publish-HMH-Universal-Knob.ps1` (su PowerShell Windows puoi usare `.\tools\Publish-HMH-Universal-Knob.ps1`).
6. Controlla l’elenco dei file e digita `PUBBLICA`. Lo script invia anche le icone PNG.
7. Verifica online che siano presenti README, `esphome/`, `ha/`, `docs/`, `tests/` e `tools/`.

> Non pubblicare mai `secrets.yaml` o credenziali Home Assistant. Il repository include solo `esphome/secrets.yaml.example`.

Se il repository non e vuoto, lo script si interrompe: non tentare un push forzato.
