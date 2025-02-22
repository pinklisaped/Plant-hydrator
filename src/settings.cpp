#include <Arduino.h>
#include <Hash.h>
#include <IPAddress.h>
#include <settings.h>

IPAddress DEVICE_GATEWAY_IP(10, 0, 0, 1);
uint32_t SPILL_INTERVAL_SEC = 86400;
uint16_t SPILL_DURATION_SEC = 10;
ConfigState DEVICE_CONFIGURED = ConfigState::UNCONFIGURED;

String getWifiPassword(const String &input, const String &salt, int length)
{
    String hash_wifi_pass = sha1(salt + sha1(input));
    hash_wifi_pass.toLowerCase();
    return hash_wifi_pass.substring(4, 4 + length);
}
