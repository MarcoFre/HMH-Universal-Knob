# FAQ — problemi frequenti

### Il display mostra «Caricamento...» o nessun dispositivo

Verifica che esista l'etichetta **`HMH_Device`**, che sia applicata a un'entità/dispositivo supportato, che i tre script Home Assistant siano presenti e che la connessione ESPHome a Home Assistant sia attiva. Attendi il refresh del catalogo (fino a 30 secondi). Controlla i log ESPHome e le tracce degli script in Home Assistant.

### Il Knob non riesce a comandare Home Assistant

Nelle opzioni dell'integrazione ESPHome abilita il permesso che autorizza il dispositivo ad **eseguire azioni Home Assistant**. Controlla anche `script.hmh_knob_command` e che l'entità sia disponibile e autorizzata dall'etichetta.

### Il dispositivo appare due volte o viene scelta la funzione sbagliata

Con etichetta su un dispositivo fisico viene generata una pagina per `device_id`, con scelta automatica del controllo principale. Applica la stessa etichetta all'entità specifica che vuoi privilegiare. Dispositivi che espongono più funzioni non ricevono automaticamente più pagine.

### Potenza, tensione o corrente non compaiono

Le misure devono essere disponibili come sensori o attributi associati allo stesso dispositivo Home Assistant. Il firmware non prova a indovinare entità per nome e non inventa valori quando non sono disponibili.

### Non compila: file PNG o header non trovato

Accertati di avere copiato `hmh_universal_knob.h` e tutta la cartella `icons/` dentro `/config/esphome/`, accanto al file `hmh_universal_knob.yaml`.

### La manopola cambia pagina troppo rapidamente o troppo lentamente

Modifica `substitutions.knob_min_step_ms`: default `180` ms; per renderla meno sensibile prova `230`, per più reattività `120`. Dopo aver cambiato il valore serve ricompilare e aggiornare il firmware.

### Perché dopo alcuni secondi torno alla navigazione?

È il comportamento voluto della v0.1.3. `knob_idle_return_ms` è `6500` ms: mantiene la pagina corrente ma esce dalla modalità controllo/modifica. Qualsiasi modifica non confermata viene annullata.

### Ho già Power Control sullo stesso Knob

HMH Universal Knob è un firmware **indipendente**. Installandolo sul medesimo dispositivo **sostituisci il firmware esistente**. Prima di procedere conserva una copia dei file e verifica il dispositivo selezionato per l'installazione.

### Come faccio a sapere se è tutto testato?

Nel pacchetto sono disponibili test offline per i template Jinja e per la macchina a stati del rotary encoder. La compilazione ESPHome e l'esecuzione con il tuo impianto sono test da effettuare separatamente.
