#ifndef Mqtt_H_
#define Mqtt_H_a


#include <PubSubClient.h>
#include <arduino.h>
#include <WiFiClient.h>

class MqttConnector
{

  public:

    WiFiClient espClient;
    PubSubClient client;
    char* ClientId;

    void connect(char* MqttServer,int port,char* clientId);
    void init(char* MqttServer,int port,char* clientId);
    void callback(char* topic, byte* payload, unsigned int length);
    bool connect();
    bool publish(char* topic, char* msg);
};
#endif 