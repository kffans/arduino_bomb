#include <Servo.h>             // serwa
#include <Keypad.h>            // Keypad by Mark Stanley v3.1.1; melody
#include <Wire.h>              // timer
#include <Adafruit_NeoPixel.h> // pasek ledów
#include <Adafruit_ST7735.h>   // maze
#include <Adafruit_GFX.h>      // maze
#include <SPI.h>               // maze
#include "pitches.h"           // buzzer
#include "boom1bit.h"          // obraz wybuchu dla e-papieru
#include "epd4in2_V2.h"        // e-papier
#include "imagedata.h"         // e-papier
#include "epdpaint.h"          // e-papier



#define TIMER_ADDR          0x30 
#define TIMER_DISPLAY       0x00
#define TIMER_BRIGHTNESS    0x06


#define BOMB_RESET_BTN      45
//
#define SUCCESS_LED_STRIP_PIN   44       
#define SUCCESS_LED_STRIP_COUNT 5 
//
#define BUZZER              15  /* do morsa, przy wybuchu */
//
#define MORSE_BTN           14
// 
#define INTERVAL_LED        38  /* żółty led który z czasem miga coraz szybciej */
#define INTERVAL_BTN        39  /* przycisk do naciskania co każde 5s */
// 
#define WIRES_BASE          2   /* @NOTE to kabelek bazowy (pierwszy); kabelki muszą być podłączone obok siebie w rosnącej kolejności (jeżeli pierwszy kabelek jest na pinie 20, drugi musi być na 21, itd.) */
// 
#define MELODY_KEYBOARD     7   /* @NOTE rezerwuje 7, 8, 9, 10, 11, 12 */
#define MELODY_BTN          13
// 
#define LASER_LDR           A1  /* fotorezystor do wykrywania lasera*/
#define LASER_SERVO         16
//#define LASER               17
#define LASER_STEER_CLK     18
#define LASER_STEER_DT      19
// 
#define MAZE_TFT_CS         10
#define MAZE_TFT_RST        9
#define MAZE_TFT_DC         8
#define MAZE_JOY_X          A3
#define MAZE_JOY_Y          A4
//
#define CIRCLES_LDR         A2  /* fotorezystor do wykrywania swiatla */
#define CIRCLES_SERVO_UP    30
#define CIRCLES_SERVO_LEFT  31
#define CIRCLES_SERVO_RIGHT 32
#define CIRCLES_STEER_UP_CLK    23
#define CIRCLES_STEER_UP_DT     24
//#define CIRCLES_STEER_UP_SW     1
#define CIRCLES_STEER_LEFT_CLK  25
#define CIRCLES_STEER_LEFT_DT   26
//#define CIRCLES_STEER_LEFT_SW   1
#define CIRCLES_STEER_RIGHT_CLK 27
#define CIRCLES_STEER_RIGHT_DT  28
#define CIRCLES_STEER_RIGHT_SW  29
//
// @NOTE piny do e-papieru są zdefinioweane w pliku epdif.h, jest ich cztery
#define COLORED     0
#define UNCOLORED   1


enum class Status {NEUTRAL, SUCCESS, FAILURE};
enum class Buzzer {SILENT, MORSE, MELODY, KEYBOARD};



// GLOBAL VARS
// @TODO zmienne globalne tutaj
Adafruit_NeoPixel strip(SUCCESS_LED_STRIP_COUNT, SUCCESS_LED_STRIP_PIN, NEO_GRB + NEO_KHZ800); // LED strip

Status gameStatus = Status::NEUTRAL;
Buzzer buzzerMode = Buzzer::SILENT;

const byte ID_CHAR_COUNT            = 5;
const byte ID_MORSE_CODE_CHAR_COUNT = 2;
const unsigned int MORSE_DOT_TIME   = 150;
const unsigned int MORSE_DASH_TIME  = 500;
const unsigned int MORSE_BREAK      = 600;
const unsigned int MORSE_LONG_BREAK = 1600;
String ID = "";
String morseCodeLetters     = "";
byte morseCodeLettersLength = 0;
byte morseCodeLetterIndex   = 0;
bool isMorsePaused = false;

const unsigned int INTERVAL_TOTAL_ACTIVATION_TIME = 5000;
const unsigned int INTERVAL_TOTAL_EXPLOSION_TIME  = 5000;
unsigned int intervalActivationTime = 0;
unsigned int intervalExplosionTime  = 0;
bool hasIntervalActivated = false;
bool intervalLEDState = false;

const byte WIRES_COUNT = 5;
bool wiresMask[WIRES_COUNT]; // tam gdzie jest "true", ma być przecięty kabel
byte wiresCutCount = 0;

const byte MELODY_TONES_COUNT = 3;
const byte MELODY_KEYPAD_ROWS = 3;
const byte MELODY_KEYPAD_COLS = 3;
const unsigned int MELODY_KEY_TIME     = 500;
const unsigned int MELODY_KEY_BREAK    = 500;
const unsigned int MELODY_WAITING_TIME = 5000;
byte rowPins[MELODY_KEYPAD_ROWS] = { MELODY_KEYBOARD,     MELODY_KEYBOARD + 1, MELODY_KEYBOARD + 2 }; // Piny wierszy
byte colPins[MELODY_KEYPAD_COLS] = { MELODY_KEYBOARD + 3, MELODY_KEYBOARD + 4, MELODY_KEYBOARD + 5 }; // Piny kolumn
char keymap[MELODY_KEYPAD_ROWS][MELODY_KEYPAD_COLS] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8', '9' } }; 
Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, MELODY_KEYPAD_ROWS, MELODY_KEYPAD_COLS); //inicjalizacja klawiatury
const int keyboardTones[MELODY_KEYPAD_ROWS * MELODY_KEYPAD_COLS] = {NOTE_C1, NOTE_CS4, NOTE_D4, NOTE_DS5, NOTE_E5, NOTE_F6, NOTE_FS6, NOTE_G7, NOTE_GS7}; 
int melodyTones[MELODY_TONES_COUNT];
char melodyKeys[MELODY_TONES_COUNT];
bool isMelodyPaused = false;
bool isMelodySuccessful = false;
bool isKeyPaused = false;
bool isKeyCorrect = true;
byte toneIndex = 0;
byte keyIndex = 0;

const unsigned int LASER_TARGET_ACCURACY = 930;
Servo laserServo;
int laserRotation = 90;
int laserHold     = 0;
bool isLaserDone  = false;
int laserLastCLKState = 0;
int laserCurrCLKState = 0;

const int mazeWidth = 8;
const int mazeHeight = 10;
const int maze[mazeHeight][mazeWidth] = {
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 1, 0, 1, 1, 0, 1, 0},
  {0, 0, 0, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 0, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 1, 0},
  {1, 0, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 2, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0, 0}
};
Adafruit_ST7735 mazeTFT = Adafruit_ST7735(MAZE_TFT_CS, MAZE_TFT_DC, MAZE_TFT_RST);
int mazePlayerX = 0;
int mazePlayerY = 0;
bool isMazeDone = false;
unsigned int mazeDebounceCounter = 0;

const unsigned int CIRCLES_TARGET_ACCURACY = 50;
int circlesHold = 0;
bool areCirclesDone = false;
Servo circlesServoUp, circlesServoLeft, circlesServoRight;
int circlesRotationUp = 0, circlesRotationLeft = 0, circlesRotationRight = 0;
int circlesUpLastCLK, circlesLeftLastCLK, circlesRightLastCLK;

const unsigned int TIME_TOTAL_MS = 120000; // 2 minuty = 120000ms
const unsigned int TIME_DELAY_DURATION_MS = 1;
unsigned int TIMER_SECONDS_LEFT = (TIME_TOTAL_MS / 1000) - 1;
unsigned int TIME_MS = 0;
unsigned int currentTime = 0;
unsigned int rememberedTime = millis();


// TIMER
void timerShowDigits(int d1, int d2, int d3, int d4) {
    Wire.beginTransmission(TIMER_ADDR);
    Wire.write(TIMER_DISPLAY);
    Wire.write(digitTo7Seg(d1)); Wire.write(digitTo7Seg(d2)); Wire.write(digitTo7Seg(1)); Wire.write(digitTo7Seg(d3)); Wire.write(digitTo7Seg(d4));
    Wire.endTransmission();
}
void timerSetBrightness(int level) {
    if (level < 0) level = 0;
    if (level > 7) level = 7;
    Wire.beginTransmission(TIMER_ADDR);
    Wire.write(TIMER_BRIGHTNESS);
    Wire.write(level);
    Wire.endTransmission();
}
uint8_t digitTo7Seg(int digit) {
    static const uint8_t map7seg[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
    if (digit < 0 || digit > 9) return 0x00;
    return map7seg[digit];
}


// MAZE
void mazeDraw() {
    mazeTFT.fillScreen(ST7735_BLACK);
    for (int y = 0; y < mazeHeight; y++) {
        for (int x = 0; x < mazeWidth; x++) {
            if (maze[y][x] == 1) {
                mazeTFT.fillRect(x * 16, y * 16, 16, 16, ST77XX_WHITE); // ściana
            }
        }
    }
}
void mazeDrawPlayer() {
    // Gracz = biały kwadrat 8x8 px w środku pola
    mazeTFT.fillRect(mazePlayerX * 16 + 4, mazePlayerY * 16 + 4, 8, 8, ST77XX_WHITE);
}

void mazeDrawFinish(){
    //wyjscie - pusty bialy kwadracik
    mazeTFT.drawRect(3 * 16 + 4, 8 * 16 + 4, 8, 8, ST77XX_WHITE);
}

void mazeDrawEnd(){
    mazeTFT.fillScreen(ST7735_BLACK);
    //wyjscie - pusty bialy kwadracik
    mazeTFT.setTextSize(3);
    mazeTFT.write("Wygrana!");
}

// GENERATE
String generateID(){ // examples of ID: A23C1, H5833, J11GU. Two last characters are from morse code, 1st char is always a letter, 2nd and 3rd are always digits
    String letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    String digits = "0123456789";
    unsigned int lettersLength = letters.length();
    unsigned int digitsLength = digits.length();
    ID += letters[random(lettersLength)];
    ID += digits[random(digitsLength)];
    ID += digits[random(digitsLength)];
    for (byte i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
        int putDigit = random(2); // losuje 0 albo 1
        if (putDigit) { ID += digits[random(digitsLength)];   }
        else          { ID += letters[random(lettersLength)]; }
    }
}
void generateMorseCode() { // converts last two characters from ID to dots and dashes ("0" is dot, "1" is dash, "2" is space/end); example: "AB" -> "._ _... " -> "01210002"
    if (ID.length() != ID_CHAR_COUNT ) { return; }

    for (byte i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
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
    
    morseCodeLettersLength = morseCodeLetters.length();
}
void generateWiresMask(){
    for (byte i = 0; i < WIRES_COUNT; i++) {
        wiresMask[i] == false;
    }

    byte digitCount = 0;
    byte letterCount = 0;
    unsigned int digitSum = 0;
    byte evenDigitCount = 0;
    byte vowelCount = 0;      // liczba samogłosek
    for (byte i = 0; i < ID_CHAR_COUNT; i++) {
        if (isDigit(ID[i])) {
            byte digit = ID[i] - 48;
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

    if ( (isDigit(ID[3]) && isDigit(ID[4])) || (!isDigit(ID[3]) && !isDigit(ID[4])) ) { wiresMask[4] = false; }

    bool canCutSomething = false;
    for (int i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            canCutSomething = true;
            break;
        }
    }
    if (!canCutSomething) { wiresMask[0] = true; wiresMask[1] = true; wiresMask[3] = true; wiresMask[4] = true; }


    if ( (isDigit(ID[3]) && isDigit(ID[4])) || (!isDigit(ID[3]) && !isDigit(ID[4])) ) { wiresMask[4] = false; }


    for (byte i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            wiresCutCount++;
        }
    }
}
void generateMelodyTones(){
    unsigned int keyboardTonesLength = MELODY_KEYPAD_ROWS * MELODY_KEYPAD_COLS;
    for (byte i = 0; i < MELODY_TONES_COUNT; i++){
        unsigned int index = random(keyboardTonesLength);
        melodyKeys[i] = index + 48;
        melodyTones[i] = keyboardTones[index];
    }
}


// PLAY
void playMorse() {
    unsigned int difference = currentTime - rememberedTime;
    if (isMorsePaused && difference >= MORSE_BREAK){
        rememberedTime = currentTime;
        isMorsePaused = false;
        difference = currentTime - rememberedTime;
    }
    if (!isMorsePaused) {
        unsigned int waitingTime = 100;
        switch (morseCodeLetters[morseCodeLetterIndex]) {
            case '0': tone(BUZZER, 1000); waitingTime = MORSE_DOT_TIME;   break;
            case '1': tone(BUZZER, 1000); waitingTime = MORSE_DASH_TIME;  break;
            case '2': noTone(BUZZER);     waitingTime = MORSE_LONG_BREAK; break;
            default: break;
        }
        if (difference >= waitingTime) {
            rememberedTime = currentTime;
            noTone(BUZZER);
            if (morseCodeLetterIndex < morseCodeLettersLength - 1) { morseCodeLetterIndex++; }
            else                                                   { morseCodeLetterIndex = 0; buzzerMode = Buzzer::SILENT; }
            isMorsePaused = true;
        }
    }
}
void playMelody() {
    unsigned int difference = currentTime - rememberedTime;
    if (!isMelodyPaused) { //kiedy 0 gra dzwiek
        tone(BUZZER, melodyTones[toneIndex]);
    }
    if (difference >= MELODY_KEY_TIME && !isMelodyPaused) { //gdy minie czas dzwieku wylacz buzzer
        noTone(BUZZER);
        rememberedTime = currentTime;
        isMelodyPaused = true;
    }
    else if (difference >= MELODY_KEY_BREAK && isMelodyPaused) { //gdy minie czas przerwy zagraj nastepny dzwiek
        if (toneIndex < MELODY_TONES_COUNT) {
            toneIndex++;
        }
        isMelodyPaused = false;
        rememberedTime = currentTime;
    }
    if(toneIndex == MELODY_TONES_COUNT){
        buzzerMode = Buzzer::KEYBOARD;
        toneIndex = 0;
    }
}
void playKeys() {
    unsigned int difference = currentTime - rememberedTime;
    if (difference >= MELODY_WAITING_TIME) { //po 5 sekundach braku aktywnosci wracam do morse'a
        buzzerMode = Buzzer::SILENT;
        keyIndex = 0;
    }
    char currentKey = keypad.getKey();
    if (currentKey != NO_KEY) { //po wcisnieciu klawisza
        rememberedTime = millis();
        if (currentKey >= 49 && currentKey <= 57) {
            tone(BUZZER, keyboardTones[currentKey - 49]); //gram dzwiek przypisany do klawisza
        }
        delay(200);
        isKeyPaused = true;
        if (currentKey != melodyKeys[keyIndex]) {//sprawdzenie poprawnosci
            isKeyCorrect = false;
        }
        keyIndex++;
        if (keyIndex == MELODY_TONES_COUNT) {
            if (isKeyCorrect) { //gdy 3 klawisze wcisniecie sprawdzenie czy nie bylo pomylki
                noTone(BUZZER);
                buzzerMode = Buzzer::SILENT;
                isMelodySuccessful = true;
            }
            else { //gdy zdarzyla sie pomylka mozna wpisac kod ponownie
                isKeyCorrect = true;
            }
            keyIndex = 0;
        }
    }
    difference = currentTime - rememberedTime;
    if (isKeyPaused && difference >= MELODY_KEY_TIME) { //gdy minie 200ms koncze grac dzwiek
        noTone(BUZZER);
        isKeyPaused = false;
    }
}


// CHECK
Status checkWires(){
    byte successfulCuts = 0;
    for (byte i = 0; i < WIRES_COUNT; i++) {
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
    if (isMelodySuccessful) {
        return Status::SUCCESS;
    }
    return Status::NEUTRAL;
}
Status checkLaser() {
    unsigned int laserAccuracy = analogRead(LASER_LDR);
    if (laserAccuracy >= LASER_TARGET_ACCURACY) { // @TODO skalibrować wartość LASER_TARGET_ACCURACY
        laserHold++;
        if (laserHold >= 63) { // 63 * 16ms > 1000ms
            return Status::SUCCESS;
        }
    }
    else {
        laserHold = 0;
    }
    return Status::NEUTRAL;
}
Status checkMaze() {
    if (mazeDebounceCounter > 1) {
        mazeDebounceCounter--;
        return Status::NEUTRAL;
    }
    else if (mazeDebounceCounter == 1) {
        mazeDebounceCounter--;
        goto MazeDebounce;
    }
    
    int xVal = analogRead(MAZE_JOY_X), yVal = analogRead(MAZE_JOY_Y);
    int dx = 0, dy = 0;

    if (xVal < 300) dx =  1;  // w prawo
    if (xVal > 700) dx = -1;  // w lewo
    if (yVal < 300) dy =  1;  // w dół
    if (yVal > 700) dy = -1;  // w górę

    int newX = mazePlayerX + dx;
    int newY = mazePlayerY + dy;

    if (newX != mazePlayerX || newY != mazePlayerY) { // has player position changed
        // sprawdź granice i ścianę
        if (newX >= 0 && newX < mazeWidth && newY >= 0 && newY < mazeHeight && maze[newY][newX] == 0) {
            mazePlayerX = newX;
            mazePlayerY = newY;
            mazeDebounceCounter = 13; // 13 * 16ms > 200ms
            return Status::NEUTRAL;
            MazeDebounce:
            ;
        }
        if (maze[newY][newX] == 2) {
            mazePlayerX = newX;
            mazePlayerY = newY;
            return Status::SUCCESS;
        }
        mazeDraw();
        mazeDrawPlayer();
        mazeDrawFinish();
    }
    
    return Status::NEUTRAL;
}
Status checkCircles() {
    // @TODO check if steering some wheel rotation

    unsigned int circlesAccuracy = analogRead(CIRCLES_LDR);
    if (circlesAccuracy >= CIRCLES_TARGET_ACCURACY) { // @TODO skalibrować wartość CIRCLES_TARGET_ACCURACY
        circlesHold++;
        if (circlesHold >= 63) { // 63 * 16ms > 1000ms
            return Status::SUCCESS;
        }
    }
    else {
        circlesHold = 0;
    }
    return Status::NEUTRAL;
}

void initBomb(){
    randomSeed(analogRead(A0)); // @NOTE pierwszy pin analogu jest zajęty tutaj, by generować liczby losowe
	
    gameStatus = Status::NEUTRAL;
    buzzerMode = Buzzer::SILENT;

    ID = "";
    morseCodeLetters       = "";
    morseCodeLettersLength = 0;
    morseCodeLetterIndex   = 0;
    isMorsePaused          = false;

    intervalActivationTime = 0;
    intervalExplosionTime  = 0;
    hasIntervalActivated = false;
    intervalLEDState = false;

    wiresCutCount = 0;

    isMelodyPaused = false;
    isMelodySuccessful = false;
    isKeyPaused = false;
    isKeyCorrect = true;
    toneIndex = 0;
    keyIndex = 0;

    laserRotation = 90;
    laserHold     = 0;
    isLaserDone  = false;
    laserLastCLKState = 0;
    laserCurrCLKState = 0;

    mazePlayerX = 0;
    mazePlayerY = 0;
    isMazeDone = false;
    mazeDebounceCounter = 0;

    circlesHold = 0;
    areCirclesDone = false;
    circlesRotationUp = 0; circlesRotationLeft = 0; circlesRotationRight = 0;
    circlesUpLastCLK = 0; circlesLeftLastCLK = 0; circlesRightLastCLK = 0;

    TIMER_SECONDS_LEFT = (TIME_TOTAL_MS / 1000) - 1;
    TIME_MS = 0;
    currentTime = rememberedTime;
    rememberedTime = millis();



    // timer
    Wire.begin();
    timerSetBrightness(7);

    pinMode(BOMB_RESET_BTN, INPUT_PULLUP);
    
    pinMode(BUZZER, OUTPUT);
    
    pinMode(MORSE_BTN, INPUT_PULLUP);
    
    pinMode(INTERVAL_LED, OUTPUT);
    pinMode(INTERVAL_BTN, INPUT_PULLUP);

    for (byte i = 0; i < WIRES_COUNT; i++) {
        pinMode(WIRES_BASE + i, INPUT_PULLUP);
    }

    pinMode(MELODY_BTN, INPUT_PULLUP);

    pinMode(LASER_STEER_CLK, INPUT);
    pinMode(LASER_STEER_DT, INPUT);
    laserServo.attach(LASER_SERVO);
    //pinMode(LASER, OUTPUT);
    laserLastCLKState = digitalRead(LASER_STEER_CLK);

    mazeTFT.initR(INITR_BLACKTAB); 
    mazeTFT.fillScreen(ST7735_BLACK);
    
    circlesServoUp.attach(CIRCLES_SERVO_UP);
    circlesServoLeft.attach(CIRCLES_SERVO_LEFT);
    circlesServoRight.attach(CIRCLES_SERVO_RIGHT);
    pinMode(CIRCLES_STEER_UP_CLK, INPUT);
    pinMode(CIRCLES_STEER_UP_DT, INPUT);
    //pinMode(CIRCLES_STEER_UP_SW, INPUT_PULLUP); // @TODO zmienić ten pullup?
    pinMode(CIRCLES_STEER_LEFT_CLK, INPUT);
    pinMode(CIRCLES_STEER_LEFT_DT, INPUT);
    //pinMode(CIRCLES_STEER_LEFT_SW, INPUT_PULLUP); // @TODO zmienić ten pullup?
    pinMode(CIRCLES_STEER_RIGHT_CLK, INPUT);
    pinMode(CIRCLES_STEER_RIGHT_DT, INPUT);
    pinMode(CIRCLES_STEER_RIGHT_SW, INPUT_PULLUP); // @TODO zmienić ten pullup?
    circlesUpLastCLK = digitalRead(CIRCLES_STEER_UP_CLK);
    circlesLeftLastCLK = digitalRead(CIRCLES_STEER_LEFT_CLK);
    circlesRightLastCLK = digitalRead(CIRCLES_STEER_RIGHT_CLK);

    strip.begin();
	strip.clear();
    strip.show();

    //digitalWrite(LASER,           HIGH);
    digitalWrite(INTERVAL_LED,    LOW);

    // generowanie rozgrywki
    ID = generateID();    
    generateMorseCode();
    generateWiresMask();
    generateMelodyTones();
    laserServo.write(0);
    circlesServoUp.write(0);
    circlesServoLeft.write(0);
    circlesServoRight.write(0);
	// @TODO obrocic o 120 stopni wszystkie

    Epd epd;
    epd.Init();
    epd.Clear();
    unsigned char image[1500];
    Paint paint(image, 400, 28);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(140, 5, "ID: " + ID[0] + ID[1] + ID[2], &Font24, COLORED); // wyświetlanie ID
    epd.Display_Partial(paint.GetImage(), 0, 130, 0 + paint.GetWidth(), 130 + paint.GetHeight());
    delay(1000);
    epd.Sleep();
}

void setup(){
    initBomb();
}

void loop() {
    // if (digitalRead(BOMB_RESET_BTN) == LOW) { delay(1000); initBomb(); }
    currentTime = millis();

    switch (gameStatus) {
        case Status::NEUTRAL: { // gra toczy się
            
            
            if (digitalRead(MELODY_BTN) == LOW) {
                rememberedTime = millis();
                buzzerMode = Buzzer::MELODY;
                noTone(BUZZER);
                toneIndex = 0;
                isMelodyPaused = false;
            }
            else if (digitalRead(MORSE_BTN) == LOW) {
                rememberedTime = millis();
                buzzerMode = Buzzer::MORSE;
                noTone(BUZZER);
                morseCodeLetterIndex = 0;
                isMorsePaused = false;
            }
            else {
                switch (buzzerMode) {
                    case Buzzer::MORSE:    playMorse();  break;
                    case Buzzer::MELODY:   playMelody(); break;
                    case Buzzer::KEYBOARD: playKeys();   break;
                    default: break;
                }
            }
            
            
            if (hasIntervalActivated) {
                if (digitalRead(INTERVAL_BTN) == LOW) { // jeśli przycisk wciśnięty, resetuj timery oraz diode
                    digitalWrite(INTERVAL_LED, LOW);
                    intervalActivationTime = 0;
                    intervalExplosionTime  = 0;
                }
                if (intervalActivationTime > INTERVAL_TOTAL_ACTIVATION_TIME){
                    if (intervalExplosionTime > INTERVAL_TOTAL_EXPLOSION_TIME) {
                        gameStatus = Status::FAILURE;
                        break;
                    }
                    else {
                        intervalExplosionTime += 1;
                    }
                }
                else {
                    intervalActivationTime += 1;
                }
            }


            if (TIME_MS % 1000 == 0) { // wykonuje tu co każdą sekundę
                TIMER_SECONDS_LEFT--;
                if(TIMER_SECONDS_LEFT >= 0){
                    int minutesDigit1 = TIMER_SECONDS_LEFT / 600;
                    int minutesDigit2 = (TIMER_SECONDS_LEFT / 60) % 10;
                    int secondsDigit1 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) / 10;
                    int secondsDigit2 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) % 10;
                    timerShowDigits(minutesDigit1, minutesDigit2, secondsDigit1, secondsDigit2);
                }
            }
            
            if (TIME_MS % 100 == 0) { // wykonuje tu co każde 100ms
                if (hasIntervalActivated && intervalActivationTime > INTERVAL_TOTAL_ACTIVATION_TIME) {
                    intervalLEDState = !intervalLEDState;
                    digitalWrite(INTERVAL_LED, intervalLEDState);
                }
            }

            if (TIME_MS % 16 == 0) { // wykonuje tu co każde 16ms
                Status wiresStatus = checkWires();
                switch (wiresStatus) {
                    case Status::FAILURE:
                        gameStatus = Status::FAILURE; // bomba wybucha
                        break;
                    default: break;
                }
               

                Status melodyStatus = checkMelody();
                
                
                laserCurrCLKState = digitalRead(LASER_STEER_CLK);
                if (laserCurrCLKState != laserLastCLKState) {
                    if (digitalRead(LASER_STEER_DT) != laserCurrCLKState) {
                        laserRotation += 2;
                    }
                    else {
                        laserRotation -= 2;
                    }
                    if (laserRotation > 180) { laserRotation = 180; }
                    if (laserRotation < 0)   { laserRotation = 0;   }
                    laserServo.write(laserRotation);
                }
                laserLastCLKState = laserCurrCLKState;
                Status laserStatus = Status::SUCCESS;
                if (!isLaserDone) {
                    laserStatus = checkLaser();
                    switch (laserStatus) {
                        case Status::SUCCESS:
                            isLaserDone = true;
                            break;
                        default:
                            break;
                    }
                }
                
                
                Status mazeStatus = Status::SUCCESS;
                if (!isMazeDone) {
                    mazeStatus = checkMaze();
                    switch (mazeStatus) {
                        case Status::SUCCESS:
                            isMazeDone = true;
                            mazeDrawEnd();
                            break;
                        default: break;
                    }
                }
                
                
                Status circlesStatus = Status::SUCCESS;
                if (!areCirclesDone) {
                    circlesStatus = checkCircles();
                    switch (circlesStatus) {
                        case Status::SUCCESS:
                            areCirclesDone = true;
                            break;
                        default:
                            break;
                    }
                }
                
                
                if (!hasIntervalActivated) {
                    if (wiresStatus   == Status::SUCCESS || 
                        melodyStatus  == Status::SUCCESS ||
                        laserStatus   == Status::SUCCESS ||
                        mazeStatus    == Status::SUCCESS ||
                        circlesStatus == Status::SUCCESS) {
                        hasIntervalActivated = true;
                    }
                }
                
                strip.clear();
                if (wiresStatus   == Status::SUCCESS) { strip.setPixelColor(0, strip.Color(0, 255, 0)); }
                if (melodyStatus  == Status::SUCCESS) { strip.setPixelColor(1, strip.Color(0, 255, 0)); }
                if (laserStatus   == Status::SUCCESS) { strip.setPixelColor(2, strip.Color(0, 255, 0)); }
                if (mazeStatus    == Status::SUCCESS) { strip.setPixelColor(3, strip.Color(0, 255, 0)); }
                if (circlesStatus == Status::SUCCESS) { strip.setPixelColor(4, strip.Color(0, 255, 0)); }
                strip.show();

                if (wiresStatus   == Status::SUCCESS && 
                    melodyStatus  == Status::SUCCESS &&
                    laserStatus   == Status::SUCCESS &&
                    mazeStatus    == Status::SUCCESS &&
                    circlesStatus == Status::SUCCESS) {
                    gameStatus = Status::SUCCESS; // wygrano grę
                    break;
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
            Epd epd;
            epd.Init();
            epd.Clear();
            unsigned char image[1500];
            Paint paint(image, 400, 28);
            paint.Clear(UNCOLORED);
            paint.DrawStringAt(50, 5, "Bomba rozbrojona", &Font24, COLORED);
            epd.Display_Partial(paint.GetImage(), 0, 130, 0 + paint.GetWidth(), 130 + paint.GetHeight());
            delay(1000);
            epd.Sleep();
            //digitalWrite(LASER, LOW);
            break;
        }
        case Status::FAILURE: { // bomba wybuchła
            /* @TODO buzzer wydaje głosy wybuchu, timer gasi się */
            Epd epd;
            epd.Init();
            epd.Display(epd_bitmap_boom1bit);
            delay(1000);
            epd.Sleep();
            
            while(1) { delay(1000); }
            //digitalWrite(LASER, LOW);
            break;
        }
        default: break;
    }
}








