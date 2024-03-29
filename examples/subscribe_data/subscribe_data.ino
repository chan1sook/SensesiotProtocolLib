/*
  Example - Subscribe Data from Sensesiot Platform
  Created by Natthawat Raocharoensinp, November 21, 2022.
  Last Updated by Natthawat Raocharoensinp, August 28, 2023.
*/
#include <Sensesiot.h>

const char key[] = "sensesiot-devicekey";
const char wifissid[] = "wifi-ssid";
const char wifipw[] = "wifi-password";

SensesiotClient sensesProtocol(key);

void dataCallback(uint8_t slot, double value);
void controlCallback(uint8_t slot, const char *state);

void setup()
{
  Serial.begin(115200);

  sensesProtocol.begin(wifissid, wifipw);
  sensesProtocol.waitUntilReady();
  Serial.println(F("Connected"));

  sensesProtocol.setDataCallback(dataCallback);
  sensesProtocol.setControlCallback(controlCallback);

  sensesProtocol.subscribeData(1);
  sensesProtocol.subscribeControl(1);
}

void loop()
{
  if (!sensesProtocol.ready())
  {
    Serial.println(F("Disconnected"));

    sensesProtocol.begin(wifissid, wifipw);
    sensesProtocol.waitUntilReady();

    Serial.println(F("Connected"));

    sensesProtocol.subscribeData(1);
    sensesProtocol.subscribeControl(1);
  }
  sensesProtocol.loop();
}

void dataCallback(uint8_t slot, double value)
{
  Serial.print(F("Data: "));
  Serial.print(slot);
  Serial.print(' ');
  Serial.println(value);
}

void controlCallback(uint8_t slot, const char *state)
{
  Serial.print(F("Control: "));
  Serial.print(slot);
  Serial.print(' ');
  Serial.println(state);
}