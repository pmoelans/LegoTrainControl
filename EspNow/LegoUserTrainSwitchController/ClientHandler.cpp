#include "ClientHandler.h"

ClientHandler::ClientHandler()
{
}
/*!
   * \brief Add the client if it does not already exists.
   *
   * \param client is the name of the client.
   * \param pMacAddress is the pointer to the macAddress reported by the train
   */

void ClientHandler::SetCurrentClient(int idx)
{
  if(clientList[idx].ClientType=="Train")
  {
    CurrentTrainId = idx;
  }
  else
  {
    CurrentSwitchId=idx;
  }  
}
bool ClientHandler::IsNewClient(String client, int i)
{
 return clientList[i].ClientId==client;
}
void  ClientHandler::AddNewClient(int idx,String client, const uint8_t* pMacAddress, String clientType)
{
  clientList[idx]=ClientState().Init(client,pMacAddress,idx,clientType);
}
bool ClientHandler::CheckClient(String client, const uint8_t* pMacAddress,String clientType)
{
   for (int i = 0; i < 15; i++) {
        if (IsNewClient(client,i))//clientList[i].TrainId == client) 
        {

            return false;
            //Serial.print("Existing Client:");
            //Serial.println(client);
        }
    }
    //get the value of the pointer and use it to initialize the trainstate
    //clientList[idx]=TrainState().Init(client,pMacAddress);
    AddNewClient(idx,client, pMacAddress,clientType);
    Serial.print("New client:");
    Serial.println(client);

    if(idx==0)
    {
      SetCurrentClient(idx);
      //CurrentTrain=clientList[idx];
      currentClientIndex=idx;
    }

    idx+=1;    
    return true;
}

bool ClientHandler::HasClients()
{
  return !(currentClientIndex==-1);
}
void ClientHandler:: ResetIndex()
{
  currentClientIndex=-1;
}
bool ClientHandler::CanFetchNext()
{
  return currentClientIndex<idx;  
}


void ClientHandler::CheckIndex()
{
     //check if we need to reset the index
  if(currentClientIndex>=idx)
  {
    currentClientIndex=0;
  }
  if(currentClientIndex<0)
  {
    currentClientIndex=idx-1;
  }
}
void ClientHandler::UpdateIndex(int direction)
{
  currentClientIndex=currentClientIndex+direction;
}

void ClientHandler::GoToNextClient(int direction,String clientType)
{  
  UpdateIndex(direction);
  CheckIndex();

  while(clientList[currentClientIndex].ClientType!=clientType)
  {
    UpdateIndex(direction);
    CheckIndex();
  }

  SetCurrentClient(currentClientIndex);//CurrentTrain= clientList[currentClientIndex];
  

}