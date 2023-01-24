/*
  Example - Publish Data to Sensesiot Platform
  Created by Natthawat Raocharoensinp, November 21, 2022.
  Last Updated by Natthawat Raocharoensinp, January 24, 2023.
*/
#include <Sensesiot.h>

const char userid[] = "sensesiot-username";
const char key[] = "sensesiot-devicekey";
const char wifissid[] = "wifi-ssid";
const char wifipw[] = "wifi-password";

SensesiotClient sensesProtocol(userid, key);

void setup()
{
  Serial.begin(115200);

  sensesProtocol.begin(wifissid, wifipw);
  sensesProtocol.waitUntilReady();
  Serial.println(F("Connected"));
}

void loop()
{
  if (!sensesProtocol.ready())
  {
    Serial.println(F("Disconnected"));

    sensesProtocol.begin(wifissid, wifipw);
    sensesProtocol.waitUntilReady();

    Serial.println(F("Connected"));
  }
  sensesProtocol.loop();

  sensesProtocol.setData(1, random(0, 100));
  // sensesProtocol.setControl(1, random(0, 100));

  Serial.println(F("Send Data"));
  delay(2000);
}
