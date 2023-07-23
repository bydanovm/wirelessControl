#ifndef HEADER_SETTINGS
#define HEADER_SETTINGS
#define DEBUG_ENABLE // Закомментировать если отладка не нужна
#define SIM_DEVICE // Симуляция девайсов
#if defined(ARDUINO) && ARDUINO >= 100
    #ifdef ESP8266
    #define pinRCSwitch          D2  // D2
    #define relayMotorOpenPin    10 // SD3
    #define relayMotorClosePin   9  // SD2
    #define relaySignalLampPin   D1  // D1
    #define endCapOpenPin        D6 // D5 opened
    #define endCapClosePin       D5 // D6 closed
    #define photoRelayPin        D7 // D7
    #define cmdMQTTEmpty         ""
    #define cmdMQTTOpen          "OPEN"
    #define cmdMQTTClose         "CLOSE"
    #define cmdMQTTLampOn        "LAMPON"
    #define cmdMQTTLampOff       "LAMPOFF"
    #define cmdMQTTHalfOpen      "HOPEN"
    #define cmdMQTTHalfClose     "HCLOSE"
    #else
    #define pinRCSwitch          0
    #define relayMotorOpenPin    14
    #define relayMotorClosePin   15
    #define relaySignalLampPin   13
    #define inRelay1Pin          3 
    #define inRelay2Pin          4 
    #define inRelay3Pin          5 
    #endif
    #define delayStart 2000
    #define delayStop  0
    #define delaySignalLamp 1000
    #define delayEmrgnyStp 40000

    #include <Arduino.h>

    #ifdef DEBUG_ENABLE
        #define DEBUGLN(x) Serial.println(x)
        #define DEBUG(x)   Serial.print(x)
    #else
        #define DEBUGLN(x)
        #define DEBUG(x)
    #endif
#endif
#endif