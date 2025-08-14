//Versions
//version 1.3: Uses the chip instead of the break-out board

//13/03/'25
//limit the power/traction when the battery voltage is dropping.
//also keep the registration going until we receive message from the transmitter

#include "secrets.h"
#include <ArduinoJson.h>
#include <esp_now.h>
#include <WiFi.h>
#include "QtPy.h"
#include "StatusLed.h"
#include "EngineControllerStep.h"
#include "BtnPress.h"

#define WaitTime 2000 //the time we wait before we engage the switch off button. Or trigger on a second button press

#define DEBUG true // Set to false to disable debug prints
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

//need to fill this in with the mac address if the receiver
//==========================================================
esp_now_peer_info_t peerInfo;

double VbatLimit = 7.5; //the battery voltage at which we are going to limit the power
double VbatMax = 9;
double vbat = 9;
bool eStop;
bool shutDown;
bool shutDownRequested=false;
bool enableBlinking=false;
long millis_pre;
long deltaMilli = 500; //interval with which we are sending data to the controller
int errorFactorVBatPower = 40;

long millisrealTimePre = 0;
char*  ClientId;

//test
StatusLed statusLed(500,500,Led,5,100);
StatusLed trainLights(500,500,LightsOn,5,100);
EngineControllerStep Engine1(100,5,en12, en1A, en2A,engineOn);
EngineControllerStep Engine2(100,5,en34, en4A, en3A,engineOn);
BtnPress Btn1(Btn,1000);


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t *incomingData, int len) {
    
  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(90);
  deserializeJson(doc, incomingData);
  
  eStop = doc["eStop"];
  shutDown = doc["ShutDown"];
  bool changeLights=doc["ChangeLights"];

  //Change the state of the lights. 
  //We don't want to keep it in memory
  //==================================
  if(changeLights)
  {
    trainLights.ToggleState();
  }

  Engine1.SetPower((int)doc["Power"]);
  Engine2.SetPower(-(int)doc["Power"]);

  //digitalWrite(engineOn, HIGH);
  //check if we are allow to blink:
  enableBlinking=doc["EnableBlink"];
  if(enableBlinking>0)
  {
    statusLed.StartFastBlink();
  }
  else
  {
    statusLed.StopBlink();
  }

  //Check if we need to perform an emergency brake
  EStop();

  //Update the power settings
  VbatUpdateEngineState();

  //only send data every 10s after registering
  deltaMilli=defaultUpdateFrequency;
  
}

void setup() 
{
 
  
  //keep the arduino powered on
  pinMode(shutDownPin, OUTPUT);
  digitalWrite(shutDownPin, HIGH);

  //Set the led state to indicate that the controller is live
  if(DEBUG)
  {
    Serial.begin(115200);
  }
 // Get the MAC address of the device
  String macAddress = WiFi.macAddress();

  // Convert the MAC address to a char*
  char macAddressChar[macAddress.length() + 1];
  macAddress.toCharArray(macAddressChar, macAddress.length() + 1);
  ClientId=macAddressChar;

  debug("ClientId:");
  debugln(ClientId);

  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    debugln("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv); 
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    debugln("Failed to add peer");
    return;
  }

  trainLights.On();
  millisrealTimePre = millis();
}

//send the status to the controller or register if it is the first time:
//=====================================================================
void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["ClientId"] = ClientId;
  statusDoc["ClientType"]= "Train"; //define the type
  statusDoc["Power1"] = Engine1.GetCurrentPower();
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Message, sizeof(Message));
   
  if (result == ESP_OK) {
    debugln("Sent with success");   
   
  }
  else 
  {
    debugln("Error sending the data");
  }  
}
double MeasureVbat() {
  vbat = (analogReadMilliVolts(Vbat)*3)/1000;
  return vbat ;  //we need to find a proper bridge for the batterymonitor
  
}

void EStop() {
  if (eStop ==1) {
    // Serial.println("Estop");
    //currentPower = 0;
    debugln("EstopActive");
    Engine1.EStop();
    Engine2.EStop();
    eStop=0;
  }
}

void PowerDown() {
  if (shutDown ==1  ) {
    debugln("ShutDown");
    Engine1.PowerDown();
    Engine2.PowerDown();
    digitalWrite(shutDownPin, LOW);
    digitalWrite(Led, LOW);
    digitalWrite(LightsOn, LOW);

  }
}
void VbatUpdateEngineState()
{
  double vbatMeas = MeasureVbat();

  Engine1.Update();
  Engine2.Update();

  PowerDown();
}

void loop() 
{  
  //send the data to the controller
  //===============================
  if ((millis() - millis_pre) > deltaMilli) 
  {    
    SendStatus();   
    
    millis_pre = millis();
  }

  //Measure the battery voltage and update the power
  //================================================
  VbatUpdateEngineState(); 
   
  //Update the state of the led:
  statusLed.UpdateState();

  millisrealTimePre = millis();

  //detect the buttonpress
  if(millis()>WaitTime)
  {
    if(Btn1.LongPressDetected())
    {
      shutDown=true;
    }    
  }
}
