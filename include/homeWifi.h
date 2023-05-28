// #include <Arduino.h>
#define numberMaxAttemption 30 // Количетсов попыток
#define delayAttemption 250 // Задержка медлу попытками

#define CON_OK     0x01
#define CON_ERR    0x10

class homeWifi {
    private:
        char* SSID;
        char* PASS;
        uint32_t currentTime = 0; // Текущее время
        uint8_t numberCurrAttemption = 0; // Текущая попытка
        uint8_t status = 0; // Статусное слово
        bool isConnected = false; // Бит подключения
    public:
        homeWifi();
        homeWifi(char* _ssid, char* pass);
        // Инициализацция соединения
        bool initConnect();
        // Простая проверка соединения
        bool checkConnect();
        // Проверка соединения с ограниченным числом попыток
        bool checkConnectAtt(); 
        // Ручной сброс проверки попыток соединения
        bool handClear(); 
};