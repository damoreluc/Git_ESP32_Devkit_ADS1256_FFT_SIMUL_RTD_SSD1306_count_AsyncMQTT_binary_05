#include <wifi/wifiCallBacks.h>
#include <auxiliar/auxiliar_functions.h>
#include <auxiliar/ssd1306.h>
#include <AsyncMqttClient.h>
#include <time.h>
#include <HW_setup.h>
//#include <_modoCompilazione.h>
#include <acquisition/sim_real_data_selector.h>

// new Wifi events handler ------------------------------------------------------------------------
void WiFiEvent(WiFiEvent_t event)
{
  char s[21] = "";
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
      case SYSTEM_EVENT_WIFI_READY:
        Serial.println("WiFi ready");
        break; 
      case SYSTEM_EVENT_SCAN_DONE:
        Serial.println("WiFi finish scanning AP");
        break;  
      case SYSTEM_EVENT_STA_START:
        Serial.println("WiFi station start");
        break;  
      case SYSTEM_EVENT_STA_STOP:
        Serial.println("WiFi station stop");
        break; 
      case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("WiFi station connected to AP");
        break;  
      case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        Serial.println("The auth mode of AP connected by ESP32 station changed");
        break;   
      case SYSTEM_EVENT_STA_LOST_IP:
        Serial.println("WiFi station lost IP and the IP is reset to 0");
        break; 
      case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
        Serial.println("WiFi station connected BSS rssi goes below threshold");
        break;        
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("WiFi station wps succeeds in enrollee mode");
        break;
      case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("WiFi station wps fails in enrollee mode");
        break;
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("WiFi station wps timeout in enrollee mode");
        break;   
      case SYSTEM_EVENT_STA_WPS_ER_PIN:
        Serial.println("WiFi station wps pin code in enrollee mode");
        break; 
      case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP:
        Serial.println("WiFi station wps overlap in enrollee mode");
        break; 
                                                                                                                                                          
      case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        if (getSensMode() == REAL_DATA)
        {
          ssd1306_publish("Wi-Fi connected\n");
          sprintf(s, "IP %s\n", WiFi.localIP().toString().c_str());
          ssd1306_publish(s);
        }

        // init and get the local time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        printLocalTime();

        // connect to MQTT broker
        connectToMqtt();
        break;

      case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");

        if (getSensMode() == REAL_DATA)
        {
          ssd1306_publish("WiFi lost connection\n");
        }

        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;

      }
}