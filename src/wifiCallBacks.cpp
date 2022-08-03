#include <wifiCallBacks.h>
#include <auxiliar_functions.h>
#include <ssd1306.h>
#include <AsyncMqttClient.h>
#include <time.h>
#include <HW_setup.h>
#include <_modoCompilazione.h>

// new Wifi events handler ------------------------------------------------------------------------
void WiFiEvent(WiFiEvent_t event)
{
        char s[21] = "";
        Serial.printf("[WiFi-event] event: %d\n", event);
        switch (event)
        {
        case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("WiFi connected");
                Serial.println("IP address: ");
                Serial.println(WiFi.localIP());

#ifndef SIMULATE_DATA
                ssd1306_publish("Wi-Fi connected\n");
                sprintf(s, "IP %s\n", WiFi.localIP().toString().c_str());
                ssd1306_publish(s);
#endif

                // init and get the local time
                configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                printLocalTime();

                // connect to MQTT broker
                connectToMqtt();
                break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println("WiFi lost connection");

#ifndef SIMULATE_DATA
                ssd1306_publish("WiFi lost connection\n");
#endif

                xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
                xTimerStart(wifiReconnectTimer, 0);
                break;
        }
}