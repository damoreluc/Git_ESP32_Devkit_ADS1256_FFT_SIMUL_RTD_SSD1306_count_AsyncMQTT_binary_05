#include <mcp3204.h>

// indice della prima posizione libera nell'array dei campionamenti
uint16_t mcp3204_pos = 0;
// array dei campionamenti con MCP3204 (4 blocchi sequenziali)
float mcp3204buffer[MCP3204_BUFFER_SIZE];

// costanti predefinite
const uint8_t command1 = 0b00000110;
const uint16_t command2 = (MCP3204_Torque1 & 0x03) << 14;
const uint16_t command3 = (MCP3204_Torque2 & 0x03) << 14;
const uint16_t command4 = (MCP3204_Speed1 & 0x03) << 14;
const uint16_t command5 = (MCP3204_Speed2 & 0x03) << 14;
const float qStep = MCP3204_VREF / 4096.0;

uint16_t mcp3204_BufferAvailable() {
    return (MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL - mcp3204_pos);
}

uint16_t mcp3204_getRaw(SPIClass &hwspi, uint8_t cs, uint8_t channel)
{
    uint16_t result;
    uint8_t command1 = 0b00000110;
    uint16_t command2 = (channel & 0x03) << 14;
    
    hwspi.begin();
    hwspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(cs, LOW);
    hwspi.transfer(command1);
    result = hwspi.transfer16(command2);
    digitalWrite(cs, HIGH);
    hwspi.endTransaction();

    return( result & 0x0fff);
}

float mcp3204_getVoltage(SPIClass &hwspi, uint8_t cs, uint8_t channel) {
    float v = mcp3204_getRaw(hwspi, cs, channel);

    return (v * qStep);
}

void mcp3204_getAllVoltage(SPIClass &hwspi, uint8_t cs, mcp3204Data *data) {

    uint16_t result[4];
    
    // hwspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));

    // canale Torque1
    digitalWrite(cs, LOW);
    hwspi.transfer(command1);
    result[0] = hwspi.transfer16(command2);
    digitalWrite(cs, HIGH);

    // canale Torque2
    digitalWrite(cs, LOW);
    hwspi.transfer(command1);
    result[1] = hwspi.transfer16(command3);
    digitalWrite(cs, HIGH);

    // canale Speed1
    digitalWrite(cs, LOW);
    hwspi.transfer(command1);
    result[2] = hwspi.transfer16(command4);
    digitalWrite(cs, HIGH);

    // canale Speed2
    digitalWrite(cs, LOW);
    hwspi.transfer(command1);
    result[3] = hwspi.transfer16(command5);
    digitalWrite(cs, HIGH);

    // hwspi.endTransaction();

    data->volt0 = (float)(result[0] & 0x0fff) * qStep;
    data->volt1 = (float)(result[1] & 0x0fff) * qStep;
    data->volt2 = (float)(result[2] & 0x0fff) * qStep;
    data->volt3 = (float)(result[3] & 0x0fff) * qStep;
}

void mcp3204_StoreBuffer(mcp3204Data *data, uint16_t position) {
    if (mcp3204_BufferAvailable() > 0) {
        mcp3204buffer[position] = data->volt0;
        mcp3204buffer[position + MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL] = data->volt1;
        mcp3204buffer[position + 2*MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL] = data->volt2;
        mcp3204buffer[position + 3*MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL] = data->volt3;
    }
}

bool mcp3204_Push(mcp3204Data *data) {
    if (mcp3204_BufferAvailable() > 0) {
      mcp3204_StoreBuffer(data, mcp3204_pos);
      mcp3204_pos++;
      return true;
    } else {
      return false;
    }
}

void mcp3204_ResetBuffer() {
    mcp3204_pos = 0;
}