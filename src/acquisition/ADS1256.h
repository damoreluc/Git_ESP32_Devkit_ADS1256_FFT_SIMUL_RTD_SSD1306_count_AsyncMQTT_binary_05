/*
 Name:		ADS1256.h
 Created:	11/21/2018 5:24:56 PM
 Author:	Yordan
*/

#ifndef _ADS1256_h
#define _ADS1256_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include "SPI.h"
#include "esp32-hal-gpio.h"

class ADS1256
{
	public:
		unsigned long adcValues[ 8 ];
		unsigned long adc_Raws[ 8 ] = { 0,0,0,0,0,0,0,0 };

		// MUX : Input Multiplexer Control Register (Address 01h)
		// Bits 7-4 PSEL3, PSEL2, PSEL1, PSEL0: Positive Input Channel (AINP) Select
		// 0000 = AIN0 (default)
		// 0001 = AIN1
		// 0010 = AIN2 (ADS1256 only)
		// 0011 = AIN3 (ADS1256 only)
		// 0100 = AIN4 (ADS1256 only)
		// 0101 = AIN5 (ADS1256 only)
		// 0110 = AIN6 (ADS1256 only)
		// 0111 = AIN7 (ADS1256 only)
		// 1xxx = AINCOM (when PSEL3 = 1, PSEL2, PSEL1, PSEL0 are “don’t care”)

		// Bits 3-0 NSEL3, NSEL2, NSEL1, NSEL0: Negative Input Channel (AINN) Select
		// 0000 = AIN0 (default)
		// 0001 = AIN1
		// 0010 = AIN2 (ADS1256 only)
		// 0011 = AIN3 (ADS1256 only)
		// 0100 = AIN4 (ADS1256 only)
		// 0101 = AIN5 (ADS1256 only)
		// 0110 = AIN6 (ADS1256 only)
		// 0111 = AIN7 (ADS1256 only)
		// 1xxx = AINCOM (when NSEL3 = 1, NSEL2, NSEL1, NSEL0 are “don’t care”)
        // single reference: 
		//       AIN0-AINCOM, AIN1-AINCOM, AIN2-AINCOM, AIN3-AINCOM,
		//       AIN4-AINCOM, AIN5-AINCOM, AIN6-AINCOM, AIN7-AINCOM,		
		byte ads1256_mux[ 8 ] = { 0x08,0x18,0x28,0x38,0x48,0x58,0x68,0x78 };
		// differential input:
		//       AIN0-AIN1, AIN2-AIN3, AIN4-AIN5, AIN6-AIN7
		byte ads1256_dmux[ 4 ] = { 0x01,0x23,0x45,0x67 }; 

		ADS1256();

		void init( SPIClass adsSpi, uint8_t _pinCS, uint8_t _pinRDY, uint8_t _pinRESET, uint32_t _speedSPI );
		void readInputToAdcValuesArray();

		//void readInputPEAKSToAdcValuesArray( int repeats );
        uint8_t readStatus();
		uint8_t readDataRate();
		uint32_t IRAM_ATTR ReadRawData();
		void standby();
		void wakeup();
		void sync();
		void reset();
		void setChannel(uint8_t channel);
		float IRAM_ATTR volt(uint32_t rawData);

	   uint8_t readRegister(uint8_t reg);
	   void    writeRegister(uint8_t reg, uint8_t value);

	   void selfcal();

	private:
        SPIClass _adsSpi;
    	uint8_t _pinCS;
		uint8_t _pinRDY;
		uint8_t _pinRESET;
		uint32_t _speedSPI;
	
	//    uint8_t readRegister(uint8_t reg);
	//    void    writeRegister(uint8_t reg, uint8_t value);
};


#endif
