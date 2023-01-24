/*
  Example - Control Device from Sensesiot Platform
  Created by Natthawat Raocharoensinp, November 21, 2022.
  Last Updated by Natthawat Raocharoensinp, January 24, 2023.
*/
#include <Sensesiot.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const char userid[] = "sensesiot-username";
const char key[] = "sensesiot-devicekey";
const char wifissid[] = "wifi-ssid";
const char wifipw[] = "wifi-password";

SensesiotClient sensesProtocol(userid, key);

void controlCallback(uint8_t slot, const char *state);

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  sensesProtocol.begin(wifissid, wifipw);
  sensesProtocol.waitUntilReady();
  Serial.println(F("Connected"));

  sensesProtocol.setControlCallback(controlCallback);
  sensesProtocol.subscribeControl(1);

  sensesProtocol.retainControl(1);
}

void loop()
{
  if (!sensesProtocol.ready())
  {
    Serial.println(F("Disconnected"));

    sensesProtocol.begin(wifissid, wifipw);
    sensesProtocol.waitUntilReady();

    Serial.println(F("Connected"));

    sensesProtocol.subscribeControl(1);
  }
  sensesProtocol.loop();
}

void controlCallback(uint8_t slot, const char *state)
{
  Serial.print(F("Get Control: "));
  Serial.print(slot);
  Serial.print(' ');
  Serial.println(state);

  if (slot != 1)
  {
    return;
  }

  if (strcmp(state, CONTROL_STATE_OFF) == 0)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if (strcmp(state, CONTROL_STATE_ON) == 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}