#ifndef _MCP6S26_H
#define _MCP6S26_H

#include <Arduino.h>
#include <SPI.h>

// refer to MCP6S26 datasheet - DS21117B-page 19

// Gain select bits
#define MCP6S26_GAIN_1  0
#define MCP6S26_GAIN_2  1
#define MCP6S26_GAIN_4  2
#define MCP6S26_GAIN_5  3
#define MCP6S26_GAIN_8  4
#define MCP6S26_GAIN_10 5
#define MCP6S26_GAIN_16 6
#define MCP6S26_GAIN_32 7

// Channel select bits
#define MCP6S26_CH0 0
#define MCP6S26_CH1 1
#define MCP6S26_CH2 2
#define MCP6S26_CH3 3
#define MCP6S26_CH4 4
#define MCP6S26_CH5 5

// put the PGA into shutdown mode
void mcp6s26_Shutdown(SPIClass &hwspi, uint8_t cs);

// set the AMUX channel
void mcp6s26_setChannel(SPIClass &hwspi, uint8_t cs, uint8_t channel);

// set the PGA gain
void mcp6s26_setGain(SPIClass &hwspi, uint8_t cs, uint8_t gain);

#endif