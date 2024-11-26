#include <WiFi.h>        // Include the WiFi library for ESP32 functionality
#include <WiFiManager.h> // Include the WiFiManager library for managing WiFi connections
#include <Preferences.h> // Include Preferences library for persistent storage

WiFiManager wm;           // Create a WiFiManager object
WiFiServer server(80);    // Initialize a web server on port 80
Preferences preferences;  // Create a Preferences object for persistent storage

String hostname = "ESP32"; // Default hostname

// Forward declarations of custom functions
void resetWiFiCredentials();
void setHostname(String newHostname);

void setup() {
  Serial.begin(115200); // Start serial communication for debugging

  // Initialize Preferences and load the saved hostname
  preferences.begin("esp32-config", false); // Namespace "esp32-config"
  hostname = preferences.getString("hostname", "ESP32"); // Retrieve saved hostname or default

  // Set the ESP hostname
  WiFi.setHostname(hostname.c_str()); // Apply the hostname
  Serial.println("Current Hostname: " + hostname);

  // Start WiFiManager in AP mode without a password
  if (!wm.autoConnect("ESP32-AP")) { // Access point name is "ESP32-AP" (open network)
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
    } 
    // Check if the request is to set a new hostname
    else if (request.indexOf("/sethostname?name=") != -1) {
      // Extract the new hostname from the request
      int start = request.indexOf("name=") + 5; // Start of the hostname parameter
      int end = request.indexOf(" ", start);   // End of the parameter
      String newHostname = request.substring(start, end);
      newHostname.trim(); // Remove any trailing spaces or newlines

      if (newHostname.length() > 0 && newHostname.length() <= 32) { // Valid hostname length
        setHostname(newHostname); // Set the new hostname
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<html><body>");
        client.println("<h1>Hostname Updated</h1>");
        client.println("<p>New Hostname: " + newHostname + "</p>");
        client.println("<p>The device will restart now.</p>");
        client.println("</body></html>");
        client.stop();
        delay(1000); // Allow time for the client to receive the response
        ESP.restart(); // Restart ESP to apply the new hostname
      } else {
        // Respond with an error if the hostname is invalid
        client.println("HTTP/1.1 400 Bad Request");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<html><body>");
        client.println("<h1>Invalid Hostname</h1>");
        client.println("<p>Hostname must be between 1 and 32 characters.</p>");
        client.println("</body></html>");
        client.stop();
      }
    } 
    else {
      // Default response for the main page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>ESP32 Web Server</h1>");
      client.println("<p>Current Hostname: " + hostname + "</p>");
      // Button to send a request to reset WiFi credentials
      client.println("<form action=\"/reset\" method=\"get\">");
      client.println("<button type=\"submit\">Reset WiFi credentials </button>");
      client.println("</form>");
      // Form to update the hostname
      client.println("<form action=\"/sethostname\" method=\"get\">");
      client.println("<label for=\"name\">Set Hostname:</label>");
      client.println("<input type=\"text\" id=\"name\" name=\"name\" maxlength=\"32\">");
      client.println("<button type=\"submit\">Update Hostname</button>");
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

// Function to set a new hostname and save it persistently
void setHostname(String newHostname) {
  Serial.println("Setting new hostname: " + newHostname);
  hostname = newHostname; // Update the global variable
  preferences.putString("hostname", newHostname); // Save the hostname persistently
  WiFi.setHostname(newHostname.c_str()); // Apply the hostname
}
