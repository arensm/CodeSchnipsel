/*********
 AP on ESP8266
 Is the ESP8266 clean without code, the ESP start as AP
 at 192.168.4.1

 You can scan and select your WLAN to integrate the ESP.

 mail: ari (@) nimaar.de

 Free for anybody - no warranties
*********/

#include <ESP8266WiFi.h>        // we need wifi to get internet access
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager

void setup()
  {
  Serial.begin(115200);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  wifiManager.autoConnect("AP-BTTF-ClockMini");
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  }

void loop()
  {
  // put your main code here, to run repeatedly:
  }
