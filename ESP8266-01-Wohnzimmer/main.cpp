#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

WiFiManager wm;
WiFiServer server(80);
Preferences preferences;

String hostname = "ESP32";

unsigned long ulReqcount = 0;  // Request counter
unsigned long ulReconncount = 0;  // Reconnection counter

void resetWiFiCredentials();
void setHostname(String newHostname);
void handleSettingsPage(WiFiClient &client);
void handleControlPage(WiFiClient &client);

void setup() {
  Serial.begin(115200);

  preferences.begin("esp32-config", false);
  hostname = preferences.getString("hostname", "ESP32");

  WiFi.setHostname(hostname.c_str());
  Serial.println("Current Hostname: " + hostname);

  if (!wm.autoConnect("ESP32-AP")) {
    Serial.println("Failed to connect to WiFi and hit timeout");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String request = client.readStringUntil('\r');
    Serial.println("Request: " + request);
    ulReqcount++;

    if (request.indexOf("GET /settings") != -1) {
      handleSettingsPage(client);
    } else if (request.indexOf("GET /control") != -1) {
      handleControlPage(client);
    } else if (request.indexOf("GET / ") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>ESP32 Web Server</h1>");
      client.println("<p>Current Hostname: " + hostname + "</p>");
      client.println("<a href=\"/settings\">Go to Settings</a>");
      client.println("<a href=\"/control\">Go to Rollo Control</a>");
      client.println("</body></html>");
      client.stop();
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>404 Not Found</h1>");
      client.println("</body></html>");
      client.stop();
    }
  }
}

void handleSettingsPage(WiFiClient &client) {
  // (Unveränderte Implementierung der Settings-Seite)
}

void handleControlPage(WiFiClient &client) {
  String sResponse = "<html><head><title>Rollo Steuerung Wohnzimmer</title></head><body>";
  sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
  sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
  sResponse += "<h1>Steuerung Wohnzimmer</h1>";
  sResponse += "<FONT SIZE=+1>";
  sResponse += "<p>Fenster links  <a href=\"?pin=MOTOR1UP\"><button>Rollo hochfahren</button></a>&nbsp;<a href=\"?pin=MOTOR1STOP\"><button>Stop</button></a>&nbsp;<a href=\"?pin=MOTOR1DOWN\"><button>Rollo runterfahren</button></a></p>";
  sResponse += "<p>Terassen Tuere <a href=\"?pin=MOTOR2UP\"><button>Rollo hochfahren</button></a>&nbsp;<a href=\"?pin=MOTOR2STOP\"><button>Stop</button></a>&nbsp;<a href=\"?pin=MOTOR2DOWN\"><button>Rollo runterfahren</button></a></p>";
  sResponse += "<p>Fenster rechts <a href=\"?pin=MOTOR3UP\"><button>Rollo hochfahren</button></a>&nbsp;<a href=\"?pin=MOTOR3STOP\"><button>Stop</button></a>&nbsp;<a href=\"?pin=MOTOR3DOWN\"><button>Rollo runterfahren</button></a></p>";
  sResponse += "<FONT SIZE=-2>";
  sResponse += "<BR>Aufrufzaehler="; 
  sResponse += ulReqcount;
  sResponse += " - Verbindungszaehler="; 
  sResponse += ulReconncount;
  sResponse += "<BR>Ari 2024.11<BR>";
  sResponse += "</body></html>";

  String sHeader = "HTTP/1.1 200 OK\r\n";
  sHeader += "Content-Length: ";
  sHeader += sResponse.length();
  sHeader += "\r\n";
  sHeader += "Content-Type: text/html\r\n";
  sHeader += "Connection: close\r\n";
  sHeader += "\r\n";

  client.print(sHeader + sResponse);
  client.stop();
}

void resetWiFiCredentials() {
  wm.resetSettings();
  ESP.restart();
}

void setHostname(String newHostname) {
  hostname = newHostname;
  preferences.putString("hostname", newHostname);
  WiFi.setHostname(newHostname.c_str());
}
