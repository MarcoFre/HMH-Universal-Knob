# Installazione di HMH Universal Knob v0.1.3

Questa guida riguarda esclusivamente **OpenNextion ONX2424G013** e **Home Assistant con ESPHome Device Builder**. È consigliato disporre già di un'integrazione ESPHome funzionante e di accesso ai file `/config` di Home Assistant.

## 1. Prima di cominciare

- **Fai un backup** del firmware e dei file dell'eventuale altro progetto già installato sul Knob (es. Power Control): il nuovo flash lo sostituisce.
- Controlla che il dispositivo sia il modello **ONX2424G013**, display GC9A01A 240×240, ESP32-S3 con 16 MB Flash e 8 MB PSRAM.
- Tieni pronti ESPHome Device Builder, Home Assistant aggiornato, accesso ai file e un cavo USB dati per il primo flash.
- Non pubblicare o condividere `secrets.yaml`, password Wi-Fi, chiavi API o token.

## 2. Scarica i file

Su GitHub scegli **Code → Download ZIP** e apri la cartella `HMH-Universal-Knob` di questo repository. In alternativa scarica i singoli file conservando la gerarchia delle cartelle.

I file da usare sono:

- `esphome/hmh_universal_knob.yaml`
- `esphome/hmh_universal_knob.h`
- `esphome/icons/` con le 9 immagini `.png`
- `ha/hmh_knob_scripts.yaml`

## 3. Crea l'etichetta Home Assistant

In **Impostazioni → Etichette** crea l'etichetta **`HMH_Device`** (attenzione a maiuscole e underscore). Applica l'etichetta a una o più **entità** (`light.*`, `switch.*`, `cover.*` ecc.) oppure a un **dispositivo** fisico.

Se etichetti un dispositivo che espone più entità, il catalogo seleziona il comando principale con priorità proprie e associa eventuali sensori (W, V, A, kWh, ecc.) appartenenti allo stesso `device_id`. Per privilegiare un comando specifico assegna `HMH_Device` anche a quell'entità. Al momento la pagina riporta un controllo principale per dispositivo, non tutti i canali contemporaneamente.

## 4. Installa i tre script Home Assistant

Apri `ha/hmh_knob_scripts.yaml`. Contiene **tre definizioni da aggiungere** al tuo `/config/scripts.yaml`:

- `hmh_knob_catalog` — crea il catalogo dinamico;
- `hmh_knob_state` — legge lo stato e le funzionalità della pagina;
- `hmh_knob_command` — valida e inoltra i comandi.

**Se usi** `script: !include scripts.yaml` in `configuration.yaml`, incolla il contenuto del file del progetto **in fondo al tuo `scripts.yaml`**, senza un secondo livello `script:` e senza cancellare gli script preesistenti. Se hai già questi tre ID, sostituisci le definizioni, non duplicarle. Se hai una gestione differente dei tuoi script, integra le tre definizioni secondo l'organizzazione già esistente.

In Home Assistant usa **Strumenti per sviluppatori → YAML → Ricarica script** (o riavvia HA). Controlla che esistano `script.hmh_knob_catalog`, `script.hmh_knob_state` e `script.hmh_knob_command`.

## 5. Copia il firmware in ESPHome

Copia così i file nel tuo sistema Home Assistant:

```text
/config/esphome/hmh_universal_knob.yaml
/config/esphome/hmh_universal_knob.h
/config/esphome/icons/light.png
/config/esphome/icons/switch.png
/config/esphome/icons/climate.png
/config/esphome/icons/cover.png
/config/esphome/icons/fan.png
/config/esphome/icons/media.png
/config/esphome/icons/scene.png
/config/esphome/icons/sensor.png
/config/esphome/icons/generic.png
```

**Attenzione:** la cartella `icons/` è un requisito per la compilazione, non una risorsa opzionale.

## 6. Prepara Wi-Fi e API cifrata

Nel tuo file **`/config/esphome/secrets.yaml`** crea o riutilizza:

```yaml
wifi_ssid: "LA_TUA_RETE_WIFI"
wifi_password: "LA_TUA_PASSWORD_WIFI"
api_encryption_key: "CHIAVE_BASE64_API_ESPHOME"
```

Usa una chiave di cifratura API ESPHome valida, generata in modo sicuro dal tuo ambiente ESPHome; la voce sopra è solo un segnaposto. **Non caricare il file reale su GitHub.** Il firmware legge le tre voci tramite `!secret`.

## 7. Convalida e installa

1. In **ESPHome Device Builder** apri `hmh_universal_knob.yaml`.
2. Esegui **Validate** e risolvi eventuali errori prima di procedere.
3. Esegui **Install**. Per il primo caricamento del firmware scegli USB; OTA solo se è già configurato e raggiungibile sul dispositivo corretto.
4. Aggiungi/riconosci il dispositivo in Home Assistant tramite l'integrazione ESPHome.
5. Nelle opzioni del dispositivo ESPHome **autorizza l'esecuzione di azioni Home Assistant**. Se l'opzione non è attiva, il Knob può non ricevere catalogo/stati o non inviare comandi.

Il firmware non contiene un token REST né richiede MQTT.

## 8. Primo test

1. Applica `HMH_Device` a un **singolo interruttore non critico** e prova la navigazione.
2. Aspetta fino a **30 s** per il refresh del catalogo; lo stato pagina è richiesto ogni **3 s**.
3. Ruota la ghiera per passare tra i dispositivi. Premi per entrare nei controlli, ruota per selezionare e premi per confermare il comando.
4. Controlla che il comando sia eseguito dalla corretta entità Home Assistant e che non partano comandi durante il solo scorrimento.
5. Se qualcosa non funziona, vedi [FAQ](FAQ.md).

## 9. Aggiornamento dalla v0.1.2 (Windows)

Lo script PowerShell `tools/Update-HMH-Knob.ps1` invia via SCP **solo YAML e header**, senza fare il flash. Se mancano le icone usa `-IncludeIcons`.

Esempio (sostituisci l'host con il tuo server HA, se SSH/SCP è configurato):

```powershell
.\tools\Update-HMH-Knob.ps1 -HaHost "IP_O_HOST_DEL_TUO_HA" -SshUser "root"
# Nuova installazione con icone:
.\tools\Update-HMH-Knob.ps1 -HaHost "IP_O_HOST_DEL_TUO_HA" -IncludeIcons
```

Esegui in seguito **ESPHome → Validate → Install**. Da v0.1.2 a v0.1.3 i tre script HA e le icone non cambiano.

## 10. Limitazioni e attenzione

- Massimo **24 pagine** in questa versione.
- `scene` e `script` vengono eseguiti solo se l'entità è esplicitamente etichettata.
- Dati elettrici disponibili solo quando Home Assistant espone attributi o sensori associati allo stesso dispositivo.
- Non utilizzare il Knob come unico comando di carichi essenziali o dispositivi con rischi di sicurezza; è un progetto sperimentale.
- I test offline sono inclusi, ma **non equivalgono a certificazione o collaudo del firmware sul tuo impianto**.
