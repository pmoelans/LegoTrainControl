#include "FastWifi8266.h"
#include "Mqtt.h"
#include "secrets.h"
#include <ArduinoJson.h>

#define en1 13  //D7
#define en2 12  //D6
#define shutDownPin 14 //D8
#define Led 0//D3

WiFiConnector wifiConnector;
MqttConnector mqttConnector;
float currentPower = 0;
int requestedPower=0;
bool eStop;
bool shutDown;
long millis_pre;
long deltaMilli=5000;
long millisrealTimePre = 0;
void setup() {

  millis_pre= millis();
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);

  //keep the arduino powered on
  pinMode(shutDownPin,OUTPUT);
  digitalWrite(shutDownPin,HIGH);

  //Set the led state to indicate that the controller is live
  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);

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

  Serial.print("Subscribed to: ");
  Serial.println(Topic2Subscribe);
  ResubScribe();
  Serial.println("Setup-done");

  millisrealTimePre= millis();
}
void Blink()
{
  digitalWrite(Led,LOW);
  delay(300);
  digitalWrite(Led,HIGH);
}
void ResubScribe()
{
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
  bool resubsribeNeeded=mqttConnector.publish(StatusTopic, Message);
  if(resubsribeNeeded)
  {
    ResubScribe();
  }

}
float MeasureVbat() {
  Serial.println(analogRead(A0));
  return ((float)map(analogRead(A0),0,1023,0,99))/10;  //we need to find a proper bridge for the batterymonitor
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived for this train topic: ");
  Serial.println(topic);

  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  eStop= doc["EStop"];
  Serial.println(eStop);
  shutDown= doc["ShutDown"];
  Serial.println(shutDown);
  requestedPower = doc["Power"];

  //Blink the status led to indicate that we have received a message
  Blink();
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
void EStop()
{
  if(eStop>0)
  {
   // Serial.println("Estop");
    currentPower = 0;
    requestedPower = 0;
    analogWrite(en1, 0);
    analogWrite(en2, 0);
  }
}

void PowerDown()
{
  if(shutDown>0)
  {
    //Serial.println("PowerDown");
    digitalWrite(shutDownPin,LOW);
  }
}

void loop() {
  mqttConnector.client.loop();
  // put your main code here, to run repeatedly:
  if((millis()-millis_pre)>deltaMilli)
  {
      SendStatus();
      millis_pre = millis();
  }

  //Check if we need to perform an emergency brake
  EStop();

  //Check if we need to perform a power down
  PowerDown();

  //Update the power settings
  UpdatePower();  

  millisrealTimePre=millis();
}
