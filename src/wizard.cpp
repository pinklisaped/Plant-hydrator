#include "wizard.h"

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <settings.h>
#include <wifi_manager.h>

DNSServer dnsServer;
bool isWizardStarted = false;
volatile bool _isWifiScanning = false;

unsigned long _lastMillisDiode = 0;
unsigned long _lastMillisDns = 0;

void startWizard(AsyncWebServer &server)
{
    isWizardStarted = true;

    dnsServer.start(53, "*", DEVICE_GATEWAY_IP); // DNS spoofing (Only for HTTP)
    enableAPMode();

    Serial.println("Setting up Wi-Fi Wizard...");
    WiFi.scanNetworks(true);

    server.onNotFound([](AsyncWebServerRequest *request) { request->redirect("/"); });

    server.on("/", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/wizard.html", "text/html"); });

    server.on("/style.css", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/style.css", "text/css"); });

    // Wi-Fi scan
    server.on("/start_scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!_isWifiScanning)
        {
            WiFi.scanDelete();
            WiFi.scanNetworks(true); // Async scan
            _isWifiScanning = true;
        }
        request->send(200, "application/json", "{\"status\":\"started\"}");
    });

    // Get SSID list
    server.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (_isWifiScanning && WiFi.scanComplete() == WIFI_SCAN_RUNNING)
        {
            request->send(200, "application/json", "{\"status\":\"in_progress\"}");
        }
        else if (_isWifiScanning && WiFi.scanComplete() >= 0)
        {
            _isWifiScanning = false;
            int n = WiFi.scanComplete();
            String networks = "{\"status\":\"done\", \"networks\":[";
            for (int i = 0; i < n; i++)
            {
                networks += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
                if (i < n - 1)
                    networks += ",";
            }
            networks += "]}";
            request->send(200, "application/json", networks);
        }
        else
        {
            request->send(500, "application/json", "{\"status\":\"error\"}");
        }
    });

    server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        String ssid = request->getParam("ssid", true)->value();
        String pass = request->getParam("password", true)->value();
        request->send(200, "text/html", "Please wait, when device are connecting...");

        beginConnectWiFi(ssid, pass, true);
    });

    server.begin();
}

void tickWizard()
{
    if (isWizardStarted)
    {
        unsigned long currentMillis = millis();
        // if (currentMillis - _lastMillisDiode >= 700) {
        //   _lastMillisDiode = currentMillis;
        //   digitalWrite(LED_BUILTIN, LOW);
        //   delay(100);
        //   digitalWrite(LED_BUILTIN, HIGH);
        // }
        if (currentMillis - _lastMillisDns >= 200)
        {
            _lastMillisDns = currentMillis;
            dnsServer.processNextRequest();
        }
        connectWiFi();
    }
}
