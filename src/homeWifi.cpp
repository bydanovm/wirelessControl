#include "homeWifi.h"

// homeWifi::mqttClient(wifiClient);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

homeWifi::homeWifi(){}
homeWifi::homeWifi(char* _ssid, char* _pass){
    homeWifi::wifiSSID = _ssid;
    homeWifi::wifiPASS = _pass;
}
bool homeWifi::initConnect(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(homeWifi::wifiSSID, homeWifi::wifiPASS);
    return true;
}
bool homeWifi::checkConnect(){
    if(WiFi.status() != WL_CONNECTED)
        return false;
    else
        return true;
}
bool homeWifi::checkConnectAtt(){
    // Проверяем соединение первый раз
    if(numberCurrAttemption == 0) {
        currentTime = millis();
        isConnected = homeWifi::checkConnect();
        DEBUGLN("First condition...");
        numberCurrAttemption++;
    }
    // Если соединения нет, то проверяем еще "numberMaxAttemption" раз 
    // через каждые "delayAttemption" времени
    if(numberCurrAttemption <= numberMaxAttemption && !isConnected){
        if(millis() >= currentTime + delayAttemption){
            currentTime = millis();
            if(homeWifi::checkConnect()){
                isConnected = true;
                status |= CON_OK;
                numberCurrAttemption = numberMaxAttemption;
            }
            DEBUGLN("Attemption " + (String)numberCurrAttemption);
            numberCurrAttemption++;
        }
    }
    // Если количество попыток превышает "numberMaxAttemption", то
    // проверка не пройдена, нужен ручной сброс попыток
    if(numberCurrAttemption >= numberMaxAttemption && !isConnected) {
        isConnected = false;
        status |= CON_ERR;
        DEBUGLN("Max Attemption " + (String)numberCurrAttemption);
    }
    // Простая проверка соединения каждые "delayAttemptionConn" времени
    if(isConnected){
        if(millis() >= currentTime + delayAttemptionConn){
            currentTime = millis();
            if(homeWifi::checkConnect()){
                isConnected = true;
                status ^= CON_ERR;
                status |= CON_OK;
                // DEBUGLN("Is connected");
            }
            else{
                isConnected = false;
                status ^= CON_OK;
                status |= CON_ERR;
                numberCurrAttemption = 0;
                DEBUGLN("Not connected");
            }
        }
    }
    return isConnected;
}
bool homeWifi::handClear(){
    if(!isConnected && (status & CON_ERR)){
        numberCurrAttemption = 0;
    }
    return true;
}
void homeWifi::setEnableMQTT(){
    enableMQTT = true;
}
void homeWifi::unsetEnableMQTT(){
    enableMQTT = false;
}
bool homeWifi::getEnableMQTT(){
    return enableMQTT;
}
bool homeWifi::initMQTT(char* _server, uint16_t _port, char* _clientId, char* _user, char* _pass){
    mqttServer = _server;
    mqttPort = _port;
    mqttClientId = _clientId;
    mqttUser = _user;
    mqttPass = _pass;
    return true;
}
bool homeWifi::getStatusConnectedWifi(){
    return isConnected;
}
bool homeWifi::checkConnectMQTT(){
    if(!mqttClient.connected()){
        DEBUGLN("Connecting to MQTT...");
        mqttClient.setServer(mqttServer, mqttPort);
        // Подключение с LWT сообщением "offline"
        if(mqttClient.connect(mqttClientId, 
                              mqttUser,
                              mqttPass,
                              mqttTopicDeviceStatus,
                              mqttDeviceStatusQos,
                              mqttDeviceStatusRetained,
                              mqttDeviceStatusOff)){
            DEBUGLN("ok");
            // Публикуем статус устройства в тот же топик
            mqttClient.publish(mqttTopicDeviceStatus,
                               mqttDeviceStatusOn,
                               mqttDeviceStatusRetained);
            mqttClient.subscribe(mqttTopicGatesCmd);
            mqttClient.subscribe(mqttTopicSigLampCmd);
            mqttClient.subscribe(mqttTopicHalfGatesCmd);
        }
        else{
            DEBUGLN("error: " + (String)mqttClient.state());
        }
        return mqttClient.connected();
    }
        // return false;
    // else
    return true;
}
void homeWifi::mqttLoop(){
    mqttClient.loop();
}
void homeWifi::setGatesStatus(bool statusGates){
    mqttClient.publish(mqttTopicGatesStatus,
                       statusGates == true ? mqttGatesStatusOpen: mqttGatesStatusClose,
                       mqttGatesStatusRetained);
}
void homeWifi::setGatesCallback(){
    mqttClient.setCallback([this](char *t, uint8_t *p, unsigned int l) { callback(t, p, l); });
}
void homeWifi::callback(char* topic, uint8_t* payload, unsigned int length) {
    
    String data_pay;
    for (unsigned int i = 0; i < length; i++) {
        data_pay += String((char)payload[i]);
    }

    DEBUGLN((String)topic + " " + data_pay);

    if( String(topic) == mqttTopicGatesCmd ){
        if(data_pay == "ON" || data_pay == "1") cmdMQTT = cmdMQTTOpen;
        if(data_pay == "OFF" || data_pay == "0") cmdMQTT = cmdMQTTClose;
    }
    else if( String(topic) == mqttTopicSigLampCmd ){
        if(data_pay == "ON" || data_pay == "1") cmdMQTT = cmdMQTTLampOn;
        if(data_pay == "OFF" || data_pay == "0") cmdMQTT = cmdMQTTLampOff;
    }
    else if( String(topic) == mqttTopicHalfGatesCmd ){
        if(data_pay == "ON" || data_pay == "1") cmdMQTT = cmdMQTTHalfOpen;
        if(data_pay == "OFF" || data_pay == "0") cmdMQTT = cmdMQTTHalfClose;
    }
}
void homeWifi::clearCmdMQTT(){
    cmdMQTT = cmdMQTTEmpty;
}