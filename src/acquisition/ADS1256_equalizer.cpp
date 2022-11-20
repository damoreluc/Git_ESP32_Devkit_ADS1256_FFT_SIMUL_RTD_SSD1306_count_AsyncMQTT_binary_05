#include <Arduino.h>
#include <acquisition/ADS1256_equalizer.h>
#include <acquisition/dati.h>

void create_equalizer(float *table)
{
    // parametri dell'equalizzatore
    uint16_t Num_Ave = 4;
    float R_lp = 1e2;
    float C_lp = 180e-9;
    float F_CLKIN = 7.68e6;

    float Ts = 1.0 / FSAMPLE;         // sample period
    float df = 1.0 / (Ts * FFT_SIZE); // FFT frequency resolution
    float I_F_CLKIN = 1.0 / F_CLKIN;
    float tau_lp = R_lp * C_lp;
    float f_lp = (1 / tau_lp) / (2 * PI);
    uint16_t i;
    float f, f_n;
    float aa, n1, d1, n2, d2;
    float H_sinc, H_averager, p;

    // calcolo tabella equalizzazione
    for (i = 0; i < FFT_HALF_SIZE; i++)
    {
        f = i * df;
        f_n = f * I_F_CLKIN;
        n1 = sin(256 * PI * f_n);
        d1 = 64 * sin(4 * PI * f_n);
        n2 = sin(256 * PI * Num_Ave * f_n);
        d2 = Num_Ave * sin(256 * PI * f_n);

        aa = abs(n1 / d1);
        H_sinc = aa * aa * aa * aa * aa; // aa^5
        H_averager = abs(n2 / d2);
        p = H_sinc * H_averager / sqrt(1 + (f / f_lp) * (f / f_lp));
        // se p == 0, assegna 1 di default
        table[i] = (p > 0) ? 1.0 / p : 1.0;
        // if ((i == 0) || (i == (FFT_HALF_SIZE - 1)))
        // {
        //     Serial.printf("table[%d] = %f\n", i, table[i]);
        // }
    }
}