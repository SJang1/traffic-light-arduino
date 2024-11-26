#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// WiFi Credentials
const char* ssid = "SJ iPhone 15 PM";
const char* password = "1234567890";

// Server URLs
// String serverName = "https://traffic.sjang.dev/api/update"; // Production
String serverName = "https://traffic.sjy-dae.zone/api/update"; // Development

// GPIO Pin for Button
const int buttonPin = 0;

// State Variables
int flashbutton;
int currentState = 0;

// Data Holders
String data1 = ""; // Holds data for ID 1
String data2 = ""; // Holds data for ID 2

// Send Queue Configuration
#define QUEUE_SIZE 10
String sendQueue[QUEUE_SIZE];
int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// HTTP Request State
bool isSending = false;
String currentPayload = "";
WiFiClientSecure client;

// Function Prototypes
bool enqueueData(String data);
bool dequeueData(String &data);
String parseData(String state, int id);
String constructJsonPayload();
String formatData(String message, int id);
void processSendQueue();
void sendHttpRequest(String payload);
String getHost(String url);
int getPort(String url);
String getPath(String url);

void setup() {
  Serial.begin(9600);  // Initialize Serial for debugging
  WiFi.begin(ssid, password); // Connect to WiFi

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  pinMode(buttonPin, INPUT_PULLUP); // Initialize button pin
}

void loop() {
  flashbutton = digitalRead(buttonPin);

  // Handle Button Press with Debounce
  if (flashbutton == LOW) {
    static unsigned long lastPressTime = 0;
    unsigned long currentPressTime = millis();

    // Debounce Delay
    if (currentPressTime - lastPressTime > 500) { // 500ms debounce
      // Define the states as combined strings
      String states[] = {
        "1:1000:red 2:-1:green",
        "1:500:red 2:-1:yellow",
        "1:800:green 2:-1:red"
      };

      // Get the current state string
      String state = states[currentState];

      // Parse data for ID 1 and ID 2
      data1 = parseData(state, 1);
      data2 = parseData(state, 2);

      // Update state for next button press
      currentState = (currentState + 1) % 3;

      // Construct JSON Payload
      String jsonPayload = constructJsonPayload();

      // Enqueue the payload
      if (enqueueData(jsonPayload)) {
        Serial.println("Enqueued JSON Payload: " + jsonPayload);
      }

      // Update last press time
      lastPressTime = currentPressTime;
    }
  }

  // Handle Incoming Serial Data
  if (Serial.available()) {
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim();
    if (incomingData.length() > 0) {
      String state = incomingData;
      data1 = parseData(state, 1);
      data2 = parseData(state, 2);

      String jsonPayload = constructJsonPayload();

      if (enqueueData(jsonPayload)) {
        Serial.println("Enqueued JSON Payload from Serial: " + jsonPayload);
      }
    }
  }

  // Process the Send Queue
  processSendQueue();
}

// Enqueue Data into Send Queue
bool enqueueData(String data) {
  if (queueCount >= QUEUE_SIZE) {
    Serial.println("Send queue is full! Dropping data.");
    return false;
  }
  sendQueue[queueTail] = data;
  queueTail = (queueTail + 1) % QUEUE_SIZE;
  queueCount++;
  return true;
}

// Dequeue Data from Send Queue
bool dequeueData(String &data) {
  if (queueCount == 0) {
    return false;
  }
  data = sendQueue[queueHead];
  queueHead = (queueHead + 1) % QUEUE_SIZE;
  queueCount--;
  return true;
}

// Parse Data String for a Given ID
String parseData(String state, int id) {
  String prefix = String(id) + ":";
  int startIndex = state.indexOf(prefix);
  if (startIndex != -1) {
    int endIndex = state.indexOf(" ", startIndex);
    if (endIndex == -1) {
      endIndex = state.length();
    }
    return state.substring(startIndex, endIndex);
  }
  return "";
}

// Construct JSON Payload from data1 and data2
String constructJsonPayload() {
  String jsonPayload = "{";
  bool firstEntry = true;

  if (data1.length() > 0) {
    jsonPayload += formatData(data1, 1);
    firstEntry = false;
  }

  if (data2.length() > 0) {
    if (!firstEntry) jsonPayload += ", ";
    jsonPayload += formatData(data2, 2);
  }

  jsonPayload += "}";
  return jsonPayload;
}

// Format Data into JSON Structure
String formatData(String message, int id) {
  int firstColon = message.indexOf(":");
  int lastColon = message.lastIndexOf(":");

  if (firstColon == -1 || lastColon == -1 || firstColon == lastColon) {
    return "";
  }

  String distance = message.substring(firstColon + 1, lastColon);
  String color = message.substring(lastColon + 1);

  return "\"" + String(id) + "\":{\"status\": \"" + color + "\", \"distance_cm\": " + distance + "}";
}

// Process the Send Queue
void processSendQueue() {
  if (!isSending && dequeueData(currentPayload)) {
    isSending = true;
    sendHttpRequest(currentPayload);
  }

  if (isSending) {
    if (!client.connected()) {
      Serial.println("HTTP Send Complete or Failed");
      client.stop();
      isSending = false;
    }
  }
}

// Send HTTP POST Request
void sendHttpRequest(String payload) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Cannot send data.");
    isSending = false;
    return;
  }

  client.setInsecure(); // Disable SSL certificate verification (not recommended for production)

  String host = getHost(serverName);
  int port = getPort(serverName);
  String path = getPath(serverName);

  Serial.println("Connecting to server...");
  if (!client.connect(host.c_str(), port)) {
    Serial.println("Connection to server failed!");
    isSending = false;
    return;
  }

  // Construct HTTP POST Request
  String request = "POST " + path + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + String(payload.length()) + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   payload;

  // Send HTTP Request
  client.print(request);
  Serial.println("HTTP request sent.");

  // Do not wait for the response to keep it non-blocking
}

// Helper Functions to Parse URL Components
String getHost(String url) {
  int protocolEnd = url.indexOf("://");
  int hostStart = (protocolEnd != -1) ? protocolEnd + 3 : 0;
  int hostEnd = url.indexOf('/', hostStart);
  if (hostEnd == -1) hostEnd = url.length();
  String host = url.substring(hostStart, hostEnd);

  // Remove port if present
  int colonIndex = host.indexOf(':');
  if (colonIndex != -1) {
    host = host.substring(0, colonIndex);
  }
  return host;
}

int getPort(String url) {
  int protocolEnd = url.indexOf("://");
  int hostStart = (protocolEnd != -1) ? protocolEnd + 3 : 0;
  int colonIndex = url.indexOf(':', hostStart);
  if (colonIndex != -1) {
    int portStart = colonIndex + 1;
    int portEnd = url.indexOf('/', portStart);
    if (portEnd == -1) portEnd = url.length();
    String portStr = url.substring(portStart, portEnd);
    return portStr.toInt();
  }
  // Default ports
  if (url.startsWith("https://")) return 443;
  if (url.startsWith("http://")) return 80;
  return 80; // Default to HTTP port if not specified
}

String getPath(String url) {
  int pathStart = url.indexOf('/', url.indexOf("://") + 3);
  if (pathStart == -1) return "/";
  return url.substring(pathStart);
}
