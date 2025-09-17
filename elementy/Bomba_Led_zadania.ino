#include <Adafruit_NeoPixel.h>

#define LED_PIN 50       
#define LED_COUNT 8    
#define BUTTON_PIN 42

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int counter = 0;
bool lastButtonState = HIGH;

void setup() {
  strip.begin();
  strip.show();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Start programu...");
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  // wykrywanie zbocza (puszczenie->naciśnięcie)
  if (lastButtonState == HIGH && buttonState == LOW) {
    counter++;
    if (counter > 5) {
      counter = 0;
    }

    Serial.print("Licznik = ");
    Serial.println(counter);

    updateLEDs();
  }

  lastButtonState = buttonState;
}

void updateLEDs() {
  strip.clear();


  if (counter == 1) {
    strip.setPixelColor(3, strip.Color(0, 255, 0));
  }
  if (counter == 2) {
    strip.setPixelColor(4, strip.Color(0, 255, 0));
  }
  if (counter == 3) {
    strip.setPixelColor(5, strip.Color(0, 255, 0));
  }
  if (counter == 4) {
    strip.setPixelColor(6, strip.Color(0, 255, 0));
  }
  if (counter == 5) {
    strip.setPixelColor(7, strip.Color(0, 255, 0));
  }

  strip.show();
}
