#include "homeWifi.h"
#include <ESP8266WiFi.h>

homeWifi::homeWifi(){}
homeWifi::homeWifi(char* _ssid, char* _pass){
    homeWifi::SSID = _ssid;
    homeWifi::PASS = _pass;
}
bool homeWifi::initConnect(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(homeWifi::SSID, homeWifi::PASS);
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
        }
        numberCurrAttemption++;
    }
    // Если количество попыток превышает "numberMaxAttemption", то
    // проверка не пройдена, нужен ручной сброс попыток
    if(numberCurrAttemption >= numberMaxAttemption && !isConnected) {
        isConnected = false;
        status |= CON_ERR;
    }
    // Простая проверка соединения каждые "delayAttemption" времени
    if(isConnected){
        if(millis() >= currentTime + delayAttemption){
            currentTime = millis();
            if(homeWifi::checkConnect()){
                isConnected = true;
                status ^= CON_ERR;
                status |= CON_OK;
            }
            else{
                isConnected = false;
                status ^= CON_OK;
                status |= CON_ERR;
                numberCurrAttemption = 0;
            }
        }
    }
    return isConnected;
}
bool homeWifi::handClear(){
    if(!isConnected && (status & CON_ERR)){
        numberCurrAttemption = 0;
    }
}