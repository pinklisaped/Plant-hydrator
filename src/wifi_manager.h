#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>
#include <settings.h>

void enableAPMode(const IPAddress &address = DEVICE_GATEWAY_IP);
void beginConnectWiFi(const String &ssid, const String &password, bool reboot = false);
bool endConnectWiFi();
void connectWiFi();
void saveWiFiConfig(const String &ssid, const String &password);
bool loadWiFiConfig();
bool deleteWiFiConfig();

#endif // WIFI_MANAGER_H
