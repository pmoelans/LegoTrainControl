//Versions
//version 1.3: Uses the chip instead of the break-out board

#include "secrets.h"
#include <ArduinoJson.h>
#include <esp_now.h>
#include <WiFi.h>
#include "QtPy.h"

#define WaitTime 2000 //the time we wait before we engage the switch off button. Or trigger on a second button press

//need to fill this in with the mac address if the receiver
//==========================================================
//C0:4E:30:80:72:4C
esp_now_peer_info_t peerInfo;

float currentPower = 0;
float requestedPower = 0;
bool eStop;
bool shutDown;
bool shutDownRequested=false;
long millis_pre;
long deltaMilli = 500;
long millisrealTimePre = 0;

//test
const int freq = 30000;
const int motorchannel1 = 0;
const int motorchannel2 = 1;
const int resolution = 8;
int dutyCycle = 0;

// callback function that will be executed when data is received
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
  //Serial.println(incomingData);

  eStop = doc["eStop"];
  Serial.println(eStop);
  shutDown = doc["ShutDown"];
  Serial.println(shutDown);
  requestedPower = doc["Power"];
  Serial.println(requestedPower);
  //Blink the status led to indicate that we have received a message
  Blink(1); 
}

void setup() 
{
  
  millis_pre = millis();
  pinMode(en1A, OUTPUT);
  pinMode(en2A, OUTPUT);
  pinMode(en12, OUTPUT);
  pinMode(en3A, OUTPUT);
  pinMode(en4A, OUTPUT);
  pinMode(en34, OUTPUT);
  pinMode(engineOn, OUTPUT);
  digitalWrite(engineOn, HIGH);
  digitalWrite(en34,LOW);
  digitalWrite(en12,LOW);
  
  pinMode(BtnPress,INPUT);

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

  millisrealTimePre = millis();

  //esp32 PWM
 ledcSetup(motorchannel1,freq,resolution);
 ledcSetup(motorchannel2,freq,resolution);
 ledcAttachPin(en12,motorchannel1);
 ledcAttachPin(en34,motorchannel2);

  digitalWrite(engineOn, HIGH);
}

void Blink(int cnt) {

  for(int i=0;i<cnt;i++)
  {
    digitalWrite(Led, LOW);
    delay(50);
    digitalWrite(Led, HIGH);
    delay(50);
  }
  
}

void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["TrainId"] = ClientId;
  statusDoc["Power"] = (int)currentPower;
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Message, sizeof(Message));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    deltaMilli=defaultUpdateFrequency;//only send data every 10s after registering
  }
  else {
    Serial.println("Error sending the data");
  }  
}
float MeasureVbat() {
  return ((float)map(analogRead(Vbat), 0, 4095, 0, 360)) / 10;  //we need to find a proper bridge for the batterymonitor
  
}

void UpdatePower() {
  
  if (requestedPower == ((int)currentPower)) {
   // Serial.println("Power is the same, returning");
    return;
  }

  
  float deltaV = Acceleration;//*pow(deltaMillis/1000,2)/2;  //*(millis()-millisrealTimePre);
 
  //Serial.print("DeltaV:");
  //Serial.println(deltaV);
  if (requestedPower > currentPower) {
    currentPower += deltaV;
  } else {
    currentPower -= deltaV;
  }

  //Serial.println(currentPower);
  //map it to a percentage
  int percentage = map(abs((int)currentPower), 0, 100, minPercentage, 255);

  if (percentage<=minPercentage)
  {
    percentage=0;
  }

  bool direction = HIGH;
  bool inv = LOW;

  if (currentPower < 0) 
  {
      direction=LOW;
      inv = HIGH;    
  }

  digitalWrite(en1A, direction);
  digitalWrite(en2A, inv);

    //the second image just gives the invers of the other
  digitalWrite(en3A, inv);
  digitalWrite(en4A, direction);   

  //set the power:
  //==============
  ESP32PWM(percentage);  
}
void ESP32PWM(int percentage)
{
  ledcWrite(motorchannel1, percentage);
  ledcWrite(motorchannel2, percentage);
}

void StopEngine() {
  digitalWrite(en1A, LOW);
  digitalWrite(en2A, LOW);
  digitalWrite(en3A, LOW);
  digitalWrite(en4A, LOW);
  //digitalWrite(engineOn, LOW);
}

void EStop() {
  if (eStop > 0) {
    // Serial.println("Estop");
    currentPower = 0;
    requestedPower = 0;
    StopEngine();
  }
}

void PowerDown() {
  if (shutDown > 0) {
    //Serial.println("PowerDown");
    digitalWrite(shutDownPin, LOW);


    digitalWrite(en1A, LOW);
    digitalWrite(en2A, LOW);
    digitalWrite(en12, LOW);
    digitalWrite(en3A, LOW);
    digitalWrite(en4A, LOW);
    digitalWrite(en34, LOW);
    digitalWrite(Led, LOW);
    digitalWrite(engineOn, LOW);



  }
}

void loop() 
{  
  // put your main code here, to run repeatedly:
  if ((millis() - millis_pre) > deltaMilli) {
    SendStatus();
    millis_pre = millis();
  }

  //Check if we need to perform an emergency brake
  EStop();

  //Check if we need to perform a power down
  PowerDown();

  //Update the power settings
  UpdatePower();

  
  millisrealTimePre = millis();

  //detect the buttonpress
  if(millis()>WaitTime)
  {
    if(digitalRead(BtnPress)==HIGH)
    {
      shutDownRequested=true;
    }
    if(digitalRead(BtnPress)==LOW && shutDownRequested)
    {
      delay(200);
      shutDown=true;
    }
  }
}
