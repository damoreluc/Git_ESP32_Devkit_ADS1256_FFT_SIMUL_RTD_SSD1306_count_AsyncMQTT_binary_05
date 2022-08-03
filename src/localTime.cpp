#include <localTime.h>

// NTP local time synchronization -----------------------------------------------------------------
// https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;