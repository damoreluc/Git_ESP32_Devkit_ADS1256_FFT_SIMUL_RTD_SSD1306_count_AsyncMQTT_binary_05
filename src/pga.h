#ifndef _PGA_H
#define _PGA_H_H

#include <Arduino.h>

// impostazioni correnti del PGA MCP6S26
typedef struct
{
  uint8_t channel;
  uint8_t gain;
} stPGA;

extern volatile stPGA pga0;

#endif