#ifndef _DATA_H
#define _DATA_H

#include <Arduino.h>

// 1. ADS1256 and FFT section --------------------------------------------------
// number of FFT channels on ADS1256
#define CHANNELS_N 2

// ADS1256 sampling frequency
#define FSAMPLE 7500.0

// log2 of FFT size
#define FFT_N 12

// FFT_N Must be a power of 2
#define FFT_SIZE (1<<FFT_N)
#define FFT_HALF_SIZE (FFT_SIZE >> 1)
#define FFT_DOUBLE_SIZE (FFT_SIZE<<1)
#define MQTT_MAX_SIZE_BYTE 4096
#define BUFFER_SIZE_BYTE (FFT_SIZE * sizeof( float )/2)
#define BUFFER_FLOAT_SIZE_BYTE (FFT_SIZE * sizeof(float)/2)
#define BLOCKS (BUFFER_SIZE_BYTE / MQTT_MAX_SIZE_BYTE)
#define BLOCK_SIZE (FFT_SIZE / BLOCKS)

#define BLOCKS_FLOAT (BUFFER_FLOAT_SIZE_BYTE / MQTT_MAX_SIZE_BYTE)
#define BLOCK_FLOAT_SIZE (FFT_SIZE / BLOCKS_FLOAT)
#define BLOCK_FLOAT_HALF_SIZE (BLOCK_FLOAT_SIZE/2)

// 2. MCP3204 and torque/speed section ----------------------------------------------
#define MCP3204_NUMBER_OF_CHANNELS 4
#define MCP3204_PERIOD 10   // sampling period, in ms
#define MCP3204_NUMBER_OF_SAMPLE 50  //  must be < FFT_SIZE/FSAMPLE/MCP3204_PERIOD
#define MCP3204_BUFFER_SIZE (MCP3204_NUMBER_OF_SAMPLE * MCP3204_NUMBER_OF_CHANNELS)
#define MCP3204_BUFFER_HALF_SIZE ( MCP3204_BUFFER_SIZE >> 1 )
#endif