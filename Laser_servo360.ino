#include <Servo.h>

int czujnik = A1;

// Piny enkodera
const int encoderPinA = 2;
const int encoderPinB = 3;

// Pin serwa
const int servoPin = 9;

Servo myServo;

volatile int encoderPos = 0;
int lastEncoded = 0;

int servoPos = 90; // startowa pozycja serwa (środek)

void setup() {
  Serial.begin(9600);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Podłącz przerwania do pinów enkodera
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  myServo.attach(servoPin);
  myServo.write(servoPos);
}

void loop() {
  static int lastPos = 0;
  int war = analogRead(czujnik); 
  Serial.print(war);
  if (war > 700)                      //gdy wartość przekroczy pewien ustalony próg, wtedy dioda na pinie 13 zaświeci się
  {
    Serial.println("Wygrana");  
  }
  else
  {
     Serial.println("Przegrana"); 
  }
  noInterrupts();
  int pos = encoderPos;
  interrupts();

  if (pos != lastPos) {
    int diff = pos - lastPos;
    lastPos = pos;

    // Zmiana pozycji serwa o 8 stopni na jeden przeskok enkodera
    servoPos += diff * 4;

    // Ogranicz pozycję serwa do 0-180 stopni
    servoPos = constrain(servoPos, 0, 180);

    myServo.write(servoPos);

    Serial.print("Encoder: ");
    Serial.print(pos);
    Serial.print(" Servo position: ");
    Serial.println(servoPos);
  }
}

// Funkcja obsługi enkodera (quadrature decoding)
void updateEncoder() {
  int MSB = digitalRead(encoderPinA); // Most Significant Bit
  int LSB = digitalRead(encoderPinB); // Least Significant Bit

  int encoded = (MSB << 1) | LSB; // 2-bitowy odczyt
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos--;

  lastEncoded = encoded;
}
