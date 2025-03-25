void setup() {
    pinMode(5,INPUT_PULLUP);
    pinMode(6,INPUT_PULLUP);
    Serial.begin(9600);
}

void loop() {
    byte value5 = digitalRead(5); //kabelek nr.1
    byte value6 = digitalRead(6);

    Serial.print(F("Wejscie cyfrowe 5:"));
    if(value5==LOW){
        Serial.println(F("LOW"));
    }
    else{
        Serial.println(F("HIGH"));
    }


    Serial.print(F("Wejscie cyfrowe 6:"));
    if(value6==LOW){
        Serial.println(F("LOW"));
    }
    else{
        Serial.println(F("HIGH"));
    }
    delay(500);
}






