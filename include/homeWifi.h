#ifndef HEADER_HOME_WIFI
#define HEADER_HOME_WIFI
#include "settings.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define numberMaxAttemption 60 // Количетсов попыток
#define delayAttemption     2000 // Задержка между попытками
#define delayAttemptionConn 2000 // Задержка между попытками при имеющемся соединении

#define CON_OK     0x01
#define CON_ERR    0x10

class homeWifi {
    protected:
        const int   mqttDeviceStatusQos      = 1;
        ////////////////////////////////////////////////////////////
        // Состояние устройства (онлайн/оффлайн)
        const char* mqttTopicDeviceStatus    = "gates/status";
        const char* mqttDeviceStatusOn       = "online";
        const char* mqttDeviceStatusOff      = "offline";
        const bool  mqttDeviceStatusRetained = true;
        ////////////////////////////////////////////////////////////
        // Состояние ворот (открыты/закрыты)
        const char* mqttTopicGatesStatus     = "gates/statusGates";
        const char* mqttGatesStatusOpen      = "1";
        const char* mqttGatesStatusClose     = "0";
        const bool  mqttGatesStatusRetained  = false;
        ////////////////////////////////////////////////////////////
        // Команда открыть/закрыть
        const char* mqttTopicGatesCmd        = "gates/ctrlGates";
        // Команда приоткрыть ворота
        const char* mqttTopicHalfGatesCmd    = "gates/ctrlHalfGates";
        // Команда включить сигнальную лампу
        const char* mqttTopicSigLampCmd      = "gates/signalLamp";
        void callback(char* topic, uint8_t* payload, unsigned int length);
    private:
        char* wifiSSID;
        char* wifiPASS;
        char* mqttServer;
        uint16_t mqttPort;
        char* mqttClientId;
        char* mqttUser;
        char* mqttPass;
        uint32_t currentTime = 0; // Текущее время
        uint8_t numberCurrAttemption = 0; // Текущая попытка
        uint8_t status = 0; // Статусное слово
        bool isConnected = false; // Бит подключения
        bool enableMQTT = false; // Активация MQTT брокера
        // std::function<void(char*, uint8_t*, unsigned int)> yourFunction;
        // std::function<void(char*, uint8_t*, unsigned int)> callback;
    public:
        homeWifi();
        homeWifi(char* _ssid, char* pass);
        // Инициализацция соединения
        bool initConnect();
        // Простая проверка соединения
        bool checkConnect();
        // Получить статус соединения
        bool getStatusConnectedWifi();
        // Проверка соединения с ограниченным числом попыток
        bool checkConnectAtt(); 
        // Ручной сброс проверки попыток соединения
        bool handClear(); 
        // Активировать MQTT брокера
        void setEnableMQTT();
        // Деактивировать MQTT брокера
        void unsetEnableMQTT();
        // Статус MQTT брокера
        bool getEnableMQTT();
        // Инициализация MQTT брокера
        bool initMQTT(char*,uint16_t,char*,char*,char*);
        // Проверка соединения с брокером
        bool checkConnectMQTT();
        // Проверка соединения с брокером с ограниченным количеством попыток
        bool checkConnectMQTTAtt();
        void mqttLoop();
        void setGatesStatus(bool statusGates);
        void setGatesCallback();
        char* cmdMQTT;
        void clearCmdMQTT();
};
#endif