#include <sim_real_data_selector.h>

eSensMode sensMode = SYM_DATA;

// acquisisce il pin SENS_MODE e aggiorna lo stato dati_simulati/dati_reali
void readSensMode()
{
    pinMode(SENS_MODE, INPUT_PULLUP);

    if (digitalRead(SENS_MODE) == HIGH)
    {
        sensMode = REAL_DATA;
    }
}

// fornisce lo stato dati_simulati/dati_reali
eSensMode getSensMode()
{
    return sensMode;
}