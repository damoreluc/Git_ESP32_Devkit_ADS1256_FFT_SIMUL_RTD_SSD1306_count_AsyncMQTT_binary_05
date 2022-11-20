#include <acquisition/mcp3204.h>

// indice della prima posizione libera nell'array dei campionamenti
uint16_t mcp3204_pos = 0;
// array dei campionamenti con MCP3204 (4 blocchi sequenziali)
float mcp3204buffer[MCP3204_BUFFER_SIZE];

// costanti predefinite
const uint8_t MCP3204byteLength = 3;
const uint8_t command1 = 0b00000110;
const uint16_t command2 = (MCP3204_Torque1 & 0x03) << 14;
const uint16_t command3 = (MCP3204_Torque2 & 0x03) << 14;
const uint16_t command4 = (MCP3204_Speed1 & 0x03) << 14;
const uint16_t command5 = (MCP3204_Speed2 & 0x03) << 14;

const float qStep = MCP3204_VREF / 4096.0;

// dati da inviare a MCP3204 con transferBytes
const uint8_t c1[MCP3204byteLength] = {command1, (command2 >> 8), (command2 & 0x00ff)};
const uint8_t c2[MCP3204byteLength] = {command1, (command3 >> 8), (command3 & 0x00ff)};
const uint8_t c3[MCP3204byteLength] = {command1, (command4 >> 8), (command4 & 0x00ff)};
const uint8_t c4[MCP3204byteLength] = {command1, (command5 >> 8), (command5 & 0x00ff)};

// dati trasmessi dal MCP3204 con transferBytes
uint8_t o1[MCP3204byteLength];
uint8_t o2[MCP3204byteLength];
uint8_t o3[MCP3204byteLength];
uint8_t o4[MCP3204byteLength];

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

// VARIANTE della mcp3204_getAllVoltage
// uso il metodo  void transferBytes(const uint8_t * data, uint8_t * out, uint32_t size)
// dove:
//  const uint8_t * data            è l'array dei byte da inviare allo slave
//  uint8_t * out                   è l'array dei byte trasmessi dallo slave, stessa dimensione del precedente
//  uint32_t size                   è il numero di byte da scambiare

void mcp3204_getAllVoltage(SPIClass &hwspi, uint8_t cs, mcp3204Data *data) {

    uint16_t result[4];

    // canale Torque1
    digitalWrite(cs, LOW);
    hwspi.transferBytes(c1, o1, MCP3204byteLength);
    digitalWrite(cs, HIGH);

    // canale Torque2
    digitalWrite(cs, LOW);
    hwspi.transferBytes(c2, o2, MCP3204byteLength);   
    digitalWrite(cs, HIGH);

    // canale Speed1
    digitalWrite(cs, LOW);
    hwspi.transferBytes(c3, o3, MCP3204byteLength);
    digitalWrite(cs, HIGH);

    // canale Speed2
    digitalWrite(cs, LOW);
    hwspi.transferBytes(c4, o4, MCP3204byteLength);
    digitalWrite(cs, HIGH);

    // calcolo delle tensioni
    data->volt0 = (float)((((uint16_t)o1[1])<<8 | o1[2]) & 0x0fff) * qStep;
    data->volt1 = (float)((((uint16_t)o2[1])<<8 | o2[2]) & 0x0fff) * qStep;
    data->volt2 = (float)((((uint16_t)o3[1])<<8 | o3[2]) & 0x0fff) * qStep;
    data->volt3 = (float)((((uint16_t)o4[1])<<8 | o4[2]) & 0x0fff) * qStep;
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