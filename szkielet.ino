// @TODO wszystkie include
#include <Servo.h>
#include <Keypad.h>
#include "pitches.h"



// @TODO define wszystkie piny
// 
#define BOMB_RESET 1
#define TIMER 1     /* timer 7-seg */
#define LED_FAIL 1  /* led przy timerze gdy popełniony błąd */
#define BUZZER 1    /* do morsa, przy wybuchu */
// 
#define LED_SUCC_WIRES 1
#define WIRES_BASE 1 /* @NOTE to kabelek bazowy (pierwszy); kabelki muszą być podłączone obok siebie w rosnącej kolejności (jeżeli pierwszy kabelek jest na pinie 20, drugi musi być na 21, itd.) */
// 
#define LED_SUCC_MELODY 1
#define MELODY_KEYBOARD 1
#define MELODY_BTN 1
// 
#define LED_SUCC_LASER 1
#define LASER_STEER_LEFT 2   /* sterowanie serwo w lewo */
#define LASER_STEER_RIGHT 3  /* sterowanie serwo w prawo */
#define LASER_SERVO 4
#define LASER 6
#define LASER_LDR 1    /* fotorezystor do wykrywania lasera*/
// 
#define INTERVAL_BTN 1  /* przycisk do naciskania co każde 10s */
#define INTERVAL_LED 1  /* żółty led który z czasem miga coraz szybciej */
// 
#define OLED 1
#define OLED_JOYSTICK 1



enum class Status {NEUTRAL, SUCCESS, FAILURE};



// GLOBAL VARS
// @TODO zmienne globalne tutaj
Status gameStatus = Status::NEUTRAL;
string ID = "";
string morseCodeLetters = "";

// @TODO zmienne dla 7-SEG

#define WIRES_COUNT 5
bool wiresMask[WIRES_COUNT]; // tam gdzie jest "true", ma być przecięty kabel
byte wiresCutCount = 0;

#define MELODY_TONES_COUNT 4
int melodyTones[MELODY_TONES_COUNT];

Servo laserServo;
int laserRotation = 0;
unsigned int LASER_TARGET_ACCURACY = 50;

// @TODO zmienne dla OLED

unsigned int TIME_MS = 0;
const unsigned int TIME_TOTAL_MS = 120000; // 2 minuty = 120000ms
const unsigned int TIME_DELAY_DURATION_MS = 1;



// GENERATE
string generateID(){
    // @TODO using random() or randomSeed(); examples of ID: A23C1, H5833, J11GU. Two last characters are from morse code, 1st char is always a letter, 2nd and 3rd are always digits
}
void generateMorseCode() {
    // @TODO converts last two characters from ID to dots and dashes ("0" is dot, "1" is dash, "2" is space/end)
    // morseCode = "01210002" // -> "._ _... "
}
void generateWiresMask(){
    /* @TODO maska i jej wartości są obliczane na podstawie ID */
    // wiresMask[0] = true; // oznacza że pierwszy kabel ma być przecięty
    // wiresCutCount = 1; // policz ile jest wartości true w masce
}
void generateMelodyTones(){
    /* @TODO nuty są generowane losowo, znowu random() lub randomSeed() */
    // for (int i = 0; i < MELODY_TONES_COUNT; i++ ){
    //     melodyTones[i] = NOTE_G3; /* @TODO generowanie random */
    // }
}



// CHECK
Status checkWires(){
    int successfulCuts = 0;
    for (int i = 0; i < WIRES_COUNT; i++) {
        if (digitalRead(WIRES_BASE + i) == LOW) { // jeżeli kabel jest wyjęty
            if (wiresMask[i] == true) { successfulCuts++; }
            else                      { return Status::FAILURE; }
        }
    }
    if (successfulCuts == wiresCutCount) {
        return Status::SUCCESS;
    }
    return Status::NEUTRAL;
}
Status checkMelody() {
    // @TODO sprawdza czy melodia została rozwiązana przez rozbrajacza
    return Status::NEUTRAL;
}
Status checkLaser() {
    unsigned int laserAccuracy = analogRead(LASER_LDR);
    if (laserAccuracy >= LASER_TARGET_ACCURACY) { // @TODO skalibrować wartość LASER_TARGET_ACCURACY
        return Status::SUCCESS;
    }
    return Status::NEUTRAL;
}

void initBomb(){
    // @TODO jeżeli dodano wcześniej zmienne globalne, należy je resetować tutaj
    gameStatus = Status::NEUTRAL;
    ID = "";
    morseCodeLetters = "";

    // @TODO zmienne dla 7-SEG

    wiresCutCount = 0;

    laserServo;
    laserRotation = 0;

    // @TODO zmienne dla OLED

    TIME_MS = 0;

    

    // @TODO wypisać wszystkie pinMode i output/input
    // pinMode(TIMER, ); // @TODO uruchomić timer
    pinMode(LED_FAIL, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    pinMode(LED_SUCC_WIRES, OUTPUT);
    for (int i = 0; i < WIRES_COUNT; i++) {
        pinMode(WIRES_BASE + i, INPUT_PULLUP);
    }

    pinMode(LED_SUCC_MELODY, OUTPUT);
    // pinMode(MELODY_KEYBOARD, ); // @TODO zainicjować tutaj klawiaturę

    pinMode(LED_SUCC_LASER, OUTPUT);
    pinMode(LASER_STEER_LEFT, INPUT);
    pinMode(LASER_STEER_RIGHT, INPUT);
    laserServo.attach(LASER_SERVO);
    pinMode(LASER, OUTPUT);
    pinMode(LASER_LDR, INPUT); /* @TODO fotorezystor jest input/input_pullup? */

    pinMode(INTERVAL_LED, OUTPUT);
   
    // @TODO dokonczyc inicjalizacje/pinmode OLED'a oraz jego joysticka
    //pinMode(OLED,);
    //pinMode(OLED_JOYSTICK, INPUT);

    digitalWrite(LED_FAIL,        LOW);
    digitalWrite(LED_SUCC_WIRES,  LOW);
    digitalWrite(LED_SUCC_MELODY, LOW);
    digitalWrite(LED_SUCC_LASER,  LOW);
    digitalWrite(LASER,           HIGH);
    digitalWrite(INTERVAL_LED,    LOW);


    // generowanie rozgrywki
    ID = generateID();
    generateMorseCode();
    generateWiresMask();
    generateMelodyTones();
    laserServo.write(0);
}

void setup(){
    initBomb();
}

void loop() {
    // if (digitalRead(BOMB_RESET) == HIGH) { initBomb(); } // @TODO inaczej to zrobić, niech się wykonuje tylko po puszczeniu przycisku

    switch (gameStatus) {
        case Status::NEUTRAL: { // gra toczy się
            // @TODO melody + buzzer + kod morsa
            // @TODO interval, dioda + przycisk; załącza się wtedy gdy rozwiązemy przynajmniej jeden moduł?

            if (TIME_DELAY_MS % 100 == 0) { // wykonuje tu co każde 100ms
                // @TODO clear 7-seg; calculate current time from TIME_MS; set time on 7-seg; doesnt need to be updated that much?
            }

            if (TIME_DELAY_MS % 16 == 0) { // wykonuje tu co każde 16ms
                // @TODO clear OLED; display info
                
                Status wiresStatus = checkWires();
                switch (wiresStatus) {
                    case Status::SUCCESS:
                        digitalWrite(LED_SUCC_WIRES, HIGH);
                        break;
                    case Status::FAILURE:
                        gameStatus = Status::FAILURE; // bomba wybucha
                        break;
                    default: break;
                }
               

                // @TODO sprawdzanie melodii
                Status melodyStatus = checkMelody();
                switch (melodyStatus) {
                    case Status::SUCCESS:
                        digitalWrite(LED_SUCC_WIRES, HIGH);
                        break;
                    case Status::FAILURE:
                        // @TODO zapala się led od fail? szybciej czas idzie? 
                        break;
                    default: break;
                }
                

                if (digitalRead(LASER_STEER_LEFT)  == 0) { laserRotation += 5; }
                if (digitalRead(LASER_STEER_RIGHT) == 0) { laserRotation -= 5; }
                if (laserRotation > 180) { laserRotation = 180; } 
                if (laserRotation < 0) { laserRotation = 0; } 
                laserServo.write(laserRotation);
                Status laserStatus = checkLaser();

                if (wiresStatus  == Status::SUCCESS && 
                    melodyStatus == Status::SUCCESS &&
                    laserStatus  == Status::SUCCESS) {
                    gameStatus = Status::SUCCESS; // wygrano grę
                }
            }
            delay(TIME_DELAY_DURATION_MS);
            TIME_MS += TIME_DELAY_DURATION_MS;
            if (TIME_MS >= TIME_TOTAL_MS) { // bomba wybucha po upływie czasu
                gameStatus = Status::FAILURE;
            }
            break;
        }
        case Status::SUCCESS: { // wygrano grę
            /* @TODO ekran timera się gasi, OLED się gasi */
            digitalWrite(LASER, LOW);
            break;
        }
        case Status::FAILURE: { // bomba wybuchła
            /* @TODO buzzer wydaje głosy wybuchu, na OLED jest pokazany wybuch, timer pokazuje 0:00 */
            digitalWrite(LASER, LOW);
            break;
        }
        default: break;
    }
}



