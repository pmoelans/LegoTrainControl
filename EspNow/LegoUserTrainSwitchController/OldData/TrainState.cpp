#include "TrainState.h"

TrainState::TrainState()
{
  ClientId="NULL";
}

TrainState TrainState::Init(String clientId,const uint8_t* pmacAddress)
{
  ClientId=clientId;  
  SpeedState=0;
  Vbat=0;
  ShutDown=false;
  EStop=false;
  //*MacAddress=*pmacAddress;

  SaveMyMac(pmacAddress);
  return *this;
}
void TrainState::ReverseDirection()
{
  _direction=_direction*1;
}
 void TrainState::IncreaseSpeed()
 {
  SpeedState+=_deltaSpeed*_direction;
  CheckSpeed();
 }
void TrainState::DecreaseSpeed()
{
  SpeedState-=_deltaSpeed*_direction;
  CheckSpeed();
}
void TrainState::CheckSpeed()
{
  if(SpeedState>_maxSpeed)
  {
      SpeedState=_maxSpeed;
  }
  if(SpeedState<_minSpeed)
  {
      SpeedState=_minSpeed;
  }

}
void TrainState::EnableEmergencyBrake()
{
  EStop=true;
  SpeedState = 0;
}
void TrainState::CreateJson()
{
  StaticJsonDocument<90> statusDoc;
  statusDoc["TrainId"] = ClientId;
  statusDoc["Power"] = SpeedState;
  statusDoc["ShutDown"] = ShutDown;
  statusDoc["eStop"] = EStop;
  serializeJson(statusDoc, Msg);
}
