#ifndef _MQTT_TOPICS_H
#define _MQTT_TOPICS_H

// MQTT client ID
#define thisClient "ESP32DevKit123" //"ESP32Udine"

// message topic to deal with
extern const char *inTopic;         // first subscribed topic
extern const char *triggerTopic;    // subscribed topic to start another acquisition
extern const char *pgaSetGainTopic; // subscribed topic to Set the PGA gain value
extern const char *outTopic0;       // topic to publish channel 0 FFT data on
extern const char *outTopic1;       // topic to publish channel 1 FFT data on
extern const char *outTopic2;       // topic to publish RTD1 data on
extern const char *outTopic3;       // topic to publish RTD1 fault state on
extern const char *outTopic4;       // topic to publish RTD2 data on
extern const char *outTopic5;       // topic to publish RTD2 fault state on
extern const char *outTopic6;       // topic to publish count of ADC torque access
extern const char *outTopic7;       // topic to publish 2 Torques and 2 Speeds
extern const char *pgaGetGainTopic; // topic to publish actual PGA gain value
#endif