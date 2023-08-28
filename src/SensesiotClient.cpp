#include "Arduino.h"
#include "SensesiotClient.h"

#if TEST_PROTOCOL
const char *SensesiotClient::_API_HOST = "test.sensesiot.net";
#else
const char *SensesiotClient::_API_HOST = "www.sensesiot.net";
#endif

const char *SensesiotClient::_DATA_TOPIC_START = "data/";
const char *SensesiotClient::_CONTROL_TOPIC_START = "control/";
const char *SensesiotClient::_RETAIN_TOPIC_START = "retain/";

SensesiotClient::SensesiotClient(const char *devicekey)
{
  setDevicekey(devicekey);
}

SensesiotClient::SensesiotClient(const char *userid, const char *devicekey)
{
  setUserid(userid);
  setDevicekey(devicekey);
}

const char *SensesiotClient::getUserid()
{
  return _userid.c_str();
}

void SensesiotClient::setUserid(const char *userid)
{
  this->setUserid(String(userid));
}

const char *SensesiotClient::getDevicekey()
{
  return _devicekey.c_str();
}

void SensesiotClient::setDevicekey(const char *devicekey)
{
  this->setDevicekey(String(devicekey));
}

bool SensesiotClient::begin(const char *wifissid, const char *wifipw)
{
  _mqttClient.begin(_API_HOST, _MQTT_PORT, _wifiClient);
  _mqttClient.onMessage([=](String topic, String payload)
                        { mqttCallback(topic, payload); });

  WiFi.begin(wifissid, wifipw);
  connectMqtt();

  return ready();
}

wl_status_t SensesiotClient::wifiStatus()
{
  return WiFi.status();
}

bool SensesiotClient::ready()
{
  return WiFi.isConnected() && _mqttClient.connected();
}

void SensesiotClient::waitUntilReady()
{
  while (!WiFi.isConnected())
  {
#if defined(ESP8266)
    delay(10);
#endif
  }
  while (!_mqttClient.connected())
  {
#if defined(ESP8266)
    delay(10);
#endif
    connectMqtt();
  }
}

void SensesiotClient::disconnect()
{
  mqttDisconnect();
  wifiDisconnect();
}

bool SensesiotClient::wifiDisconnect()
{
  return WiFi.disconnect();
}

bool SensesiotClient::mqttDisconnect()
{
  return _mqttClient.disconnect();
}

bool SensesiotClient::setData(uint8_t slot, double value)
{
  String topicStr = _DATA_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);

  String payloadStr = String(value);
  return _mqttClient.publish(topicStr.c_str(), payloadStr.c_str());
}

bool SensesiotClient::setControl(uint8_t slot, const char *stateStr)
{
  String topicStr = _CONTROL_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);

  return _mqttClient.publish(topicStr.c_str(), stateStr);
}

bool SensesiotClient::setControl(uint8_t slot, String stateStr)
{
  return this->setControl(slot, stateStr.c_str());
}

bool SensesiotClient::setControl(uint8_t slot, double value)
{
  String payloadStr = String(value);
  return this->setControl(slot, payloadStr.c_str());
}

bool SensesiotClient::retainData(uint8_t slot)
{
  String topicStr = _RETAIN_TOPIC_START;
  topicStr.concat(_DATA_TOPIC_START);
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);

  return _mqttClient.publish(topicStr.c_str(), "");
}

bool SensesiotClient::retainControl(uint8_t slot)
{
  String topicStr = _RETAIN_TOPIC_START;
  topicStr.concat(_CONTROL_TOPIC_START);
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);

  return _mqttClient.publish(topicStr.c_str(), "");
}

bool SensesiotClient::connectMqtt()
{
  String macStr = "wf-";
  macStr.concat(WiFi.macAddress());

  String usernameStr = this->_devicekey;
  bool status = _mqttClient.connect(macStr.c_str(), usernameStr.c_str());

  return status;
}

void SensesiotClient::loop()
{
  _mqttClient.loop();
}

lwmqtt_err_t SensesiotClient::mqttErrorStatus()
{
  return _mqttClient.lastError();
}

bool SensesiotClient::subscribeData(uint8_t slot)
{
  String topicStr = _DATA_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);
  return _mqttClient.subscribe(topicStr.c_str());
}

bool SensesiotClient::subscribeControl(uint8_t slot)
{
  String topicStr = _CONTROL_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);
  return _mqttClient.subscribe(topicStr.c_str());
}

bool SensesiotClient::unsubscribeData(uint8_t slot)
{
  String topicStr = _DATA_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);
  return _mqttClient.unsubscribe(topicStr.c_str());
}

bool SensesiotClient::unsubscribeControl(uint8_t slot)
{
  String topicStr = _CONTROL_TOPIC_START;
  topicStr.concat(_devicekey);
  topicStr.concat("/");
  topicStr.concat(slot);
  return _mqttClient.unsubscribe(topicStr.c_str());
}

void SensesiotClient::setDataCallback(sensesiotDataCallback callback)
{
  _mqttDataCallback = callback;
}

void SensesiotClient::setControlCallback(sensesiotControlCallback callback)
{
  _mqttControlCallback = callback;
}

SensesiotClient::SensesiotClient(String devicekey)
{
  setDevicekey(devicekey);
}

SensesiotClient::SensesiotClient(String userid, String devicekey)
{
  setUserid(userid);
  setDevicekey(devicekey);
}

void SensesiotClient::setUserid(String userid)
{
  this->_userid.clear();
  this->_userid.concat(userid);
}

void SensesiotClient::setDevicekey(String devicekey)
{
  this->_devicekey.clear();
  this->_devicekey.concat(devicekey);
}

void SensesiotClient::mqttCallback(String topic, String payload)
{
  uint8_t slot;

  const char *topicStr = topic.c_str();
  const char *payloadStr = payload.c_str();

  if (isTopicStartWith(topicStr, _DATA_TOPIC_START) && _mqttDataCallback != NULL)
  {
    slot = slotFromTopic(topicStr);

    double value = atof(payloadStr);

    _mqttDataCallback(slot, value);

    return;
  }

  if (isTopicStartWith(topicStr, _CONTROL_TOPIC_START) && _mqttControlCallback != NULL)
  {
    slot = slotFromTopic(topicStr);
    _mqttControlCallback(slot, payloadStr);
  }
}

bool SensesiotClient::isTopicStartWith(const char *topic, const char *str)
{
  return String(topic).startsWith(str);
}

uint8_t SensesiotClient::slotFromTopic(const char *topic)
{
  int len = strlen(topic);
  int start = len;
  for (; start >= 0; start--)
  {
    if (topic[start] == '/')
    {
      break;
    }
  }

  return atoi(topic + start + 1);
}