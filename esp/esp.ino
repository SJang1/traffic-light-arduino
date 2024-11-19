#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "SJ iPhone 15 PM";
const char* password = "1234567890";

String serverName = "https://traffic-light-demo.pages.dev/api/update";

void setup() {
  Serial.begin(9600);  // For communication with Mega
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read data from Mega
    data.trim();
    if (data.length() > 0) {
      sendToServer(data);
    }
  }
}

void sendToServer(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String color = message.substring(0, message.indexOf(':'));
    int distance = message.substring(message.indexOf(':') + 1).toInt();

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"status\": \"" + color + "\", \"distance_cm\": " + String(distance) + "}";
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
