#include "auxiliar_functions.h"
#include <mqtt_topics.h>
#include <ssd1306.h>
#include <sim_real_data_selector.h>
#include <MCP6S26.h>
#include <pga.h>

// Manage list of subscribed topics
// topic to subscribe
const uint8_t MAX_TOPIC_NAME = 100;
typedef struct tSubTopic
{
  char topic[MAX_TOPIC_NAME + 1];
  uint8_t qos;
  uint16_t pktId;
  tSubTopic *next;
} tSubTopic;

tSubTopic *pListOfTopics = NULL;

// add a new topic to the subscribed list (return TRUE on success)
bool AddSubscribedTopic(const char *topic, uint8_t qos)
{

  tSubTopic *item = (tSubTopic *)malloc(sizeof(tSubTopic));
  if (item == NULL)
  {
    return false;
  }
  else
  {
    strncpy(item->topic, topic, MAX_TOPIC_NAME);
    item->qos = qos;
    item->pktId = 0;

    // point to the old first one
    item->next = pListOfTopics;

    // point first to the new first node
    pListOfTopics = item;

    return true;
  }
}

// // NTP local time synchronization -----------------------------------------------------------------
// // https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
// const char *ntpServer = "pool.ntp.org";
// const long gmtOffset_sec = 3600;
// const int daylightOffset_sec = 3600;

// the MQTT client --------------------------------------------------------------------------------
AsyncMqttClient mqttClient;

// Auxiliar functions -----------------------------------------------------------------------

// FreeRTOS timer to force MQTT reconnection when WiFi is connected
TimerHandle_t mqttReconnectTimer;
// FreeRTOS timer to force WiFi reconnection
TimerHandle_t wifiReconnectTimer;

// set RTOS timers to handle automatic reconnection to WiFi / MQTT broker
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(timeout_ms), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

// pretty print local time and date
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println(F("Failed to obtain time"));
    return;
  }
  Serial.println(&timeinfo, "%A %d %B %Y %H:%M:%S");
}

// connect to WiFi network - specify your WiFi ID in perf.h file --------------
void connectToWifi(const char *_ssid, const char *_pswd)
{
  Serial.println(F("Connecting to Wi-Fi..."));
  WiFi.begin(_ssid, _pswd);
  Serial.println(WiFi.macAddress());

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connecting to Wi-Fi\n");
    char s[21] = "";
    sprintf(s, "MAC %s\n", WiFi.macAddress().c_str());
    ssd1306_publish(s);
  }
}

// MQTT event handlers ----------------------------------------------------------------------------
// begin connection to MQTT broker
void connectToMqtt()
{
  Serial.println(F("Connecting to MQTT..."));

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connecting to MQTT\n");
  }
  mqttClient.connect();
}

// handle onMqttConnect event: used to call subscribe topic- mandatory
// fired by mqttClient.onConnect
// Note: place here all the subscribing you need, use mqttClient.subscribe()
void onMqttConnect(bool sessionPresent)
{
  Serial.println(F("Connected to MQTT"));
  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connected to MQTT\n");
  }

  // traverse the list of topics to subscribe to
  tSubTopic *ptr = pListOfTopics;
  while (ptr != NULL)
  {
    ptr->pktId = mqttClient.subscribe(ptr->topic, ptr->qos);
    Serial.printf("Subscribing %s at QoS %d, packetId: %d\n", ptr->topic, ptr->qos, ptr->pktId);

    // point to the next item
    ptr = ptr->next;
  }

  // publish actual PGA0 gain
  if (mqttClient.connected())
  {
    uint16_t res = 0;
    char s[3];
    // on first connect or reconnect, set pga0 gain to x1
    pga0.gain = MCP6S26_GAIN_1;
    sprintf(s, "%1d", pga0.gain);
    do
    {
      res = mqttClient.publish(pgaGetGainTopic, 0, false, (const char *)&s[0], strlen(s));
      delay(10);
    } while (res == 0);
  }
}

// fired by mqttClient.onDisconnect and debug printing- mandatory
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Disconnected from MQTT\n");
  }

  Serial.print(F("Disconnected from MQTT. Reason:"));
  switch (reason)
  {
  case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
    Serial.println(F("TCP disconnected"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
    Serial.println(F("MQTT unacceptable protocolo version"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
    Serial.println(F("MQTT identifier rejected"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
    Serial.println(F("MQTT server unavailable"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
    Serial.println(F("MQTT malformed credentials"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
    Serial.println(F("MQTT not authorized"));
    break;
  case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
    Serial.println(F("ESP8266 not enough space"));
    break;
  case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
    Serial.println(F("TLS bad fingerprint"));
    break;
  default:
    Serial.println(F("unknown"));
    break;
  }

  // try reconnect to MQTT broker if WiFi is still ok
  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

// handle onMqttSubscribe event: usually print subscribe acknowledged - optional
// fired by mqttClient.onSubscribe
// useful when debugging subscription errors
void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.print(F("Subscribe acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.print(packetId);
  Serial.print(F("  qos: "));
  Serial.println(qos);
}

// handle onMqttUnsubscribe event: usually print unsubscribe acknowledged - optional
// fired by mqttClient.onUnsubscribe
void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println(F("Unsubscribe acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.println(packetId);
}

// handle onMqttPublish event: fired when message publish is acknowledged - optional
//   note: publish your message calling mqttClient.publish method
// fired by mqttClient.onPublish on publishing to MQTT broker
void onMqttPublish(uint16_t packetId)
{
  Serial.println(F("Publish acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.println(packetId);
}

// ---------------------------------------------------------------------------------------------
void printRcvMsg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{

  // print some information about the received message
  Serial.print(F("Publish received on "));
  printLocalTime();
  Serial.print(F("  topic: "));
  Serial.print(topic);
  Serial.print(F("  payload: "));
  for (int i = 0; i < len; i++)
  {
    Serial.print(payload[i]);
  }
  Serial.print("  qos: ");
  Serial.print(properties.qos);
  Serial.print(F("  dup: "));
  Serial.print(properties.dup);
  Serial.print(F("  retain: "));
  Serial.print(properties.retain);
  Serial.print(F("  len: "));
  Serial.print(len);
  Serial.print(F("  index: "));
  Serial.print(index);
  Serial.print(F("  total: "));
  Serial.println(total);
  Serial.println();
}