#include "relayIn.h"

RelayIn::RelayIn(){}
RelayIn::RelayIn(byte _pin, byte _mode){
    pin = _pin;
    mode = _mode;
    invert = false;
}
RelayIn::RelayIn(byte _pin, byte _mode, bool _invert){
    pin = _pin;
    mode = _mode;
    invert = _invert;
}
bool RelayIn::getCondition(void){
    condition = digitalRead(pin);
    if(invert)
        return !condition;     
    return condition;
}

byte RelayIn::getErrorStatus(void){
    return errorStatus;
}
void RelayIn::clearErrorStatus(void){
    errorStatus = 0x00;
}

void RelayIn::beginInit(){
    if(((mode & INPUT) == INPUT) || ((mode & INPUT_PULLUP) == INPUT_PULLUP)){   
        pinMode(pin, mode);
    }
    else
        errorStatus |= ERRMODE;
}
// Включение прерывания
void RelayIn::beginInterrupt(void (*userFunc)(void)){
    attachInterrupt(digitalPinToInterrupt(pin), userFunc, CHANGE);
}   
// Отключение прерывания
void RelayIn::offInt(){
    detachInterrupt(digitalPinToInterrupt(pin));
}   

// Описать в setup main.cpp
void RelayIn::intRising(){
    intFlag = true;
}

bool RelayIn::getInt(){
    return intFlag;
}

void RelayIn::clearInt(){
    intFlag = false;
}