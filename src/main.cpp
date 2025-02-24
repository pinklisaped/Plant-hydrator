#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <interface.h>
#include <settings.h>
#include <wifi_manager.h>
#include <wizard.h>

// Consts
const uint8_t SIGNAL_PIN = D1;
const uint8_t UNSLEEP_PIN = D3;
const uint32_t START_DELAY_MS = 5000;
const uint32_t SLEEP_DELAY_MS = 60000;

AsyncWebServer server(80);
ulong minStartTimeMs;
ulong minSleepTimeMs;
bool isAuthenticated = false;
bool forcedSpill = false;
volatile bool delaySleep = false;

void spill(uint16_t duration = SPILL_DURATION_SEC);
void sleep(uint32_t interval = SPILL_INTERVAL_SEC);
void noSleepMiddleware(AsyncWebServerRequest *request, ArMiddlewareNext next);
void IRAM_ATTR btnUnsleep();

void handleReset()
{
    unsigned long pressStart = millis();

    // Wait press RESET_PIN
    while (digitalRead(UNSLEEP_PIN) == LOW)
    {
        if (millis() - pressStart > 10000)
        {
            Serial.println("Long press detected. Resetting configuration...");

            // Delete configs
            LittleFS.remove("/conf/wifi.cfg");
            LittleFS.remove("/conf/password.cfg");
            LittleFS.remove("/conf/config.cfg");

            // Reboot ESP
            ESP.restart();
        }
    }
    Serial.println("Short press detected. Reset skipped.");
}

void setup()
{
    Serial.begin(115200); // COM init
    pinMode(SIGNAL_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(UNSLEEP_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(UNSLEEP_PIN), btnUnsleep, FALLING);

    // LittleFS init
    if (!LittleFS.begin())
    {
        Serial.println("Error mounting LittleFS");
        return;
    }

    server.onNotFound([](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/style.css", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/style.css", "text/css"); });
    server.on("/common.js", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/www/common.js", "text/javascript"); });
    server.on("/serial", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(200, "text/plain", DEVICE_SERIAL); });
    server.on("/spill", HTTP_POST, [](AsyncWebServerRequest *request) {
        forcedSpill = true;
        request->send(200, "text/plain", "Success!");
    });

    // Run wizard
    if (!loadWiFiConfig())
    {
        Serial.println("Load wizard...");
        startWizard(server);
    }
    else
    {
        Serial.println("Load interface...");
        if (loadDeviceConfig())
        {
            minStartTimeMs = 5000;
            minSleepTimeMs = 6000;
        }
        startInterface(server);
    }

    server.addMiddleware(&noSleepMiddleware);
}

void loop()
{
    tickWizard();

    switch (DEVICE_CONFIGURED)
    {
    case ConfigState::UNCONFIGURED:
        minStartTimeMs = 0;
        minSleepTimeMs = 0;
        break;
    case ConfigState::CONFIGURED:
        if (minStartTimeMs == 0)
            minStartTimeMs = millis() + START_DELAY_MS;
        if (minSleepTimeMs == 0)
            minSleepTimeMs = millis() + SLEEP_DELAY_MS;
        break;
    case ConfigState::LOADED:
    default:
        break;
    }

    if ((minStartTimeMs > 1 && minStartTimeMs <= millis()) || forcedSpill)
    {
        spill(SPILL_DURATION_SEC);
        minStartTimeMs = 1;
    }

    if (minSleepTimeMs > 1)
    {
        if (delaySleep)
        {
            minSleepTimeMs = millis() + SLEEP_DELAY_MS;
            delaySleep = false;
            Serial.println("Sleep was delayed");
        }

        if (minSleepTimeMs <= millis())
            sleep(SPILL_INTERVAL_SEC);
    }
}

void spill(uint16_t duration_sec)
{
    Serial.println("Spill " + String(duration_sec) + " sec");
    digitalWrite(SIGNAL_PIN, HIGH);

    for (uint32_t i = 0; i < duration_sec; i++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(750);
    }
    digitalWrite(SIGNAL_PIN, LOW);
    forcedSpill = false;
}

void sleep(uint32_t interval_sec)
{
    Serial.println("Go sleep " + String(interval_sec) + " sec");
    ESP.deepSleep(interval_sec * 1000000);
}

void IRAM_ATTR btnUnsleep()
{
    delaySleep = true;
}

void noSleepMiddleware(AsyncWebServerRequest *request, ArMiddlewareNext next)
{
    if (minSleepTimeMs > 1)
        minSleepTimeMs = millis() + SLEEP_DELAY_MS / 2;
    next(); // continue processing
}
