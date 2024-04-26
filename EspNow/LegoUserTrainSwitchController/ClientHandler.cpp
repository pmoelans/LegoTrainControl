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
bool ClientHandler::IsNewClient(String client, const uint8_t* pMacAddress)
{
   for (int i = 0; i < 15; i++) {
        if (clientList[i].TrainId == client) {
            return false;
            Serial.print("Existing Client:");
            Serial.println(client);
        }
    }
    //get the value of the pointer and use it to initialize the trainstate
    clientList[idx]=TrainState().Init(client,pMacAddress);
    Serial.print("New client:");
    Serial.println(client);

    if(idx==0)
    {
      CurrentTrain=clientList[idx];
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

void ClientHandler::UpdateSpeed(int speed)
{ 
  Serial.println("Updating Speed");
  CurrentTrain.UpdateSpeed(speed);

  Serial.print("Updating train at index:");
  Serial.println(currentClientIndex);

  clientList[currentClientIndex]= CurrentTrain;
  Serial.println("Speed Updated");

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

void ClientHandler::GoToNextClient(int direction)
{
  currentClientIndex=currentClientIndex+direction;
  CheckIndex();
  CurrentTrain= clientList[currentClientIndex];
  Serial.print("Current train is:");
  Serial.println(CurrentTrain.TrainId);

}