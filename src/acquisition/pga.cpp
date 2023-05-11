#include <acquisition/pga.h>
#include <acquisition/MCP6S26.h>



// impostazioni del PGA0
stPGA pga0 = {.channel = MCP6S26_CH1, .gain = MCP6S26_GAIN_1, .gain_changed = true};

