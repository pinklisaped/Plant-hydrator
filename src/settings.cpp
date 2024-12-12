#include <Arduino.h>
#include <Hash.h>
#include <IPAddress.h>

IPAddress DEVICE_GATEWAY_IP(10, 0, 0, 1);
uint16_t INTERVAL_SEC = 1440;
uint16_t DURATION_SEC = 10;
bool DEVICE_CONFIGURED = false;

String getWifiPassword(const String &input, const String &salt, int length)
{
    String hash_wifi_pass = sha1(salt + sha1(input));
    hash_wifi_pass.toLowerCase();
    return hash_wifi_pass.substring(4, 4 + length);
}
