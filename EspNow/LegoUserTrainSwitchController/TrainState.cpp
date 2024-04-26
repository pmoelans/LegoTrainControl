#include "TrainState.h"

TrainState::TrainState()
{
  
}
TrainState TrainState::Init(String clientId,const uint8_t* pmacAddress)
{
  TrainId=clientId;  
  SpeedState=0;
  Vbat=0;
  ShutDown=false;
  EStop=false;
  MacAddress=pmacAddress;

 
  return *this;
}
void  TrainState::EnableShutDown()
{
  ShutDown=true;
}
void  TrainState::EnableEmergencyBrake()
{
  EStop=true;
}
void  TrainState::UpdateSpeed(int speed)
{
  EStop= false;
  SpeedState=speed;
}
String TrainState::CreateJsonMessage()
{
  char Message[80] = { 0 };
  StaticJsonDocument<90> statusDoc;
  statusDoc["TrainId"] = TrainId;
  statusDoc["Power"] =SpeedState;
  statusDoc["ShutDown"] = ShutDown;
  statusDoc["eStop"] = EStop;
  
  

  serializeJson(statusDoc, Message); 
  String myString(Message);
 
  return myString;
}