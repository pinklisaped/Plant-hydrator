#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <interface.h>
#include <settings.h>
#include <wifi_manager.h>
#include <wizard.h>

// Consts
const uint8_t SIGNAL_PIN = D5;
const uint8_t RESET_PIN = D6;
const uint32_t START_DELAY_MS = 5000;
const uint32_t SLEEP_DELAY_MS = 60000;

AsyncWebServer server(80);
bool isAuthenticated = false;

void handleReset()
{
    unsigned long pressStart = millis();

    // Wait press RESET_PIN
    while (digitalRead(RESET_PIN) == HIGH)
    {
        if (millis() - pressStart > 10000)
        { // 10 sec
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
    // pinMode(SIGNAL_PIN, OUTPUT);
    // pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(RESET_PIN, INPUT_PULLUP);

    // LittleFS init
    if (!LittleFS.begin())
    {
        Serial.println("Error mounting LittleFS");
        return;
    }

    // Run wizard
    if (!loadWiFiConfig())
    {
        Serial.println("Load wizard.");
        startWizard(server);
    }
    else
    {
        Serial.println("Load interface.");
        loadDeviceConfig();
        startInterface(server);
    }
}

void spill(uint16_t duration = DURATION_SEC);
void sleep(uint16_t interval = INTERVAL_SEC);

void loop()
{
    tickWizard();
    // handleReset();
    delay(500);
}

void spill(uint16_t duration_sec)
{
    digitalWrite(SIGNAL_PIN, HIGH);
    delay(duration_sec * 1000);
    digitalWrite(SIGNAL_PIN, LOW);
}

void sleep(uint16_t interval_sec)
{
    system_deep_sleep(interval_sec * 1000000);
}
