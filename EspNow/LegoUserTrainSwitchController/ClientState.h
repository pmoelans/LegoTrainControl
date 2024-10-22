#ifndef ClientState_H
#define ClientState_H

#include <Arduino.h>
#include <ArduinoJson.h>

//this class contains all the information of the trains
class ClientState{
  public:
    void EnableShutDown();
    float Vbat;
    String ClientType;
    int Idx; //index in the array:
    String ClientId;
    uint8_t  MacAddress[6];
    bool ShutDown;
    bool EnableBlinking;
    void SaveMyMac(const uint8_t* pmacAddress);
   
    ClientState();
    ClientState Init(String clientId,const uint8_t* pmacAddress, int idx, String clientType);  

    void UpdateState(int delta);
    void ReverseDirection();
    void EnableEmergencyBrake();
    void DisableEnableEmergencyBrake();
    void DeactivateBlink();
    void ActivateBlink();

    String CreateJson();// override;
   
    int State=0;

    bool EStop=false;
    void PrintMyMac();  
  private:
    int _maxState=100;
    int _minState = -100;
    int _direction=1;

    void CheckState();
  };

  #endif
