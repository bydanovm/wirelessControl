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

RelayIn endCapOpen  = RelayIn(inRelay1Pin, INPUT_PULLUP);
RelayIn endCapClose = RelayIn(inRelay2Pin, INPUT_PULLUP);

uint32_t currentTime5S; // Текущее время для таймера в 1 сек
uint32_t currentTime1S; // Текущее время для таймера в 5 сек
unsigned long recData = 0; // Буфер
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
  endCapOpen.onInt();
  endCapClose.onInt();
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
    Serial.print("Recieve ");
    switch(recData){
      case 5393:
        cmdOpen = true;
        cmdClose = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        Serial.println((String)recData + " Opening...");
        break;
      case 9678:
        cmdClose = true;
        cmdOpen = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        Serial.println((String)recData + " Closing...");        
        break;
      default:
        Serial.println((String)recData + " Unknown command");
        break;
    }
    mySwitch.resetAvailable();
  }
  
  // Проверка срабатывания фотоэлемента
  if(photoRelay.getInt() && !cmdStopClose){
    cmdStopClose = true;
    Serial.println("Blocking photorelay is up");
  }
  else if(!photoRelay.getInt() && cmdStopClose){
    cmdStopClose = false;
    Serial.println("Blocking photorelay is down");
  }

  // Проверка концевика на открытие
  if(endCapOpen.getInt() && !isOpened){
    isOpened = true;
    Serial.println("Endcap open is up");
  }
  else if(!endCapOpen.getInt() && isOpened){
    isOpened = false;
    Serial.println("Endcap open is down");
  }

  // Проверка концевика на закрытие
  if(endCapClose.getInt() && !isClosed){
    isClosed = true;
    Serial.println("Endcap close is up");
  }
  else if(!endCapClose.getInt() && isClosed){
    isClosed = false;
    Serial.println("Endcap close is down");
  }

  if(isClosed && isOpened && (cmdOpen || cmdClose)){
    cmdOpen = cmdClose = false;
    Serial.println("Blocking endcap");
  }

  // Сработано фотореле, очистка команд
  if(cmdStopClose && (cmdOpen || cmdClose)){
    cmdOpen = cmdClose = false;
    Serial.println("Blocking photorelay");
  }
  // Сработано фотореле и идет закрытие, то
  // сбрасываем реле 2 (закрытие)
  // Иначе, фотореле не сработано и идет закрытие, то
  // выдаем команду на срабатывание реле 2 (открытие)
  if(cmdStopClose && isClosing && relayMotorClose.getCondition()){
    if(relayMotorClose.close(delayStop))
      Serial.println("Blocking. Relay 2 is down");
  }
  else if(!cmdStopClose && isClosing && !relayMotorClose.getCondition() && !cmdOpen){
    cmdClose = true;
    isClosing = false;
    Serial.println("Relay 2 is uping");
  }

  // Сработано фотореле и идет открытие, то
  // сбрасываем реле 1 (открытие)
  // Иначе, фотореле не сработано и идет открытие, то
  // выдаем команду на срабатывание реле 1 (открытие)
  if(cmdStopClose && isOpening && relayMotorOpen.getCondition()){
    if(relayMotorOpen.close(delayStop))
      Serial.println("Relay 1 is down");
  }
  else if(!cmdStopClose && isOpening && !relayMotorOpen.getCondition() && !cmdClose){
    cmdOpen = true;
    isOpening = false;
    Serial.println("Relay 1 is uping");
  }

  // Если поступила команда открыть и не идет открытие и не открыто, то
  // проверяем сработано ли реле 2 (закрытие), если да, то
  // закрываем реле через 200 мс, иначе, даем команду на срабатывание реле 1 (открытие)
  // Иначе, если поступила команда открыть и идет открытие, то
  // сбрасываем реле 1 (открытие)
  if(cmdOpen && !isOpening && !isOpened){
    if(relayMotorClose.getCondition()){
      if(relayMotorClose.close(delayStop))
        Serial.println("Opening. Relay 2 is down");
    }
    else{
      if(relayMotorOpen.open(delayStart)){
        isOpening = true;
        cmdOpen = false;
        if(isClosing)
          isClosing = false;
        Serial.println("Opening. Relay 1 is up");
      }
    }
  }
  else if((cmdOpen && isOpening) || isOpened){
    if(relayMotorOpen.close(delayStop)){
      cmdOpen = false;
      isOpening = false;
      Serial.println("Opening. Relay 1 is down");
    }
  }

  // Если поступила команда закрыть и не закрытие, то
  // проверяем сработано ли реле 1 (открытие), если да, то
  // сбрасываем реле через 200 мс, иначе, выдаем команду на срабатывание реле 2 (закрытие)
  // Иначе, если поступила команда открыть и идет открытие, то
  // сбрасываем реле 2 (закрытие)  
  if(cmdClose && !isClosing && !isClosed){
    if(relayMotorOpen.getCondition()){
      if(relayMotorOpen.close(delayStop))
        Serial.println("Closing. Relay 1 is down");
    }
    else{
      if(relayMotorClose.open(delayStart)){
        isClosing = true;
        cmdClose = false;
        if(isOpening)
          isOpening = false;
        Serial.println("Closing. Relay 2 is up");
      }
    }
  }
  else if((cmdClose && isClosing) || isClosed){
    if(relayMotorClose.close(delayStop)){
      cmdClose = false;
      isClosing = false;
      Serial.println("Closing. Relay 2 is down");
    }
  }

  // Опрос раз в 5 сек. 
  if (millis() >= (currentTime5S + 5000))
  {
    currentTime5S = millis();
  }
  // Опрос раз в 1 сек.
  if (millis() >= (currentTime1S + 1000))
  {
    currentTime1S = millis();

    // Serial.println("-------------------------------------");
    // Serial.print("photoRelay is: " + (String)cmdStopClose + " ");
    // Serial.print("isClosing is: " + (String)isClosing + " ");
    // Serial.print("cmdClose is: " + (String)cmdClose + " ");
    // Serial.println("getCondition is: " + (String)relayMotorClose.getCondition() + " ");

    // Serial.print("photoRelay is: " + (String)cmdStopClose + " ");
    // Serial.print("isOpening is: " + (String)isOpening + " ");
    // Serial.print("cmdOpen is: " + (String)cmdOpen + " ");
    // Serial.println("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
    // Serial.println("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
  }  
  // myOLED.clrScr();
  // myOLED.print(hello, CENTER, 0);
  // myOLED.update();
}