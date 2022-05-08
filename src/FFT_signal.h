#include <dati.h>

double Ts = 1.0/((float)FSAMPLE);
double df = 1.0 / (Ts * FFT_SIZE);

const double ONE_OVER_FFT_SIZE = 1.0 / ((double)FFT_SIZE);

//The time in which data was captured. This is equal to FFT_SIZE/sampling_freq
const float TOTAL_TIME=(float)FFT_SIZE/(float)FSAMPLE;
 
//float fft_input[FFT_SIZE];
//float fft_output[FFT_SIZE];

float max_magnitude = 0;
float fundamental_freq = 0;
