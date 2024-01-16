#include "Mqtt.h"

void MqttConnector::connect(char* mqtt_server, int mqtt_port,char* clientId)
{
  Serial.print("Attempting MQTT connection...");
  ClientId = clientId;
  
  client.setClient(espClient);
  client.setServer(mqtt_server, mqtt_port);
  reconnect();
 
}

void MqttConnector::reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
   
    // Attempt to connect
    if (client.connect(ClientId)) 
    {
      Serial.println("Connected");
      client.loop();
      return;   
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
 
}

void MqttConnector::publish(char* topic, char* msg)
{
  reconnect();
  client.publish(topic, msg);
 Serial.print("Sending:");
  Serial.println(msg);
}