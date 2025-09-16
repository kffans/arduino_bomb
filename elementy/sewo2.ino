#include <Servo.h>

const int encoderPinA = 2;
const int encoderPinB = 3;
const int servoPin = 9;

Servo myServo;

volatile int encoderPos = 0;
int lastEncoded = 0;

int servoPos = 90; // startowa pozycja serwa

// --- Nowe zmienne ---
unsigned long lastActivityTime = 0;   // czas ostatniego ruchu enkodera
bool autoMode = false;                // czy działa tryb automatyczny
unsigned long autoStartTime = 0;      // czas startu automatu
unsigned long lastAutoStep = 0;       // ostatni krok automatycznego ruchu

void setup() {
  Serial.begin(9600);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  myServo.attach(servoPin);
  myServo.write(servoPos);

  lastActivityTime = millis(); // startowy czas
}

void loop() {
  static int lastPos = 0;

  noInterrupts();
  int pos = encoderPos;
  interrupts();

  if (pos != lastPos) {
    // ruch enkodera -> wychodzimy z trybu automatycznego
    autoMode = false;
    int diff = pos - lastPos;
    lastPos = pos;

    servoPos += diff * 4;  // krok 4 stopnie
    servoPos = constrain(servoPos, 0, 180);
    myServo.write(servoPos);

    lastActivityTime = millis();  // reset czasu bezczynności

    Serial.print("Encoder: ");
    Serial.print(pos);
    Serial.print(" Servo position: ");
    Serial.println(servoPos);
  }

  // --- Tryb automatyczny ---
  unsigned long now = millis();

  if (!autoMode && (now - lastActivityTime > 5000)) {
    // brak aktywności > 5 s -> start automatu
    autoMode = true;
    autoStartTime = now;
    lastAutoStep = now;
    Serial.println("AUTO MODE START");
  }

  if (autoMode) {
    // jeśli minęło 5 sekund działania -> koniec automatu
    if (now - autoStartTime > 5000) {
      autoMode = false;
      lastActivityTime = now; // żeby znów nie startował od razu
      Serial.println("AUTO MODE END");
    } else {
      // ruch automatyczny (co 50 ms krok w prawo)
      if (now - lastAutoStep > 50) {
        lastAutoStep = now;
        servoPos += 1; // 1° co 50ms = ok. 20°/s
        servoPos = constrain(servoPos, 0, 180);
        myServo.write(servoPos);

        Serial.print("AUTO Servo position: ");
        Serial.println(servoPos);
      }
    }
  }
}

void updateEncoder() {
  int MSB = digitalRead(encoderPinA);
  int LSB = digitalRead(encoderPinB);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos--;

  lastEncoded = encoded;
}
