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

String parseData(String state, int id) {
  String prefix = String(id) + ":";
  int startIndex = state.indexOf(prefix);
  if (startIndex != -1) {
    int endIndex = state.indexOf(" ", startIndex); // Find the end of this ID's data
    if (endIndex == -1) {
      endIndex = state.length(); // If no space, take the rest of the string
    }
    return state.substring(startIndex, endIndex); // Extract the substring for the specific ID
  }
  return ""; // Return an empty string if the ID is not found
}

void setup() {
  Serial.begin(9600);  // For communication with Mega
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
      String states[] = {"1:1000:red 2:-1:green", "1:500:red 2:-1:yellow", "1:800:green 2:-1:red"};

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
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read data from Serial1
    data.trim();
    if (data.length() > 0) {
      String state = data;
      data1 = parseData(state, 1); // Parse for ID 1
      data2 = parseData(state, 2); // Parse for ID 2
      sendToServer(); // Send the combined payload for ID 1 and ID 2
    }
  }
}

String formatData(String message, int id) {
  int firstColonIndex = message.indexOf(":");
  int lastColonIndex = message.lastIndexOf(":");
  
  if (firstColonIndex == -1 || lastColonIndex == -1 || firstColonIndex == lastColonIndex) {
    return ""; // Return an empty string if the format is incorrect
  }

  String distance = message.substring(firstColonIndex + 1, lastColonIndex);
  String color = message.substring(lastColonIndex + 1);

  return "\"" + String(id) + "\":{\"status\": \"" + color + "\", \"distance_cm\": " + distance + "}";
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
    bool firstData = true;

    if (data1.length() > 0) {
      jsonPayload += formatData(data1, 1);
      firstData = false;
    }

    if (data2.length() > 0) {
      if (!firstData) jsonPayload += ", ";
      jsonPayload += formatData(data2, 2);
    }

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
