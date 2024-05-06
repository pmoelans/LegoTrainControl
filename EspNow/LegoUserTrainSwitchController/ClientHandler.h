#ifndef ClientHandler_H
#define ClientHandler_H

#include <Arduino.h>
#include "TrainState.h"

class ClientHandler {
  public:
    ClientHandler();
    bool IsNewClient(String client, const uint8_t* pMacAddress);
      
    //advance the cursor such that the next client in line is the current client
    void GoToNextClient(int direction);
   
    //check if there are clients in the list
    bool HasClients();
    void UpdateSpeed(int speed);
    void ResetIndex();
    TrainState CurrentTrain;
    bool CanFetchNext();
    
  private:
    
     //index at which we need to add the new client
    int currentClientIndex=-1;
   
    TrainState clientList[15];
    int idx;
    void CheckIndex();

  };

  #endif
