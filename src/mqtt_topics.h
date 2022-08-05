#ifndef _MQTT_TOPICS_H
#define _MQTT_TOPICS_H

// MQTT client ID
#define thisClient "ESP32DevKit123" //"ESP32Udine"

// message topic to deal with
const char *inTopic = thisClient "/inTopic";            // first subscribed topic
const char *triggerTopic = thisClient "/trigger";       // subscribed topic to start another acquisition
const char *outTopic0 = thisClient "/FFTBinTopic0";     // topic to publish channel 0 FFT data on
const char *outTopic1 = thisClient "/FFTBinTopic1";     // topic to publish channel 1 FFT data on
const char *outTopic2 = thisClient "/RTD1BinTopic";     // topic to publish RTD1 data on
const char *outTopic3 = thisClient "/RTD1FaultTopic";   // topic to publish RTD1 fault state on
const char *outTopic4 = thisClient "/RTD2BinTopic";     // topic to publish RTD2 data on
const char *outTopic5 = thisClient "/RTD2FaultTopic";   // topic to publish RTD2 fault state on
const char *outTopic6 = thisClient "/CountAdcTopic";    // topic to publish count of ADC torque access
const char *outTopic7 = thisClient "/TorqueSpeedTopic"; // topic to publish 2 Torques and 2 Speeds
#endif