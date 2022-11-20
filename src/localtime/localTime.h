#ifndef _LOCALTIME
#define _LOCALTIME

// NTP local time synchronization -----------------------------------------------------------------
// https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
extern const char *ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

#endif