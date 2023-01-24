#include "Arduino.h"
#include "SensesiotClient.h"

#if TEST_PROTOCOL
const char *SensesiotClient::_API_HOST = "test.sensesiot.net";
#else
const char *SensesiotClient::_API_HOST = "www.sensesiot.net";
#endif

const char *SensesiotClient::_CLIENT_MACADDR_FSTR = "wf-%02X:%02X:%02X:%02X:%02X:%02X";
const char *SensesiotClient::_MQTT_USERNAME_FSTR = "%s";
const char *SensesiotClient::_DATA_TOPIC_FSTR = "data/%s/%d";
const char *SensesiotClient::_CONTROL_TOPIC_FSTR = "control/%s/%d";
const char *SensesiotClient::_RETAIN_TOPIC_FSTR = "retain/%s/%s/%d";
const char *SensesiotClient::_DATA_TOPIC_START = "data/";
const char *SensesiotClient::_CONTROL_TOPIC_START = "control/";

SensesiotClient::SensesiotClient(const char *userid, const char *devicekey)
{
  setUserid(userid);
  setDevicekey(devicekey);
}

const char *SensesiotClient::getUserid()
{
  return _userid;
}

void SensesiotClient::setUserid(const char *userid)
{
  strncpy(_userid, userid, USERIDSTR_BUFFER_SIZE - 1);
  _devicekey[USERIDSTR_BUFFER_SIZE] = '\0';
}

const char *SensesiotClient::getDevicekey()
{
  return _devicekey;
}

void SensesiotClient::setDevicekey(const char *devicekey)
{
  strncpy(_devicekey, devicekey, DEVKEYSTR_BUFFER_SIZE - 1);
  _devicekey[DEVKEYSTR_BUFFER_SIZE - 1] = '\0';
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

bool SensesiotClient::setData(uint8_t slot, double value)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  char floatStrBuffer[FLOATSTR_BUFFER_SIZE];

  dtostrf(value, 0, 4, floatStrBuffer);
  sprintf(topicBuffer, _DATA_TOPIC_FSTR, _devicekey, slot);
  return _mqttClient.publish(topicBuffer, floatStrBuffer);
}

bool SensesiotClient::setControl(uint8_t slot, const char *stateStr)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _devicekey, slot);
  return _mqttClient.publish(topicBuffer, stateStr);
}

bool SensesiotClient::setControl(uint8_t slot, String stateStr)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _devicekey, slot);
  return _mqttClient.publish(topicBuffer, stateStr.c_str());
}

bool SensesiotClient::setControl(uint8_t slot, double value)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _devicekey, slot);
  return _mqttClient.publish(topicBuffer, String(value).c_str());
}

bool SensesiotClient::retainData(uint8_t slot)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];

  sprintf(topicBuffer, _RETAIN_TOPIC_FSTR, "data", _devicekey, slot);
  return _mqttClient.publish(topicBuffer, "");
}

bool SensesiotClient::retainControl(uint8_t slot)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];

  sprintf(topicBuffer, _RETAIN_TOPIC_FSTR, "control", _devicekey, slot);
  return _mqttClient.publish(topicBuffer, "");
}

bool SensesiotClient::connectMqtt()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[CLIENT_WIFI_BUFFER_SIZE];
  sprintf(macStr, _CLIENT_MACADDR_FSTR, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  char usernameStr[CLIENT_USERNAME_BUFFER_SIZE];
  sprintf(usernameStr, _MQTT_USERNAME_FSTR, _userid);

  bool status = _mqttClient.connect(macStr, usernameStr);

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
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _DATA_TOPIC_FSTR, _devicekey, slot);

  return _mqttClient.subscribe(topicBuffer);
}

bool SensesiotClient::subscribeControl(uint8_t slot)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _devicekey, slot);

  return _mqttClient.subscribe(topicBuffer);
}

bool SensesiotClient::unsubscribeData(uint8_t slot)
{
#ifndef ARDUINO_ARCH_ESP8266
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _DATA_TOPIC_FSTR, _devicekey, slot);

  return _mqttClient.unsubscribe(topicBuffer);
#endif
  return false;
}

bool SensesiotClient::unsubscribeControl(uint8_t slot)
{
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _devicekey, slot);

  return _mqttClient.unsubscribe(topicBuffer);
}

void SensesiotClient::setDataCallback(sensesiotDataCallback callback)
{
  _mqttDataCallback = callback;
}

void SensesiotClient::setControlCallback(sensesiotControlCallback callback)
{
  _mqttControlCallback = callback;
}

SensesiotClient::SensesiotClient(String userid, String devicekey)
{
  setUserid(userid);
  setDevicekey(devicekey);
}

void SensesiotClient::setUserid(String userid)
{
  strncpy(_userid, userid.c_str(), USERIDSTR_BUFFER_SIZE - 1);
  _devicekey[USERIDSTR_BUFFER_SIZE - 1] = '\0';
}

void SensesiotClient::setDevicekey(String devicekey)
{
  strncpy(_devicekey, devicekey.c_str(), DEVKEYSTR_BUFFER_SIZE - 1);
  _devicekey[DEVKEYSTR_BUFFER_SIZE - 1] = '\0';
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
  int len = strlen(str);
  char strBuffer[len + 1];
  strncpy(strBuffer, topic, len);
  strBuffer[len] = '\0';

  return strcmp(strBuffer, str) == 0;
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