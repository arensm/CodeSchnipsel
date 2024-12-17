#include <ESP8266WiFi.h> // WiFi-Bibliothek für ESP01/ESP8266
#include <ESP8266WebServer.h> // Webserver für ESP8266

ESP8266WebServer server(80);
String hostname = "ESP01";

void setup() {
  Serial.begin(115200);

  // Statische WiFi-Konfiguration (keine WiFiManager-Bibliothek)
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Webserver starten
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String message = "<html><head><title>ESP01 Steuerung</title></head><body>";
  message += "<h1>ESP01 Web Server</h1>";
  message += "<p>Hostname: " + hostname + "</p>";
  message += "</body></html>";
  server.send(200, "text/html", message);
}
