#ifndef _ADS1256EXT_H
#define _ADS1256EXT_H

#include <Arduino.h>
#include <ADS1256.h>

class ADS1256Ext : public ADS1256 {
    public: 
        ADS1256Ext();  // constructor
        typedef enum {PGA_1 = 0, PGA_2 = 1, PGA_4 = 2, PGA_8 = 3, PGA_16 = 4, PGA_32 = 5, PGA_64 = 6} pgaGains;
        void setGain(pgaGains gain);
    private:
};

#endif