#include <arduino.h>
#include <WiFi.h>        // Include the WiFi library for ESP32 functionality
#include <WiFiManager.h> // Include the WiFiManager library for managing WiFi connections

WiFiManager wm;         // Create a WiFiManager object
WiFiServer server(80);  // Initialize a web server on port 80

// Forward declaration of the function to reset WiFi credentials
void resetWiFiCredentials();

void setup() {
  Serial.begin(115200); // Start serial communication for debugging

  // Start WiFiManager in AP mode without a password
  if (!wm.autoConnect("ESP32-AP")) { // Access point name is "ESP32-AP"
    Serial.println("Failed to connect to WiFi and hit timeout");
    delay(3000);
    ESP.restart(); // Restart ESP if connection fails
  }

  // If connected, print the local IP address
  Serial.println("Connected to WiFi");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the web server
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Check if a client is connected to the server

  if (client) { // If a new client connects
    Serial.println("New client connected");
    String request = client.readStringUntil('\r'); // Read the client's HTTP request
    Serial.println("Request: " + request); // Log the request for debugging

    // Check if the request is to reset WiFi credentials
    if (request.indexOf("/reset") != -1) {
      // Respond with a confirmation page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>WiFi Credentials Reset</h1>");
      client.println("<p>The device will restart and reset the credentials.</p>");
      client.println("</body></html>");
      client.stop();
      delay(1000); // Allow time for the client to receive the response
      resetWiFiCredentials(); // Call the function to reset WiFi credentials
    } else {
      // Default response for the main page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>ESP32 Web Server</h1>");
      client.println("<p>Click the button below to reset WiFi credentials.</p>");
      // Button to send a request to the /reset route
      client.println("<form action=\"/reset\" method=\"get\">");
      client.println("<button type=\"submit\">Reset WiFi</button>");
      client.println("</form>");
      client.println("</body></html>");
      client.stop();
    }
  }
}

// Function to reset WiFi credentials and restart the ESP32
void resetWiFiCredentials() {
  Serial.println("Resetting WiFi credentials...");
  wm.resetSettings(); // Clear the saved WiFi credentials
  ESP.restart();      // Restart the ESP32
}
