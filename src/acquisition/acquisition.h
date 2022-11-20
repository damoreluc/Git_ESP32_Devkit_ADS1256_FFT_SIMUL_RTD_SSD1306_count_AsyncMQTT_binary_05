#ifndef _ACQUISITION_H
#define _ACQUISITION_H

// mappa pin analog.    Espressif ESP32 Dev module  |  Adafruit Huzzah32
#define pinCh1 36  //     ADC1_CH0 / VP / GPI36     |     A4 / GPI36
#define pinCh2 39  //     ADC1_CH3 / VN / GPI39     |     A3 / GPI39
#define pinCh3 32  //     ADC1_CH4 / GPIO32         |     A7 / GPIO32
#define pinCh4 33  //     ADC1_CH5 / GPIO33         |     A9 / GPIO33
#define pinCh5 34  //     ADC1_CH6 / GPI34          |     A2 / GPI34
#define pinCh6 35  //     ADC1_CH7 / GPIO35

typedef struct tDati {
    int ch1;
    int ch2;
    int ch3;
    int ch4;
    int ch5;
    int ch6;
} tDati;

void buildPacket(char *s);

#endif