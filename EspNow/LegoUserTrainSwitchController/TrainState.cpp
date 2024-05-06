#include "TrainState.h"

TrainState::TrainState()
{
  TrainId="NULL";
}

TrainState TrainState::Init(String clientId,const uint8_t* pmacAddress)
{
  TrainId=clientId;  
  SpeedState=0;
  Vbat=0;
  ShutDown=false;
  EStop=false;
  //*MacAddress=*pmacAddress;

  SaveMyMac(pmacAddress);
  return *this;
}

void TrainState::SaveMyMac(const uint8_t* pmacAddress)
{
  for(int i=0;i<6;i++)
  {
    MacAddress[i]=pmacAddress[i];
  } 
}

void TrainState::EnableShutDown()
{
  ShutDown=true;
}
void TrainState::EnableEmergencyBrake()
{
  EStop=true;
  SpeedState = 0;
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