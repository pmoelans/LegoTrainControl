//Versions
//version 1.3: Uses the chip instead of the break-out board



#include "Mqtt.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "WiFiAutoSelector.h"

#define en1A 5   //D1
#define en2A 4   //D2
#define en12 0   //D3
#define en3A 12  //D6
#define en4A 13  //D7
#define en34 15  //D8
#define BtnPress 16//D0

#define shutDownPin 14  //D5
#define Led 2           //D4
#define WaitTime 2000 //the time we wait before we engage the switch off button. Or trigger on a second button press

#define WIFI_CONNECT_TIMEOUT 8000
WiFiAutoSelector wifiAutoSelector(WIFI_CONNECT_TIMEOUT);

MqttConnector mqttConnector;
float currentPower = 0;
int requestedPower = 0;
bool eStop;
bool shutDown;
bool shutDownRequested=false;
long millis_pre;
long deltaMilli = 5000;
long millisrealTimePre = 0;
void setup() {

  millis_pre = millis();
  pinMode(en1A, OUTPUT);
  pinMode(en2A, OUTPUT);
  pinMode(en12, OUTPUT);
  pinMode(en3A, OUTPUT);
  pinMode(en4A, OUTPUT);
  pinMode(en34, OUTPUT);
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

  Serial.print("Subscribed to: ");


  millisrealTimePre = millis();
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
  statusDoc["TrainId"] = ClientId;
  statusDoc["Power"] = (int)currentPower;
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);
  bool resubsribeNeeded = mqttConnector.publish(StatusTopic, Message);
  if (resubsribeNeeded) {
    ResubScribe();
  }
}
float MeasureVbat() {
  Serial.println(analogRead(A0));
  return ((float)map(analogRead(A0), 0, 1023, 0, 99)) / 10;  //we need to find a proper bridge for the batterymonitor
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived for this train topic: ");
  Serial.println(topic);

  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  eStop = doc["EStop"];
  Serial.println(eStop);
  shutDown = doc["ShutDown"];
  Serial.println(shutDown);
  requestedPower = doc["Power"];

  
  //Blink the status led to indicate that we have received a message
  Blink(1);
}
void UpdatePower() {
  if (requestedPower == ((int)currentPower)) {
    return;
  }

  float deltaV = Acceleration;  //*(millis()-millisrealTimePre);
  //Serial.print("DeltaV:");
  //Serial.println(deltaV);
  if (requestedPower > currentPower) {
    currentPower += deltaV;
  } else {
    currentPower -= deltaV;
  }
  // Serial.println(currentPower);
  if ((int)currentPower == 0) {
    StopEngine();

    return;
  }
  //map it to a percentage
  int percentage = map(abs((int)currentPower), 0, 100, 0, 255);
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
  analogWrite(en12,percentage);
  analogWrite(en34,percentage);

  
}

void StopEngine() {
  digitalWrite(en1A, LOW);
  digitalWrite(en2A, LOW);
  digitalWrite(en3A, LOW);
  digitalWrite(en4A, LOW);
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
    }
  }
  
  mqttConnector.client.loop();
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
