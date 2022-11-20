#include <Arduino.h>
#include <acquisition/rtd_MAX31865.h>

// The value of the onboard Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 1000.0


bool getTemperature(Adafruit_MAX31865 &rtd, float *temperature, char *faultMsg)
{

    // Check and print any faults
    uint8_t fault = rtd.readFault();
    if (fault)
    {
        *temperature = 0.0;  // default value

        Serial.print("Fault 0x");
        Serial.println(fault, HEX);
        if (fault & MAX31865_FAULT_HIGHTHRESH)
        {
            strncpy(faultMsg,"RTD High Threshold", MSG_FAULT_LEN);
        }
        if (fault & MAX31865_FAULT_LOWTHRESH)
        {
            strncpy(faultMsg,"RTD Low Threshold", MSG_FAULT_LEN);
        }
        if (fault & MAX31865_FAULT_REFINLOW)
        {
            strncpy(faultMsg,"REFIN- > 0.85 x Bias", MSG_FAULT_LEN);
        }
        if (fault & MAX31865_FAULT_REFINHIGH)
        {
            strncpy(faultMsg,"REFIN- < 0.85 x Bias - FORCE- open", MSG_FAULT_LEN);
        }
        if (fault & MAX31865_FAULT_RTDINLOW)
        {
            strncpy(faultMsg,"RTDIN- < 0.85 x Bias - FORCE- open", MSG_FAULT_LEN);
        }
        if (fault & MAX31865_FAULT_OVUV)
        {
            strncpy(faultMsg,"Under/Over voltage", MSG_FAULT_LEN);
        }

        rtd.clearFault();
        return false;
    }
    else
    {
        uint16_t rtdCount = rtd.readRTD();

        // Serial.print("RTD value: ");
        // Serial.println(rtdCount);
        float ratio = rtdCount;
        ratio /= 32768.0;
        // Serial.print("Ratio = ");
        // Serial.println(ratio, 8);
        // Serial.print("Resistance = ");
        // Serial.println(RREF * ratio, 8);
        // Serial.print("Temperature = ");
        *temperature = rtd.temperature(RNOMINAL, RREF);
        // Serial.println(*temperature);
        strncpy(faultMsg,"Ok", MSG_FAULT_LEN);
        return true;
    }
}