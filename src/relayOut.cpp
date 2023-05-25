#include "relayOut.h"

RelayOut::RelayOut(){}
RelayOut::RelayOut(byte _pin, byte _mode){
    pin = _pin;
    mode = _mode;
    remoteControl = false;
    permition = false;
    pinMode(pin, mode);
}
bool RelayOut::open(void){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if((cmdOpen && !cmdClose) || remoteControl == true){    
            if((condition == false && permition == true) || remoteControl == true){
                condition = true;
                digitalWrite(pin, condition);
                if(getCountOpenRelay() < countObjects)
                    staticCountOpenRelayIncr();
                result = true;
                cmdOpen = false;
            }
            else if(permition == false)
                errorStatus |= NOTPERM; 
            else if(condition == true)
                errorStatus |= ISOPENED; 
        }
        else
            errorStatus |= CMDOPCL;
    }
    else
        errorStatus |= ERRMODE;
    return result;
}
bool RelayOut::open(uint32_t _delay){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if((condition == false && permition == true) || remoteControl == true){
            if(delayOpen == false){
                delayOpen = true;
                currentTimeOpen = millis();
            }
            if (millis() >= (currentTimeOpen + _delay))
            {
                condition = true;
                digitalWrite(pin, condition);
                if(getCountOpenRelay() < countObjects)
                    staticCountOpenRelayIncr();
                delayOpen = false;
                result = true;
                cmdOpen = false;
            }
        }
        else if(permition == false)
            errorStatus |= NOTPERM; 
        else if(condition == true)
            errorStatus |= ISOPENED;  
    }
    else
        errorStatus |= ERRMODE;

    return result;
}
bool RelayOut::close(void){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if(condition == true){
            condition = false;
            digitalWrite(pin, condition);
            if(getCountOpenRelay() > 0)
                staticCountOpenRelayDecr();
            result = true;
            remoteControl = false;
            cmdClose = false;
        } 
        else if(condition == false)
            errorStatus |= ISCLOSED; 
    }
    else
        errorStatus |= ERRMODE;

    return result;
}
bool RelayOut::close(uint32_t _delay){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if(condition == true){
            if(delayClose == false){
                delayClose = true;
                currentTimeClose = millis();
            }
            if (millis() >= (currentTimeClose + _delay))
            {
                condition = false;
                digitalWrite(pin, condition);
                if(getCountOpenRelay() > 0)
                    staticCountOpenRelayDecr();                    
                delayClose = false;
                result = true;
                cmdClose = false;
            }
            remoteControl = false;
        }
        else if(condition == false)
            errorStatus |= ISCLOSED;  
    }
    else
        errorStatus |= ERRMODE;

    return result;    
}
bool RelayOut::extOpen(void){
    bool result = false;
    remoteControl = true;
    if(open())
        result = true;
    return result;
}
bool RelayOut::getCondition(void){
    return condition;
}
bool RelayOut::getPermition(void){
    return permition;
}
void RelayOut::setPermition(void){
    if((mode & OUTPUT) == OUTPUT)
        permition = true;
    else
        errorStatus |= ERRMODE;
}
void RelayOut::unsetPermition(void){
    if((mode & OUTPUT) == OUTPUT)
        permition = false;
    else
        errorStatus |= ERRMODE;
}
byte RelayOut::getErrorStatus(void){
    return errorStatus;
}
void RelayOut::clearErrorStatus(void){
    errorStatus = 0x00;
    cmdClose = false;
    cmdOpen = false;
    delayOpen = false;
    delayClose = false;
    remoteControl = false;
}
// Статика
uint8_t RelayOut::staticCountOpenRelay = 0;
void RelayOut::staticCountOpenRelayIncr(){
    staticCountOpenRelay++;
}
void RelayOut::staticCountOpenRelayDecr(){
    staticCountOpenRelay--;
}

uint8_t RelayOut::getCountOpenRelay(){
    return staticCountOpenRelay;
}