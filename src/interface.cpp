#include "interface.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <settings.h>
#include <wifi_manager.h>

void startInterface(AsyncWebServer &server)
{
    server.on("/", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/interface.html", "text/html"); });

    server.on("/style.css", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/style.css", "text/css"); });

    server.on("/save_config", HTTP_POST, [](AsyncWebServerRequest *request) {
        String interval = request->getParam("interval", true)->value();
        String duration = request->getParam("duration", true)->value();
        saveDeviceConfig(interval, duration);
        request->send(200, "text/plain", "Saved!");
    });

    server.on("/get_config", HTTP_GET, [](AsyncWebServerRequest *request) {
        String response =
            "{\"interval\": " + String(SPILL_INTERVAL_SEC) + ", \"duration\": " + String(SPILL_DURATION_SEC) + "}";
        request->send(200, "application/json", response);
    });

    server.on("/reset_config", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (deleteDeviceConfig())
            request->send(200, "text/plain", "Config has been reset!");
        else
            request->send(404, "text/plain", "Config was not found!");
    });

    server.on("/reset_sifi", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (deleteWiFiConfig())
            request->send(200, "text/plain", "Wifi config has been reset! Device will be restarted");
        else
            request->send(404, "text/plain", "Wifi config was not found!");

        request->onDisconnect([]() { ESP.restart(); });
    });

    server.begin();
}

void saveDeviceConfig(const String &interval, const String &duration)
{
    SPILL_INTERVAL_SEC = interval.toInt();
    SPILL_DURATION_SEC = duration.toInt();
    File configFile = LittleFS.open(DEVICE_SETTINGS_PATH, "w");
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return;
    }

    configFile.print("interval=" + interval + '\n');
    configFile.print("duration=" + duration + '\n');
    configFile.close();
    DEVICE_CONFIGURED = ConfigState::CONFIGURED;
    Serial.println("Configuration saved");
}

bool deleteDeviceConfig()
{
    DEVICE_CONFIGURED = ConfigState::UNCONFIGURED;
    return LittleFS.remove(DEVICE_SETTINGS_PATH);
}

bool loadDeviceConfig()
{
    if (!LittleFS.exists(DEVICE_SETTINGS_PATH))
    {
        Serial.println("Config file not found, using defaults");
        return false;
    }

    File configFile = LittleFS.open(DEVICE_SETTINGS_PATH, "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
        return false;
    }

    while (configFile.available())
    {
        String line = configFile.readStringUntil('\n');
        line.trim();
        if (line.startsWith("interval="))
        {
            SPILL_INTERVAL_SEC = line.substring(9).toInt();
        }
        else if (line.startsWith("duration="))
        {
            SPILL_DURATION_SEC = line.substring(9).toInt();
        }
    }

    configFile.close();
    DEVICE_CONFIGURED = ConfigState::LOADED;
    Serial.println("Configuration loaded");
    return true;
}
