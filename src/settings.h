#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <IPAddress.h>

#define DEVICE_SERIAL "12345678"
#define DEVICE_NAME "ESP Plant Hydrator"

#define WIFI_SETTINGS_PATH "/conf/wifi.cfg"
#define DEVICE_SETTINGS_PATH "/conf/config.cfg"

extern IPAddress DEVICE_GATEWAY_IP;
extern uint16_t INTERVAL_SEC;
extern uint16_t DURATION_SEC;
extern bool DEVICE_CONFIGURED;
String getWifiPassword(const String &input, const String &salt = "NzhiZDEwNzM4ZTI", int length = 8);

#endif // SETTINGS_H