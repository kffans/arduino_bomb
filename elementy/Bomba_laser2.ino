#include <Servo.h>

#define pinCLK 2
#define pinDT 3
#define pinServo 8
#define pinPhotoresistor A0

Servo myServo;

int pos = 90; // początkowa pozycja serwa
int lastCLKState;
int currentCLKState;

void setup() {
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);
  myServo.attach(pinServo);
  myServo.write(pos);
  lastCLKState = digitalRead(pinCLK);
  //Serial.begin(9600);
}

void loop() {
  currentCLKState = digitalRead(pinCLK);
  if (currentCLKState != lastCLKState) {
    if (digitalRead(pinDT) != currentCLKState) {
      pos += 2;
    } else {
      pos -= 2;
    }
    myServo.write(pos);
  }
  lastCLKState = currentCLKState;

  int lightValue = analogRead(pinPhotoresistor);
  //Serial.print("Wartosc fotorezystora: ");
  //Serial.println(lightValue);

  if (lightValue > 930) {
    //Serial.println("wygrana");
    pos = 90;
    myServo.write(90);
  }

}
