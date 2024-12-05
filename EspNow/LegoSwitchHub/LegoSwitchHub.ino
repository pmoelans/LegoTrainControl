//arduino script is capable of handling 8 switches at once:
//It is suited for a QtPy and works with an MCP23008 port expander
//we use the default address 0x20 (all ids at GND)
//Once a message is received the corresponding switch will be acted upon
//By default all switches are in the neutral position, meaning straight.
//======================================================================

#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include "secrets.h"
#include <Wire.h>
#include <Adafruit_MCP23008.h>

#define DEBUG true // Set to false to disable debug prints
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

esp_now_peer_info_t peerInfo;
int switchId;
bool switch2End1= false;
Adafruit_MCP23008 mcp;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t *incomingData, int len) {
    
  //deserialize the data:
  //=====================
  DynamicJsonDocument doc(90);
  deserializeJson(doc, incomingData);
  
  switchId = doc["SwitchId"]; //might need to offset with 1 in order to have a safeguard agains empty data:
  switch2End1 = doc["switch2End1"];
 
  ActivateSwitch(switchId);

  debug("Switch: ");
  debug(switchId);
  debugln(", needs to switch");  

  //Blink when data has been received
  //=================================
  Blink();
}
void Blink()
{
  digitalWrite(ledPin, LOW);
  delay(200);
  digitalWrite(ledPin,HIGH);
}
void SetIO()
{
  pinMode(ledPin,OUTPUT);

  //Set all the pins from the MCP to output
  for (int i = 0; i <= 7; i++) {
   mcp.pinMode(i, OUTPUT);
  }   
}
void ActivateSwitch(int swtchId)
{
  //Check if we have actually a switch to do
  if(swtchId<0 || swtchId>7)
  {
    return;
  }
  //Put a high voltage on the line of a switch needs to be manipulated:
  //==================================================================
  mcp.digitalWrite(swtchId, HIGH); // Turn on LED connected to pin 0
  delay(500);
  mcp.digitalWrite(swtchId, LOW);
}
void setup() {
  // put your setup code here, to run once:
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

  Wire.begin(SDA, SCL); // Initialize I2C with SDA and SCL pins
  mcp.begin(); // Default address = 0x20
}

void loop() {
  // put your main code here, to run repeatedly:

}
