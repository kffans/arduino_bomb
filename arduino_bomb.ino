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

#define ID_CHAR_COUNT 5
#define ID_MORSE_CODE_CHAR_COUNT 2
string ID = "";
string morseCodeLetters = "";

// @TODO zmienne dla 7-SEG

#define WIRES_COUNT 5
bool wiresMask[WIRES_COUNT]; // tam gdzie jest "true", ma być przecięty kabel
byte wiresCutCount = 0;

#define MELODY_TONES_COUNT 4
int melodyTones[MELODY_TONES_COUNT];
char melodyKeys[MELODY_TONES_COUNT];

Servo laserServo;
int laserRotation = 0;
unsigned int LASER_TARGET_ACCURACY = 50;

// @TODO zmienne dla OLED

unsigned int TIME_MS = 0;
const unsigned int TIME_TOTAL_MS = 120000; // 2 minuty = 120000ms
const unsigned int TIME_DELAY_DURATION_MS = 1;



// GENERATE
string generateID(){ // examples of ID: A23C1, H5833, J11GU. Two last characters are from morse code, 1st char is always a letter, 2nd and 3rd are always digits
    string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string digits = "0123456789";
    unsigned int lettersLength = letters.length();
    unsigned int digitsLength = digits.length();
    ID += letters[random(lettersLength)];
    ID += digits[random(digitsLength)];
    ID += digits[random(digitsLength)];
    for (int i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
        int putDigit = random(2); // losuje 0 albo 1
        if (putDigit) { ID += digits[random(digitsLength)];   }
        else          { ID += letters[random(lettersLength)]; }
    }
}
void generateMorseCode() { // converts last two characters from ID to dots and dashes ("0" is dot, "1" is dash, "2" is space/end); example: "AB" -> "._ _... " -> "01210002"
    if (ID.length() != ID_CHAR_COUNT ) { return; }

    for (int i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
        switch (ID[ID_CHAR_COUNT - ID_MORSE_CODE_CHAR_COUNT + i]) {
            case '0': morseCodeLetters += "11111"; break; case '1': morseCodeLetters += "01111"; break;
            case '2': morseCodeLetters += "00111"; break; case '3': morseCodeLetters += "00011"; break;
            case '4': morseCodeLetters += "00001"; break; case '5': morseCodeLetters += "00000"; break;
            case '6': morseCodeLetters += "10000"; break; case '7': morseCodeLetters += "11000"; break;
            case '8': morseCodeLetters += "11100"; break; case '9': morseCodeLetters += "11110"; break;
            case 'A': morseCodeLetters += "01";    break; case 'B': morseCodeLetters += "1000";  break;
            case 'C': morseCodeLetters += "1010";  break; case 'D': morseCodeLetters += "100";   break;
            case 'E': morseCodeLetters += "0";     break; case 'F': morseCodeLetters += "0010";  break;
            case 'G': morseCodeLetters += "110";   break; case 'H': morseCodeLetters += "0000";  break;
            case 'I': morseCodeLetters += "00";    break; case 'J': morseCodeLetters += "0111";  break;
            case 'K': morseCodeLetters += "101";   break; case 'L': morseCodeLetters += "0100";  break;
            case 'M': morseCodeLetters += "11";    break; case 'N': morseCodeLetters += "10";    break;
            case 'O': morseCodeLetters += "111";   break; case 'P': morseCodeLetters += "0110";  break;
            case 'Q': morseCodeLetters += "1101";  break; case 'R': morseCodeLetters += "010";   break;
            case 'S': morseCodeLetters += "000";   break; case 'T': morseCodeLetters += "1";     break;
            case 'U': morseCodeLetters += "001";   break; case 'V': morseCodeLetters += "0001";  break;
            case 'W': morseCodeLetters += "011";   break; case 'X': morseCodeLetters += "1001";  break;
            case 'Y': morseCodeLetters += "1011";  break; case 'Z': morseCodeLetters += "1100";  break;
            default: break;
        }
        morseCodeLetters += "2";
    }
}
void generateWiresMask(){
    for (int i = 0; i < WIRES_COUNT; i++) {
        wiresMask[i] == false;
    }

    unsigned int digitCount = 0;
    unsigned int letterCount = 0;
    unsigned int digitSum = 0;
    unsigned int evenDigitCount = 0;
    unsigned int vowelCount = 0;      // liczba samogłosek
    for (int i = 0; i < ID_CHAR_COUNT; i++) {
        if (isDigit(ID[i])) {
            int digit = ID[i] - 48;
            digitSum += digit;
            digitCount++;
            if (digit % 2 == 0) {
                evenDigitCount++;
            }
        }
        else {
            letterCount++;
            if (ID[i] == 'A' || ID[i] == 'I' || ID[i] == 'E' || ID[i] == 'O' || ID[i] == 'U' || ID[i] == 'Y') {
                vowelCount++;
            }
        }
    }

    // numery kabli należy liczyć od góry w dół:
        // jeżeli dwa ostatnie znaki w ID są obie cyframi lub obie literami, POD ŻADNYM POZOREM nie przecinaj ostatniego kabla
        // jeżeli w ID jest przynajmniej jedna samogłoska, przetnij ostatni kabel
        // jeżeli cyfry w ID dodają się do liczby większej lub równej 15, przetnij drugi kabel
        // jeżeli są same cyfry parzyste (zero też się liczy), przetnij kable o nieparzystych numerach
        // jeżeli w ID występuje przynajmniej jedna para liter, które są obok siebie w alfabecie, przetnij czwarty kabel
        // jeżeli do tej pory nie przetnąłeś żadnego kabla, przetnij wszystkie kable oprócz trzeciego
     
    if (vowelCount > 0) { wiresMask[4] = true; }
    if (digitSum >= 15) { wiresMask[1] = true; }
    if (digitCount == evenDigitCount) { wiresMask[0] = true; wiresMask[2] = true; wiresMask[4] = true; }
    if ( (!isDigit(ID[3]) && abs(ID[0] - ID[3]) == 1) || (!isDigit(ID[4]) && abs(ID[0] - ID[4]) == 1) || (!isDigit(ID[3]) && !isDigit(ID[4]) && abs(ID[3] - ID[4]) == 1) ) { wiresMask[3] = true; }

    if ( (isDigit(ID[3]) && isDigit(ID[4])) || (!isDigit(ID[3]) && !idDigit(ID[4])) ) { wiresMask[4] = false; }

    bool canCutSomething = false;
    for (int i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            canCutSomething = true;
            break;
        }
    }
    if (!canCutSomething) { wiresMask[0] = true; wiresMask[1] = true; wiresMask[3] = true; wiresMask[4] = true; }


    if ( (isDigit(ID[3]) && isDigit(ID[4])) || (!isDigit(ID[3]) && !idDigit(ID[4])) ) { wiresMask[4] = false; }


    for (int i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            wiresCutCount++;
        }
    }
}
void generateMelodyTones(){
    int tones[10] = [NOTE_G1, NOTE_G3, NOTE_G5, NOTE_E4, NOTE_A4, NOTE_C4, NOTE_A4, NOTE_E1, NOTE_E2, NOTE_E3];
    int tonesLength = 10;
    for (int i = 0; i < MELODY_TONES_COUNT; i++){
        int index = random(tonesLength);
        melodyKeys[i] = index + 48;
        melodyTones[i] = tones[index];
    }
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
    randomSeed(analogRead(A0)); // @NOTE pierwszy pin analogu jest zajęty tutaj, by generować liczby losowe

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



