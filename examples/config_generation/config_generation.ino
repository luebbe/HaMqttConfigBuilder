#include "Arduino.h"
#include "HaMqttConfigBuilder.h"

String generateDeviceConfig()
{
    return DeviceConfigBuilder("AA:BB:CC:DD:EE:FF", "MyDevice", "1.0.0", "MyCompany", "MyModel")
        .createLight("Living Room Light", "living_room_light", "living_room/light/state", "mdi:lightbulb", true);
}
String generateDeviceAttributes()
{
    return HaMqttConfigBuilder()
        .addStr("IP", "192.168.0.100")
        .addFloat("Temperature", 23.5)
        .addBool("Online", true)    
        .generatePayload();
}

void setup()
{
    Serial.begin(SERIAL_SPEED);
    Serial.println();
    Serial.println("--------------------------------");
    Serial.println(generateDeviceConfig());
    Serial.println("--------------------------------");
    Serial.println(generateDeviceAttributes());
    Serial.println("--------------------------------");
}

void loop()
{
}
