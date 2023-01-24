/*
  Example - Integration with Seeeduino WIO Terminal
  Created by Natthawat Raocharoensinp, January 23, 2023.

  Seeeduino Board Library Info: https://wiki.seeedstudio.com/Seeed_Arduino_Boards/
  Using Library
  - Seeed Arduino rpcWiFi 1.0.5

  Note
  - Conflict with TFT_eSPI by Bodmer (Remove before use)
*/

#include <TFT_eSPI.h>
#include <rpcWiFi.h>
#include <Sensesiot.h>

TFT_eSPI tft;
TFT_eSprite statusTextSprite = TFT_eSprite(&tft);
TFT_eSprite dataTextSprite = TFT_eSprite(&tft);
TFT_eSprite ledSprite = TFT_eSprite(&tft);

uint16_t bgColor = tft.color565(135, 206, 235);
uint16_t ledRedColor = tft.color565(155, 36, 35);
uint16_t ledGreenColor = tft.color565(28, 128, 81);

String statusText;
String dataText;
uint8_t btnStates;
uint8_t prevBtnStates;
bool ctrlState;
String ctrlStateText;

const char userid[] PROGMEM = "sensesiot-username";
const char key[] PROGMEM = "sensesiot-devicekey";
const char wifissid[] PROGMEM = "wifi-ssid";
const char wifipw[] PROGMEM = "wifi-password";

SensesiotClient sensesProtocol(userid, key);
void controlCallback(uint8_t slot, const char *state);

int dataToSent;
uint32_t sendTime;
uint32_t heldTime;

#define SENT_DATA_DELAY 2000
#define HELD_REPEAT_DELAY 250

void drawTextBuffer(TFT_eSprite *sprite, String text, int32_t x, int32_t y);
void drawStatusText(String text);
void drawDataText(String text);
void drawLedControl(bool state, String stateText);

void setRandomData();
void tuneDataForward();
void tuneDataReverse();

enum WIObutton
{
    WIO_BTN_A,
    WIO_BTN_B,
    WIO_BTN_C,
    WIO_BTN_5S_UP,
    WIO_BTN_5S_DOWN,
    WIO_BTN_5S_LEFT,
    WIO_BTN_5S_RIGHT,
    WIO_BTN_5S_CENTER,
};

uint8_t getWIOBtnStates();
bool checkWIOState(uint8_t state, WIObutton btn);

void setup()
{
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);

    Serial.begin(115200);

    statusTextSprite.createSprite(320, 28);
    statusTextSprite.fillSprite(bgColor);

    dataTextSprite.createSprite(320, 28);
    dataTextSprite.fillSprite(bgColor);

    ledSprite.createSprite(60, 60);
    ledSprite.fillSprite(bgColor);

    tft.init();
    tft.setRotation(3);

    tft.fillScreen(bgColor);

    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SensesIoT WIO Test", 10, 10);
    drawStatusText("Connecting...");

    sensesProtocol.begin(wifissid, wifipw);
    sensesProtocol.setControlCallback(controlCallback);
    sensesProtocol.subscribeControl(1);
    sensesProtocol.retainControl(1);

    Serial.println("SensesIoT WIO Ready!");

    setRandomData();
    sensesProtocol.setData(1, dataToSent);
    sendTime = millis();
}

void loop()
{
    prevBtnStates = btnStates;
    btnStates = getWIOBtnStates();

    if (checkWIOState(btnStates, WIO_BTN_A) && !checkWIOState(prevBtnStates, WIO_BTN_A))
    {
        if (!checkWIOState(prevBtnStates, WIO_BTN_A) || (millis() - heldTime) >= HELD_REPEAT_DELAY)
        {
            setRandomData();
            heldTime = millis();
        }
    }

    if (checkWIOState(btnStates, WIO_BTN_5S_LEFT))
    {
        if (!checkWIOState(prevBtnStates, WIO_BTN_5S_LEFT) || (millis() - heldTime) >= HELD_REPEAT_DELAY)
        {
            tuneDataReverse();
            heldTime = millis();
        }
    }

    if (checkWIOState(btnStates, WIO_BTN_5S_RIGHT))
    {
        if (!checkWIOState(prevBtnStates, WIO_BTN_5S_RIGHT) || (millis() - heldTime) >= HELD_REPEAT_DELAY)
        {
            tuneDataForward();
            heldTime = millis();
        }
    }

    dataText = String("Data: ");
    dataText.concat(dataToSent);
    drawDataText(dataText);

    drawLedControl(ctrlState, ctrlStateText);

    if (!sensesProtocol.ready())
    {
        drawStatusText("Reconnect...");
        sensesProtocol.begin(wifissid, wifipw);
        sensesProtocol.subscribeData(1);
        sensesProtocol.subscribeControl(1);
        sensesProtocol.retainControl(1);
    }
    else
    {
        drawStatusText("A: Set Data, L,R: Tune Data");
    }

    if (millis() - sendTime >= SENT_DATA_DELAY)
    {
        sensesProtocol.setData(1, dataToSent);
        sendTime = millis();
    }

    sensesProtocol.loop();
    delay(5);
}

void controlCallback(uint8_t slot, const char *state)
{
    if (slot != 1)
    {
        return;
    }

    ctrlStateText = String(state);
    Serial.print("State: ");
    Serial.println(state);

    // empty
    if (strlen(state) == 0)
    {
        ctrlState = !ctrlState;
        return;
    }

    // on/off
    if (strcmp(state, CONTROL_STATE_ON) == 0)
    {
        ctrlState = true;
        return;
    }
    else if (strcmp(state, CONTROL_STATE_OFF) == 0)
    {
        ctrlState = false;
        return;
    }

    // range
    int n = String(state).toInt();
    ctrlState = n > 0;
}

void drawTextBuffer(TFT_eSprite *sprite, String text, int32_t x, int32_t y)
{
    sprite->setTextFont(4);
    sprite->setTextColor(TFT_BLACK);
    sprite->setTextDatum(TL_DATUM);
    sprite->fillRect(0, 0, sprite->width(), sprite->height(), bgColor);
    sprite->drawString(text.c_str(), 10, 0);
    sprite->pushSprite(x, y);
}

void drawStatusText(String text)
{
    drawTextBuffer(&statusTextSprite, text, 0, 45);
}

void drawDataText(String text)
{
    drawTextBuffer(&dataTextSprite, text, 0, 80);
}

void drawLedControl(bool state, String stateText)
{
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("Ctrl1:", 70, 160);

    ledSprite.fillCircle(30, 30, 30, state ? ledGreenColor : ledRedColor);
    ledSprite.setTextFont(4);
    ledSprite.setTextColor(TFT_BLACK);
    ledSprite.setTextDatum(MC_DATUM);
    ledSprite.drawString(stateText.c_str(), 30, 30);
    ledSprite.pushSprite(170, 130);
}

void setRandomData()
{
    dataToSent = random(0, 100);
}

void tuneDataForward()
{
    dataToSent = (dataToSent < 100) ? (dataToSent + 1) : 0;
}

void tuneDataReverse()
{
    dataToSent = (dataToSent > 0) ? (dataToSent - 1) : 100;
}

uint8_t getWIOBtnStates()
{
    uint8_t state = 0;
    state |= (digitalRead(WIO_KEY_A) == LOW) << WIO_BTN_A;
    state |= (digitalRead(WIO_KEY_B) == LOW) << WIO_BTN_B;
    state |= (digitalRead(WIO_KEY_C) == LOW) << WIO_BTN_C;
    state |= (digitalRead(WIO_5S_UP) == LOW) << WIO_BTN_5S_UP;
    state |= (digitalRead(WIO_5S_DOWN) == LOW) << WIO_BTN_5S_DOWN;
    state |= (digitalRead(WIO_5S_LEFT) == LOW) << WIO_BTN_5S_LEFT;
    state |= (digitalRead(WIO_5S_RIGHT) == LOW) << WIO_BTN_5S_RIGHT;
    state |= (digitalRead(WIO_5S_PRESS) == LOW) << WIO_BTN_5S_CENTER;
    return state;
}

bool checkWIOState(uint8_t state, WIObutton btn)
{
    return (state >> btn) & 0x1;
}
