// Config payload generation for Home Assistant MQTT discovery
// Initial version by Patrick Lapointe <https://github.com/plapointe6>
// Upgrades by Lübbe Onken <https://github.com/luebbe>

#include "HaMqttConfigBuilder.h"

String HaMqttConfigBuilder::generatePayload()
{
    JsonDocument doc;

    for (Elem elem : _elements)
    {
        if (elem.isJson)
        {
            JsonDocument json;
            deserializeJson(json, elem.strVal);
            doc[elem.key] = json.as<JsonVariant>();
        }
        else if (elem.isStr)
            doc[elem.key] = elem.strVal;
        else if (elem.isNum)
            doc[elem.key] = elem.numVal;
        else if (elem.isBool)
            doc[elem.key] = elem.boolVal;
    }

    String payload;
#ifdef DEBUG
    serializeJsonPretty(doc, payload);
#else
    serializeJson(doc, payload);
#endif
    return payload;
}

DeviceConfigBuilder::DeviceConfigBuilder(
    String uniqueId,
    String fwName,
    String fwVersion,
    String fwManufacturer,
    String fwModel) : _uniqueId(uniqueId)
{
    _deviceId = _uniqueId.substring(6); // Copy the second half of the MAC address to use it as the short ID
    _deviceInfo =
        HaMqttConfigBuilder::addStr("ids", _uniqueId)
            .addStr("name", fwName + " " + _deviceId)
            .addStr("sw", fwVersion)
            .addStr("mf", fwManufacturer)
            .addStr("mdl", fwModel)
            .generatePayload();
}

HaMqttConfigBuilder &DeviceConfigBuilder::addDefaults(String friendlyName, String id, String stateTopic, String icon, String unit, String deviceClass)
{
    String uniq_id = id;
    uniq_id.toLowerCase();
    uniq_id.replace(" ", "_");

    clear();
    return addStr("name", friendlyName)
        .addStr("uniq_id", _deviceId + "_" + uniq_id)
        .addStr("dev_cla", deviceClass, false)
        .addStr("ic", icon, false)
        .addStr("unit_of_meas", unit, false)
        .addSource("dev", _deviceInfo)
        .addStr("~", _deviceTopic + "/" + _deviceId)
        .addStr("avty_t", "~/" + _availabilityTopic)
        .addStr("pl_avail", _payloadAvailable)
        .addStr("pl_not_avail", _payloadNotAvailable)
        .addStr("stat_t", "~/" + stateTopic);
}

String DeviceConfigBuilder::createLight(String friendlyName, String id, String stateTopic, String icon, bool supportsBrightness)
{
    addDefaults(friendlyName, id, stateTopic, icon, "", "")
        .addStr("cmd_t", "~/" + stateTopic + "/set")
        .addStr("pl_off", "Off")
        .addStr("pl_on", "On");

    if (supportsBrightness)
    {
        addStr("bri_stat_t", "~/" + stateTopic + "/brightness");
        addStr("bri_cmd_t", "~/" + stateTopic + "/brightness/set");
    }

    String config = generatePayload();

    sendConfig("light", id, config);
    return config;
}

String DeviceConfigBuilder::createSelect(String friendlyName, String id, String stateTopic, String icon, String options)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, icon, "", "")
            .addSource("options", options)
            .addStr("cmd_t", "~/" + stateTopic + "/set")
            .generatePayload();

    sendConfig("select", id, config);
    return config;
}

String DeviceConfigBuilder::createSensor(String friendlyName, String id, String stateTopic, String icon, String unit, String deviceClass)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, icon, unit, deviceClass)
            .generatePayload();

    sendConfig("sensor", id, config);
    return config;
}

String DeviceConfigBuilder::createSwitch(String friendlyName, String id, String stateTopic, String icon)
{
    String config =
        addDefaults(friendlyName, id, stateTopic, icon, "", "")
            .addStr("cmd_t", "~/" + stateTopic + "/set")
            .addStr("pl_off", "Off")
            .addStr("pl_on", "On")
            .generatePayload();

    sendConfig("switch", id, config);
    return config;
}

void DeviceConfigBuilder::sendConfig(String confType, String id, const String &config)
{
    String mqttTopic;

    // HA auto discovery topic format: <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
    mqttTopic =
        _discoveryTopic                        // <discovery_prefix>
        + "/" + confType                       // <component>
        + "/" + _deviceTopic + "_" + _deviceId // <node_id>
        + "/" + id + "/config";                // <object_id>/config

    if (_sendCallback)
    {
        _sendCallback(mqttTopic, config);
    }
}
