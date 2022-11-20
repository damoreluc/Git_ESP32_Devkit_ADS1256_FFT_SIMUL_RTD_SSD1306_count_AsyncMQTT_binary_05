#ifndef _MCP3204_H
#define _MCP3204_H

#include <Arduino.h>
#include <HW_setup.h>
#include <SPI.h>
#include <acquisition/dati.h>

typedef struct {
  float volt0;
  float volt1;
  float volt2;
  float volt3;
} mcp3204Data;

extern float mcp3204buffer[MCP3204_BUFFER_SIZE];
uint16_t mcp3204_getRaw(SPIClass &hwspi, uint8_t cs, uint8_t channel);
float mcp3204_getVoltage(SPIClass &hwspi, uint8_t cs, uint8_t channel);
void mcp3204_getAllVoltage(SPIClass &hwspi, uint8_t cs, mcp3204Data *data);
void mcp3204_StoreBuffer(mcp3204Data *data, uint16_t position);
bool mcp3204_Push(mcp3204Data *data);
void mcp3204_ResetBuffer();
uint16_t mcp3204_BufferAvailable();
#endif