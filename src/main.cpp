#define relay1Pin 13

#include <Arduino.h>
#include "RCSwitch.h"
#include "relay.h"
// #include <OLED_I2C.h>
// #define hello "Ghbdtn" // Привет
// OLED  myOLED(SDA, SCL, 8);

RCSwitch mySwitch = RCSwitch();
Relay relay1 = Relay(relay1Pin);

uint32_t currentTime; // Текущее время
int64_t recData = 0;

// extern uint8_t RusFont[];

void setup()
{
  Serial.begin(9600);
  relay1.setPermition();
  // myOLED.begin();
  // myOLED.setFont(RusFont);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2

  currentTime = millis();
}

void loop()
{
  if (mySwitch.available()) {
    recData = mySwitch.getReceivedValue();
    if(recData == 5393)
      relay1.cmdOpen = true;
    if(recData == 9678)
      relay1.cmdClose = true;
    mySwitch.resetAvailable();
  }

  relay1.open(1000);

  relay1.close(2000);

  // Опрос об ошибках раз в 5 сек. 
  if (millis() >= (currentTime + 5000))
  {
    currentTime = millis();
    if((relay1.getErrorStatus() & CMDOPCL) == CMDOPCL)
      relay1.clearErrorStatus();
  }
  // myOLED.clrScr();
  // myOLED.print(hello, CENTER, 0);
  // myOLED.update();
}