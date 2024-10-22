#ifndef TrainState_H
#define TrainState_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "AbstractState.h"

//this class contains all the information of the trains
class TrainState:public AbstractState {
  public:
    TrainState();
    TrainState Init(String clientId,const uint8_t* pmacAddress);  
    void IncreaseSpeed();
    void DecreaseSpeed();
    void ReverseDirection();
    void EnableEmergencyBrake();
    void CreateJson();// override;
   
    int SpeedState;
    bool EStop;
    void PrintMyMac();  
  private:
    int _deltaSpeed=10;
    int _maxSpeed=100;
    int _minSpeed = -100;
    int _direction=1;

    void CheckSpeed();
  };

  #endif
