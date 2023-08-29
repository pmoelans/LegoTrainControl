#ifndef FastWifi8266_H_
#define FastWifi8266_H_

#include <arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>


/*!
 * \brief Connects to the wifi upon first connect with the slow connection and stores the relevant parameters in EEPROM.
 *On restart it will use the parameters from the EEPROM to accelerate the connection
 *
 * 
 */
 
class WiFiConnector 
{

int CtrlNumber =38;



public:

bool ConnectToWifi(const char* SSID, const char* passWord, IPAddress ipaddress, IPAddress gate, IPAddress mask);
bool ConnectDynamicWifi(const char* SSID,const char* passWord);
private:

void WriteData(const char* SSID, const char* passWord);
void WriteData();
void printBuffer(uint8_t *buffer);
};

#endif 