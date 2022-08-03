#ifndef _AUXILIAR_FUNCTIONS_H
#define _AUXILIAR_FUNCTIONS_H 

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "time.h"
#include "localTime.h"
#include "HW_setup.h"

/******************************************************************************
 * bunch of intermediata functions to ease wifi and MQTT support
 * 
 *****************************************************************************/

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

// FreeRTOS timer to force MQTT reconnection when WiFi is connected
extern TimerHandle_t mqttReconnectTimer;
// FreeRTOS timer to force WiFi reconnection
extern TimerHandle_t wifiReconnectTimer;

// add a new topic to the subscribed list (return TRUE on success)
bool AddSubscribedTopic(const char* topic, uint8_t qos);

// set RTOS timers to handle automatic reconnection to WiFi / MQTT broker
void setTimersRTOS(uint16_t timeout_ms);

// auxiliar functions definition ------------------------------------------------------------------
// pretty print local time and date
void printLocalTime();

// connect to WiFi network - specify your WiFi ID in perf.h file --------------
void connectToWifi(const char* _ssid, const char* _pswd);

// // wifi event handler -----------------------------------------------------------------------------
// void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
// void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info); // <--- call connectToMqtt() when we've got our IP address
// void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// the MQTT client --------------------------------------------------------------------------------
extern AsyncMqttClient mqttClient;

// MQTT event handlers ----------------------------------------------------------------------------
// begin connection to MQTT broker
void connectToMqtt();
// handle onMqttConnect event: used to call subscribe topic- mandatory
void onMqttConnect(bool sessionPresent);
// handle onMqttDisconnect event- mandatory
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
// handle onMqttSubscribe event: usually print subscribe acknowledged - optional
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
// handle onMqttUnsubscribe event: usually print unsubscribe acknowledged - optional
void onMqttUnsubscribe(uint16_t packetId);
// handle onMqttMessage event: fired on receiving new message from a subscribed topic - mandatory
// to be implemented on user's code (eg. in main.cpp file)
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
// handle onMqttPublish event: fired when message publish is acknowledged - optional
//   note: publish your message calling mqttClient.publish method
void onMqttPublish(uint16_t packetId);

#endif