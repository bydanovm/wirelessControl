#ifndef HEADER_RELAY_IN
#define HEADER_RELAY_IN
#include <Arduino.h>
#include "settings.h"

#if defined(ESP8266)
    #define RECIEVE_ATTR ICACHE_RAM_ATTR
#else
    #define RECIEVE_ATTR
#endif

#define NOTPERM    0x01
#define ISOPENED   0x02
#define ISCLOSED   0x03
#define CMDOPCL    0x10 // Пришли две команды
#define ERRMODE    0x11 // Ошибка режима работы

class RelayIn {
    private:
        byte pin; // Ножка
        byte mode; // Тип работы
        bool invert; // Сигнал инвертирован
        bool condition; // Открыто/Закрыто
        byte errorStatus; // Байт ошибок
    public:
        RelayIn();
        RelayIn(byte _pin, byte _mode);
        RelayIn(byte _pin, byte _mode, bool _invert);
        bool getCondition(void);
        byte getErrorStatus(void);
        void clearErrorStatus(void);
        // Инициализация типа работы
        void beginInit(); 
        void beginInterrupt(void (*userFunc)(void));
        void offInt();
        bool getInt();
        void clearInt();
        volatile bool intFlag;
        void RECIEVE_ATTR intRising();
};

#endif