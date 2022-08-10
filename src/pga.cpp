#include <pga.h>
#include <MCP6S26.h>



// impostazioni del PGA0
volatile stPGA pga0 = {.channel = MCP6S26_CH0, .gain = MCP6S26_GAIN_1};

