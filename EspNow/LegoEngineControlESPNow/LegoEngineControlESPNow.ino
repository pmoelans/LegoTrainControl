//Versions
//version 1.3: Uses the chip instead of the break-out board

#include "secrets.h"
#include <ArduinoJson.h>
#include <esp_now.h>
#include <WiFi.h>
#include "QtPy.h"
#include "StatusLed.h"

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
//C0:4E:30:80:72:4C
esp_now_peer_info_t peerInfo;

float currentPower = 0;
float requestedPower = 0;
float Acceleration= 0.004;
bool eStop;
bool shutDown;
bool shutDownRequested=false;
bool enableBlinking=false;
long millis_pre;
long deltaMilli = 500;
long millisrealTimePre = 0;
int previousDirection=0;
int previousPercentage=0;

//test
const int freq = 5000;//5000;//30000;
const int motorchannel1 = 0;
const int motorchannel2 = 1;
const int resolution = 8;
int dutyCycle = 0;
int factor=1;
StatusLed statusLed(500,500,Led,5,100);

long buttonTimer = 0;
long longPressTime = 1000;

bool buttonActive = false;
bool longPressActive = false;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t *incomingData, int len) {
    
  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(90);
  deserializeJson(doc, incomingData);
  
  eStop = doc["eStop"];
  //debug("Estop:");
  //debugln(eStop);
  shutDown = doc["ShutDown"];
  //debug("ShutDown:");
  //debugln(shutDown);
  requestedPower = doc["Power"];
  debug("Requested power:");
  debugln(requestedPower);
  //Blink the status led to indicate that we have received a message

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

  //Check if we need to perform a power down
  PowerDown();

  //Update the power settings
  UpdatePower();
  
}

void setup() 
{
  

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
  //pinMode(Led, OUTPUT);
  //digitalWrite(Led, HIGH);
  if(DEBUG)
  {
    Serial.begin(115200);
  }
 
  //Set device as a Wi-Fi Station
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

  millisrealTimePre = millis();

  //esp32 PWM
 ledcSetup(motorchannel1,freq,resolution);
 ledcSetup(motorchannel2,freq,resolution);
 ledcAttachPin(en12,motorchannel1);
 ledcAttachPin(en34,motorchannel2);

  digitalWrite(engineOn, HIGH);
}

void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["ClientId"] = ClientId;
  statusDoc["ClientType"]= "Train"; //define the type
  statusDoc["Power1"] = (int)currentPower;
  statusDoc["Vbat"] = 0;//MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Message, sizeof(Message));
   
  if (result == ESP_OK) {
    debugln("Sent with success");
    deltaMilli=defaultUpdateFrequency;//only send data every 10s after registering
    //SendUpdate=false;
   
  }
  else 
  {
    debugln("Error sending the data");
  }  
}
float MeasureVbat() {
  return ((float)map(analogRead(Vbat), 0, 4095, 0, 99)/10) ;  //we need to find a proper bridge for the batterymonitor
  
}

void UpdatePower() {
  
  if (requestedPower == ((int)currentPower)) {
   // Serial.println("Power is the same, returning");
    return;
  }
  
  float deltaV = Acceleration;
 
  if (requestedPower > currentPower) 
  {
    currentPower += deltaV;
  } else {
    currentPower -= deltaV;
  }
  // currentPower= requestedPower;

  //debug("currentPower:");
  //debugln(currentPower);
  //map it to a percentage
  int percentage = map(abs((int)currentPower), 0, 100, minPercentage, 255);

  if (percentage<=minPercentage)
  {
    percentage=0;
  }

  //only do an update if the percentage has changed
  if(previousPercentage==percentage)
  {
    previousPercentage=percentage;
    return;
  }

  bool direction = HIGH;
  bool inv = LOW;
  int currentDirection=1;

  if (currentPower < 0) 
  {
      direction=LOW;
      inv = HIGH;    
      currentDirection=-1;
  }

  //if(previousDirection!=currentDirection)
  //{
    digitalWrite(en1A, direction);
    digitalWrite(en2A, inv);

    //the second image just gives the invers of the other
    digitalWrite(en3A, inv);
    digitalWrite(en4A, direction);   
  //}
  previousDirection=currentDirection;

  //set the power:
  //==============
  debug("Percentage:");
  debugln(percentage);
  ESP32PWM(percentage);  
  previousPercentage=percentage;
}
void ESP32PWM(int percentage)
{
  ledcWrite(motorchannel1, percentage);
  ledcWrite(motorchannel2, percentage);
}

void StopEngine() {
   ESP32PWM(0);
   currentPower=0;

  // digitalWrite(en1A, LOW);
  // digitalWrite(en2A, LOW);
  // digitalWrite(en3A, LOW);
  // digitalWrite(en4A, LOW);
  
}

void EStop() {
  if (eStop ==1) {
    // Serial.println("Estop");
    //currentPower = 0;
    requestedPower = 0;
    debugln("EstopActive");
    StopEngine();
    eStop=0;
  }
}

void PowerDown() {
  if (shutDown ==1  ) {
    debugln("ShutDown");
    StopEngine();
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
      shutDown=true;
      longPressActive = true;
      return;
    }
  } else  //readbtn press is LOW
  {
    //Check that a button press is active
    if (buttonActive == true) {
      if (longPressActive == false) {
        //short press, do nothing
        
      } else {
        //Long press is detected:
        //=======================
        shutDown = true;
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
  // put your main code here, to run repeatedly:
  if ((millis() - millis_pre) > deltaMilli) 
  {    
    SendStatus();   

    UpdatePower();
    millis_pre = millis();
  }
  UpdatePower();
  PowerDown();
  
  float vbat = MeasureVbat();
  if (vbat < 7.5)
  {
    requestedPower = requestedPower*0.75;
    //StopEngine();
  }
 // debug("Vbat:");
  //debugln(MeasureVbat());

  //Update the state of the led:
  statusLed.UpdateState();

  millisrealTimePre = millis();


  //detect the buttonpress
  if(millis()>WaitTime)
  {
    CheckBtnState();
    // if(digitalRead(BtnPress)==HIGH)
    // {
    //   shutDownRequested=true;
    // }
    // if(digitalRead(BtnPress)==LOW && shutDownRequested)
    // {      
    //   shutDown=true;
    //   delay(200);
    // }
  }
}
