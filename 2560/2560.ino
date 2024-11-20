#define trigPin 31
#define echoPin 32
#define green_light_pin 39
#define yellow_light_pin 36
#define red_light_pin 35

String current_light = "red";

// defines variables
long duration;
int distance;

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

  if (distance_loop <= 8)
    print_green_light();
    current_light = "green";
    Serial1.println("green:" + String(distance_loop));
    delay(2000);
  } else if (distance_loop > 8) {
    if (current_light == "green") {
      print_yellow_light();
      current_light = "yellow";
      Serial1.println("yellow:" + String(distance_loop));
      delay(2000);
      distance_loop = HC_SR04_LOOP(); // revalidate because of delay time
    }
    print_red_light();
    current_light = "red";
    Serial1.println("red:" + String(distance_loop));
    delay(2000);
  }

}
