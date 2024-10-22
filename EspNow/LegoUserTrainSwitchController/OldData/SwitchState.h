#ifndef SwitchState_H
#define SwitchState_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "AbstractState.h"

//this class contains all the information of the trains
class SwitchState: public AbstractState {
  public:
    SwitchState();
    SwitchState Init(String clientId,const uint8_t* pmacAddress);  
    void UpdateSwitchState();
    bool DoSwitch=false;
    void CreateJson();// override;

  };

  #endif
