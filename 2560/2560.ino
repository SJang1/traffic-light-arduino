#define trigPin 31
#define echoPin 32
#define green_light_pin 41
#define yellow_light_pin 40
#define red_light_pin 39

unsigned long yellowStartTime = 0;
unsigned long redStartTime = 0;
unsigned long greenStartTime = 0;
bool yellowInProgress = false;
bool redInProgress = false;
bool greenInProgress = false;
unsigned long hc_time = 0;
unsigned long internet = 0;


const int redgreenchangecm = 15;

const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // A, B, C, D, E, F, G, DP
const int digitPins[] = {10, 11, 12, 13}; // Digit control pins (common anodes)

// 4digit 7segment display 숫자
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
String car_light = "green";

long duration;
int distance;
int distance_loop = 0;
unsigned long lastUpdateTime = 0;
unsigned long lightChangeTime = 0;
unsigned long yellow_lightChangeTime = 0;
unsigned long displayUpdateTime = 0;
const unsigned long lightInterval = 500; // Time interval for light changes
const unsigned long displayInterval = 5; // Time interval for refreshing display

bool displayOn = true;
int displayNumber = 0;

// 4digit 7segment display 초기화
void DISP_SETUP() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
}

// 초음파 센서 초기화
void HC_SR04_SETUP() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

// LED 초기화
void light_setup() {
  pinMode(green_light_pin, OUTPUT);
  pinMode(yellow_light_pin, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
}

// 초기설정
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  HC_SR04_SETUP();
  light_setup();
  DISP_SETUP();
  greenInProgress = true;
  yellowInProgress = false;
  redInProgress = false;
  tram_light = "red";
  car_light = "green";
  print_green_light();

}

// 4digit 7segment display 표시. 
// 각 digit별로 아주 짧게 표시를 하여 사람의 눈에는 모두 동시에 불 들어온 것 처럼 보이게 됩니다.
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

// 초음파 센서로 거리 측정해서 숫자형식으로 반환
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

// 초록불 LED 출력
void print_green_light() {
  digitalWrite(green_light_pin, HIGH);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, LOW);
}

// 노란불 LED 출력
void print_yellow_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, HIGH);
  digitalWrite(red_light_pin, LOW);
}

// 빨간불 LED 출력
void print_red_light() {
  digitalWrite(green_light_pin, LOW);
  digitalWrite(yellow_light_pin, LOW);
  digitalWrite(red_light_pin, HIGH);
}

// 반복문.
void loop() {
  unsigned long currentTime = millis(); // Get the current time

  // Read distance every 500ms
  if (currentTime - hc_time >= 500) {
    distance_loop = HC_SR04_LOOP();
    hc_time = currentTime;
  }
  DISP_PRINT(distance_loop);


  // 차량불 초록불일때, 거리가 15cm 이내로 들어오면 노란불로
  if (greenInProgress && distance_loop <= 15) {
    greenInProgress = false;
    yellowInProgress = true;
    redInProgress = false;
    tram_light = "red";
    car_light = "yellow";
    print_yellow_light();
    yellowStartTime = currentTime;
  }
  // 노란불 상태에서 3초 이후 차량 빨간불
  if (yellowInProgress &&  currentTime - yellowStartTime >= 3000) {
    greenInProgress = false;
    yellowInProgress = false;
    redInProgress = true;
    tram_light = "green";
    car_light = "red";
    print_red_light();
  }
  // 차량 빨간불 상태에서 거리가 15cm 밖으로 나가면 차량 초록불로
  if (redInProgress && distance_loop > 15) {
    greenInProgress = true;
    yellowInProgress = false;
    redInProgress = false;
    tram_light = "red";
    car_light = "green";
    print_green_light();
  }

  // 2초마다 한번씩 인터넷에 전송
  if (currentTime - internet >= 2000) {
  Serial1.println("1:" + String(distance_loop) + ":" + tram_light + " 2:-1:" + car_light);
  internet = currentTime;
  }
}


