#define DEBUG_ENABLE // Закомментировать если отладка не нужна
#define SIM_DEVICE // Симуляция девайсов
#if defined(ARDUINO) && ARDUINO >= 100
    #ifdef ESP8266
    #define pinRCSwitch          5  // D1
    #define relayMotorOpenPin    10 // SD3
    #define relayMotorClosePin   9  // SD2
    #define relaySignalLampPin   4  // D2
    #define inRelay1Pin          14 // D5 opened
    #define inRelay2Pin          12 // D6 closed
    #define inRelay3Pin          13 // D7
    #define inRelay4Pin          15 // D8
    #define cmdMQTTEmpty         ""
    #define cmdMQTTOpen          "OPEN"
    #define cmdMQTTClose         "CLOSE"
    #define cmdMQTTLampOn        "LAMPON"
    #define cmdMQTTLampOff       "LAMPOFF"
    #else
    #define pinRCSwitch          0
    #define relayMotorOpenPin    14
    #define relayMotorClosePin   15
    #define relaySignalLampPin   13
    #define inRelay1Pin          3 
    #define inRelay2Pin          4 
    #define inRelay3Pin          5 
    #define inRelay4Pin          6 
    #endif
    #define delayStart 2000
    #define delayStop  200
    #define delaySignalLamp 1000

    #include <Arduino.h>

    #ifdef DEBUG_ENABLE
        #define DEBUGLN(x) Serial.println(x)
        #define DEBUG(x)   Serial.print(x)
    #else
        #define DEBUGLN(x)
        #define DEBUG(x)
    #endif
#endif