#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <IPAddress.h>

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#define DEVICE_SERIAL "00001"
#define DEVICE_NAME "ESP Hydrator " DEVICE_SERIAL

#define WIFI_SETTINGS_PATH "/conf/wifi.cfg"
#define DEVICE_SETTINGS_PATH "/conf/config.cfg"

enum class ConfigState : uint8_t
{
    UNCONFIGURED = 0,
    LOADED = 1,
    CONFIGURED = 2
};

extern IPAddress DEVICE_GATEWAY_IP;
extern uint32_t SPILL_INTERVAL_SEC;
extern uint16_t SPILL_DURATION_SEC;
extern ConfigState DEVICE_CONFIGURED;
String getWifiPassword(const String &input, const String &salt = "NzhiZDEwNzM4ZTI", int length = 8);

#endif // SETTINGS_H