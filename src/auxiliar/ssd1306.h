#ifndef _SSD1306_H
#define _SSD1306_H
#include <U8g2lib.h>

/*
 *  Mappa interfacciamento con display oled SSD1306 su I2C
 *    SSD106            ESP32
 *    GND               GND 
 *    Vcc               +3.3V
 *    SCL      <--      I2C_SCL (IO22)
 *    SDA      <->      I2C_SDA (IO21)
 */


// Create a U8g2log object
extern U8G2LOG u8g2log;

// display setup
void ssd1306_log_setup();

// publish message
void ssd1306_publish(const char *s);

#endif