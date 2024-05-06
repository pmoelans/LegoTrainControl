#ifndef TrainState_H
#define TrainState_H

#include <Arduino.h>
#include <ArduinoJson.h>

//this class contains all the information of the trains
class TrainState {
  public:
    TrainState();
    TrainState Init(String clientId,const uint8_t* pmacAddress);  
    void UpdateSpeed(int speed);
    void EnableEmergencyBrake();
    void EnableShutDown();

   
    float Vbat;
    String TrainId;
    uint8_t  MacAddress[6];
    int SpeedState;
    bool ShutDown;
    bool EStop;
    String CreateJsonMessage();
    void PrintMyMac();
  private:
    void SaveMyMac(const uint8_t* pmacAddress);
  };

  #endif
