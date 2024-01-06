#include "HaMqttConfigBuilder.h"

HaMqttConfigBuilder::HaMqttConfigBuilder(
    const char *uniqueId,
    const char *fwName,
    const char *fwVersion,
    const char *fwManufacturer,
    const char *fwModel,
    const char *mqttPrefix,
    const char *availabilityTopic,
    const char *payloadAvailable,
    const char *payloadNotAvailable) : _uniqueId(uniqueId),
                                       _mqttPrefix(mqttPrefix),
                                       _availabilityTopic(String("~/") + availabilityTopic),
                                       _payloadAvailable(payloadAvailable),
                                       _payloadNotAvailable(payloadNotAvailable)
{
    _deviceId = _uniqueId.substring(6);
    _deviceInfo = add("ids", _uniqueId)
                      .add("name", fwName)
                      .add("sw", fwVersion)
                      .add("mf", fwManufacturer)
                      .add("mdl", fwModel)
                      .generatePayload();
    Serial.println(_uniqueId);
    Serial.println(_deviceId);
}

HaMqttConfigBuilder &HaMqttConfigBuilder::addDefaults(const char *name, const char *id, const char *stateTopic, const char *icon, const char *unit, const char *deviceClass)
{
    String uniq_id = id;
    uniq_id.toLowerCase();
    uniq_id.replace(" ", "_");

    clear();
    return add("name", name)
        .add("uniq_id", String(_deviceId) + String("_") + uniq_id)
        .add("dev_cla", deviceClass, false)
        .add("ic", icon, false)
        .add("unit_of_meas", unit, false)
        .addSource("dev", _deviceInfo)
        .add("~", _mqttPrefix)
        .add("avty_t", _availabilityTopic)
        .add("pl_avail", _payloadAvailable)
        .add("pl_not_avail", _payloadNotAvailable)
        .add("stat_t", String("~/") + stateTopic);
}

String HaMqttConfigBuilder::createLight(const char *friendlyName, const char *id, const char *stateTopic, const char *icon)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, "", icon, "")
            .add("cmd_t", String("~/") + stateTopic + String("/set"))
            .add("pl_off", "Off")
            .add("pl_on", "On")
            .generatePayload();

    sendConfig("light", id, config);
    return config;
}

String HaMqttConfigBuilder::createSelect(const char *friendlyName, const char *id, const char *stateTopic, const char *icon, const char *options)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, "", icon, "")
            .addSource("options", options)
            .add("cmd_t", String("~/") + stateTopic + String("/set"))
            .generatePayload();

    sendConfig("select", id, config);
    return config;
}

String HaMqttConfigBuilder::createSensor(const char *friendlyName, const char *id, const char *stateTopic, const char *icon, const char *unit, const char *deviceClass)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, unit, icon, deviceClass)
            .generatePayload();

    sendConfig("sensor", id, config);
    return config;
}

String HaMqttConfigBuilder::generatePayload()
{
    String payload = "{";

    for (Elem elem : _elements)
    {
        payload.concat('"');
        payload.concat(elem.key);
        payload.concat("\":");

        if (elem.str.length() > 0)
        {
            if (!elem.asSource)
                payload.concat('"');
            payload.concat(elem.str);
            if (!elem.asSource)
                payload.concat('"');
        }
        else
        {
            payload.concat(elem.num);
        }
        payload.concat(',');
    }

    if (payload.endsWith(","))
        payload.setCharAt(payload.length() - 1, '}');
    else
        payload.concat('}');

    return payload;
}

void HaMqttConfigBuilder::sendConfig(const char *confType, const char *id, const String &config)
{
    const uint8_t MAX_MQTT_LENGTH = 255;
    char mqttTopic[MAX_MQTT_LENGTH];
    snprintf(mqttTopic, MAX_MQTT_LENGTH, cHaDiscoveryTopic "/%s/%s/%s/config", confType, _mqttPrefix.c_str(), id);

    if (_sendCallback)
    {
        _sendCallback(mqttTopic, config.c_str());
    }
}
