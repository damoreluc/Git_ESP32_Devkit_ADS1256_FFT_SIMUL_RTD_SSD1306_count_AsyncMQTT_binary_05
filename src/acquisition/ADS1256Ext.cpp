#include <acquisition/ADS1256.h>
#include <acquisition/ADS1256Ext.h>

#define ADS1256_ADCON 0x02

// constructor
ADS1256Ext::ADS1256Ext() : ADS1256() {} 

// set PGA gain
void  ADS1256Ext::setGain(pgaGains gain) {
    // read ADCON register
    byte adcon_reg = ADS1256_ADCON;
    byte adcon_data = readRegister(adcon_reg);

    // update gain bits
    adcon_data = (adcon_data & 0xF8) | gain;

    // write back ADC register
    writeRegister(adcon_reg, adcon_data);

}