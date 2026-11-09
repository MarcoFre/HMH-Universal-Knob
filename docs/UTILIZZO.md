# Utilizzo e comandi

Il Knob mostra una pagina alla volta, generata in base alle entità associate all'etichetta `HMH_Device`. Il catalogo può contenere fino a 24 pagine. **Il display non è touch**: tutti i comandi avvengono tramite rotazione e pressione dell'encoder.

| Gesto | Risultato |
| --- | --- |
| Ruota dalla schermata principale | Pagina precedente/successiva |
| Premi brevemente | Entra nella modalità di controllo |
| Ruota in modalità di controllo | Seleziona la funzione disponibile |
| Premi su `ON/OFF`, `OPEN`, `STOP`, `RUN`, ecc. | Conferma l'azione corrispondente |
| Premi su `TARGET`, `BRIGHTNESS`, `COLOR`, ecc. | Inizia la modifica del valore |
| Ruota mentre modifichi | Cambia la bozza locale senza inviare comandi |
| Premi brevemente mentre modifichi | Invia il valore selezionato |
| Pressione lunga (almeno 650 ms) | Annulla / torna alla navigazione |
| Inattività di 6,5 secondi | Torna alla navigazione, scartando modifiche non confermate |

Le modifiche al valore **non vengono inviate durante la rotazione**. Il filtro dell'encoder accetta uno scatto ogni 180 ms (parametro `knob_min_step_ms`) e il timeout è regolabile con `knob_idle_return_ms`.

## Interfacce per dominio

| Dominio Home Assistant | Funzionalità |
| --- | --- |
| `light` | ON/OFF e, secondo capacità esposte, luminosità, tonalità, saturazione, temperatura colore |
| `switch` / `input_boolean` | ON/OFF; informazioni elettriche correlate quando disponibili |
| `climate` | Temperatura target e modalità HVAC quando supportata |
| `cover` | Apri, chiudi, stop e posizione quando supportata |
| `fan` | ON/OFF e percentuale velocità dove prevista |
| `media_player` | Play/pausa, volume, mute dove previsti |
| `scene` / `script` | Esecuzione, solo se entità etichettata direttamente |
| `sensor` | Visualizzazione sola lettura |

Per i dispositivi fisici con più canali, la versione 0.1.3 sceglie un controllo principale. Le misure di telemetria possono comparire sulla stessa pagina se gli opportuni sensori appartengono allo stesso `device_id`.
