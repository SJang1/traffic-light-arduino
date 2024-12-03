#define trigPin 31
#define echoPin 32
#define green_light_pin 39
#define yellow_light_pin 38
#define red_light_pin 39

const int redgreenchangecm = 15;

const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // A, B, C, D, E, F, G, DP
const int digitPins[] = {10, 11, 12, 13}; // Digit control pins (common anodes)

const byte digitToSegment[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

String tram_light = "red";
String car_light_status = "green";

// defines variables
long duration;
int distance;

unsigned long lastUpdateTime = 0;
unsigned long lightChangeTime = 0;
unsigned long displayUpdateTime = 0;
const unsigned long lightInterval = 3000; // Time interval for light changes
const unsigned long displayInterval = 5; // Time interval for refreshing display

bool displayOn = true;
int displayNumber = 0;

void DISP_SETUP() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
}

void HC_SR04_SETUP() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void light_setup() {
  pinMode(green_light_pin, OUTPUT);
  pinMode(yellow_light_pin, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  HC_SR04_SETUP();
  light_setup();
  DISP_SETUP();
}

void DISP_PRINT(int number) {
  int digits[4] = {0, 0, 0, 0};
  digits[3] = number % 10;
  digits[2] = (number / 10) % 10;
  digits[1] = (number / 100) % 10;
  digits[0] = (number / 1000) % 10;

  unsigned long currentTime = millis();
  if (currentTime - displayUpdateTime >= displayInterval) {
    displayUpdateTime = currentTime;

    for (int i = 0; i < 4; i++) {
      digitalWrite(digitPins[i], LOW);

      for (int j = 0; j < 8; j++) {
        bool isOn = digitToSegment[digits[i]] & (1 << j);
        digitalWrite(segmentPins[j], isOn ? HIGH : LOW);
      }

      delay(5); // Short delay to stabilize the display
      digitalWrite(digitPins[i], HIGH);
    }
  }
}

int HC_SR04_LOOP() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

void set_lights(String tram_light, String car_light_status) {
  if (tram_light == "red") {
    if (car_light_status == "green") {
      digitalWrite(green_light_pin, HIGH);
      digitalWrite(yellow_light_pin, LOW);
      digitalWrite(red_light_pin, LOW);
    } else if (car_light_status == "yellow") {
      digitalWrite(green_light_pin, LOW);
      digitalWrite(yellow_light_pin, HIGH);
      digitalWrite(red_light_pin, LOW);
    }
  } else if (tram_light == "green") {
    digitalWrite(green_light_pin, LOW);
    digitalWrite(yellow_light_pin, LOW);
    digitalWrite(red_light_pin, HIGH);
  }
}

void loop() {
  unsigned long currentTime = millis();

  // Update the distance and display
  if (currentTime - lastUpdateTime >= 100) { // Check distance every 100ms
    lastUpdateTime = currentTime;
    int distance_loop = HC_SR04_LOOP();
    displayNumber = distance_loop;
    DISP_PRINT(displayNumber);

    if (distance_loop <= redgreenchangecm && tram_light != "green") {
      car_light_status = "yellow";
      tram_light = "red";
      lightChangeTime = currentTime;
      Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    } else if (distance_loop > redgreenchangecm && tram_light == "green") {
      car_light_status = "green";
      tram_light = "red";
      lightChangeTime = currentTime;
      Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    } else {
      Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    }
  }

  // Handle light state changes
  if (currentTime - lightChangeTime >= lightInterval) {
    if (car_light_status == "yellow") {
      car_light_status = "red";
      tram_light = "green";
      lightChangeTime = currentTime;
      Serial1.println("1:" + String(displayNumber) + ":" + tram_light + " 2:-1:" + car_light_status);
    } else if (tram_light == "red" && car_light_status == "red") {
      car_light_status = "green";
      tram_light = "red";
      lightChangeTime = currentTime;
      Serial1.println("1:" + String(displayNumber) + ":" + tram_light + " 2:-1:" + car_light_status);
    }
  }

  // Update lights
  set_lights(tram_light, car_light_status);
}