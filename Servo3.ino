#include <Servo.h>

// --- Konfiguracja pinów ---
const int pinCLK = 2;     // kanał A enkodera
const int pinDT  = 3;     // kanał B enkodera
const int pinSW  = 4;     // przycisk enkodera
const int pinServo = 9;   // pin serwa

Servo myServo;

// --- Zmienne sterujące ---
bool running = false;     // czy serwo jest włączone
int direction = -1;       // -1 = lewo, +1 = prawo
int lastCLK;
bool lastButtonState = HIGH;

// --- Ustawienia serwa 360 ---
int servoStop = 90;       // zatrzymanie
int servoSpeed = 18;      // ok. 1 obrót / 5 s -> offset od 90

void setup() {
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);
  pinMode(pinSW, INPUT_PULLUP);

  myServo.attach(pinServo);
  myServo.write(servoStop); // na starcie stop

  lastCLK = digitalRead(pinCLK);
  Serial.begin(9600);
}

void loop() {
  // --- Obsługa przycisku ---
  bool buttonState = digitalRead(pinSW);
  if (lastButtonState == HIGH && buttonState == LOW) {
    running = !running; // zmiana stanu
    if (!running) {
      myServo.write(servoStop);
    }
    delay(200); // prosty debounce
  }
  lastButtonState = buttonState;

  // --- Obsługa enkodera (zmiana kierunku) ---
  int currentCLK = digitalRead(pinCLK);
  if (currentCLK != lastCLK && currentCLK == LOW) {
    if (digitalRead(pinDT) != currentCLK) {
      direction = 1;   // prawo
    } else {
      direction = -1;  // lewo
    }
  }
  lastCLK = currentCLK;

  // --- Sterowanie serwem ---
  if (running) {
    if (direction == 1) {
      myServo.write(servoStop + servoSpeed); // prawo
    } else {
      myServo.write(servoStop - servoSpeed); // lewo
    }
  }
}
