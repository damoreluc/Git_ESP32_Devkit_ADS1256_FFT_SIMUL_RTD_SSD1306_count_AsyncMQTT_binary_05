#include <acquisition/acquisition.h>
#include <Arduino.h>
#include <stdio.h>

void buildPacket(char *s)
{
    tDati dati;

    // debug
    static long mintime = 5000;
    static long maxtime = 0;
    long adctime;
    long prevtime = micros();
    dati.ch1 = analogRead(pinCh1);
    adctime = micros()-prevtime;
    if(adctime < mintime) mintime = adctime;
    if(adctime > maxtime) maxtime = adctime;
    Serial.printf("T acq = (%3ld) %3ld (%3ld) us\n", mintime, adctime, maxtime);
    // fine debug

    dati.ch2 = analogRead(pinCh2);
    dati.ch3 = analogRead(pinCh3);
    dati.ch4 = analogRead(pinCh4);
    dati.ch5 = analogRead(pinCh5);
    dati.ch6 = analogRead(pinCh6);
    sprintf(s, "@;%010lu;%05u;%05u;%05u;%05u;%05u;%05u\n", millis(), dati.ch1, dati.ch2, dati.ch3, dati.ch4, dati.ch5, dati.ch6);
}