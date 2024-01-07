#pragma once

#include "Arduino.h"

// Home Assistant integration

#define cDiscoveryTopic "homeassistant"
#define cDeviceTopic  "device"
#define cAvailabilityTopic "$state"
#define cPlAvailable "ready"
#define cPlNotAvailable "lost"

typedef std::function<void(const char *topic, const char *payload)> SendCallback;

// A helper class to generate a json Key/Value config payload for Home Assistant
class HaMqttConfigBuilder
{
private:
  struct Elem
  {
    String key;
    String str;
    double num;
    boolean asSource;
  };
  std::vector<Elem> _elements;

public:
  // Add an element to the config payload if it's not empty or if it is required
  HaMqttConfigBuilder &add(const String &key, const String &str, bool isRequired = true)
  {
    if (isRequired || (str.length() > 0))
    {
      _elements.push_back({key, str, 0, false});
    }
    return *this;
  }

  HaMqttConfigBuilder &add(const String &key, const double num)
  {
    _elements.push_back({key, "", num, false});
    return *this;
  }

  HaMqttConfigBuilder &addSource(const String &key, const String &str)
  {
    _elements.push_back({key, str, 0, true});
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

  HaMqttConfigBuilder &addDefaults(const char *name, const char *id, const char *stateTopic, const char *icon, const char *unit, const char *deviceClass);
  void sendConfig(const char *confType, const char *name, const String &config);

public:
  DeviceConfigBuilder(
      const char *uniqueId,
      const char *fwName,
      const char *fwVersion,
      const char *fwManufacturer,
      const char *fwModel);

  // Create different types of home assistant auto configurations
  String createLight(const char *name, const char *id, const char *stateTopic, const char *icon);
  String createSelect(const char *name, const char *id, const char *stateTopic, const char *icon, const char *options);
  String createSensor(const char *name, const char *id, const char *stateTopic, const char *icon, const char *unit, const char *deviceClass);

  DeviceConfigBuilder setDeviceTopic(char *deviceTopic)
  {
    _deviceTopic = deviceTopic;
    return *this;
  }

  DeviceConfigBuilder setAvailabilityTopic(const char *availabilityTopic)
  {
    _availabilityTopic = availabilityTopic;
    return *this;
  }

  DeviceConfigBuilder setPayloadAvailable(const char *payloadAvailable)
  {
    _payloadAvailable = payloadAvailable;
    return *this;
  }

  DeviceConfigBuilder setPayloadNotAvailable(const char *payloadNotAvailable)
  {
    _payloadNotAvailable = payloadNotAvailable;
    return *this;
  }

  DeviceConfigBuilder setSendCallback(SendCallback sendCallback)
  {
    _sendCallback = sendCallback;
    return *this;
  }
};