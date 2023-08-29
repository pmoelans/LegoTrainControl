#include "FastWifi8266.h"
#include "Mqtt.h"
#include "secrets.h"
#include <ArduinoJson.h>

#define en1 13  //D7
#define en2 12  //D6

WiFiConnector wifiConnector;
MqttConnector mqttConnector;
float currentPower = 0;
int requestedPower=0;
long millis_pre;
long deltaMilli=5000;
long millisrealTimePre = 0;
void setup() {
  millis_pre= millis();
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);
  Serial.begin(115200);
  // while (!Serial) {
  //   ;  // wait for serial port to connect. Needed for native USB
  // }

  // put your setup code here, to run once:
  if (!wifiConnector.ConnectDynamicWifi(SECRET_SSID, SECRET_PASS)) {
    Serial.println("Restarting ESP now");
    ESP.restart();
  }

  mqttConnector.connect(MqttBroker, MqttPort, ClientId);

  mqttConnector.client.subscribe(Topic2Subscribe);
  mqttConnector.client.setCallback(callback);
  Serial.println("Setup-done");

  millisrealTimePre= millis();
}

void SendStatus() {
  StaticJsonDocument<300> statusDoc;
  statusDoc["TrainId"] = ClientId;
  statusDoc["Power"] = (int)currentPower;
  statusDoc["Vbat"] = MeasureVbat();

  char Message[80] = { 0 };
  serializeJson(statusDoc, Message);
  mqttConnector.publish(StatusTopic, Message);
}
float MeasureVbat() {
  return analogRead(A0) * 3;  //we need to find a proper bridge for the batterymonitor
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived for this train topic: ");
  Serial.println(topic);

  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);

  requestedPower = doc["Power"];

  Serial.print("Requested speed");
  Serial.println(requestedPower);
  
}
void UpdatePower() 
{
  if(requestedPower==((int)currentPower))
  {
    return;
  }

  float deltaV = Acceleration;//*(millis()-millisrealTimePre);
  //Serial.print("DeltaV:");
  //Serial.println(deltaV);
  if(requestedPower>currentPower)
  {
    currentPower+=deltaV;
  }
  else
  {
    currentPower-=deltaV;
  }
 // Serial.println(currentPower);
  if((int)currentPower==0)
  {
    analogWrite(en1, 0);
    analogWrite(en2, 0);
   
    return;
  }
  //map it to a percentage
  int percentage=map(abs((int) currentPower),0,100,0,255);

  if (currentPower>0) {
    analogWrite(en1, percentage);
    analogWrite(en2, 0);
   
    return;
  }
  analogWrite(en1, 0);
  analogWrite(en2, percentage);
}

void loop() {
 mqttConnector.client.loop();
  // put your main code here, to run repeatedly:
  if((millis()-millis_pre)>deltaMilli)
  {
      SendStatus();
      millis_pre = millis();
  }

  UpdatePower();
  millisrealTimePre=millis();
  //delay(5000);
}
