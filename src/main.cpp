#include "settings.h"
#include "RCSwitch.h"
#include "relayOut.h"
#include "relayIn.h"
// #include <OLED_I2C.h>
// #define hello "Ghbdtn" // Привет
// OLED  myOLED(SDA, SCL, 8);

RCSwitch mySwitch = RCSwitch();

RelayOut relayMotorOpen = RelayOut(relayMotorOpenPin, OUTPUT, true);
RelayOut relayMotorClose = RelayOut(relayMotorClosePin, OUTPUT, true);
RelayOut relaySignalLamp = RelayOut(relaySignalLampPin, OUTPUT, false);

RelayIn * RelayIn::instances[4] = {NULL, NULL};
RelayIn photoRelay = RelayIn(inRelay3Pin, INPUT_PULLUP); // Фотоэлемент

RelayIn endCapOpen  = RelayIn(inRelay1Pin, INPUT_PULLUP); // Концеквик открытия
RelayIn endCapClose = RelayIn(inRelay2Pin, INPUT_PULLUP); // Концевик закрытия

uint32_t currentTime5S; // Текущее время для таймера в 1 сек
uint32_t currentTime1S; // Текущее время для таймера в 5 сек
uint32_t currentTimeSignalLamp;
unsigned long recData = 0; // Буфер для приема команд
bool cmdOpen, cmdClose, cmdStopClose, cmdSignalLamp = false;
bool isOpening, isClosing = false;
bool isOpened, isClosed = false;

// extern uint8_t RusFont[];

void setup()
{
  #ifdef SIM_DEVICE
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    pinMode(9, OUTPUT);
    pinMode(18,OUTPUT);
    digitalWrite(18,LOW);
    pinMode(19,OUTPUT);
    digitalWrite(19,LOW);
  #endif
  Serial.begin(115200);
  relayMotorOpen.setPermition();
  relayMotorClose.setPermition();
  relaySignalLamp.setPermition();
  photoRelay.onInt();
  endCapOpen.onInt();
  endCapClose.onInt();
  // myOLED.begin();
  // myOLED.setFont(RusFont);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2

  delay(1000);
  currentTime5S = currentTime1S =  millis();
}

void loop()
{
  if (mySwitch.available()) {
    recData = mySwitch.getReceivedValue();
    DEBUG("Recieve ");
    switch(recData){
      case 5393:
        cmdOpen = true;
        cmdClose = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        DEBUGLN((String)recData + " Opening...");
        break;
      case 9678:
        cmdClose = true;
        cmdOpen = false;
        relayMotorClose.clearErrorStatus();
        relayMotorOpen.clearErrorStatus();
        DEBUGLN((String)recData + " Closing...");        
        break;
      case 2341:
        cmdSignalLamp = true;
        relaySignalLamp.clearErrorStatus();
        currentTimeSignalLamp = millis();
        DEBUGLN((String)recData + " Signal lamp...");
        break;
      default:
        DEBUGLN((String)recData + " Unknown command...");
        break;
    }
    mySwitch.resetAvailable();
  }
  
  // Проверка срабатывания фотоэлемента
  if(photoRelay.getInt() && !cmdStopClose){
    cmdStopClose = true;
    DEBUGLN("Blocking photorelay is up");
  }
  else if(!photoRelay.getInt() && cmdStopClose){
    cmdStopClose = false;
    DEBUGLN("Blocking photorelay is down");
  }

  // Проверка концевика на открытие
  if(endCapOpen.getInt() && !isOpened){
    isOpened = true;
    DEBUGLN("Endcap open is up");
  }
  else if(!endCapOpen.getInt() && isOpened){
    isOpened = false;
    DEBUGLN("Endcap open is down");
  }

  // Проверка концевика на закрытие
  if(endCapClose.getInt() && !isClosed){
    isClosed = true;
    DEBUGLN("Endcap close is up");
  }
  else if(!endCapClose.getInt() && isClosed){
    isClosed = false;
    DEBUGLN("Endcap close is down");
  }

  if(isClosed && isOpened && (cmdOpen || cmdClose)){
    cmdOpen = cmdClose = false;
    DEBUGLN("Blocking endcap");
  }

  // Сработано фотореле, очистка команд
  if(cmdStopClose && (cmdOpen || cmdClose)){
    cmdOpen = cmdClose = false;
    DEBUGLN("Blocking photorelay");
  }
  // Сработано фотореле и идет закрытие, то
  // сбрасываем реле 2 (закрытие)
  // Иначе, фотореле не сработано и идет закрытие, то
  // выдаем команду на срабатывание реле 2 (открытие)
  if(cmdStopClose && isClosing && relayMotorClose.getCondition()){
    if(relayMotorClose.close(delayStop))
      DEBUGLN("Blocking. Relay 2 is down");
  }
  else if(!cmdStopClose && isClosing && !relayMotorClose.getCondition() && !cmdOpen){
    cmdClose = true;
    isClosing = false;
    DEBUGLN("Relay 2 is uping");
  }

  // Сработано фотореле и идет открытие, то
  // сбрасываем реле 1 (открытие)
  // Иначе, фотореле не сработано и идет открытие, то
  // выдаем команду на срабатывание реле 1 (открытие)
  if(cmdStopClose && isOpening && relayMotorOpen.getCondition()){
    if(relayMotorOpen.close(delayStop))
      DEBUGLN("Relay 1 is down");
  }
  else if(!cmdStopClose && isOpening && !relayMotorOpen.getCondition() && !cmdClose){
    cmdOpen = true;
    isOpening = false;
    DEBUGLN("Relay 1 is uping");
  }

  // Если поступила команда открыть и не идет открытие и не открыто, то
  // проверяем сработано ли реле 2 (закрытие), если да, то
  // закрываем реле через 200 мс, иначе, даем команду на срабатывание реле 1 (открытие)
  // Иначе, если поступила команда открыть и идет открытие, то
  // сбрасываем реле 1 (открытие)
  if(cmdOpen && !isOpening && !isOpened){
    if(relayMotorClose.getCondition()){
      if(relayMotorClose.close(delayStop))
        DEBUGLN("Opening. Relay 2 is down");
    }
    else{
      if(relayMotorOpen.open(delayStart)){
        isOpening = true;
        cmdOpen = false;
        if(isClosing)
          isClosing = false;
        DEBUGLN("Opening. Relay 1 is up");
      }
    }
  }
  else if((cmdOpen && isOpening) || isOpened){
    if(relayMotorOpen.close(delayStop)){
      cmdOpen = false;
      isOpening = false;
      DEBUGLN("Opening. Relay 1 is down");
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
        DEBUGLN("Closing. Relay 1 is down");
    }
    else{
      if(relayMotorClose.open(delayStart)){
        isClosing = true;
        cmdClose = false;
        if(isOpening)
          isOpening = false;
        DEBUGLN("Closing. Relay 2 is up");
      }
    }
  }
  else if((cmdClose && isClosing) || isClosed){
    if(relayMotorClose.close(delayStop)){
      cmdClose = false;
      isClosing = false;
      DEBUGLN("Closing. Relay 2 is down");
    }
  }

  // Если пришла команда на сигнальную лампу
  // или команда на открытие/закрытие, то 
  // активировать реле каждые N времени
  // Иначе, проверяем активно ли реле, если да, то 
  // деактивируем его через N времени
  if(cmdSignalLamp || isOpening || isClosing){
    if(!relaySignalLamp.getCondition())
      relaySignalLamp.open(delaySignalLamp);
    else
      relaySignalLamp.close(delaySignalLamp);
  }
  else{
    if(relaySignalLamp.getCondition())
      relaySignalLamp.close(delaySignalLamp);
  }
  // Сброс команды включения сигнальной лампы
  if (millis() >= (currentTimeSignalLamp + 60000) && cmdSignalLamp)
  {
    // currentTimeSignalLamp = millis();
    cmdSignalLamp = false;
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

    // DEBUGLN("-------------------------------------");
    // DEBUG("photoRelay is: " + (String)cmdStopClose + " ");
    // DEBUG("isClosing is: " + (String)isClosing + " ");
    // DEBUG("cmdClose is: " + (String)cmdClose + " ");
    // DEBUGLN("getCondition is: " + (String)relayMotorClose.getCondition() + " ");

    // DEBUG("photoRelay is: " + (String)cmdStopClose + " ");
    // DEBUG("isOpening is: " + (String)isOpening + " ");
    // DEBUG("cmdOpen is: " + (String)cmdOpen + " ");
    // DEBUGLN("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
    // DEBUGLN("getCondition is: " + (String)relayMotorOpen.getCondition() + " ");
  }  
  // myOLED.clrScr();
  // myOLED.print(hello, CENTER, 0);
  // myOLED.update();
}