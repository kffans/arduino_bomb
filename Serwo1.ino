#include <Servo.h>

// --- Piny enkodera ---
const int pinCLK = 2;   // kanał A
const int pinDT  = 3;   // kanał B

// --- Serwo ---
const int pinServo = 9;
Servo myServo;
int servoStop = 90;     // zatrzymanie
int servoRight = 100;   // ruch w prawo (dobierz doświadczalnie)
int servoLeft  = 80;    // ruch w lewo (dobierz doświadczalnie)

// --- Parametry ruchu ---
int stepDeg = 2;        // krok obrotu
float degPerMs = 0.1;   // ile stopni na 1 ms (dobierz doświadczalnie!)
int moveTime;           // czas ruchu dla kroku

// --- Stan enkodera ---
int lastCLK;

void setup() {
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);

  myServo.attach(pinServo);
  myServo.write(servoStop);

  lastCLK = digitalRead(pinCLK);

  Serial.begin(9600);

  moveTime = stepDeg / degPerMs;
  Serial.print("moveTime = ");
  Serial.println(moveTime);
}

void loop() {
  int currentCLK = digitalRead(pinCLK);

  // detekcja zbocza
  if (currentCLK != lastCLK && currentCLK == 1) {
    if (digitalRead(pinDT) != currentCLK) {
      // obrót w lewo
      Serial.println("Lewo o 8°");
      myServo.write(servoLeft);
      delay(moveTime);
      myServo.write(servoStop);
    } else {
      // obrót w prawo
      Serial.println("Prawo o 8°");
      myServo.write(servoRight);
      delay(moveTime);
      myServo.write(servoStop);
    }
  }

  lastCLK = currentCLK;
}