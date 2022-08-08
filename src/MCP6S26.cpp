#include <MCP6S26.h>

// instruction set
#define MCP6S26_NOP 0
#define MCP6S26_SHUTDOWN 0x20
#define MCP6S26_WRITE 0x60
#define MCP6S26_CHANNEL_REG 1
#define MCP6S26_GAIN_REG 0

// put the PGA into shutdown mode
void mcp6s26_Shutdown(SPIClass &hwspi, uint8_t cs) {
    // Note 1: All other bits in the 16-bit word (including A0) are “don’t cares”.
    uint16_t cmd = ((uint16_t)MCP6S26_SHUTDOWN) << 8;

    digitalWrite(cs, LOW);
    hwspi.write16(cmd);
    digitalWrite(cs, HIGH);
}

// set the AMUX channel
void mcp6s26_setChannel(SPIClass &hwspi, uint8_t cs, uint8_t channel) {
    uint16_t cmd = ((uint16_t)MCP6S26_CHANNEL_REG) << 8;

    // check if channel is valid, else fall back to channel 0
    if (channel > MCP6S26_CH5 ) {
        channel = MCP6S26_CH0;
    }

    cmd |= channel;

    digitalWrite(cs, LOW);
    hwspi.write16(cmd);
    digitalWrite(cs, HIGH);
}

// set the PGA gain
void mcp6s26_setGain(SPIClass &hwspi, uint8_t cs, uint8_t gain) {
    uint16_t cmd = ((uint16_t)MCP6S26_GAIN_REG) << 8;

    // check if gain is valid, else fall back to gain = 1
    if (gain > MCP6S26_GAIN_32 ) {
        gain = MCP6S26_GAIN_1;
    }

    cmd |= gain;

    digitalWrite(cs, LOW);
    hwspi.write16(cmd);
    digitalWrite(cs, HIGH);
}