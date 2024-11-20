#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "SJ iPhone 15 PM";
const char* password = "1234567890";

// String serverName = "https://traffic.sjang.dev/api/update"; // Production (Turn on Argo before Showing users)
String serverName = "https://traffic.sjy-dae.zone/api/update"; // dev

int flashbutton;
int currentState = 0;

String data1 = ""; // Holds data for ID 1
String data2 = ""; // Holds data for ID 2

void setup() {
  Serial.begin(9600);  // For communication with Mega
  Serial1.begin(9600); // For communication with another device
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  pinMode(0, INPUT_PULLUP);
}

void loop() {
  flashbutton = digitalRead(0);

if (flashbutton == 0) {
  static unsigned long lastPressTime = 0;
  unsigned long currentPressTime = millis();

  // Debounce logic to prevent rapid toggling
  if (currentPressTime - lastPressTime > 500) { // 500ms debounce delay
    // Define the states as combined strings
    String states[] = {"1:1000:green 2:800:red", "1:500:yellow 2:800:red", "1:800:red 2:1000:green"};

    // Get the current state string
    String state = states[currentState];

    // Split the string into data1 and data2
    data1 = parseData(state, 1); // Parse for ID 1
    data2 = parseData(state, 2); // Parse for ID 2

    // Update the current state for the next button press
    currentState = (currentState + 1) % 3;

    // Send the combined payload for ID 1 and ID 2
    sendToServer();

    // Update the last press time
    lastPressTime = currentPressTime;
  }
}


  // Use Serial1 for receiving data and prepare data for ID 2
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n'); // Read data from Serial1
    data.trim();
    if (data.length() > 0) {
      data2 = formatData(data, 2); // Format data for ID 2
      sendToServer(); // Send the combined payload for ID 1 and ID 2
    }
  }
}

String formatData(String message, int id) {
  String color = message.substring(0, message.indexOf(':'));
  int distance = message.substring(message.indexOf(':') + 1).toInt();

  return String(id) + ":{\"status\": \"" + color + "\", \"distance_cm\": " + String(distance) + "}";
}

void sendToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // Use insecure connection for HTTPS

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    // Construct the payload with data1 and data2
    String jsonPayload = "{";
    if (data1.length() > 0) jsonPayload += data1; // Add data for ID 1 if available
    if (data1.length() > 0 && data2.length() > 0) jsonPayload += ", "; // Add comma if both data exist
    if (data2.length() > 0) jsonPayload += data2; // Add data for ID 2 if available
    jsonPayload += "}";

    Serial.println("Sending JSON Payload: " + jsonPayload);
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode != 200) {
        String responseBody = http.getString(); // Get the response body
        Serial.print("Response body: ");
        Serial.println(responseBody);
      }
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    Serial.println("Please RST the ESP8266 NodeMCU");
    delay(2000);
  }
}
