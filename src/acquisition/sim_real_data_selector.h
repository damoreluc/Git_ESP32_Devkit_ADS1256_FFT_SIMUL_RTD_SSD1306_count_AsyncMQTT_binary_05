#ifndef _SIM_REAL_DATA_SEL
#define _SIM_REAL_DATA_SEL

#include <Arduino.h>
#include <HW_setup.h>

// modalità generazione dei dati
typedef enum {REAL_DATA, SYM_DATA} eSensMode;

// acquisisce il pin SENS_MODE e aggiorna lo stato dati_simulati/dati_reali
void readSensMode();

// fornisce lo stato dati_simulati/dati_reali
eSensMode getSensMode();

#endif