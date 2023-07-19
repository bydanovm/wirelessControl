#include "settings.h"
#if defined(ESP8266)
    // #define MQTT_ON
    #include "homeWifi.h"
#else

#endif
#include "RCSwitch.h"
#include "relayOut.h"
#include "relayIn.h"
// #include <OLED_I2C.h>
// #define hello "Ghbdtn" // Привет
// OLED  myOLED(SDA, SCL, 8);
#if defined(ESP8266) && defined(MQTT_ON)
homeWifi myWifi = homeWifi("Redmi_5777","S_tjmx8inu123");
#endif
///////////////////////////////////////////////////
// Добавить запоминалку времени открывания ворот //
///////////////////////////////////////////////////
RCSwitch mySwitch = RCSwitch();

RelayOut relayMotorOpen = RelayOut(relayMotorOpenPin, OUTPUT, true);
RelayOut relayMotorClose = RelayOut(relayMotorClosePin, OUTPUT, true);
RelayOut relaySignalLamp = RelayOut(relaySignalLampPin, OUTPUT, true);

RelayIn photoRelay = RelayIn(photoRelayPin, INPUT_PULLUP, true); // Фотоэлемент
RelayIn endCap451  = RelayIn(endCapOpenPin, INPUT_PULLUP, false); // Концевик открытия
RelayIn endCap443 = RelayIn(endCapClosePin, INPUT_PULLUP, false); // Концевик закрытия

uint32_t currentTime5S; // Текущее время для таймера в 1 сек
uint32_t currentTime1S; // Текущее время для таймера в 5 сек
uint32_t currentTime250ms;
uint32_t currentTimeSignalLamp;
uint32_t currentTimeEmergencyStop;

unsigned long recData = 0; // Буфер для приема команд
bool cmdOpen, cmdClose, cmdStopClose, cmdSignalLamp, cmdEmergencyStop = false;
bool isOpening, isClosing = false;
bool isOpened, isClosed = false;
bool tStatusMain, tStatusAdd = false;

// Аварийный СТОП по истечении времени
void emergencyStop();
void getEndCap();

// void RECIEVE_ATTR capCloseInt();
// void RECIEVE_ATTR capOpenInt();
// void RECIEVE_ATTR photoRelayInt();

void setup()
{
  #ifdef SIM_DEVICE
    #ifndef ESP8266
      pinMode(8, OUTPUT);
      digitalWrite(8, HIGH);
      pinMode(9, OUTPUT);
      pinMode(18,OUTPUT);
      digitalWrite(18,LOW);
      pinMode(19,OUTPUT);
      digitalWrite(19,LOW);
    #endif
  #endif
  Serial.begin(115200);
  relayMotorOpen.setPermition();
  relayMotorClose.setPermition();
  relaySignalLamp.setPermition();
  photoRelay.beginInit();
  endCap451.beginInit();
  endCap443.beginInit();
  // photoRelay.beginInterrupt(capCloseInt);
  // endCap451.beginInterrupt(capOpenInt);
  // endCap443.beginInterrupt(photoRelayInt);
  #if defined(ESP8266) && defined(MQTT_ON)
  myWifi.initConnect();
  myWifi.initMQTT("m3.wqtt.ru", 8361, "esp8266_gates", "u_1AYLFH", "AWAblv6p");
  myWifi.setGatesCallback();
  #endif

  mySwitch.enableReceive(pinRCSwitch);

  delay(1000);
  currentTime5S = millis();
  currentTime1S = millis();
  currentTime250ms = millis();
}

void loop()
{
  #if defined(ESP8266) && defined(MQTT_ON)
  // Работа с брокером MQTT
  if(myWifi.checkConnectAtt()){
    if(myWifi.checkConnectMQTT()){
      myWifi.mqttLoop();
      // Проверка открытия ворот
      if(isOpened) tStatusAdd = true;
      if(isClosed) tStatusAdd = false;
      if(tStatusMain != tStatusAdd){
        tStatusMain = tStatusAdd;
        myWifi.setGatesStatus(tStatusMain);
      }
    }
  }
  if(myWifi.cmdMQTT != cmdMQTTEmpty){
    if((myWifi.cmdMQTT == cmdMQTTOpen) && !cmdOpen && !isOpened && !isOpening){
      cmdOpen = true;
      cmdClose = false;
      relayMotorClose.clearErrorStatus();
      relayMotorOpen.clearErrorStatus();
      DEBUGLN((String)myWifi.cmdMQTT + " Opening MQTT cmd...");
    }
    if((myWifi.cmdMQTT == cmdMQTTClose) && !cmdClose && !isClosed && !isClosing){
      cmdClose = true;
      cmdOpen = false;
      relayMotorClose.clearErrorStatus();
      relayMotorOpen.clearErrorStatus();
      DEBUGLN((String)myWifi.cmdMQTT + " Closing MQTT cmd..."); 
    }
    if(myWifi.cmdMQTT == cmdMQTTLampOn){
      cmdSignalLamp = true;
      relaySignalLamp.clearErrorStatus();
      currentTimeSignalLamp = millis();
      DEBUGLN((String)myWifi.cmdMQTT + " Signal lamp ON MQTT cmd...");
    }
    if(myWifi.cmdMQTT == cmdMQTTLampOff){
      cmdSignalLamp = false;
      relaySignalLamp.clearErrorStatus();
      currentTimeSignalLamp = millis();
      DEBUGLN((String)myWifi.cmdMQTT + " Signal lamp OFF MQTT cmd...");
    }
    myWifi.clearCmdMQTT();
  }
  #endif

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

  getEndCap();

  if(isClosed && isOpened && (cmdOpen || cmdClose)){
    cmdOpen = false;
    cmdClose = false;
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
  emergencyStop();
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
}
void getEndCap(){
  // Прерывания - не актуально
  // // Проверка концевика на открытие
  // if(endCap451.getInt() && !isOpened){
  //   endCap451.clearInt();
  //   isOpened = true;
  //   DEBUGLN("Endcap open is up");
  // }
  // else if(!endCap451.getInt() && isOpened){
  //   endCap451.clearInt();
  //   isOpened = false;
  //   DEBUGLN("Endcap open is down");
  // }

  // // Проверка концевика на закрытие
  // if(endCap443.getInt() && !isClosed){
  //   endCap443.clearInt();
  //   isClosed = true;
  //   DEBUGLN("Endcap close is up");
  // }
  // else if(!endCap443.getInt() && isClosed){
  //   endCap443.clearInt();
  //   isClosed = false;
  //   DEBUGLN("Endcap close is down");
  // }
  if(millis() >= currentTime250ms + 50){
    currentTime250ms = millis();
    bool flendCap451 = endCap451.getCondition();
    bool flendCap443 = endCap443.getCondition();
    // if(flendCap443)
    //   DEBUGLN("443 on");
    // else
    //   DEBUGLN("443 off");
    // if(flendCap451)
    //   DEBUGLN("451 on");
    // else
    //   DEBUGLN("451 off");
    if(!flendCap451 && !flendCap443 && !isOpened && !isClosed){
      isClosed = true;
      DEBUGLN("Endcap close is up");
    }
    else if(flendCap451 && flendCap443 && isClosed)
    {
      isClosed = false;
      DEBUGLN("Endcap close is down");
    }
    if(!flendCap451 && flendCap443 && !isClosed && !isOpened){
      isOpened = true;
      DEBUGLN("Endcap open is up");
    }
    else if(flendCap451 && flendCap443 && isOpened){
      isOpened = false;
      DEBUGLN("Endcap open is down");
    }
  }
}

// Аварийный СТОП по истечении времени
void emergencyStop(){
  if((isOpening || isClosing) && !cmdEmergencyStop){
    currentTimeEmergencyStop = millis();
    cmdEmergencyStop = true;
  }
  if(cmdEmergencyStop && (millis() >= currentTimeEmergencyStop + delayEmrgnyStp)){
    isOpening = false;
    isClosing = false;
    cmdEmergencyStop = false;

    if(relayMotorOpen.close())
      DEBUGLN("Closing. Relay 1 is down");
    if(relayMotorClose.close())
      DEBUGLN("Closing. Relay 2 is down");

  }
}
// void RECIEVE_ATTR capCloseInt(){
//     endCap443.intFlag = true;
// }
// void RECIEVE_ATTR capOpenInt(){
//     endCap451.intFlag = true;
// }
// void RECIEVE_ATTR photoRelayInt(){
//     photoRelay.intFlag = true;
// }