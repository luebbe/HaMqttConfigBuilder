#pragma once

// Config payload generation for Home Assistant MQTT discovery
// Initial version by Patrick Lapointe <https://github.com/plapointe6>
// Upgrades by Lübbe Onken <https://github.com/luebbe>

#include "Arduino.h"
#include "ArduinoJson.h"

#define cDiscoveryTopic "homeassistant"
#define cDeviceTopic "device"
#define cAvailabilityTopic "$state"
#define cPlAvailable "ready"
#define cPlNotAvailable "lost"

typedef std::function<void(String topic, String payload)> SendCallback;

// A helper class to generate a json Key/Value config payload for Home Assistant
class HaMqttConfigBuilder
{
private:
  struct Elem
  {
    String key;
    boolean isStr;
    boolean isJson;
    String strVal;
    boolean isNum;
    double numVal;
    boolean isBool;
    boolean boolVal;
  };
  std::vector<Elem> _elements;

public:
  // Add an element to the config payload if it's not empty or if it is required
  HaMqttConfigBuilder &addStr(const String &key, const String &strVal, bool isRequired = true)
  {
    if (isRequired || (strVal.length() > 0))
    {
      _elements.push_back({key, true, false, strVal, false, 0, false, false});
    }
    return *this;
  }

  HaMqttConfigBuilder &addFloat(const String &key, const double numVal)
  {
    _elements.push_back({key, false, false, "", true, numVal, false, false});
    return *this;
  }

  HaMqttConfigBuilder &addBool(const String &key, const boolean boolVal)
  {
    _elements.push_back({key, false, false, "", false, 0, true, boolVal});
    return *this;
  }

  HaMqttConfigBuilder &addSource(const String &key, const String &str)
  {
    _elements.push_back({key, false, true, str, false, 0, false, false});
    return *this;
  }

  // Clear all elements
  void clear() { _elements.clear(); };

  // Generate the json key/value payload for home assistant
  String generatePayload();
};

class DeviceConfigBuilder : HaMqttConfigBuilder
{
private:
  String _uniqueId = "";
  String _deviceId = "";
  String _deviceInfo = "";
  String _discoveryTopic = cDiscoveryTopic;
  String _deviceTopic = cDeviceTopic;
  String _availabilityTopic = cAvailabilityTopic;
  String _payloadAvailable = cPlAvailable;
  String _payloadNotAvailable = cPlNotAvailable;

  SendCallback _sendCallback = NULL;

  HaMqttConfigBuilder &addDefaults(String friendlyName, String id, String stateTopic, String icon, String unit, String deviceClass);
  void sendConfig(String confType, String id, const String &config);

public:
  DeviceConfigBuilder(
      String uniqueId,
      String fwName,
      String fwVersion,
      String fwManufacturer,
      String fwModel);

  // Create different types of home assistant auto configurations
  String createLight(String name, String id, String stateTopic, String icon, bool supportsBrightness = false);
  String createSelect(String name, String id, String stateTopic, String icon, String options);
  String createSensor(String name, String id, String stateTopic, String icon, String unit, String deviceClass);
  String createSwitch(String name, String id, String stateTopic, String icon);

  DeviceConfigBuilder &setDeviceTopic(String value)
  {
    _deviceTopic = value;
    return *this;
  }

  DeviceConfigBuilder &setAvailabilityTopic(String value)
  {
    _availabilityTopic = value;
    return *this;
  }

  DeviceConfigBuilder &setPayloadAvailable(String value)
  {
    _payloadAvailable = value;
    return *this;
  }

  DeviceConfigBuilder &setPayloadNotAvailable(String value)
  {
    _payloadNotAvailable = value;
    return *this;
  }

  DeviceConfigBuilder &setSendCallback(SendCallback value)
  {
    _sendCallback = value;
    return *this;
  }
};