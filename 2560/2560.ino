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

  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], LOW);

    for (int j = 0; j < 8; j++) {
      bool isOn = digitToSegment[digits[i]] & (1 << j);
      digitalWrite(segmentPins[j], isOn ? HIGH : LOW);
    }

    delay(5);

    digitalWrite(digitPins[i], HIGH);
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

void print_green_light() {
  digitalWrite(green_light_pin, HIGH);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, LOW);
}

void print_yellow_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, HIGH);
  digitalWrite(red_light_pin, LOW);
}

void print_red_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, HIGH);
}

void loop() {
  int distance_loop = HC_SR04_LOOP();
  DISP_PRINT(distance_loop);

  if (distance_loop <= redgreenchangecm && tram_light != "green") {
    // Car light to yellow, then red
    print_yellow_light();
    car_light_status = "yellow";
    // 1: tram, 2:car
    tram_light = "red";
    Serial1.println("1:" + String(distance_loop) + ":" + tram_light +" 2:-1:" + car_light_status);
    delay(3000);

    print_red_light();
    distance_loop = HC_SR04_LOOP();
    DISP_PRINT(distance_loop);
    car_light_status = "red";
    tram_light = "green";
    Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    delay(3000);
  } else if (distance_loop > redgreenchangecm && tram_light == "green") {
    // Tram light to red, car light to green
    print_red_light();
    tram_light = "red";
    delay(3000);

    distance_loop = HC_SR04_LOOP();
    DISP_PRINT(distance_loop);

    print_green_light();
    car_light_status = "green";
    Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    delay(3000);
  } else {
    // Maintain current states
    Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light_status);
    delay(3000);
  }
}
