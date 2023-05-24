#include <Arduino.h>
#include "tools.h"

#define NOTPERM    0x01
#define ISOPENED   0x02
#define ISCLOSED   0x03
#define CMDOPCL    0x10 // Пришли две команды

class Relay : public numcopies {
    private:
        byte pin; // Ножка
        bool condition; // Открыто/Закрыто
        bool permition; // Разрешение
        bool remoteControl; // Внешнее управление
        byte errorStatus; // Байт ошибок
        uint32_t currentTime; // Текущее время
        bool delay; // Бит начала работы таймера
        uint32_t delayTime; // Общая задержка
    public:
        Relay();
        Relay(byte _pin, uint32_t _delay = 0);
        bool open(void);
        bool open(uint32_t _delay);
        bool close(void);
        bool close(uint32_t _delay);
        bool extOpen(void);
        bool getCondition(void);
        bool getPermition(void);
        void setPermition(void);
        void unserPermition(void);
        byte getErrorStatus(void);
        void clearErrorStatus(void);
        bool cmdOpen; // Команда открыть
        bool cmdClose; // Команда закрыть
};
