#ifndef _WIFICALLBACKS
#define _WIFICALLBACKS

#include <Arduino.h>
#include <WiFi.h>
#include <localtime/localTime.h>


/******************************************************************************
 * bunch of intermediata functions to ease wifi and MQTT support
 * 
 *****************************************************************************/

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

// Wifi events handler ----------------------------------------------------------------------------
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);

// fired when we got our IP address. Here it is safe to call connectToMqtt
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

// fired when connection to WiFi AP is lost
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// new Wifi events handler ------------------------------------------------------------------------
void WiFiEvent(WiFiEvent_t event);
#endif