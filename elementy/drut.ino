// Definicje pinów
const int pin2 = 2; // Pin do monitorowania "Start"
const int pin3 = 3; // Pin do monitorowania błędów
const int pin4 = 4; // Pin wyzwalający komunikaty wygrana/przegrana

int lowCount = 0; // Licznik stanów LOW na pinie 3

void setup() {
  // Inicjalizacja pinów
  pinMode(pin2, INPUT_PULLUP);
  pinMode(pin3, INPUT_PULLUP);
  pinMode(pin4, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("System gotowy");
  
}

void loop() {
  // Obsługa pinu 2 - Start
  if (digitalRead(pin2) == LOW) {
    Serial.println("Start");
    lowCount = 0;
    delay(1500);
  }

  // Obsługa pinu 3 - Błędy
  if (digitalRead(pin3) == LOW) {
    lowCount++;
    Serial.print("Popełniłeś błąd ");
    Serial.println(lowCount);
    delay(1000);
  }

  // Obsługa pinu 4 - Wynik
  if (digitalRead(pin4) == LOW) {
    if (lowCount == 0) {
      Serial.println("Wygrałeś!");
    } else {
      Serial.println("Przegrałeś!");
    }
    delay(500); // Debounce
  }
}
