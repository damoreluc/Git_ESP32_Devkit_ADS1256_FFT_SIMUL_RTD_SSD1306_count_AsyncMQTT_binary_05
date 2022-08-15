# Git_ESP32_Devkit_ADS1256_FFT_SIMUL_RTD_SSD1306_count_AsyncMQTT_binary_05

## Repository del progetto ITEM - Banco prova riduttori meccanici

### Firmware per ESP32

Il codice è sviluppato con il framework Arduino in ambiente VS Code/PlatformIO.

## La versione corrente supporta le seguenti funzionalità:

1. Acquisizione dei due ingressi analogici per [accelerometri piezo elettrici IEPE](https://www.allaboutcircuits.com/technical-articles/introduction-to-piezoelectric-accelerometers-with-integral-electronics-piezo-electric-iepe-sensor/)
2. Impiego dell'amplificatore a guadagno programmabile [MCP6S26](https://www.microchip.com/en-us/product/MCP6S26#) con multiplexer analogico per la selezione del canale accelerometrico e la regolazione del guadagno
3. Impiego del convertitore analogico digitale [ADS1256](https://www.ti.com/product/ADS1256) con risoluzione di 24 bit 
4. Campionamento di ciascun canale accelerometrico a 7500 Sa/s
5. Calcolo dello spettro delle ampiezze di ciascun canale accelerometrico su 4096 campioni, risoluzione in frequenza 1,83Hz
6. Acquisizione di 4 canali analogici con ampiezza +/-10V per i due segnali di coppia e i due segnali di velocità mediante ADC [MCP3204](https://www.microchip.com/en-us/product/MCP3204), risoluzione di 12 bit, frequenza di campionamento 100 Sa/s
7. Acquisizione di due segnali di temperatura tramite trasduttori RTD PT1000 e moduli di condizionamento e conversione [Adafruit PT1000 RTD Temperature Sensor Amplifier - MAX31865](https://www.adafruit.com/product/3648)
8. Visualizzazione locale dello stato di funzionamento tramite display OLED [SSD1306](https://nettigo.eu/system/images/3580/original.JPG?1578141142) con interfaccia I2C
9. Connessione alla rete dati mediante Wifi
10. Trasmissione dei dati telemetrici mediante protocollo [MQTT](https://www.hivemq.com/mqtt-essentials/) su broker a scelta: [mosquitto](https://test.mosquitto.org/) remoto o locale, [shiftr.io](https://www.shiftr.io/)
11. Gestione da remoto tramite messaggi MQTT


## Configurazione WiFi
La scheda ESP32 opera come _device_ o _station_ da collegare ad un WiFi Access Point.

I parametri di connessione all'Access Point vanno dichiarati nel file `pref.h` 

```
#define WIFI_SSID  "...SSID del tuo Access Point..."
#define WIFI_PASSWORD  "...password del tuo Access Point..."
```

**Poi il firmware va ricompilato**.


## Configurazione del broker MQTT

Per ciascun broker MQTT è disponibile un file di configurazione nel quale inserire i parametri di accesso.

* Broker *_Shiftr.io_* file `shiftr_io.h`  
```
#define SHIFTR_HOST  "item2122a.cloud.shiftr.io"
#define SHIFTR_PORT  1883
#define SHIFTR_USER  "item2122a"
#define SHIFTR_PASS  "K2Xpe5XV3yvL7zcu"
```

* Broker *_Mosquitto_* locale su Raspberry Pi4 file `raspi4.h`  
```
#define RASPI4_HOST  "192.168.0.46"
#define RASPI4_PORT  1883
#define RASPI4_USER  "tinytracker314"
#define RASPI4_PASS  "EJeSDjwY7AfDyFvE"
```

La selezione del broker da impiegare è decisa nel file `main.cpp` nella sezione iniziale. Ad esempio, per utilizzare Shiftr.io:

```
// uncomment one of following #include to set the MQTT broker.
// Leaving all the comments will use the default broker (test.mosquitto.org)
#include <shiftr_io.h>
//#include <raspi4.h>
```

**Come aggiungere un altro broker**

1. creare un file con i parametri di connessione, ad esempio: `altro.h`  
```
#ifndef __ALTRO_BROKER
#define __ALTRO_BROKER

#define ALTRO_HOST  "indirizzo IP o URL del broker"
#define ALTRO_PORT  1883
#define ALTRO_USER  "nome utente"
#define ALTRO_PASS  "password utente"

#endif
```

2. includere in `main.cpp` il file `altro.h` :
```
// uncomment one of following #include to set the MQTT broker.
// Leaving all the comments will use the default broker (test.mosquitto.org)
#include <altro.h>
//#include <shiftr_io.h>
//#include <raspi4.h>
```

3. nel file `main.cpp` impostare le variabili globali con i parametri di connessione:
```

// your MQTT Broker:
// MQTT broker's connection parameters

#ifdef __SHIFTRIO
const char *mqttServer = SHIFTR_HOST;
const int mqttPort = SHIFTR_PORT;
const char *mqttUser = SHIFTR_USER;
const char *mqttPassword = SHIFTR_PASS;

#elif defined(__RASPI4)
const char *mqttServer = RASPI4_HOST;
const int mqttPort = RASPI4_PORT;
const char *mqttUser = RASPI4_USER;
const char *mqttPassword = RASPI4_PASS;

#elif defined(__ALTRO_BROKER)
const char *mqttServer = ALTRO_HOST;
const int mqttPort = ALTRO_PORT;
const char *mqttUser = ALTRO_USER;
const char *mqttPassword = ALTRO_PASS;

#else
const char *mqttServer = "test.mosquitto.org"; // or local (MQTT 3.1, not 3.1.1): "192.168.0.48";
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPassword = "";
#endif

```
**Poi il firmware va ricompilato**.

## Identificativo MQTT della scheda ESP32

Nel file `mqtt_topics.h` è definita la tag **_thisClient_** che identifica tutti i messaggi MQTT pubblicati da o destinati a questa scheda di acquisizione. Moficicarlo in base alle proprie esigenze.

```
// MQTT client ID
#define thisClient "ESP32DevKit123" //"ESP32Udine"

```

## Elenco dei Topics MQTT impiegati

L'elenco di tutti i topics è dichiarato nel file `mqtt_topics.cpp`  
in questo file modificare o aggiungere solo il nome del topic, non rimuovere la tag **_thisClient_**

Ogni topic è nella forma sintattica: `identificativo_MQTT_scheda/nome_topic` ad esempio: `ESP32DevKit123/trigger`

**Nota:** il testo dei topics è _case sensitive_.


Il controllo remoto della ESP32 avviene tramite i seguenti _subscribed topics_ :

| nome_topic | Tipo | Valori | Descrizione |
|:--:|:---:|:---:|----|
| /trigger | stringa | 0, 1, 2 | Controlla la MSF delle acquisizioni. <ul><li>0: Fine acquisizione</li><li>1: Singola acquisizione</li><li>2: Acquisizione ciclica</li> </ul>|
| /pgaSetGain | stringa | 0..7 | Controlla il guadagno del PGA: <ul><li>0: Guadagno x 1</li><li>1: Guadagno x 2</li><li>2: Guadagno x 4</li><li>3: Guadagno x 5</li><li>4: Guadagno x 8</li><li>5: Guadagno x 10</li><li>6: Guadagno x 16</li><li>7: Guadagno x 32</li> </ul>|


La scheda ESP32 comunica la telemetria attraverso i seguenti _published topics_ :

| nome_topic | Tipo | Note | Descrizione |
|:--:|:---:|:---:|----|
| /FFTBinTopic0 | array di valori float in forma binaria, <br> _little-endian_ | due messaggi consecutivi da 4096 byte ciascuno [^1] | contiene i 2048 moduli dello spettro dell'accelerazione sul canale 0 |
| /FFTBinTopic1 | array di valori float in forma binaria, <br> _little-endian_ | due messaggi consecutivi da 4096 byte ciascuno [^1] | contiene i 2048 moduli dello spettro dell'accelerazione sul canale 1 |
| /RTD1BinTopic | stringa |  | temperatura in °C rilevata dalla RTD1 |
| /RTD2BinTopic | stringa |  | temperatura in °C rilevata dalla RTD2 |
| /RTD1FaultTopic | stringa |  | messaggio diagnostico dal modulo RTD1 <ul><li>Ok</li><li>RTD High Threshold</li><li>RTD Low Threshold</li><li>REFIN- > 0.85 x Bias</li><li>REFIN- < 0.85 x Bias - FORCE- open</li><li>RTDIN- < 0.85 x Bias - FORCE- open</li><li>Under/Over voltage</li></ul> |
| /RTD2FaultTopic | stringa |  | messaggio diagnostico dal modulo RTD2 <ul><li>Ok</li><li>RTD High Threshold</li><li>RTD Low Threshold</li><li>REFIN- > 0.85 x Bias</li><li>REFIN- < 0.85 x Bias - FORCE- open</li><li>RTDIN- < 0.85 x Bias - FORCE- open</li><li>Under/Over voltage</li></ul> |
| /CountAdcTopic | stringa |   | numero di campionamenti acquisiti sui canali coppia/velocità |
| /TorqueSpeedTopic | array di valori float in forma binaria, <br> _little-endian_ | un messaggio con 4 x CountAdcTopic elementi | contiene i campioni dei due canali di coppia e dei due canali di velocità, nel formato seguente <br> \[campioni coppia 1\]\[campioni coppia 2\]\[campioni velocità 1\]\[campioni velocità 2\]  |
| /pgaGetGain | stringa  |   | valore attuale del guadagno impostato sul PGA  |



[^1]:  il primo elemento del primo messaggio viene cambiato di segno (sarà sempre negativo) per consentire a chi riceve di discriminare il primo messaggio dal secondo messaggio e ricostruire l'ordine dei 4096+4096 byte, altrimenti lo spettro delle ampiezze potrebbe risultare traslato di metà banda passante.
---
