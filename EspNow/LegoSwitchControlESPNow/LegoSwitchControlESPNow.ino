//Versions
//version 1.3: Uses the chip instead of the break-out board
//01/04/2024: adding multi-tasking to the mix

#include "secrets.h"
#include <ArduinoJson.h>
#include <esp_now.h>
#include <WiFi.h>
#include "QtPy.h"

#define WaitTime 1000  //the time we wait before we engage the switch off button. Or trigger on a second button press

esp_now_peer_info_t peerInfo;

bool shutDown;
bool shutDownRequested = false;
long millis_pre;
long millis_switch;
bool msgReceived = false;

long buttonTimer = 0;
long longPressTime = 750;
long switchingTimeOut = 100;

bool buttonActive = false;
bool longPressActive = false;
bool DirectionWhenAtEnd1 = false;//was false
bool need2Switch= false;
//test
const int freq = 30000;
const int motorchannel1 = 0;
const int motorchannel2 = 1;
const int resolution = 8;
const int activationTime = 250;
int dutyCycle = 0;

//all the communication stuff runs on a seperate core
//===================================================
void OnDataRecv(const uint8_t* mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  
  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(90);
  deserializeJson(doc, incomingData);
  
  for (size_t i = 0; i < sizeof(incomingData); ++i) {
    Serial.print(incomingData[i]);
    //Serial.print(" "); // Add a space between values
  }
  Serial.println();
  shutDown = doc["ShutDown"];
  Serial.println(shutDown);
  //requestedPower = doc["Power"];
  //Serial.println(requestedPower);
  //Blink the status led to indicate that we have received a message
  Blink(1); 
}
void setup() {
  millis_pre = millis();
  pinMode(en1A, OUTPUT);
  pinMode(en2A, OUTPUT);
  pinMode(en12, OUTPUT);
  pinMode(en3A, OUTPUT);
  pinMode(en4A, OUTPUT);
  pinMode(en34, OUTPUT);
  pinMode(End1, INPUT);
  pinMode(End2, INPUT);

  digitalWrite(en34, LOW);
  digitalWrite(en12, LOW);

  pinMode(BtnPress, INPUT);

  //keep the arduino powered on
  pinMode(shutDownPin, OUTPUT);
  digitalWrite(shutDownPin, HIGH);

  //Set the led state to indicate that the controller is live
  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);

  Serial.begin(115200);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
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
    Serial.println("Failed to add peer");
    return;
  }

  //esp32 PWM
  ledcSetup(motorchannel1, freq, resolution);
  ledcSetup(motorchannel2, freq, resolution);

  ledcAttachPin(en12, motorchannel1);
  ledcAttachPin(en34, motorchannel2);

  DetermineStartPosition();
}

void DetermineStartPosition()
{
  //Determine the start position:
  //=============================
  if(digitalRead(End1)==1)
  {
    DirectionWhenAtEnd1=true;
    return;
  }
  if(digitalRead(End2)==1)
  {
    DirectionWhenAtEnd1=false;
    return;
  }
  //unkown position, switch to a position:
  //======================================
  UpdateSwitchState();
}

void Blink(int cnt) {

  for (int i = 0; i < cnt; i++) {
    digitalWrite(Led, LOW);
    delay(50);
    digitalWrite(Led, HIGH);
    delay(50);
  }
}

void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["SwitchId"] = ClientId;
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Message, sizeof(Message));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    deltaMilli=defaultUpdateFrequency;//only send data every 10s after registering   
  }
  else 
  {
    Blink(1);
    Serial.println("Error sending the data");
  }  
}
float MeasureVbat() {

  return ((float)map(analogRead(Vbat), 0, 4095, 0, 360)) / 10;  //we need to find a proper bridge for the batterymonitor
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived for this train topic: ");
  Serial.println(topic);

  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  //divertA = doc["Direction"];
  shutDown = doc["ShutDown"];

  Serial.println(shutDown);
  //Blink the status led to indicate that we have received a message
  Blink(1);

  UpdateSwitchState();

  millis_switch = millis();
}
void UpdateSwitchState() {
 
  if (DirectionWhenAtEnd1) {
    digitalWrite(en1A, LOW);
    digitalWrite(en2A, HIGH);    
  } else {
    digitalWrite(en1A, HIGH);
    digitalWrite(en2A, LOW);
  }
  need2Switch = true;
  //set the power:
  //==============
  ESP32PWM(254);  
}
void ESP32PWM(int percentage) {

  ledcWrite(motorchannel1, percentage);
  ledcWrite(motorchannel2, percentage);
  millis_switch = millis();
}

void StopEngine() {
  ESP32PWM(0);
}

void PowerDown() {
  if (shutDown > 0) {
    //Serial.println("Shutting down");
    digitalWrite(shutDownPin, LOW);
    digitalWrite(en1A, LOW);
    digitalWrite(en2A, LOW);
    digitalWrite(en12, LOW);
    digitalWrite(en3A, LOW);
    digitalWrite(en4A, LOW);
    digitalWrite(en34, LOW);
    digitalWrite(Led, LOW);
  }
}
void CheckBtnState() {
  if (digitalRead(BtnPress) == HIGH) {
    //button press is detected
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
      return;
    }

    if ((millis() - buttonTimer > longPressTime)) {
      //There is a long button press detected
      //=====================================
      longPressActive = true;
      return;
    }
  } else  //readbtn press is LOW
  {
    //Check that a button press is active
    if (buttonActive == true) {
      if (longPressActive == false) {
        //short press, just do the switch
        Serial.println("Switching!");       
        UpdateSwitchState();
      } else {
        //Long press is detected:
        //=======================
        shutDown = 1;
        delay(200);
      }
      //reset the button state
      buttonActive = false;
      return;
    }
  }
}

void loop() 
{
  //Check if we need to perform a power down
  PowerDown();

  //detect the buttonpress
  if (millis() > WaitTime) {
    CheckBtnState();
  }

  if((millis()-millis_switch>switchingTimeOut) && need2Switch)
  {
    //Check if end1/2 is Reached
    //=======================
    if(digitalRead(End1)==1|| digitalRead(End2)==1 )
    {
      Serial.print("End1:");
      Serial.println(digitalRead(End1));
      Serial.print("End2:");
      Serial.println(digitalRead(End2));
     // delay(20);
      ESP32PWM(0);
      //update the direction:
      //=====================
      DirectionWhenAtEnd1=!DirectionWhenAtEnd1;

      need2Switch=false;
    }    
  }
}
