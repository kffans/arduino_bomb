#include <Servo.h>

// --- Definicje i zmienne z programu 1 ---
#define CLK1_1 24  // Enkoder1A (program 1)
#define DT1_1  26  // Enkoder1B (program 1)
#define pinServo_1 43  // Servo1 (program 1)
#define PHOTO_PIN_3 A6


Servo myServo1_1;

const int servoStop_1 = 90;     // zatrzymanie (program 1)
const int servoRight_1 = 100;   // ruch w prawo (program 1)
const int servoLeft_1  = 80;    // ruch w lewo (program 1)

const int stepDeg_1 = 3;        // krok obrotu (program 1)
const float degPerMs_1 = 0.1f;  // ile stopni na 1 ms (program 1)
const int moveTime_1 = (int)(stepDeg_1 / degPerMs_1);
int lastCLK_1;

// --- Definicje i zmienne z programu 2 ---
const int encoderPinA_2 = 2;    // (program 2)
const int encoderPinB_2 = 3;    // (program 2)
const int servoPin_2 =  45;     // (program 2)

Servo myServo_2;

volatile int encoderPos_2 = 0;
int lastEncoded_2 = 0;

int servoPos_2 = 90; // startowa pozycja serwa (program 2)

unsigned long lastActivityTime_2 = 0;   // czas ostatniego ruchu enkodera (program 2)

// --- Definicje i zmienne z programu 3 ---
#define ENCODER_CLK_3 22   // kanał A (CLK) (program 3)
#define ENCODER_DT_3 23    // kanał B (DT) (program 3)
#define BUTTON_PIN_3 25
#define SERVO_PIN_3 44

Servo servo_3;

int lastCLK_3 = HIGH;
int direction_3 = 1;       // 1 = prawo, -1 = lewo
bool servoRunning_3 = false;

// --- Funkcje z programu 1 ---
void moveServo_1(int direction) 
{
  myServo1_1.write(direction);
  delay(moveTime_1);
  myServo1_1.write(servoStop_1);
}

// --- Funkcje z programu 2 ---
void updateEncoder_2() {
  int MSB = digitalRead(encoderPinA_2);
  int LSB = digitalRead(encoderPinB_2);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded_2 << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos_2++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos_2--;

  lastEncoded_2 = encoded;
}

void setup() 
{
  // --- Setup programu 1 ---
  pinMode(CLK1_1, INPUT);
  pinMode(DT1_1, INPUT);

  myServo1_1.attach(pinServo_1);
  myServo1_1.write(servoStop_1);

  lastCLK_1 = digitalRead(CLK1_1);

  // --- Setup programu 2 ---
  pinMode(encoderPinA_2, INPUT_PULLUP);
  pinMode(encoderPinB_2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA_2), updateEncoder_2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB_2), updateEncoder_2, CHANGE);

  myServo_2.attach(servoPin_2);
  myServo_2.write(servoPos_2);

  lastActivityTime_2 = millis();

  // --- Setup programu 3 ---
  servo_3.attach(SERVO_PIN_3);
  pinMode(ENCODER_CLK_3, INPUT);
  pinMode(ENCODER_DT_3, INPUT);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  servo_3.writeMicroseconds(1500); // stop na starcie
  lastCLK_3 = digitalRead(ENCODER_CLK_3);

}

void loop() 
{
  
  // --- Program 1: obsługa enkodera i ruch serwa ---
  int currentCLK_1 = digitalRead(CLK1_1);

  if (currentCLK_1 != lastCLK_1 && currentCLK_1 == HIGH)     // detekcja zbocza narastającego
  {
    if (digitalRead(DT1_1) != currentCLK_1) {
      moveServo_1(servoLeft_1);       // obrót w lewo
    } 
    else {
      moveServo_1(servoRight_1);        // obrót w prawo
    }
  }
  lastCLK_1 = currentCLK_1;

  // --- Program 2: obsługa enkodera z przerwami i sterowanie serwem ---
  static int lastPos_2 = 0;

  noInterrupts();
  int pos_2 = encoderPos_2;
  interrupts();

  if (pos_2 != lastPos_2) {
    int diff_2 = pos_2 - lastPos_2;
    lastPos_2 = pos_2;

    servoPos_2 += diff_2 * 4;  // krok 4 jednostki
    servoPos_2 = constrain(servoPos_2, 0, 180);
    myServo_2.write(servoPos_2);

    lastActivityTime_2 = millis();
  }

  // --- Program 3: obsługa enkodera, przycisku i sterowanie serwem ---
int currentCLK_3 = digitalRead(ENCODER_CLK_3);
  if (currentCLK_3 != lastCLK_3 && currentCLK_3 == LOW) {
    if (digitalRead(ENCODER_DT_3) != currentCLK_3) {
      direction_3 = 1;   // prawo
    } else {
      direction_3 = -1;  // lewo
    }
  }
  lastCLK_3 = currentCLK_3;

  // === Obsługa przycisku ===
  if (digitalRead(BUTTON_PIN_3) == LOW && !servoRunning_3) {
    servoRunning_3 = true;
    delay(200); // anty-dbounce
  }

  // === Obsługa serwa ===
  if (servoRunning_3) {
    int lightValue = analogRead(PHOTO_PIN_3); // odczyt fotorezystora

    if (direction_3 == 1) {
      servo_3.writeMicroseconds(1700); // ruch w prawo
    } else {
      servo_3.writeMicroseconds(1300); // ruch w lewo
    }

    // zatrzymanie gdy jasność przekroczy 100
    if (lightValue > 100) {
      servo_3.writeMicroseconds(1500); // stop
      servoRunning_3 = false;
    }
  }
}
