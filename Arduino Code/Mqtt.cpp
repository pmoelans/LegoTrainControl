#include "Mqtt.h"

void MqttConnector::connect(char* mqtt_server, int mqtt_port,char* clientId)
{
  Serial.print("Attempting MQTT connection...");
  init(mqtt_server,mqtt_port,clientId);
    
  connect();
 
}

void MqttConnector::init(char* mqtt_server, int mqtt_port,char* clientId)
{

  ClientId = clientId;
  
  client.setClient(espClient);
  client.setServer(mqtt_server, mqtt_port);
 
 
}
bool MqttConnector::connect() {
  // Loop until we're reconnected
  if(client.connected())
  {
    return false;
  }
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
   
    // Attempt to connect
    if (client.connect(ClientId)) 
    {
      Serial.println("Connected");
      client.loop();
      return true;   
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return true;
}

bool MqttConnector::publish(char* topic, char* msg)
{
  bool resubscribe= connect();
  client.publish(topic, msg);
 Serial.print("Sending:");
  Serial.println(msg);
  return resubscribe;
}