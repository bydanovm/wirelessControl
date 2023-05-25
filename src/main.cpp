#include <Arduino.h>
#include "RCSwitch.h"
#include "relayOut.h"
#include "relayIn.h"
// #include <OLED_I2C.h>
// #define hello "Ghbdtn" // Привет
// OLED  myOLED(SDA, SCL, 8);

RCSwitch mySwitch = RCSwitch();

RelayOut relayMotorOpen = RelayOut(relayMotorOpen1Pin, OUTPUT);
RelayOut relayMotorClose = RelayOut(relayMotorClose1Pin, OUTPUT);

RelayIn * RelayIn::instances[4] = {NULL, NULL};
RelayIn photoRelay = RelayIn(inRelay3Pin, INPUT_PULLUP);

uint32_t currentTime5S; // Текущее время
uint32_t currentTime1S; // Текущее время
unsigned long recData = 0;
bool cmdOpen, cmdClose, cmdStopClose = false;
bool isOpening, isClosing = false;
bool isOpened, isClosed = false;

// extern uint8_t RusFont[];

void setup()
{
  Serial.begin(115200);
  relayMotorOpen.setPermition();
  relayMotorClose.setPermition();
  photoRelay.onInt();
  // myOLED.begin();
  // myOLED.setFont(RusFont);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2

  delay(1000);
  currentTime5S = millis();
  currentTime1S = millis();
}

void loop()
{
  if (mySwitch.available()) {
    recData = mySwitch.getReceivedValue();
    Serial.println("Recieve: "+ (String)recData);
    switch(recData){
      case 5393:
        cmdOpen = true;
        cmdClose = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        break;
      case 9678:
        cmdClose = true;
        cmdOpen = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        break;
    }
    mySwitch.resetAvailable();
  }

  if(photoRelay.getInt())
    cmdStopClose = true;
  else
    cmdStopClose = false;

  // Сработано фотореле, очистка команд
  if(cmdStopClose && (cmdOpen || cmdClose)){
    cmdOpen = cmdClose = false;
  }
  // Сработано фотореле и идет закрытие, то
  // сбрасываем реле 2 (закрытие)
  // Иначе, фотореле не сработано и идет закрытие, то
  // выдаем команду на срабатывание реле 2 (открытие)
  if(cmdStopClose && isClosing && relayMotorClose.getCondition()){
    relayMotorClose.close(delayStop);
  }
  else if(!cmdStopClose && isClosing && !relayMotorClose.getCondition() && !cmdOpen){
    cmdClose = true;
    isClosing = false;
  }

  // Сработано фотореле и идет открытие, то
  // сбрасываем реле 1 (открытие)
  // Иначе, фотореле не сработано и идет открытие, то
  // выдаем команду на срабатывание реле 1 (открытие)
  if(cmdStopClose && isOpening && relayMotorOpen.getCondition()){
    relayMotorOpen.close(delayStop);
  }
  else if(!cmdStopClose && isOpening && !relayMotorOpen.getCondition() && !cmdClose){
    cmdOpen = true;
    isOpening = false;
  }

  // Если поступила команда открыть и не открыто, то
  // проверяем сработано ли реле 2 (закрытие), если да, то
  // закрываем реле через 200 мс
  // Иначе, даем команду на срабатывание реле 1 (открытие)
  if(cmdOpen && !isOpening){
    if(relayMotorClose.getCondition())
      relayMotorClose.close(delayStop);
    else{
      if(relayMotorOpen.open(delayStart)){
        isOpening = true;
        cmdOpen = false;
        if(isClosing)
          isClosing = false;
      }
    }
  }
  else if(cmdOpen && isOpening){
    if(relayMotorOpen.close(delayStop)){
      cmdOpen = false;
      isOpening = false;
    }
  }

  // Если поступила команда закрыть и не закрытие, то
  // проверяем сработано ли реле 1 (открытие), если да, то
  // сбрасываем реле через 200 мс
  // Иначе, выдаем команду на срабатывание реле 2 (закрытие)
  if(cmdClose && !isClosing){
    if(relayMotorOpen.getCondition())
      relayMotorOpen.close(delayStop);
    else{
      if(relayMotorClose.open(delayStart)){
        isClosing = true;
        cmdClose = false;
        if(isOpening)
          isOpening = false;
      }
    }
  }
  else if(cmdClose && isClosing){
    if(relayMotorClose.close(delayStop)){
      cmdClose = false;
      isClosing = false;
    }
  }

  // Опрос раз в 5 сек. 
  if (millis() >= (currentTime5S + 5000))
  {
    currentTime5S = millis();
    // if((relayMotorOpen.getErrorStatus() & CMDOPCL) == CMDOPCL)
    //   relayMotorOpen.clearErrorStatus();
  }
  // Опрос раз в секунду
  if (millis() >= (currentTime1S + 1000))
  {
    currentTime1S = millis();
    
    Serial.println("-------------------------------------");
    Serial.print("photoRelay is: " + (String)cmdStopClose + " ");
    Serial.print("isClosing is: " + (String)isClosing + " ");
    Serial.print("cmdClose is: " + (String)cmdClose + " ");
    Serial.println("getCondition is: " + (String)relayMotorClose.getCondition() + " ");

    Serial.print("photoRelay is: " + (String)cmdStopClose + " ");
    Serial.print("isOpening is: " + (String)isOpening + " ");
    Serial.print("cmdOpen is: " + (String)cmdOpen + " ");
    Serial.println("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
    Serial.println("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
    // Serial.println(relayMotorOpen.getErrorStatus());
    // Serial.println(relayMotorClose.getErrorStatus());
  }  
  // myOLED.clrScr();
  // myOLED.print(hello, CENTER, 0);
  // myOLED.update();
}