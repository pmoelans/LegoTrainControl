#include "FastWifi8266.h"

bool WiFiConnector::ConnectDynamicWifi(const char* SSID,const char* passWord)
{
   WiFi.begin( SSID, passWord );

   int counter=0;
    while (WiFi.status() != WL_CONNECTED  && (counter<10)) 
    {   
      delay(500);    
      counter+=1;   
    }
  

   if (WiFi.status() != WL_CONNECTED) {   
      Serial.println("Failed to connect");   
	    return false;
   }

     Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());
   return true;
}
bool WiFiConnector::ConnectToWifi(const char* SSID,const char* passWord, IPAddress ipaddress, IPAddress gate, IPAddress mask)
{
  int maxCounter = 1000;
   enableWiFiAtBootTime(); 
    unsigned long t1;
	t1=millis();
 
   WiFi.config(ipaddress, gate, mask);

  //WiFi.mode(WIFI_STA );
  Serial.print("Launching the slow connect");
  bool fastConnect = false;
  bool saveData = false;
  
 

  
      EEPROM.begin(512);
    
      int readId= (int)(EEPROM.read(0));
      int channel1 = (int)(EEPROM.read(1));

      Serial.print("Ctrl number:");
      Serial.println(readId);

      Serial.print("Channel:");
      Serial.println(channel1);


      if(readId!=38)
      {
        Serial.println("Slow Connect");
       
        if (!WiFi.getAutoConnect()) WiFi.setAutoConnect(true);	// autoconnect from saved credentials
        if (!WiFi.getPersistent()) WiFi.persistent(true);			// save the wifi credentials to flash
        WiFi.begin( SSID, passWord );
        saveData=true;
      }
      else
      {
        Serial.println("Fast Connect");
      
        fastConnect = true;
        uint8_t buffer[6];
        EEPROM.get(2, buffer); 
        WiFi.begin( SSID, passWord,channel1, buffer,true );      
      }
  
  
  int counter=0;
  while (WiFi.status() != WL_CONNECTED  && (counter<10)) {
    if(fastConnect)
    {
      delay(120);      
    }
    else
    {
      delay(500);
    }
    counter+=1;
    Serial.print(".");
  }
  Serial.println("");

   if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi, going back to sleep");
    EEPROM.put(0,0); //reset the verification code
    EEPROM.commit();   
	return false;
  }
  
   if(saveData)
  {
    WriteData();
  }
  
   
  Serial.println("");
  Serial.print("WiFi connected in: ");
  Serial.print(millis()-t1);
  Serial.println("msec");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());
  
  return true;
  	
}

void WiFiConnector::WriteData()
{
  Serial.println("Writing to EEPROM");
  EEPROM.put(0,38);
 
  EEPROM.put(1,WiFi.channel());
 
  uint8_t  buffer[6];
  memcpy(buffer, WiFi.BSSID(), 6 );
  printBuffer(buffer);
  EEPROM.put(2,buffer);
  EEPROM.commit();
}
void WiFiConnector::printBuffer(uint8_t *buffer)
{
	for(int i = 0; i < 6; i++)
	{
		Serial.println(buffer[i]);
	}
}