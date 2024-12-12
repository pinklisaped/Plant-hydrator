#include "wifi_manager.h"

#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <settings.h>

String _wizardWifiSSID;
String _wizardWifiPassword;
bool _connectingProcess = false;
bool _rebootESP = false;

bool checkAuth()
{
    if (LittleFS.exists("/conf/password.cfg"))
    {
        File file = LittleFS.open("/conf/password.cfg", "r");
        file.close();
        return true;
    }
    return false;
}

bool deleteWiFiConfig()
{
    return LittleFS.remove(WIFI_SETTINGS_PATH);
}

bool loadWiFiConfig()
{
    if (LittleFS.exists(WIFI_SETTINGS_PATH))
    {
        Serial.println("Load wifi config");
        File file = LittleFS.open(WIFI_SETTINGS_PATH, "r");
        String ssid = file.readStringUntil('\n');
        String pass = file.readStringUntil('\n');
        file.close();

        WiFi.hostname(DEVICE_NAME);
        beginConnectWiFi(ssid, pass);
        connectWiFi();
        return WiFi.isConnected();
    }
    return false;
}

void enableAPMode(const IPAddress &address)
{
    String wifi_pass = getWifiPassword(DEVICE_SERIAL);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(address, address, IPAddress(255, 255, 255, 0));
    WiFi.softAP(DEVICE_NAME, wifi_pass);
    Serial.println("WiFi start " + String(DEVICE_NAME) + " / " + wifi_pass);
}

void beginConnectWiFi(const String &ssid, const String &password, bool reboot)
{
    _wizardWifiSSID = ssid.c_str();
    _wizardWifiPassword = password.c_str();
    _connectingProcess = true;
    _rebootESP = reboot;
}

void connectWiFi()
{
    if (!_connectingProcess)
        return;
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wizardWifiSSID, _wizardWifiPassword);
    Serial.println("Connecting to WiFi..." + _wizardWifiSSID + " " + _wizardWifiPassword);

    int8_t status = WiFi.waitForConnectResult(20000);
    if (status == WL_CONNECTED)
    {
        Serial.println("WiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        saveWiFiConfig(_wizardWifiSSID, _wizardWifiPassword);
        if (_rebootESP)
            ESP.restart();
        _connectingProcess = false;
        return;
    }
    else if (status == WL_CONNECT_FAILED)
    {
        Serial.println("Connection failed. Check credentials.");
    }

    Serial.println("Failed to connect to WiFi. Returning to AP mode.");
    LittleFS.remove(WIFI_SETTINGS_PATH);
    enableAPMode();

    _connectingProcess = false;
}

bool endConnectWiFi()
{
    return WiFi.isConnected();
}

void saveWiFiConfig(const String &ssid, const String &password)
{
    Serial.println("Save wifi config");
    File file = LittleFS.open(WIFI_SETTINGS_PATH, "w");
    file.print(ssid + '\n');
    file.print(password + '\n');
    file.close();
    Serial.println("Wifi config saved!");
}
