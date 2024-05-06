#include "RotaryBtn.h"
#include "Settings.h"
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ClientHandler.h"
#include <Arduino_Helpers.h>
#include <AH/Hardware/Button.hpp>

#define DEBUG false // Set to false to disable debug prints
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

int previousSpeed = 0;
int counter = 0;
int currentElement = 0;
int resendCount = 0;
int maxResendCount = 10;
int restartCount=0;
bool preState=1;


int lastSpeedState;
int lastSwitchState;


Button buttonReverseDirection{ 3 };

ClientHandler trainHandler;
RotaryBtn SpeedBtn(5, 6, 10, 5, -100, 100);
bool trainSelected = false;
bool reverseDirection = false;
long debounceDelta = 1000;
long pressTime =millis();
char msg[80] = { 0 };
esp_now_peer_info_t peerInfo;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) {

  debug("Packet received from: ");
  PrintMac(mac_addr);

  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(256);
  deserializeJson(doc, incomingData);

  String clientId = doc["TrainId"];

  //check if we need to add the client:
  //===================================
  if (trainHandler.IsNewClient(clientId, (uint8_t*)mac_addr)) {
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

void setup() {

  if(DEBUG)
  {
    Serial.begin(115200);
  }

  InitButtons();

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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
}


void InitButtons() {
  pinMode(buttonEBreak, INPUT);
  pinMode(buttonShutDown, INPUT);
  pinMode(nextTrainBtn,INPUT);
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
    SpeedBtn.ReverseDirection();
    return;
  }
}
void GetNextClient()
{
  bool currentState=digitalRead(nextTrainBtn);
  
  if(currentState==0 && preState==1&& ((millis()-pressTime)>debounceDelta))
  {
    pressTime= millis();

    debugln("Fetching next client");
    trainHandler.GoToNextClient(1);
    SpeedBtn.SetCounter(trainHandler.CurrentTrain.SpeedState);    
  }
  preState=currentState;
}
void CheckShutDownStateBtn() {
  if (digitalRead(buttonShutDown) == HIGH) {
    trainHandler.ResetIndex();
    while (trainHandler.CanFetchNext()) {
      trainHandler.GoToNextClient(1);
      trainHandler.CurrentTrain.EnableShutDown();
      SendTrainMessage();
    }
  }
}
void CheckEmergencyBrakeStateBtn() {
  if (digitalRead(buttonEBreak) == HIGH) {
    trainHandler.CurrentTrain.EnableEmergencyBrake();
    SendTrainMessage();
    //lastSpeedState=0;
    //trainHandler.UpdateSpeed(0);
    SpeedBtn.ResetCounter();
  }
}

void loop() {
  CheckShutDownStateBtn();
  CheckEmergencyBrakeStateBtn();
  GetNextClient();
  //CheckReverseBtn(buttonReverseDirection);

  int speedSetting = SpeedBtn.ReadRotaryState();

  if (speedSetting != lastSpeedState) {
    trainHandler.UpdateSpeed(speedSetting);
    SendTrainMessage();
  }
  lastSpeedState = speedSetting;
}
void SendTrainMessage() {
  // Send message via ESP-NOW
  
  StaticJsonDocument<90> statusDoc;
  statusDoc["TrainId"] = trainHandler.CurrentTrain.TrainId;
  statusDoc["Power"] = trainHandler.CurrentTrain.SpeedState;
  statusDoc["ShutDown"] = trainHandler.CurrentTrain.ShutDown;
  statusDoc["eStop"] = trainHandler.CurrentTrain.EStop;
  serializeJson(statusDoc, msg);
  
  resendCount=0;
  SendEspNow();
}
void SendEspNow()
{
  debug("Sending data to:");
  debug(trainHandler.CurrentTrain.TrainId);
  debugln(msg);

  //Serial.println(trainHandler.CurrentTrain.MacAddress);
  esp_err_t result = esp_now_send(trainHandler.CurrentTrain.MacAddress, (uint8_t*)&msg, sizeof(msg));

  if (result == ESP_OK) {
    debugln("Sent with success");
    resendCount = 0;
  } else {

    debugln("Error sending the data");
    if (resendCount < maxResendCount) {
      resendCount += 1;
      SendEspNow();
    }
  }
}
void PrintMac(const uint8_t* mac_addr) {
  char macStr[18];

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  debugln(macStr);
}
