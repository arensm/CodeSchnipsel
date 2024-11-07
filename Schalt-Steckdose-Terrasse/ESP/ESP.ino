/*********
 Project ESP8266 with Arduino
 Communication over serial

 Part 1 ESP8266 as WEBSERVER
 Is the ESP8266 clean without code, the ESP start as AP
 at 192.168.4.1
 You can scan and select your WLAN to integrate the ESP.

 mail: droidbox.ma () gmail.com

 Free for anybody - no warranties
*********/

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>

// counters
unsigned long ulReqcount;
unsigned long ulReconncount;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {
  Serial.begin(9600);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AC-AP-KELTENCLAN");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  server.begin();

  // Over the air active
  ArduinoOTA.setHostname("esp-terrasse");
  //ArduinoOTA.setPassword("test123");
  ArduinoOTA.begin ();
}

void loop() {
  ArduinoOTA.handle();
  WiFiClient client = server.available(); // Listen for incoming clients
    if (!client) 
      {
        return;
      }
    Serial.println("new client");         // Wait until the client sends some data
    unsigned long ultimeout = millis()+250;
    while(!client.available() && (millis()<ultimeout) )
      {
        delay(1);
      }
        if(millis()>ultimeout) 
      { 
        Serial.println("client connection time-out!");
        return; 
      }
      String sRequest = client.readStringUntil('\r');     // Read the first line of the request
      Serial.println(sRequest);
      client.flush();
        if(sRequest=="")
          {
          Serial.println("empty request! - stopping client");
          client.stop();
          return;
          }
      // get path; end of path is either space or ?
      // Syntax is e.g. GET /?pin=DOSE1ON HTTP/1.1
      String sPath="",sParam="", sCmd="";
      String sGetstart="GET ";
      int iStart,iEndSpace,iEndQuest;
      iStart = sRequest.indexOf(sGetstart);
        if (iStart>=0)
          {
            iStart+=+sGetstart.length();
            iEndSpace = sRequest.indexOf(" ",iStart);
            iEndQuest = sRequest.indexOf("?",iStart);
            if(iEndSpace>0) // are there parameters?
              {
                if(iEndQuest>0)
                  { 
                    sPath  = sRequest.substring(iStart,iEndQuest); // there are parameters
                    sParam = sRequest.substring(iEndQuest,iEndSpace);
                  }
                  else
                    {
                    sPath  = sRequest.substring(iStart,iEndSpace); // NO parameters
                    }
              } 
            Serial.print("Path=");
            Serial.println(sPath);
            Serial.print("Parameters=");
            Serial.println(sParam);
          }

///////////////////////////
// format the html response
  String sResponse,sHeader;
  
////////////////////////////
// 404 for non-matching path
  if(sPath!="/")
    {
      sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
      sHeader  = "HTTP/1.1 404 Not found\r\n";
      sHeader += "Content-Length: ";
      sHeader += sResponse.length();
      sHeader += "\r\n";
      sHeader += "Content-Type: text/html\r\n";
      sHeader += "Connection: close\r\n";
      sHeader += "\r\n";
    }
  else
///////////////////////
// format the html page
    {
      ulReqcount++;
      sResponse  = "<html><head><title>SchaltSteckdosen Terrasse</title></head><body>";
      sResponse += "<font color=\"#000000\"><body bgcolor=\"#34ccc3\">";
      sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
      sResponse += "<h1>Schalt-Steckdosen Terrasse</h1>";
      sResponse += "<FONT SIZE=+1>";
      sResponse += "<p>Steckdose 1 <a href=\"?pin=DOSE1ON\"><button>Einschalten</button></a>&nbsp;<a href=\"?pin=DOSE1OFF\"><button>Ausschalten</button></a></p>";
      sResponse += "<p>Steckdose 2 <a href=\"?pin=DOSE2ON\"><button>Einschalten</button></a>&nbsp;<a href=\"?pin=DOSE2OFF\"><button>Ausschalten</button></a></p>";
      sResponse += "<p>Steckdose 3 <a href=\"?pin=DOSE3ON\"><button>Einschalten</button></a>&nbsp;<a href=\"?pin=DOSE3OFF\"><button>Ausschalten</button></a></p>";
      sResponse += "<p>Steckdose 4 <a href=\"?pin=DOSE4ON\"><button>Einschalten</button></a>&nbsp;<a href=\"?pin=DOSE4OFF\"><button>Ausschalten</button></a></p>";
  
  if (sCmd.length()>0)
    {
      sResponse += "Komando:" + sCmd + "<BR>";
    }
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<BR>Aufrufz&auml;hler="; 
    sResponse += ulReqcount;
    sResponse += " - Verbindungsz&auml;hler="; 
    sResponse += ulReconncount;
    sResponse += "<BR>";
    sResponse += "NerdWochenende 2022.01 November / Ari<BR>";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  client.stop();
  Serial.println("Client disonnected");
}