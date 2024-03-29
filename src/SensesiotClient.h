/*
  SensesiotClient.h - A library for send and receive data from "Sensesiot Platform"
  Created by Natthawat Raocharoensinp, November 21, 2022.
  Last Updated by Natthawat Raocharoensinp, August 28, 2023.
*/
#ifndef SensesiotClient_h
#define SensesiotClient_h

#define USERIDSTR_BUFFER_SIZE (33)
#define DEVKEYSTR_BUFFER_SIZE (65)
#define CLIENT_WIFI_BUFFER_SIZE (21)
#define CLIENT_USERNAME_BUFFER_SIZE (USERIDSTR_BUFFER_SIZE)
#define URL_BUFFER_SIZE (128)
#define MQTTTOPIC_BUFFER_SIZE (65 + DEVKEYSTR_BUFFER_SIZE)
#define FLOATSTR_BUFFER_SIZE (33)

#define CONTROL_STATE_ON "on"
#define CONTROL_STATE_OFF "off"

typedef void (*sensesiotDataCallback)(uint8_t, double);
typedef void (*sensesiotControlCallback)(uint8_t, const char *);

#include "Arduino.h"

#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>
#else
#include "WiFi.h"
#include "HTTPClient.h"
#endif

#include <MQTT.h>

class SensesiotClient
{
private:
  static const char *_API_HOST;
#if TEST_PROTOCOL
  static const uint16_t _MQTT_PORT = 4057;
#else
  static const uint16_t _MQTT_PORT = 3057;
#endif
  static const char *_DATA_TOPIC_START;
  static const char *_CONTROL_TOPIC_START;
  static const char *_RETAIN_TOPIC_START;

  String _userid = "";
  String _devicekey = "";
  WiFiClient _wifiClient;
  MQTTClient _mqttClient;

  sensesiotDataCallback _mqttDataCallback;
  sensesiotControlCallback _mqttControlCallback;

public:
  SensesiotClient(const char *devicekey);
  SensesiotClient(const char *userid, const char *devicekey);

  const char *getUserid();
  void setUserid(const char *userid);
  const char *getDevicekey();
  void setDevicekey(const char *userid);

  bool begin(const char *wifissid, const char *wifipw);
  wl_status_t wifiStatus();
  bool ready();
  void waitUntilReady();
  void disconnect();
  bool wifiDisconnect();
  bool mqttDisconnect();

  bool setData(uint8_t slot, double value);
  bool setControl(uint8_t slot, const char *state);
  bool setControl(uint8_t slot, String state);
  bool setControl(uint8_t slot, double value);
  bool retainData(uint8_t slot);
  bool retainControl(uint8_t slot);

  bool connectMqtt();
  void loop();
  lwmqtt_err_t mqttErrorStatus();

  bool subscribeData(uint8_t slot);
  bool subscribeControl(uint8_t slot);
  bool unsubscribeData(uint8_t slot);
  bool unsubscribeControl(uint8_t slot);

  void setDataCallback(sensesiotDataCallback callback);
  void setControlCallback(sensesiotControlCallback callback);

  SensesiotClient(String devicekey);
  SensesiotClient(String userid, String devicekey);
  void setUserid(String userid);
  void setDevicekey(String userid);

protected:
  void mqttCallback(String topic, String payload);
  bool isTopicStartWith(const char *topic, const char *str);
  uint8_t slotFromTopic(const char *topic);
};

#endif
