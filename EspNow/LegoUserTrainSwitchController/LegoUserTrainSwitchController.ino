#include "RotaryBtn.h"
#include "Settings.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include "ClientHandler.h"
#include <Arduino_Helpers.h>
#include <AH/Hardware/Button.hpp>
#include "enums.h"

#define DEBUG true // Set to false to disable debug prints
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

int counter = 0;
int resendCount = 0;
int maxResendCount = 10;
int restartCount=0;
bool preState=1;
uint8_t ownMac[] = {0xC0, 0x4E, 0x30, 0x80, 0x72, 0x4C};

//char msg[80]={0};

ClientHandler clientHandler;

RotaryBtn SpeedBtn(5, 6, 1);
RotaryBtn SwitchBtn(7,8,11);

bool trainSelected = false;
bool reverseDirection = false;
long debounceDelta = 1000;
long pressTime =millis();

esp_now_peer_info_t peerInfo;
const int freq = 5000;//30000;
const int motorchannel1 = 0;
const int resolution = 8;


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) {

  debug("Packet received from: ");
  PrintMac(mac_addr);

  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(256);
  deserializeJson(doc, incomingData);

  String clientId = doc["ClientId"];
  String clientType= doc["ClientType"];

  //check if we need to add the client:
  //===================================
  if (clientHandler.CheckClient(clientId, (uint8_t*)mac_addr,clientType)) {
    //register the peer device:
    //=========================

    memcpy(peerInfo.peer_addr, mac_addr, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    //Serial.println(peerInfo.peer_addr);
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      debugln("Failed to add peer");
      return;
    }
  }
}

void IncreaseSpeedCallBack()
{
  debugln("Increase Speed");
  clientHandler.clientList[clientHandler.CurrentTrainId].UpdateState(10);
  SendMessage(clientHandler.clientList[clientHandler.CurrentTrainId].Idx);
}
void DecreaseSpeedCallBack()
{
  debugln("Decrease Speed");
  clientHandler.clientList[clientHandler.CurrentTrainId].UpdateState(-10);
  SendMessage(clientHandler.clientList[clientHandler.CurrentTrainId].Idx);
}
void UpdateSwitchStateCallback()
{
  debugln("Engage Switch");
  clientHandler.clientList[clientHandler.CurrentSwitchId].UpdateState(1);
  SendMessage(clientHandler.clientList[clientHandler.CurrentSwitchId].Idx); 
}
void NextTrainClient()
{
  debugln("Fetch next train");
  //deactivate blinking on current train
  clientHandler.clientList[clientHandler.CurrentTrainId].DeactivateBlink();
  SendMessage(clientHandler.clientList[clientHandler.CurrentTrainId].Idx);
  clientHandler.GoToNextClient(1,"Train");
  clientHandler.clientList[clientHandler.CurrentTrainId].ActivateBlink();
  SendMessage(clientHandler.clientList[clientHandler.CurrentTrainId].Idx);
}
void NextSwitchClient()
{
  debugln("Fetch next switch");
  clientHandler.GoToNextClient(1,"Switch");
}
void PreviousSwitchClient()
{
  debugln("Fetch previous switch");
  clientHandler.GoToNextClient(-1,"Switch"); 
}
void setup() {

  if(DEBUG)
  {
    Serial.begin(115200);
  }

  InitButtons();

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, ownMac);

  //Init ESP-NOW && restart the esp if it does not work
  while ((esp_now_init() != ESP_OK) && (restartCount<maxRetryCountEspNowStart)) {
    debugln("Error initializing ESP-NOW");
    restartCount+=1;
    delay(100);
  }
  if(restartCount==maxRetryCountEspNowStart)
  {
    ESP.restart();
  }

  debugln("EspNow started");

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  //Register the funtions:
  SpeedBtn.RegisterCallBack(Motion::Positive,IncreaseSpeedCallBack);
  SpeedBtn.RegisterCallBack(Motion::Negative, DecreaseSpeedCallBack);
  SpeedBtn.RegisterCallBack(Motion::BtnPress,NextTrainClient);

  SwitchBtn.RegisterCallBack(Motion::Positive, NextSwitchClient);
  SwitchBtn.RegisterCallBack(Motion::Negative, PreviousSwitchClient);
  SwitchBtn.RegisterCallBack(Motion::Negative, UpdateSwitchStateCallback);

  ledcSetup(motorchannel1,freq,resolution); 
  ledcAttachPin(Led,motorchannel1);
  ledcWrite(motorchannel1, 180);
}


void InitButtons() {
  pinMode(buttonEBreak, INPUT);
  pinMode(buttonShutDown, INPUT);
  pinMode(nextTrainBtn,INPUT);
  pinMode(PowerHold, OUTPUT);
  //pinMode(Led,OUTPUT);
  digitalWrite(PowerHold,HIGH);
  //digitalWrite(Led,HIGH);
}
void Blink(int cnt) {

  //for (int i = 0; i < cnt; i++) {
  //  digitalWrite(Led, LOW);
  //  delay(50);
  //  digitalWrite(Led, HIGH);
  //  delay(50);
  //}
}

void CheckReverseBtn(Button btn) {
  if (btn.update() == Button::Falling) {
    clientHandler.clientList[clientHandler.CurrentTrainId].ReverseDirection();
    return;
  }
}
//Shut yourself down please:
void ShutDown()
{
  digitalWrite(PowerHold,LOW);
}
void CheckShutDownStateBtn() 
{
  if (digitalRead(buttonShutDown) == HIGH) {
    //Go over all the clients and perform a shutDown
    for(int i=0;i<clientHandler.idx;i++)
    {
      clientHandler.clientList[i].EnableShutDown();
      SendMessage(i);     
    }   

    delay(100);
    ShutDown(); 
  }
}

void CheckEmergencyBrakeStateBtn() {
  if (digitalRead(buttonEBreak) == HIGH) {
     for(int i=0;i<clientHandler.idx;i++)
    {
      clientHandler.clientList[i].EnableEmergencyBrake();
      SendMessage(i);     
      clientHandler.clientList[clientHandler.CurrentTrainId].DisableEnableEmergencyBrake();
    }       
  }
}

void loop() {
  CheckShutDownStateBtn();
  CheckEmergencyBrakeStateBtn();
  //GetNextClient();
  
  SpeedBtn.CheckState();
  SwitchBtn.CheckState();  
}

void SendMessage(int index)
{
  debug(clientHandler.clientList[index].ClientId);
  PrintMac(clientHandler.clientList[index].MacAddress);
  String msg = clientHandler.clientList[index].CreateJson();
 
  char Msg[90];
  StaticJsonDocument<90> statusDoc;
  statusDoc["ClientId"] = clientHandler.clientList[index].ClientId;
  statusDoc["Power"] = clientHandler.clientList[index].State;
  statusDoc["ShutDown"] = clientHandler.clientList[index].ShutDown;
  statusDoc["eStop"] = clientHandler.clientList[index].EStop;
  statusDoc["EnableBlink"]=clientHandler.clientList[index].EnableBlinking;
  serializeJson(statusDoc, Msg);


  esp_err_t result = esp_now_send(clientHandler.clientList[index].MacAddress, (uint8_t*)&Msg, sizeof(Msg));
  //const uint8_t* myCString = (const uint8_t*)msg.c_str();
  debugln(Msg);
  //esp_err_t result = esp_now_send(clientHandler.clientList[index].MacAddress, Msg, sizeof(Msg));

  if (result == ESP_OK) {
    debugln("Sent with success");
    resendCount = 0;
  } else {

    debugln("Error sending the data");
    if (resendCount < maxResendCount) {
      resendCount += 1;
      SendMessage(index);
    }
  }

}

// void SendEspNow()
// {
//   debug("Sending data to:");
//   debug(clientHandler.CurrentTrain.ClientId);
//   //printMac(clientHandler.CurrentTrain.MacAddress);
//   debugln(msg);

//   //Serial.println(clientHandler.CurrentTrain.MacAddress);
  

//   if (result == ESP_OK) {
//     debugln("Sent with success");
//     resendCount = 0;
//   } else {

//     debugln("Error sending the data");
//     if (resendCount < maxResendCount) {
//       resendCount += 1;
//       SendEspNow();
//     }
//   }
// }
void PrintMac(const uint8_t* mac_addr) {
  char macStr[18];

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  debugln(macStr);
}
