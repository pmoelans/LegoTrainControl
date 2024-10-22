#include "SwitchState.h"

SwitchState::SwitchState()
{
  ClientId="NULL";
}

SwitchState SwitchState::Init(String clientId,const uint8_t* pmacAddress)
{
  ClientId=clientId;  
  Vbat=0;
  DoSwitch=false;
  ShutDown=false;
  SaveMyMac(pmacAddress);
  return *this;
}
void SwitchState::UpdateSwitchState()
{
  DoSwitch=true;
}
void SwitchState::CreateJson()
{
  StaticJsonDocument<90> statusDoc;
  statusDoc["SwitchId"] = ClientId;
  statusDoc["ShutDown"] = ShutDown;
  statusDoc["UpdateState"] = DoSwitch;
  serializeJson(statusDoc, Msg);
}



