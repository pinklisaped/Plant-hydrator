#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

bool isConfigExists;
void startInterface(AsyncWebServer &server);
void saveDeviceConfig(const String &interval, const String &duration);
void loadDeviceConfig();

#endif // INTERFACE_H
