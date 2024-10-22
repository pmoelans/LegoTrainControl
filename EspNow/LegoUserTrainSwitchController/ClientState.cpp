#include "ClientState.h"

ClientState::ClientState()
{
  ClientId="NULL";
}

ClientState ClientState::Init(String clientId,const uint8_t* pmacAddress,int idx,String clientType)
{
  ClientId=clientId;  
  EnableBlinking=false;
  State=0;
  Vbat=0;
  ShutDown=false;
  EStop=false;
 
  if(clientType=="Train")
  {
    _minState=-100;
    _maxState= 100;
  }
  else
  {
    _minState=0;
    _maxState= 1;    
  }

  Idx= idx;
  ClientType= clientType;

  //*MacAddress=*pmacAddress;

  SaveMyMac(pmacAddress);
  return *this;
}
void ClientState::ReverseDirection()
{
  _direction=_direction*-1;
}
 void ClientState::UpdateState(int delta)
 {
    State+=(delta);//*_direction);
    ShutDown=false;
    EStop=false;
    EnableBlinking=true;
    CheckState();
 }

void ClientState::CheckState()
{
  if(State>_maxState)
  {
    State=_maxState;
  }
  if(State<_minState)
  {
    State=_minState;
  }

}
void ClientState::EnableEmergencyBrake()
{
  EStop=true;
  State= 0;
  EnableBlinking=true;
}
void ClientState::DisableEnableEmergencyBrake()
{
  EStop=false;
  State= 0;
  EnableBlinking=true;
}
void ClientState::SaveMyMac(const uint8_t* pmacAddress)
{
  for(int i=0;i<6;i++)
  {
    MacAddress[i]=pmacAddress[i];
  } 
}
void ClientState::DeactivateBlink()
{
  EnableBlinking=false;
}
void ClientState::ActivateBlink()
{
  EnableBlinking=true;
}
void ClientState::EnableShutDown()
{
  EStop=true;
  ShutDown=true;
  State=0;
}

String ClientState::CreateJson()
{
  char Msg[80];
  StaticJsonDocument<90> statusDoc;
  statusDoc["ClientId"] = ClientId;
  statusDoc["Power"] = State;
  statusDoc["ShutDown"] = ShutDown;
  statusDoc["eStop"] = EStop;
  serializeJson(statusDoc, Msg);
  String stringObject = String(Msg);

  return stringObject;
}
