#include <Arduino.h>
#include <pref.h>
#include <HW_setup.h>
#include <WiFi.h>
#include <wifiCallBacks.h>
#include <Wire.h>
#include <SPI.h>
#include <ssd1306.h>
#include <AsyncMqttClient.h>
#include <time.h>
#include <localTime.h>
#include <dati.h>
#include "auxiliar_functions.h"
#include "FFT.h"
#include "FFT_signal.h"
#include <fast_sqrt.h>
#include <ADS1256.h>
#include <ADS1256_equalizer.h>
#include <Adafruit_MAX31865.h>
#include <rtd_MAX31865.h>
#include <mcp3204.h>
#include <mqtt_topics.h>
#include <sim_real_data_selector.h>

// uncomment this #define to print fft components
//#define PRINT_COMPONENTS

// uncomment one of following #include to set the MQTT broker.
// Leaving all the comments will use the default broker (test.mosquitto.org)
#include <shiftr_io.h>
//#include <raspi4.h>

// canale SPI per connessione con ADC ADS1256
SPIClass hspi = SPIClass(HSPI);

// ADC equalization table - see create_equalizer(m)
float m[FFT_SIZE >> 1];

// ADS1256 DRDY
volatile bool newData = false;
volatile uint16_t countData = 0;

// ADC instance
ADS1256 adc;
// list of channels to sample
byte channels[CHANNELS_N] = {adc.ads1256_mux[0], adc.ads1256_mux[1]};
// index of current channel
uint8_t current_channel = 0;
// index of channel to publish
uint8_t publish_channel = 0;

// RTD1 module instance:
// use hardware SPI, just pass in the CS pin
// spi module: VSPI (MISO = GPIO19, MOSI = GPIO23, SCK = GPIO18)
Adafruit_MAX31865 RTD1 = Adafruit_MAX31865(CS1_MAX31865);

// RTD2 module instance:
// use hardware SPI, just pass in the CS pin
// spi module: VSPI (MISO = GPIO19, MOSI = GPIO23, SCK = GPIO18)
Adafruit_MAX31865 RTD2 = Adafruit_MAX31865(CS2_MAX31865);

// MCP3204 ADC oputput data
mcp3204Data mcp3204_dati;

// canale SPI per connessione con ADC MCP3204
SPIClass vspi = SPIClass(VSPI);

// ISR per la gestione dell'arrivo di un nuovo campione sul fronte di discesa di DRDY
void IRAM_ATTR ISR_DRDY();

// puntatore al buffer dei dati in ingresso per FFT
// viene inizializzato nella funzione process, allo stato
IRAM_ATTR float *pInput;

// time of last message published
unsigned long lastMsg = millis();
// number of published messages
int counter = 0;

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
#else
const char *mqttServer = "test.mosquitto.org"; // or local (MQTT 3.1, not 3.1.1): "192.168.0.48";
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPassword = "";
#endif

// flag true quando i dati sono pronti
volatile bool dataReady = false;

// task elaborazione FFT
void process(void *pvParameters);
// task pubblicazione FFT
void publishFFT(void *pvParameters);
// print some boot messages
void bootMsg();
// print some information about the received message
void printRcvMsg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

// funzione per acquisizione temperature RTD e inserimento in coda
void readRTD();

// parametri per il calcolo della FFT
fft_config_t *real_fft_plan;

// handle del task di elaborazione FFT e stampa
TaskHandle_t processTaskHandle;
// handle del task di pubblicazione FFT
TaskHandle_t publishTaskHandle;

// struct per le temperature delle RTD
typedef struct
{
  float rtd1;
  float rtd2;
  char fault1[MSG_FAULT_LEN + 1];
  char fault2[MSG_FAULT_LEN + 1];
} stRTD;

stRTD temperature;

// coda per temperature RTD
QueueHandle_t xQueueRTD;

// coda per conteggio accessi ADC dei sensori di coppia
QueueHandle_t xQueueCountADCTorque;

// usata nel debug
char datapkt[20] = "";

// modalità di acquisizione
typedef enum
{
  Stop,
  OneShot,
  FreeRun
} tMode;

tMode triggered = Stop;

// macchina a stati finiti acquisizione
typedef enum
{
  StartADC,
  Sampling,
  Compute,
  Publish,
  WaitTrigger
} tStati;
volatile tStati _stato = WaitTrigger;

// stampa stato attuale ---------------------------------------------------------------------------
void DebugCurrentStatus(tStati st)
{
  static tStati pst = Sampling; // initialize with any value different from WaitTrigger
  if (st != pst)
  {
    Serial.printf("_stato: %d\n", st);
    pst = st;
  }
}

// system setup -----------------------------------------------------------------------------------
void setup()
{
  // acquisisce il pin SENS_MODE e aggiorna lo stato dati_simulati/dati_reali
  readSensMode();

  Serial.begin(115200);
  bootMsg();

  if (getSensMode() == REAL_DATA)
  {
    // some debug/informative message
    ssd1306_log_setup();
    ssd1306_publish("Create EQ table\n");
  }
  else if (getSensMode() == SYM_DATA)
  {
    Serial.println("Dati simulati, senza sensori e senza display");
  }

  // set MCP3204 Chip select line
  if (getSensMode() == REAL_DATA)
  {
    pinMode(CS_MCP3204, OUTPUT);
    digitalWrite(CS_MCP3204, HIGH);
    // initialise vspi with default pins
    // SCLK = 18, MISO = 19, MOSI = 23, SS = 5
    vspi.begin();
  }

  // create ADS1256 equalization table
  Serial.println(F("Creazione della tabella di equalizzazione"));
  create_equalizer(m);

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Setting up RTD1\n");
  }

  // setup RTD1 object: set to 2WIRE, 3WIRE or 4WIRE as necessary
  RTD1.begin(MAX31865_3WIRE);
  delay(500);

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Setting up RTD2\n");
  }

  // setup RTD2 object: set to 2WIRE, 3WIRE or 4WIRE as necessary
  RTD2.begin(MAX31865_3WIRE);
  delay(500);

  // creazione coda per pubblicazione temperatura delle RTD
  xQueueRTD = xQueueCreate(5, sizeof(stRTD));

  // creazione coda per pubblicazione conteggio accessi ADC coppie
  xQueueCountADCTorque = xQueueCreate(2, sizeof(uint32_t));

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Create FFT task\n");
  }

  BaseType_t xReturned;
  // crea il task di elaborazione della FFT
  xReturned = xTaskCreatePinnedToCore(
      process,            // function that implements the task
      "process",          // name for the task
      4096,               // task size
      NULL,               // parameter passed into the task
      10,                 // task priority
      &processTaskHandle, // the task's handle
      1                   // pinned to core 1
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Errore nella creazione del task FFT"));
    while (1)
    {
      if (getSensMode() == REAL_DATA)
      {
        ssd1306_publish("Error on FFT task\n");
      }
      yield();
    }
  }

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Create publish task\n");
  }

  // crea il task di pubblicazione della FFT su MQTT
  xReturned = xTaskCreatePinnedToCore(
      publishFFT,         // function that implements the task
      "publishFFT",       // name for the task
      4096,               // task size
      NULL,               // parameter passed into the task
      1,                  // task priority
      &publishTaskHandle, // the task's handle
      0                   // pinned to core 0
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Errore nella creazione del task publishFFT"));
    while (1)
    {
      if (getSensMode() == REAL_DATA)
      {
        ssd1306_publish("Error on publish task\n");
      }
      yield();
    }
  }

  if (getSensMode() == REAL_DATA)
  {
    Serial.println(F("Configurazione ADC ADS1256"));
    ssd1306_publish("ADS1256 config...\n");
    // imposta la isr dedicata al data ready dell'ADC ADS1256, triggerata sul fronte di discesa dell'interrupt
    // configurazione dell'ADS1256 e delle sue linee di controllo
    //  NB: SPI clock <= F_clkin / 4 = 7.68e6 / 4 = 1920000
    adc.init(hspi, nCS, nDRDY, nPDWN, 1900000);
    adc.setChannel(adc.ads1256_mux[0]);
    adc.standby();
  }

  // set RTOS timers to handle automatic reconnection to WiFi / MQTT broker
  setTimersRTOS(4000);

  // set WiFi onEvent callback function
  // see https://github.com/OttoWinter/async-mqtt-client/blob/master/examples/FullyFeatured-ESP32/FullyFeatured-ESP32.ino
  WiFi.onEvent(WiFiEvent);

  // define topics to be subscribed to
  bool result;
  result = AddSubscribedTopic(inTopic, 0);
  if (result == false)
  {
    Serial.printf("ERROR: Unable to add topic %s to the list\n", inTopic);
    while (1)
    {
      yield();
    }
  }

  result = AddSubscribedTopic(triggerTopic, 0);
  if (result == false)
  {
    Serial.printf("ERROR: Unable to add topic %s to the list\n", triggerTopic);
    while (1)
    {
      yield();
    }
  }

  // set mqttClient event's callback functions
  mqttClient.onConnect(onMqttConnect);       // mandatory
  mqttClient.onDisconnect(onMqttDisconnect); // mandatory
                                             // mqttClient.onSubscribe(onMqttSubscribe);        // optional (subscribing to topic acknowledged)
                                             // mqttClient.onUnsubscribe(onMqttUnsubscribe);    // optional
  mqttClient.onMessage(onMqttMessage);       // mandatory
  mqttClient.onPublish(onMqttPublish);       // optional (publishing acknowledged)

  // set MQTT broker server
  mqttClient.setServer(mqttServer, mqttPort);

  // set user's credentials on MQTT broker
  mqttClient.setCredentials(mqttUser, mqttPassword);

  // connect to WiFi AP
  connectToWifi(WIFI_SSID, WIFI_PASSWORD);

  // sblocca il task di elaborazione, l'array output è libero
  xTaskNotifyGive(processTaskHandle);
}

// loop function ----------------------------------------------------------------------------------
void loop()
{
}

// ------------------------------------------------------------------------------------------------
// handle onMqttMessage event: fired on receiving new message from a subscribed topic - mandatory
// fired by mqttClient.onMessage on receiving new message
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  // Trigger topic? restart acquisition
  if (strcmp(topic, triggerTopic) == 0) // è arrivato un messaggio da triggerTopic
  {
    if (strncmp(payload, "0", 1) == 0)
    {
      triggered = Stop;
      Serial.println("Fine acquisizione");
    }

    if (strncmp(payload, "1", 1) == 0)
    {
      triggered = OneShot;
      Serial.println("Singola acquisizione");
    }

    if (strncmp(payload, "2", 1) == 0)
    {
      triggered = FreeRun;
      Serial.println("Acquisizione ciclica");
    }
  }

  // print some information about the received message
  printRcvMsg(topic, payload, properties, len, index, total);
}

// process and output task ------------------------------------------------------------------------
void process(void *pvParameters)
{
  static uint64_t last = millis();
  static uint64_t lastFFT;
  static uint16_t sampleCounter = 0; // number of samples gathered
  static uint32_t numberOfAds1256Cycles = 0;
  static uint32_t countADCTorque = 0; // conteggio accessi ADC sensori di coppia
  float adcValue;
  int kk;

  while (1)
  {
    DebugCurrentStatus(_stato);

    switch (_stato)
    {
    case StartADC:
      // reset conteggio ADC torque
      countADCTorque = 0;
      dataReady = false;
      // imposta il canale corrente dell'ADC
      Serial.printf("Channel: %d\n", current_channel);

      // inizializzazione fft
      if (real_fft_plan == NULL)
      {
        Serial.println(F("Inizializzazione FFT"));
        // real_fft_plan = fft_init(FFT_SIZE, FFT_REAL, FFT_FORWARD, f_input, f_output);
        real_fft_plan = fft_init(FFT_SIZE, FFT_REAL, FFT_FORWARD, NULL, NULL);
        pInput = real_fft_plan->input;
      }
      _stato = Sampling;

      last = millis();
      lastFFT = last;
      countADCTorque = 0;
      mcp3204_ResetBuffer();

      if (getSensMode() == REAL_DATA)
      {
        adc.setChannel(channels[current_channel]);
        adc.wakeup();
        // associa l'interrupt esterno di nDRDY alla sua ISR
        attachInterrupt(nDRDY, ISR_DRDY, FALLING);

        //
        vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
      }

      break;

    case Sampling:
      // acquisizione da ADS1256
      if (getSensMode() == REAL_DATA)
      {

        if (newData) // settato dalla ISR su DRDY dell'ADS1256
        {
          newData = false;

          if (sampleCounter < FFT_SIZE)
          {
            // get ADS1256 new sample
            adcValue = (float)adc.ReadRawData();
            real_fft_plan->input[sampleCounter] = adc.volt(adcValue);
            sampleCounter++;
            numberOfAds1256Cycles++;

            // get MCP3204 new samples
            if ((numberOfAds1256Cycles >= MCP3204_NUMBER_OF_ADS1256_CYCLES) && (mcp3204_BufferAvailable() > 0))
            {
              numberOfAds1256Cycles = 0;
              countADCTorque++;

              // Nota: ad ogni fase di Sampling (durata: 4096/7500 = 546,133ms)
              //       questa sezione viene eseguita 3075 volte
              //       corrisponde ad un intervallo di campionamento sul MCP3204 di 177,604us
              //       ovvero 5630 campionamenti dei 4 canali
              // vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
              mcp3204_getAllVoltage(vspi, CS_MCP3204, &mcp3204_dati);
              // fine della transazione con MCP3204
              // vspi.endTransaction();
              /*
               * debug
               */
              // mcp3204_dati.volt0 = 0.5;
              // mcp3204_dati.volt1 = 1.0;
              // mcp3204_dati.volt2 = 1.5;
              // mcp3204_dati.volt3 = 2.0;
              /*
               * fine debug
               */
              mcp3204_Push(&mcp3204_dati);
            }
          }
          else
          {
            detachInterrupt(nDRDY);
            //  ferma il campionamento al completamento del numero di campioni
            adc.standby();

            // 
                vspi.endTransaction();

            // debug: campioni persi?
            Serial.print("campioni memorizzati: ");
            Serial.print(sampleCounter);
            Serial.print("   campioni acquisiti: ");
            Serial.print(countData - 1);
            Serial.print("   differenza: ");
            Serial.println(countData - 1 - sampleCounter);

            // resetta l'indice dell'array dei dati
            sampleCounter = 0;
            countData = 0;

            // segnala la fine del campionamento alla loop()
            dataReady = true;
          }
        }
      }

      // simulazione campionamento per FFT (durata 576ms)
      else if (getSensMode() == SYM_DATA)
      {
        // dati ADS1256 simulati per singolo canale
        for (kk = 0; kk < FFT_SIZE; kk++)
        {
          real_fft_plan->input[kk] = 1.0 + 0.5 * current_channel;
        }

        // dati coppie e velocità simulati
        for (kk = 0; kk < MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL; kk++)
        {
          mcp3204_dati.volt0 = 0.5;
          mcp3204_dati.volt1 = 1.0;
          mcp3204_dati.volt2 = 1.5;
          mcp3204_dati.volt3 = 2.0;
          mcp3204_Push(&mcp3204_dati);
        }

        // durata del ciclo di acquisizione reale
        delay(546);
        dataReady = true;
      }

      // termine dello stato di campionamento
      if (dataReady == true)
      {
        dataReady = false;
        _stato = Compute;
      }

      break;

    case Compute:
      // hang here until FFT output data are published
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      // Execute FFT transformation
      fft_execute(real_fft_plan);

      // read RTD1 and store result into xQueueRTD1
      //++ dati RTD simulati
      readRTD();

      // push count ADC into xQueueCountAdcTorque
      if (uxQueueSpacesAvailable(xQueueCountADCTorque) > 0)
      {
        xQueueSendToBack(xQueueCountADCTorque, (void *)&countADCTorque, portMAX_DELAY);
      }

      _stato = Publish;
      break;

    case Publish:
      publish_channel = current_channel;
      // wake-up the publish task
      xTaskNotifyGive(publishTaskHandle);

      Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

      // passa al prossimo canale
      current_channel++;

      // torna allo stato WaitTrigger
      _stato = WaitTrigger;
      break;

    case WaitTrigger:

      // stay here until next Trigger command received by onMqttMessage
      if (mqttClient.connected() && (triggered == OneShot))
      {
        if (current_channel < CHANNELS_N)
        {
          _stato = StartADC;
        }
        else
        {
          triggered = Stop;
          current_channel = 0;
        }
      }
      else if (mqttClient.connected() && (triggered == FreeRun))
      {
        // passa al prossimo canale o termina la scansione
        if (current_channel >= CHANNELS_N)
        {
          current_channel = 0;
        }
        _stato = StartADC;
      }
      else if (mqttClient.connected() && (triggered == Stop))
      {
        current_channel = 0;
        _stato = WaitTrigger;
      }
      delay(5);

      break;

    default:
      _stato = WaitTrigger;
      break;
    }
  }
}

//-----------------------------------------------------------------------------
// task pubblicazione FFT
void publishFFT(void *pvParameters)
{
  while (1)
  {
    // hang here until FFT is computed
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (real_fft_plan != NULL)
    {
      /*Multiply the magnitude of the DC component with (1/FFT_N) to obtain the DC component*/
      /* m[0] is the equalization coefficient */
      real_fft_plan->output[0] = real_fft_plan->output[0] * ONE_OVER_FFT_SIZE * m[0];

#ifdef PRINT_COMPONENTS
      // Print the output
      Serial.printf("Freq[Hz]\tAmpl[V]\n");
      sprintf(datapkt, "%7.2f\t%5.3f\n", 0.0, (f_output[0]));
      Serial.printf(datapkt);
#endif

      float a, b, c;

      for (int k = 1; k < real_fft_plan->size / 2; k++)
      {
        /*The real part of a magnitude at a frequency is followed by the corresponding imaginary part in the output*/
        a = real_fft_plan->output[2 * k];
        b = real_fft_plan->output[2 * k + 1];
        c = (a * a + b * b);

        float mag = Q_sqrt(c); // 1.59ms

        // store magnitude result in the lower half of output[] array
        // m[k] is the equalization coefficient
        real_fft_plan->output[k] = mag * 2.0 * ONE_OVER_FFT_SIZE * m[k];

#ifdef PRINT_COMPONENTS
        double freq = k * df;
        sprintf(datapkt, "%4d\t%7.2f\t%5.3f\n", k, freq, mag);
        // strcat(publishpkt, datapkt);
        // pos++;
        Serial.printf(datapkt);
#endif
      }

      // publish FFT data on MQTT  when connected to the MQTT broker...
      if (mqttClient.connected())
      {

        // publish the message
        uint16_t j;
        uint16_t res = 0;
        uint32_t begin = millis();

        for (j = 0; j < BLOCKS_FLOAT; j++)
        {
          do
          {
            if (publish_channel == 0)
            {
              res = mqttClient.publish(outTopic0, 0, false, (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), MQTT_MAX_SIZE_BYTE);
            }
            else if (publish_channel == 1)
            {
              res = mqttClient.publish(outTopic1, 0, false, (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), MQTT_MAX_SIZE_BYTE);
            }

            delay(25);
          } while (res == 0);
        }

        uint32_t now = millis();

        Serial.print("Elapsed time: ");
        Serial.print(now - begin);
        Serial.println(" ms");

        // pubblicazione degli eventuali dati delle RTD
        if (xQueueRTD != NULL)
        {
          stRTD temp;
          char s[10];

          while (uxQueueMessagesWaiting(xQueueRTD) > 0)
          {
            xQueueReceive(xQueueRTD, &(temp), portMAX_DELAY);

            // pubblicazione valore RTD1
            sprintf(s, "%.3f", temp.rtd1);
            do
            {
              res = mqttClient.publish(outTopic2, 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
            // pubblicazione stato RTD1
            do
            {
              res = mqttClient.publish(outTopic3, 0, false, (const char *)&temp.fault1[0], strlen(&temp.fault1[0])); // MSG_FAULT_LEN);
              delay(10);
            } while (res == 0);

            // pubblicazione valore RTD2
            sprintf(s, "%.3f", temp.rtd2);
            do
            {
              res = mqttClient.publish(outTopic4, 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
            // pubblicazione stato RTD2
            do
            {
              res = mqttClient.publish(outTopic5, 0, false, (const char *)&temp.fault2[0], strlen(&temp.fault2[0])); // MSG_FAULT_LEN);
              delay(10);
            } while (res == 0);
          }
        }

        // pubblicazione conteggio accessi ADC
        if (xQueueCountADCTorque != NULL)
        {
          uint32_t nadc;
          char s[10];

          while (uxQueueMessagesWaiting(xQueueCountADCTorque) > 0)
          {
            xQueueReceive(xQueueCountADCTorque, &nadc, portMAX_DELAY);
            sprintf(s, "%9d", nadc);
            do
            {
              res = mqttClient.publish(outTopic6, 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
          }
        }

        // pubblicazione dati mcp3204 mcp3204buffer[MCP3204_BUFFER_SIZE]
        do
        {
          res = mqttClient.publish(outTopic7, 0, false, (const char *)&mcp3204buffer[0], MCP3204_BUFFER_SIZE * sizeof(float));
          delay(10);
        } while (res == 0);
      }

      // sblocca il task di elaborazione, l'array output è libero
      xTaskNotifyGive(processTaskHandle);
    }
  }
}

//-----------------------------------------------------------------------------
// lettura RTD1 ed RTD2 e inserimento dati in xQueueRTD ogni RTD_PERIOD ms
void readRTD()
{
  static long prev = millis();
  long now = millis();

  stRTD temp;

  if (now - prev >= RTD_PERIOD)
  {
    prev = now;
    // lettura del sensore RTD1
    if (getSensMode() == REAL_DATA)
    {
      getTemperature(RTD1, &temp.rtd1, &temp.fault1[0]);
    }

    if (getSensMode() == SYM_DATA)
    {
      temp.rtd1 = 20.0;
      strcpy(&temp.fault1[0], "Ok");
    }

    // lettura del sensore RTD2
    if (getSensMode() == REAL_DATA)
    {
      getTemperature(RTD2, &temp.rtd2, &temp.fault2[0]);
    }

    if (getSensMode() == SYM_DATA)
    {
      temp.rtd2 = temp.rtd1 + 0.5;
      strcpy(&temp.fault2[0], "Ok");
    }

    // inserimento in coda
    if (uxQueueSpacesAvailable(xQueueRTD) > 0)
    {
      xQueueSendToBack(xQueueRTD, (void *)&temp, portMAX_DELAY);
    }
  }
}

//-----------------------------------------------------------------------------
// implementazione ISR
// ISR per la gestione dell'arrivo di un nuovo campione
void IRAM_ATTR ISR_DRDY()
{
  newData = true;
  countData++;
}

//---------------------------------------------------------------------------------------------
void bootMsg()
{
  // some debug/informative message
  Serial.println();
  Serial.println(F("ESP32-WROOM-32 DevKit ADS1256 ADC 24 bit, FFT, RTD with MAX31865, Async MQTT client v2.0, binary data"));
  Serial.print(F(" MQTT broker: "));
  Serial.println(mqttServer);
  Serial.print(F(" Subscribed topics: "));
  Serial.printf("%s\t\t%s\n", inTopic, triggerTopic);
  Serial.print(F(" Publish CH0 FFT binary data on topic:  "));
  Serial.println(outTopic0);
  Serial.print(F(" Publish CH1 FFT binary data on topic:  "));
  Serial.println(outTopic1);
  Serial.print(F(" Publish RTD1 binary data on topic:  "));
  Serial.println(outTopic2);
  Serial.print(F(" and RTD1 fault state on topic:  "));
  Serial.println(outTopic3);
  Serial.print(F(" Publish RTD2 binary data on topic:  "));
  Serial.println(outTopic4);
  Serial.print(F(" and RTD2 fault state on topic:  "));
  Serial.println(outTopic5);
  Serial.print(F(" Publish MCP3204 access per frame on topic:  "));
  Serial.println(outTopic6);
  Serial.print(F(" Publish MCP3204 data on topic:  "));
  Serial.println(outTopic7);

  Serial.println(F("Tries automatic reconnection to MQTT in case of network errors."));
  Serial.println(F("Note: you can try MQTTLens to check MQTT functionalities"));
  Serial.println();
}