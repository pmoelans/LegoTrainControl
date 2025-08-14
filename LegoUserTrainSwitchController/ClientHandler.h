#ifndef ClientHandler_H
#define ClientHandler_H

#include <Arduino.h>
#include "ClientState.h"

class ClientHandler
{
  public:

    //AbstractClientHandler();
    bool CheckClient(String client, const uint8_t* pMacAddress,String clientType);
      
    //advance the cursor such that the next client in line is the current client
    void GoToNextClient(int direction,String clientType);
   
    //check if there are clients in the list
    bool HasClients();
    //void UpdateSpeed(int speed);
    void ResetIndex();
    
    //TrainState CurrentTrain;
    bool CanFetchNext();
  
    //index at which we need to add the new client
    int currentClientIndex=-1;
   
    //TrainState clientList[15];
    int idx;
    void CheckIndex();


    ClientHandler();
    //bool IsNewClient(String client, const uint8_t* pMacAddress);   
  
    int CurrentTrainId;
    int CurrentSwitchId;

    void SetCurrentClient(int idx) ;
    bool IsNewClient(String client, int i) ;
    void AddNewClient(int idx,String client, const uint8_t* pMacAddress,String clientType) ;
    ClientState clientList[15];
    
  private:
   
    void UpdateIndex(int direction);
   
  };

  #endif
