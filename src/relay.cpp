#include "relay.h"

Relay::Relay(){}
Relay::Relay(byte _pin, uint32_t _delay){
    pin = _pin;
    delayTime = _delay;
    remoteControl = false;
    permition = false;
    pinMode(pin, OUTPUT);
}
bool Relay::open(void){
    bool result = false;
    if((cmdOpen && !cmdClose) || remoteControl == true){    
        if((condition == false && permition == true) || remoteControl == true){
            condition = true;
            digitalWrite(pin, condition);
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

    return result;
}
bool Relay::open(uint32_t _delay){
    bool result = false;
    if(cmdOpen && !cmdClose){
        if((condition == false && permition == true) || remoteControl == true){
            if(delay == false){
                delay = true;
                currentTime = millis();
            }
            if (millis() >= (currentTime + _delay))
            {
                condition = true;
                digitalWrite(pin, condition);
                delay = false;
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
        errorStatus |= CMDOPCL;
    return result;
}
bool Relay::close(void){
    bool result = false;
    if(cmdClose && !cmdOpen){
        if(condition == true){
            condition = false;
            digitalWrite(pin, condition);
            result = true;
            remoteControl = false;
            cmdClose = false;
        } 
        else if(condition == true)
            errorStatus |= ISCLOSED; 
    }    
    else
        errorStatus |= CMDOPCL;
    return result;
}
bool Relay::close(uint32_t _delay){
    bool result = false;
    if(cmdClose && !cmdOpen){
        if(condition == true){
            if(delay == false){
                delay = true;
                currentTime = millis();
            }
            if (millis() >= (currentTime + _delay))
            {
                condition = false;
                digitalWrite(pin, condition);
                delay = false;
                result = true;
                cmdClose = false;
            }
            remoteControl = false;
        }
        else if(condition == true)
            errorStatus |= ISCLOSED;  
    }
    else
        errorStatus |= CMDOPCL;
    return result;    
}
bool Relay::extOpen(void){
    bool result = false;
    remoteControl = true;
    if(open())
        result = true;
    return result;
}
bool Relay::getCondition(void){
    return condition;
}
bool Relay::getPermition(void){
    return permition;
}
void Relay::setPermition(void){
    permition = true;
}
void Relay::unserPermition(void){
    permition = false;
}
byte Relay::getErrorStatus(void){
    return errorStatus;
}
void Relay::clearErrorStatus(void){
    errorStatus = 0x00;
    cmdClose = false;
    cmdOpen = false;
    delay = false;
    remoteControl = false;
}