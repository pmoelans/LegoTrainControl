#include "AbstractState.h"

void AbstractState::SaveMyMac(const uint8_t* pmacAddress)
{
  for(int i=0;i<6;i++)
  {
    MacAddress[i]=pmacAddress[i];
  } 
}

void AbstractState::EnableShutDown()
{
  ShutDown=true;
}