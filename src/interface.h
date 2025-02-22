#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void startInterface(AsyncWebServer &server);
void saveDeviceConfig(const String &interval, const String &duration);
bool deleteDeviceConfig();
bool loadDeviceConfig();

#endif // INTERFACE_H
