// elenco di tutti gli include
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
#include <auxiliar_functions.h>
#include <FFT.h>
#include <FFT_signal.h>
#include <fast_sqrt.h>
#include <ADS1256.h>
#include <ADS1256_equalizer.h>
#include <Adafruit_MAX31865.h>
#include <rtd_MAX31865.h>
#include <mcp3204.h>
#include <mqtt_topics.h>
#include <stdio.h>

// modificare questo file per cambiare la modalità di compilazione dati veri/simulati
#include <_modoCompilazione.h>