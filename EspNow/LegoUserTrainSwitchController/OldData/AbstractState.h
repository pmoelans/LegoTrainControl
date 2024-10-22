#ifndef AbstractState_H
#define AbstractState_H

#include <Arduino.h>
#include <ArduinoJson.h>

//this class contains all the information of the trains
class AbstractState {
  public:
    
    void EnableShutDown();
    float Vbat;
    String ClientId;
    String ClientType;
    uint8_t  MacAddress[6];
    bool ShutDown;
    void SaveMyMac(const uint8_t* pmacAddress);
    char Msg[80];
    //virtual void CreateJson();
  private:
  };

  #endif
