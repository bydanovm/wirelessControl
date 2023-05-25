#include "relayIn.h"
#include "PinChangeInterrupt.h"

RelayIn::RelayIn(){}
RelayIn::RelayIn(byte _pin, byte _mode){
    pin = _pin;
    mode = _mode;
    pinMode(pin, mode);
}
bool RelayIn::getCondition(void){
    return condition;
}

byte RelayIn::getErrorStatus(void){
    return errorStatus;
}
void RelayIn::clearErrorStatus(void){
    errorStatus = 0x00;
}

// Включение прерывания
void RelayIn::onInt(){
    if(((mode & INPUT) == INPUT) || ((mode & INPUT_PULLUP) == INPUT_PULLUP)){
        switch(pin){
            case inRelay1Pin:
                attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay1Int, CHANGE);
                instances[0] = this;
                break;
            case inRelay2Pin:
                attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay2Int, CHANGE);
                instances[1] = this;
                break;
            case inRelay3Pin:
                attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay3Int, CHANGE);
                instances[2] = this;
                break;
            case inRelay4Pin:
                attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay4Int, CHANGE);
                instances[3] = this;
                break;
        }
    }
    else
        errorStatus |= ERRMODE;
}   
// Отключение прерывания
void RelayIn::offInt(){
    detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin));
}   

void RelayIn::inRelay1Int(){
    if(RelayIn::instances[0] != NULL ){
        RelayIn::instances[0]->intRising();
    }
}

void RelayIn::inRelay2Int(){
    if(RelayIn::instances[1] != NULL ){
        RelayIn::instances[1]->intRising();
    }
}

void RelayIn::inRelay3Int(){
    if(RelayIn::instances[2] != NULL ){
        RelayIn::instances[2]->intRising();
    }
}

void RelayIn::inRelay4Int(){
    if(RelayIn::instances[3] != NULL ){
        RelayIn::instances[3]->intRising();
    }
}

void RelayIn::intRising(){
    intFlag = digitalRead(pin);
}

bool RelayIn::getInt(){
    return intFlag;
}