#define trigPin 31
#define echoPin 32
#define green_light_pin 39
#define yellow_light_pin 36
#define red_light_pin 35

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

// Timing variables
unsigned long previousMillis = 0;
const unsigned long interval = 3000; // 3 seconds

// State enumeration
enum TrafficState {
  STATE_IDLE,
  STATE_YELLOW,
  STATE_RED,
  STATE_GREEN
};

TrafficState currentState = STATE_IDLE;

// Defines variables
long duration;
int distance;

// Function Declarations
void DISP_SETUP();
void HC_SR04_SETUP();
void light_setup();
void DISP_PRINT(int number);
int HC_SR04_LOOP();
void print_green_light();
void print_yellow_light();
void print_red_light();
void handleIdleState(int distance_loop);
void handleYellowState();
void handleRedState();
void handleGreenState();
void performOtherTasks();

void setup() {
  Serial.begin(9600);
  HC_SR04_SETUP();
  light_setup();
  DISP_SETUP();

  // Initialize lights
  print_green_light();

  // Initialize previousMillis
  previousMillis = millis();
}

void DISP_SETUP() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW); // Initialize to off
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH); // Common anode off
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

void DISP_PRINT(int number) {
  int digits[4] = {0, 0, 0, 0};
  digits[3] = number % 10;
  digits[2] = (number / 10) % 10;
  digits[1] = (number / 100) % 10;
  digits[0] = (number / 1000) % 10;

  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], LOW); // Activate digit

    for (int j = 0; j < 8; j++) {
      bool isOn = digitToSegment[digits[i]] & (1 << j);
      digitalWrite(segmentPins[j], isOn ? HIGH : LOW);
    }

    delay(5); // Small delay to stabilize the display
    digitalWrite(digitPins[i], HIGH); // Deactivate digit
  }
}

int HC_SR04_LOOP() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // Timeout after ~30ms
  if (duration == 0) {
    distance = -1; // Indicate no echo received
  } else {
    distance = duration * 0.034 / 2;
  }
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

void print_green_light() {
  digitalWrite(green_light_pin, HIGH);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, LOW);
  car_light_status = "green";
}

void print_yellow_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, HIGH);
  digitalWrite(red_light_pin, LOW);
  car_light_status = "yellow";
}

void print_red_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, HIGH);
  car_light_status = "red";
}

void handleIdleState(int distance_loop) {
  if (distance_loop <= redgreenchangecm && tram_light != "green") {
    currentState = STATE_YELLOW;
    previousMillis = millis();
  }
}

void handleYellowState() {
  print_yellow_light();
  tram_light = "red";
  Serial.println("1:" + String(distance) + ":" + tram_light + " 2:-1:" + car_light_status);
  currentState = STATE_RED;
  previousMillis = millis();
}

void handleRedState() {
  print_red_light();
  distance = HC_SR04_LOOP();
  DISP_PRINT(distance);
  car_light_status = "red";
  tram_light = "green";
  Serial.println("1:" + String(distance) + ":" + tram_light + " 2:-1:" + car_light_status);
  currentState = STATE_GREEN;
  previousMillis = millis();
}

void handleGreenState() {
  print_green_light();
  car_light_status = "green";
  tram_light = "red";
  Serial.println("1:" + String(distance) + ":" + tram_light + " 2:-1:" + car_light_status);
  currentState = STATE_IDLE;
  previousMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Continuously read distance and update display
  distance = HC_SR04_LOOP();
  DISP_PRINT(distance);

  switch (currentState) {
    case STATE_IDLE:
      handleIdleState(distance);
      break;

    case STATE_YELLOW:
      if (currentMillis - previousMillis >= interval) {
        handleYellowState();
      }
      break;

    case STATE_RED:
      if (currentMillis - previousMillis >= interval) {
        handleRedState();
      }
      break;

    case STATE_GREEN:
      if (currentMillis - previousMillis >= interval) {
        handleGreenState();
      }
      break;
  }

  // Example of another task that runs continuously
  performOtherTasks();
}
