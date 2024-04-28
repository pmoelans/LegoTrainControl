#include "TrainState.h"

TrainState::TrainState()
{
  TrainId="NULL";
}

void TrainState::PrintMac(uint8_t* mac_addr)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
}

TrainState TrainState::Init(String clientId,const uint8_t* pmacAddress)
{
  TrainId=clientId;  
  SpeedState=0;
  Vbat=0;
  ShutDown=false;
  EStop=false;
  //*MacAddress=*pmacAddress;

  PrintMyMac();
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
void TrainState::PrintMyMac()
{
  char macStr[18];
  Serial.print("MyMac:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
  Serial.println(macStr);
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
  PrintMyMac();
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