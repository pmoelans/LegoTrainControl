//Versions
//version 1.3: Uses the chip instead of the break-out board

#include "Mqtt.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include "WiFiAutoSelector.h"
#include "QtPy.h"

#define WaitTime 2000 //the time we wait before we engage the switch off button. Or trigger on a second button press

#define WIFI_CONNECT_TIMEOUT 8000
WiFiAutoSelector wifiAutoSelector(WIFI_CONNECT_TIMEOUT);


MqttConnector mqttConnector;
bool divertA = 0;
bool divertB=0;

bool eStop;
bool shutDown;
bool shutDownRequested=false;
long millis_pre;
long millis_switch;
bool msgReceived = false;

//test
const int freq = 30000;
const int motorchannel1 = 0;
const int motorchannel2 = 1;
const int resolution = 8;
int dutyCycle = 0;



void setup() {

  
  millis_pre = millis();
  pinMode(en1A, OUTPUT);
  pinMode(en2A, OUTPUT);
  pinMode(en12, OUTPUT);
  pinMode(en3A, OUTPUT);
  pinMode(en4A, OUTPUT);
  pinMode(en34, OUTPUT);
  pinMode(engineOn, OUTPUT);

  digitalWrite(en34,LOW);
  digitalWrite(en12,LOW);
  digitalWrite(engineOn, LOW);

  pinMode(BtnPress,INPUT);

  //keep the arduino powered on
  pinMode(shutDownPin, OUTPUT);
  digitalWrite(shutDownPin, HIGH);

  //Set the led state to indicate that the controller is live
  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);

  Serial.begin(115200);

  wifiAutoSelector.add(SECRET_SSID, SECRET_PASS);
  wifiAutoSelector.add(SECRET_SSID2, SECRET_PASS2);

  mqttConnector.init(MqttBroker, MqttPort, ClientId);

  //esp32 PWM
 ledcSetup(motorchannel1,freq,resolution);
 ledcSetup(motorchannel2,freq,resolution);

 ledcAttachPin(en12,motorchannel1);
 ledcAttachPin(en34,motorchannel2);

 digitalWrite(engineOn, HIGH);
 //millisAcceleration = millis();
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
void ResubScribe() {
  mqttConnector.client.subscribe(Topic2Subscribe);
  mqttConnector.client.setCallback(callback);
}
void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["SwitchId"] = ClientId;
  statusDoc["Direction"] = divertA;
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);
  bool resubsribeNeeded = mqttConnector.publish(StatusTopic, Message);
  if (resubsribeNeeded) {
    ResubScribe();
  }
}
float MeasureVbat() {
 
  return ((float)map(analogRead(Vbat), 0, 4095, 0, 360)) / 10;  //we need to find a proper bridge for the batterymonitor
  
}
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived for this train topic: ");
  Serial.println(topic);

  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  divertA= doc["Direction"];  
  shutDown = doc["ShutDown"];

  Serial.println(shutDown);
 //Blink the status led to indicate that we have received a message
  Blink(1);

  UpdateSwitchState();

  millis_switch= millis();
}
void UpdateSwitchState() {

  //Serial.println(currentPower);
  //map it to a percentage

  bool directionA = HIGH;
  bool invA = LOW;

  if(divertA==1)
  {
    directionA = LOW;
    invA=HIGH;
  } 

  Serial.print("Direction Change:");
  Serial.println(directionA);

  digitalWrite(en1A, directionA);
  digitalWrite(en2A, invA);

  digitalWrite(en3A, directionA);
  digitalWrite(en4A, invA);

  //set the power:
  //==============
  ESP32PWM(254);

  delay(170);

  ESP32PWM(0);
  
}
void ESP32PWM(int percentage)
{
  
  ledcWrite(motorchannel1, percentage);
  ledcWrite(motorchannel2, percentage);
  millis_switch= millis();
}

void StopEngine() {
  digitalWrite(en1A, LOW);
  digitalWrite(en2A, LOW);
  digitalWrite(en3A, LOW);
  digitalWrite(en4A, LOW); 
}

void EStop() {
  if (eStop > 0) {
    StopEngine();
  }
}

void PowerDown() {
  if (shutDown > 0) {
   
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

void loop() {


  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting wifi ");
    if (-1 < wifiAutoSelector.scanAndConnect()) {
      int connectedIndex = wifiAutoSelector.getConnectedIndex();
      Serial.print("to '");
      Serial.print(wifiAutoSelector.getSSID(connectedIndex));
      Serial.println("'. Done.");

      //Reconnect to the broker:
      mqttConnector.connect();
      ResubScribe();
      Blink(3);
      
    } else {
      Serial.println("failed.");
      delay(100);
      return;
    }
  }
  
  mqttConnector.client.loop();
  // put your main code here, to run repeatedly:
  if ((millis() - millis_pre) > deltaMilli) {
    SendStatus();
    millis_pre = millis();
  }

  
  //Check if we need to perform a power down
  PowerDown();


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
