#define trigPin 31
#define echoPin 32
#define green_light_pin 39
#define yellow_light_pin 36
#define red_light_pin 35

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

String current_light = "red";

// defines variables
long duration;
int distance;

void DISP_SETUP() {
  // Set segment pins as OUTPUT
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  // Set digit pins as OUTPUT
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
}

void HC_SR04_SETUP() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}

void light_setup() {
  pinMode(green_light_pin, OUTPUT);
  pinMode(yellow_light_pin, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
}

void setup() {
  Serial.begin(9600);      // Serial monitor
  Serial1.begin(9600);     // Serial to ESP8266
  HC_SR04_SETUP();
  light_setup();
  DISP_SETUP();
}

void DISP_PRINT(int number) {
  // Extract individual digits
  int digits[4] = {0, 0, 0, 0};
  digits[3] = number % 10;
  digits[2] = (number / 10) % 10;
  digits[1] = (number / 100) % 10;
  digits[0] = (number / 1000) % 10;

  // Multiplex through each digit
  for (int i = 0; i < 4; i++) {
    // Activate the current digit (set LOW for common anode)
    digitalWrite(digitPins[i], LOW);

    // Write the segments
    for (int j = 0; j < 8; j++) {
      bool isOn = digitToSegment[digits[i]] & (1 << j);
      digitalWrite(segmentPins[j], isOn ? HIGH : LOW);
    }

    // Wait for a short time to allow the digit to appear
    delay(5);

    // Deactivate the current digit (set HIGH for common anode)
    digitalWrite(digitPins[i], HIGH);
  }
}

int HC_SR04_LOOP() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

void print_green_light() {
  digitalWrite(green_light_pin, HIGH);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, LOW);
  delay(5);
}

void print_yellow_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, HIGH);
  digitalWrite(red_light_pin, LOW);
  delay(5);
}

void print_red_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, HIGH);
  delay(5);
}

void loop() {
  int distance_loop = HC_SR04_LOOP();
  DISP_PRINT(distance_loop);

  if (distance_loop >= 10) {
    print_green_light();
    current_light = "green";
    Serial1.println("green:" + String(distance_loop));
    delay(2000);
  } else if (distance_loop < 10) {
    if (current_light == "green") {
      print_yellow_light();
      current_light = "yellow";
      Serial1.println("yellow:" + String(distance_loop));
      delay(2000);
      distance_loop = HC_SR04_LOOP(); // revalidate because of delay time
      DISP_PRINT(distance_loop);
    }
    print_red_light();
    current_light = "red";
    Serial1.println("red:" + String(distance_loop));
    delay(2000);
  }

}
