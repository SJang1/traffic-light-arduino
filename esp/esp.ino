#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "SJ iPhone 15 PM";
const char* password = "1234567890";


//String serverName = "https://traffic.sjang.dev/api/update"; // Production (Turn on Argo before Showing users)
String serverName = "https://traffic.sjy-dae.zone/api/update"; // dev


int flashbutton;
int currentState = 0;

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
  // Using Falsh Button, Change color
  if (flashbutton== 0) {
    String states[] = {"green:1000", "yellow:500", "red:800"};
    String datasend = states[currentState];
    sendToServer(datasend);
    currentState = (currentState + 1) % 3;
    if (currentState >= 3) {
      currentState = 0; // Block to be overflow
    }
  }
  // Use Serial TX/RX
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
    //WiFiClient client;
    HTTPClient http;

    // HTTPS
    WiFiClientSecure client;
    client.setInsecure();

    String color = message.substring(0, message.indexOf(':'));
    int distance = message.substring(message.indexOf(':') + 1).toInt();

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"status\": \"" + color + "\", \"distance_cm\": " + String(distance) + "}";
    Serial.println(jsonPayload);
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
