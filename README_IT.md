# HackMyHome Universal Knob v0.1.3

Firmware ESPHome/LVGL per **OpenNextion ONX2424G013** (ESP32-S3, GC9A01A 240×240, encoder rotativo). Basato sulla mappa hardware del file **Power Control 0.27.3** allegato, ma distribuito come progetto indipendente: **non modifica** Power Control, non contiene logiche di distacco automatico, non include entità predefinite.

## File

- `esphome/hmh_universal_knob.yaml` — configurazione ESPHome completa.
- `esphome/icons/*.png` — nove icone originali 48x48 trasparenti (luce, switch, clima, cover, fan, media, scene, sensori, generico).
- `esphome/hmh_universal_knob.h` — modello dei dispositivi, parsing JSON, navigazione e renderer LVGL.
- `ha/hmh_knob_scripts.yaml` — tre script Home Assistant (catalogo, stato, comando).
- `tests/test_bridge_templates.py` — test Jinja offline con dati simulati.

## Installazione

1. **Fai una copia del firmware Power Control attuale** e salva i tuoi segreti ESPHome. Il flash di questo firmware sullo stesso ONX sostituisce il firmware esistente; non cancella/modifica però la configurazione salvata sul server Home Assistant.
2. In **Impostazioni → Etichette** di Home Assistant crea l'etichetta `HMH_Device`.
3. Assegna questa etichetta a un'**entità** specifica (`light.xxx`, `switch.xxx`, `climate.xxx`, `cover.xxx`...) oppure a un **dispositivo fisico** in Home Assistant. Se etichetti il dispositivo fisico, il catalogo crea **una sola pagina per device_id**: sceglie automaticamente un controllo principale e associa le misure secondarie (potenza, corrente, ecc.). La luce LED di servizio perde priorità rispetto a un relè. Se vuoi scegliere esattamente la funzione principale, applica HMH_Device anche all'entità preferita; nel caso di più entità esplicitamente etichettate sullo stesso device, la selezione resta automatica fra quelle etichettate.
4. Se usi `script: !include scripts.yaml` in `/config/configuration.yaml`, **aggiungi** il contenuto di `ha/hmh_knob_scripts.yaml` a `/config/scripts.yaml`, senza sovrascrivere gli script presenti. Se gestisci `script:` direttamente in `configuration.yaml`, incolla le tre definizioni sotto la chiave esistente `script:`. Controlla che gli ID `hmh_knob_catalog`, `hmh_knob_state` e `hmh_knob_command` non siano già presenti.
5. Riavvia gli script Home Assistant (`Impostazioni → Strumenti per sviluppatori → YAML → Ricarica script`) oppure riavvia Home Assistant.
6. Copia i due sorgenti della cartella `esphome/` in `/config/esphome/` e anche tutti i PNG della sottocartella `icons/` in `/config/esphome/icons/`. Il file `.h` viene caricato tramite `esphome.includes`.
7. Conferma che `/config/esphome/secrets.yaml` contenga `wifi_ssid`, `wifi_password` e `api_encryption_key`, secondo la stessa convenzione del vecchio Power Control. Per il primo flash usa USB oppure configura esplicitamente il dispositivo/IP da aggiornare dopo il backup.
8. Con ESPHome **2025.10.0 o successivo** verifica la configurazione, compila e installa. Dopo l'aggiunta del device in Home Assistant, nelle opzioni dell'integrazione ESPHome **autorizza il dispositivo ad eseguire azioni Home Assistant**. Questo permesso serve alle chiamate `script.hmh_knob_*`.
9. Verifica `script.hmh_knob_catalog` dagli Strumenti per sviluppatori → Azioni: la risposta deve avere una chiave `payload` contenente un array JSON. Gli aggiornamenti del catalogo avvengono ogni 30 secondi; lo stato della pagina selezionata si aggiorna ogni 3 secondi.

## Navigazione

| Operazione | Azione |
|---|---|
| Ruota nella schermata principale | Dispositivo precedente/successivo |
| Pressione breve | Entra nei controlli della pagina |
| Ruota nei controlli | Cambia parametro (solo quelli supportati) |
| Pressione breve su ON/OFF, OPEN, STOP, RUN, ecc. | Invia l'azione |
| Pressione breve su TARGET, BRIGHTNESS, COLOR, ecc. | Inizia la modifica |
| Ruota durante la modifica | Regola il valore **senza inviare comandi** |
| Pressione breve durante la modifica | Invia il valore scelto |
| Pressione lunga (≥650 ms) | Annulla la modifica / torna alla navigazione |

I comandi partono **soltanto** dopo pressione breve e quando lo stato dell'entità è disponibile. La rotazione non accende né spegne nulla. I controlli sono visibili solo se supportati dalle caratteristiche dichiarate da Home Assistant. Gli script validano di nuovo l'etichetta e il tipo di comando sul lato HA.

## Entità supportate dalla versione 0.1

- **Luci:** on/off, luminosità, tonalità (hue), saturazione, temperatura colore in kelvin se presenti.
- **Interruttori e input_boolean:** on/off; tensione, corrente, potenza, energia, fattore di potenza se presenti come attributi o entità sensore dello stesso device.
- **Clima:** setpoint temperatura, modalità HVAC, temperatura e umidità effettivamente esposte.
- **Cover:** apertura/chiusura/stop, posizione se supportata.
- **Ventilatori:** on/off, velocità se supportata.
- **Media player:** play/pausa, volume e mute se supportati.
- **Scene e script:** esecuzione (solo se l'entità è esplicitamente etichettata).
- **Sensori singoli etichettati:** sola lettura.

## Progettazione

- **Fino a 24 dispositivi/pagine** per questa prima versione (limite precauzionale firmware; può essere alzato in C++ dopo test reali).
- Pagina LVGL **riutilizzata** per tutte le entità: niente una schermata statica per ogni slot.
- Catalogo costruito in Home Assistant con `label_entities()` e `label_devices()`; le entità appartenenti allo stesso `device_id` forniscono le misure associate.
- Risposta di script HA (`stop` con `response_variable`) catturata tramite la Native API ESPHome con `capture_response: true` e interpretata da ArduinoJson.
- Nessun token REST memorizzato sul microcontrollore. Trasporto sulla API ESPHome crittografata.

## Limitazioni e prossimi passi

Questa è una **prima versione di sviluppo**, non ancora compilata sul toolchain ESPHome né provata su ONX reale: ho verificato il parsing YAML/Jinja, i template con dispositivi simulati e la sintassi C++ con stub di libreria. **Va fatta una validazione ESPHome sul sistema reale prima del flash.** Non interpretare i test offline come prova di compatibilità hardware.

Da perfezionare durante i test: eventuali messaggi di errore applicativi, collegamenti tra sensori non appartenenti allo stesso dispositivo, template grafici più avanzati, opzioni avanzate clima e cover, gestione di indisponibilità prolungate e localizzazione UI. Per ora i device multicanale mostrano un controllo principale per pagina; il supporto a più controlli dello stesso dispositivo nella stessa pagina è un miglioramento successivo.

**Importante:** la corrente, la potenza e l'energia sono mostrate con l'unità fornita dal sensore quando disponibile. Per gli attributi nativi non sempre esiste un'unità normalizzata: in questi casi non viene inventata.

## Correzione v0.1.1 — ESPHOME Colonna Camera

Un device etichettato HMH_Device con `light` LED Blu, `switch` Relè e sensori di consumo genera **una sola scheda principale** (switch Relè). Le misure elettriche vengono lette dalle entità `sensor` dello stesso device. Il LED resta una funzione accessoria, senza una seconda pagina.

**Aggiornamento da v0.1.0:** sostituire i tre file del pacchetto. Se gli script sono già in `/config/scripts.yaml`, non aggiungerli una seconda volta: sostituire soltanto il blocco `hmh_knob_catalog:` e, per mostrare il fattore di potenza, il blocco `hmh_knob_state:`. `hmh_knob_command:` è invariato. Ricaricare gli script e aggiornare il firmware per vedere il nuovo campo COS PHI.

Il firmware non è ancora stato compilato su ESPHome reale o provato sul dispositivo; validare prima del flash.

## Novità v0.1.2 — icona centrale dinamica

- Al centro del display circolare è presente un disco da 76 px e un'icona da 48 px corrispondente al dominio della pagina.
- Icone specifiche per `light`, `switch`/`input_boolean`, `climate`, `cover`, `fan`, `media_player`, `scene`/`script`, `sensor` e altri domini.
- L'icona diventa grigia con entità spenta, non valida oppure Home Assistant offline; quando attiva si colora secondo il dominio. Per `light` in modalità colore, viene usato il colore HS realmente restituito da HA (se disponibile).
- Modificate soltanto la parte grafica e la dimensione dei testi per lasciare spazio all'icona; catalogo, stati e invio comandi sono invariati.
- Il layout visualizza nome in alto, icona al centro, valore e parametro sotto, barra, telemetria e breve indicazione della manopola.
- La modifica firmware è indipendente dagli script HA: **da v0.1.1 non serve aggiornare `scripts.yaml`.**
- Per aggiornare da Windows usa `tools/Update-HMH-Knob.ps1`: trasferisce i 2 sorgenti e le 9 immagini nella directory di ESPHome, senza fare flash e senza toccare altre configurazioni.
- Devi mantenere la directory `/config/esphome/icons/` con i 9 PNG accanto al file YAML; la compilazione incorpora le icone nel firmware.

**Stato delle verifiche:** parsing YAML, asset PNG e regressione template HA verificati offline. La compilazione completa col toolchain ESPHome e l'esecuzione sul display reale non sono state eseguite qui. In Device Builder usa prima **Validate** e, se supera, **Install**.

## Novità v0.1.3 — controllo della sensibilità e ritorno automatico

- **Filtro software dell’encoder:** un singolo scatto accettato ogni 180 ms (configurabile da `substitutions.knob_min_step_ms`). Gli impulsi intermedi sono ignorati; non si modifica la risoluzione hardware `resolution: 4` ereditata dal Power Control. In questo modo l'interfaccia non salta più rapidamente fra pagine o parametri. Un valore più alto, ad esempio `230`, rende la manopola ancora meno sensibile; `120` la rende più rapida.
- **Timeout selezione:** dopo **6,5 secondi** dall’ultima interazione accettata (modificabile da `substitutions.knob_idle_return_ms`), la modalità di controllo o modifica torna automaticamente alla **navigazione fra i dispositivi**. Il dispositivo visualizzato resta lo stesso. La prossima rotazione cambia dispositivo.
- Se la modifica non è stata confermata, la bozza viene **annullata senza inviare alcun comando**. Se un comando era già stato confermato, il suo invio prosegue normalmente.
- Le richieste periodiche a Home Assistant **non fanno ripartire il timeout**.
- La prima rotazione dopo un click o dopo il timeout è sempre disponibile, senza la pausa del filtro.

### Aggiornamento da Windows (dalla v0.1.2)

Esegui `tools/Update-HMH-Knob.ps1` sul PC Windows dopo aver estratto il file ZIP. Lo script copia **solo** `hmh_universal_knob.yaml` e `hmh_universal_knob.h` via SCP sul server Home Assistant. Non serve ricaricare script HA né icone. Se stai installando da zero e ti mancano le icone, esegui lo script con `-IncludeIcons`. La modifica non fa flash automatico: in ESPHome esegui **Validate**, poi **Install**.

**Verifiche:** configurazione YAML parsata e test di regressione HA eseguiti offline; simulazione C++ della logica encoder/timeout superata con test stub (`tests/test_encoder_timeout.cpp`). Compilazione ESPHome effettiva e verifica hardware restano necessarie.
