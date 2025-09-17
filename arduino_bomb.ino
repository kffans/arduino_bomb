//#define MODULE_TIMER
#define MODULE_INTERVAL
#define MODULE_LED_STRIP
#define MODULE_WIRES
#define MODULE_MELODY
//#define MODULE_LASER
//#define MODULE_MAZE
//#define MODULE_CIRCLES
#define MODULE_EPAPER





    #include <Servo.h>             // serwa
    #include <Wire.h>              // timer
#ifdef MODULE_MELODY
    #include <Keypad.h>            // Keypad by Mark Stanley v3.1.1; melody
#endif
#ifdef MODULE_LED_STRIP
    #include <Adafruit_NeoPixel.h> // pasek ledów
#endif
#ifdef MODULE_MAZE
    #include <Adafruit_ST7735.h>   // maze
    #include <Adafruit_GFX.h>      // maze
    #include <SPI.h>               // maze
#endif
    #include "pitches.h"           // buzzer
#ifdef MODULE_EPAPER
    #include "boom1bit.h"          // obraz wybuchu dla e-papieru
    #include "epd4in2_V2.h"        // e-papier
    #include "imagedata.h"         // e-papier
    #include "epdpaint.h"          // e-papier
#endif



#ifdef MODULE_TIMER
    #define TIMER_ADDR          0x30 
    #define TIMER_DISPLAY       0x00
    #define TIMER_BRIGHTNESS    0x06
#endif



#ifdef MODULE_LED_STRIP
    #define SUCCESS_LED_STRIP_PIN   50      
    #define SUCCESS_LED_STRIP_COUNT 8
#endif
//
    #define BUZZER              12  /* do morsa, przy wybuchu */
//
    #define MORSE_BTN           13
// 
#ifdef MODULE_INTERVAL
    #define INTERVAL_LED        14  /* żółty led który z czasem miga coraz szybciej */
    #define INTERVAL_BTN        42  /* przycisk do naciskania co każde 5s */
#endif
// 
#ifdef MODULE_WIRES
    #define WIRES_BASE          37   /* @NOTE to kabelek bazowy (pierwszy); kabelki muszą być podłączone obok siebie w rosnącej kolejności (jeżeli pierwszy kabelek jest na pinie 20, drugi musi być na 21, itd.) */
#endif
//
#ifdef MODULE_MELODY 
    #define MELODY_KEYBOARD     28   /* @NOTE rezerwuje 28, 29, 30, 31, 32, 33 */
    #define MELODY_BTN          11
#endif
// 
#ifdef MODULE_LASER
    #define LASER_LDR           A5  /* fotorezystor do wykrywania lasera*/
    #define LASER_SERVO         10
    //#define LASER               17
    #define LASER_STEER_CLK     9
    #define LASER_STEER_DT      8
#endif
// 
#ifdef MODULE_MAZE
    #define MAZE_TFT_CS         6
    #define MAZE_TFT_RST        5
    #define MAZE_TFT_DC         4
    #define MAZE_JOY_X          A2
    #define MAZE_JOY_Y          A3
#endif
//
#ifdef MODULE_CIRCLES
    #define CIRCLES_LDR             A6  /* fotorezystor do wykrywania swiatla */
	#define CIRCLES_SERVO_UP        43  // Servo1 (program 1)
	#define CIRCLES_STEER_UP_CLK    24  // Enkoder1A (program 1)
	#define CIRCLES_STEER_UP_DT     26  // Enkoder1B (program 1)
	#define CIRCLES_SERVO_LEFT      45     // (program 2)
	#define CIRCLES_STEER_LEFT_CLK  2   // (program 2)
	#define CIRCLES_STEER_LEFT_DT   3    // (program 2)
	#define CIRCLES_SERVO_RIGHT     44
	#define CIRCLES_STEER_RIGHT_CLK 22   // kanał A (CLK) (program 3)
	#define CIRCLES_STEER_RIGHT_DT  23    // kanał B (DT) (program 3)
	#define CIRCLES_STEER_RIGHT_SW  25
#endif
//
// @NOTE piny do e-papieru są zdefinioweane w pliku epdif.h, jest ich cztery
#ifdef MODULE_EPAPER
    #define COLORED     0
    #define UNCOLORED   1
#endif

enum class Status {NONE, NEUTRAL, SUCCESS, FAILURE};
enum class Buzzer {SILENT, MORSE, MELODY, KEYBOARD};



// GLOBAL VARS
// @TODO zmienne globalne tutaj
#ifdef MODULE_LED_STRIP
Adafruit_NeoPixel strip(SUCCESS_LED_STRIP_COUNT, SUCCESS_LED_STRIP_PIN, NEO_GRB + NEO_KHZ800); // LED strip
#endif

int resetTimer = 0;
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

#ifdef MODULE_INTERVAL
const unsigned int INTERVAL_TOTAL_ACTIVATION_TIME = 6000;
const unsigned int INTERVAL_TOTAL_EXPLOSION_TIME  = 6000;
unsigned int intervalActivationTime = 0;
unsigned int intervalExplosionTime  = 0;
bool hasIntervalActivated = false;
bool intervalLEDState = false;
#endif

#ifdef MODULE_WIRES
const byte WIRES_COUNT = 5;
bool wiresMask[WIRES_COUNT]; // tam gdzie jest "true", ma być przecięty kabel
byte wiresCutCount = 0;
#endif

#ifdef MODULE_MELODY
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
#endif

#ifdef MODULE_LASER
const unsigned int LASER_TARGET_ACCURACY = 680;
Servo laserServo;
int laserRotation = 90;
int laserHold     = 0;
bool isLaserDone  = false;
int laserLastCLKState = 0;
int laserCurrCLKState = 0;
#endif

#ifdef MODULE_MAZE
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
#endif

#ifdef MODULE_CIRCLES
const unsigned int CIRCLES_TARGET_ACCURACY = 100;
int circlesHold = 0;
bool areCirclesDone = false;
int circlesProg3DebounceCounter = 0;
// CIRCLES PROG 1
Servo circlesServoUp;
const int circlesServoUpStop = 90;     // zatrzymanie (program 1)
const int circlesServoUpSteerRight = 100;   // ruch w prawo (program 1)
const int circlesServoUpSteerLeft  = 80;    // ruch w lewo (program 1)
const int circlesUpStepDegree = 3;        // krok obrotu (program 1)
const float circlesUpDegreesPerMs = 0.1f;  // ile stopni na 1 ms (program 1)
const int circlesUpMoveTime = (int)(circlesUpStepDegree / circlesUpDegreesPerMs);  // czas ruchu dla kroku (program 1)
int circlesUpLastCLK;
// CIRCLES PROG 2
Servo circlesServoLeft;
volatile int circlesLeftEncoderPos = 0;
int circlesLeftLastEncoded = 0;
int circlesServoLeftRotation = 90; // startowa pozycja serwa (program 2)
unsigned long circlesLeftLastActivityTime = 0;   // czas ostatniego ruchu enkodera (program 2)
// CIRCLES PROG 3
Servo circlesServoRight;
bool circlesIsServoRightRunning = false;
unsigned long circlesRightStartTime = 0;
int circlesRightDirection = 1; // 1 = prawo, -1 = lewo (program 3)
int circlesRightLastCLK = HIGH;
#endif

const unsigned long TIME_TOTAL_MS = 120000; // 2 minuty = 120000ms
const unsigned long TIME_DELAY_DURATION_MS = 1;
unsigned long TIME_MS = 0;
unsigned long currentTime = 0;
unsigned long rememberedTime = millis();

#ifdef MODULE_TIMER
int TIMER_SECONDS_LEFT = (TIME_TOTAL_MS / 1000);
#endif


#ifdef MODULE_TIMER
// TIMER
void timerShowDigits(int d1, int d2, int d3, int d4) {
    Wire.beginTransmission(TIMER_ADDR);
    Wire.write(TIMER_DISPLAY);
    Wire.write(digitTo7Seg(10)); Wire.write(digitTo7Seg(d2)); Wire.write(digitTo7Seg(1)); Wire.write(digitTo7Seg(d3)); Wire.write(digitTo7Seg(d4));
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
#endif


#ifdef MODULE_MAZE
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
#endif


#ifdef MODULE_CIRCLES
// CIRCLES
void circlesProg1MoveServo(int direction) 
{
  circlesServoUp.write(direction);
  delay(circlesUpMoveTime);
  circlesServoUp.write(circlesServoUpStop);
}
void circlesProg2UpdateEncoder() {
  int MSB = digitalRead(CIRCLES_STEER_LEFT_CLK);
  int LSB = digitalRead(CIRCLES_STEER_LEFT_DT);

  int encoded = (MSB << 1) | LSB;
  int sum = (circlesLeftLastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) circlesLeftEncoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) circlesLeftEncoderPos--;

  circlesLeftLastEncoded = encoded;
}
#endif

// GENERATE
String generateID(){ // examples of ID: A23C1, H5833, J11GU. Two last characters are from morse code, 1st char is always a letter, 2nd and 3rd are always digits
    String letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    String digits = "0123456789";
    unsigned int lettersLength = 26;
    unsigned int digitsLength = 10;
    ID = "";
    ID.concat(letters.charAt(random(lettersLength)));
    ID.concat(digits.charAt(random(digitsLength)));
    ID.concat(digits.charAt(random(digitsLength)));
    for (byte i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
        int putDigit = random(2); // losuje 0 albo 1
        if (putDigit) { ID.concat(digits.charAt(random(digitsLength)));   }
        else          { ID.concat(letters.charAt(random(lettersLength))); }
    }
	return ID;
}
void generateMorseCode() { // converts last two characters from ID to dots and dashes ("0" is dot, "1" is dash, "2" is space/end); example: "AB" -> "._ _... " -> "01210002"
    if (ID.length() != ID_CHAR_COUNT ) { return; }

    for (byte i = 0; i < ID_MORSE_CODE_CHAR_COUNT; i++) {
        switch (ID.charAt(ID_CHAR_COUNT - ID_MORSE_CODE_CHAR_COUNT + i)) {
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
#ifdef MODULE_WIRES
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
        if (isDigit(ID.charAt(i))) {
            byte digit = ID.charAt(i) - 48;
            digitSum += digit;
            digitCount++;
            if (digit % 2 == 0) {
                evenDigitCount++;
            }
        }
        else {
            letterCount++;
            if (ID.charAt(i) == 'A' || ID.charAt(i) == 'I' || ID.charAt(i) == 'E' || ID.charAt(i) == 'O' || ID.charAt(i) == 'U' || ID.charAt(i) == 'Y') {
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
    if ( (!isDigit(ID.charAt(3)) && abs(ID.charAt(0) - ID.charAt(3)) == 1) || (!isDigit(ID.charAt(4)) && abs(ID.charAt(0) - ID.charAt(4)) == 1) || (!isDigit(ID.charAt(3)) && !isDigit(ID.charAt(4)) && abs(ID.charAt(3) - ID.charAt(4)) == 1) ) { wiresMask[3] = true; }

    if ( (isDigit(ID.charAt(3)) && isDigit(ID.charAt(4))) || (!isDigit(ID.charAt(3)) && !isDigit(ID.charAt(4))) ) { wiresMask[4] = false; }

    bool canCutSomething = false;
    for (int i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            canCutSomething = true;
            break;
        }
    }
    if (!canCutSomething) { wiresMask[0] = true; wiresMask[1] = true; wiresMask[3] = true; wiresMask[4] = true; }


    if ( (isDigit(ID.charAt(3)) && isDigit(ID.charAt(4))) || (!isDigit(ID.charAt(3)) && !isDigit(ID.charAt(4))) ) { wiresMask[4] = false; }


    for (byte i = 0; i < WIRES_COUNT; i++) {
        if (wiresMask[i] == true) {
            wiresCutCount++;
        }
    }
}
#endif
#ifdef MODULE_MELODY
void generateMelodyTones(){
    unsigned int keyboardTonesLength = MELODY_KEYPAD_ROWS * MELODY_KEYPAD_COLS;
    for (byte i = 0; i < MELODY_TONES_COUNT; i++){
        unsigned int index = random(keyboardTonesLength);
        melodyKeys[i] = index + 48;
        melodyTones[i] = keyboardTones[index];
    }
}
#endif



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
#ifdef MODULE_MELODY
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
#endif


// CHECK
#ifdef MODULE_WIRES
Status checkWires(){
    byte successfulCuts = 0;
    for (byte i = 0; i < WIRES_COUNT; i++) {
        if (digitalRead(WIRES_BASE + i) == HIGH) { // jeżeli kabel jest wyjęty
            if (wiresMask[i] == true) { successfulCuts++; }
            else                      { return Status::FAILURE; }
        }
    }
    if (successfulCuts == wiresCutCount) {
        return Status::SUCCESS;
    }
    return Status::NEUTRAL;
}
#endif
#ifdef MODULE_MELODY
Status checkMelody() {
    if (isMelodySuccessful) {
        return Status::SUCCESS;
    }
    return Status::NEUTRAL;
}
#endif
#ifdef MODULE_LASER
Status checkLaser() {
    unsigned int laserAccuracy = analogRead(LASER_LDR);
    if (laserAccuracy >= LASER_TARGET_ACCURACY) {
        laserHold++;
        Serial.println(laserHold);
        if (laserHold >= 300) { // 63 * 16ms > 1000ms
            return Status::SUCCESS;
        }
    }
    else {
        laserHold--;
        if (laserHold < 0) {
            laserHold = 0;
        }
    }
    return Status::NEUTRAL;
}
#endif
#ifdef MODULE_MAZE
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
#endif
#ifdef MODULE_CIRCLES
Status checkCircles() {
	if (!areCirclesDone) {
		// CIRCLES PROG 1
		int currentCLK_1 = digitalRead(CIRCLES_STEER_UP_CLK);
		if (currentCLK_1 != circlesUpLastCLK && currentCLK_1 == HIGH) {    // detekcja zbocza narastającego
			if (digitalRead(CIRCLES_STEER_UP_DT) != currentCLK_1) {
				circlesProg1MoveServo(circlesServoUpSteerLeft);       // obrót w lewo
			} 
			else {
				circlesProg1MoveServo(circlesServoUpSteerRight);        // obrót w prawo
			}
		}
		circlesUpLastCLK = currentCLK_1;

		// CIRCLES PROG 2
		static int lastPos_2 = 0;
		noInterrupts();
		int pos_2 = circlesLeftEncoderPos;
		interrupts();
		if (pos_2 != lastPos_2) {
			int diff_2 = pos_2 - lastPos_2;
			lastPos_2 = pos_2;
			circlesServoLeftRotation += diff_2 * 4;  // krok 4 jednostki
			circlesServoLeftRotation = constrain(circlesServoLeftRotation, 0, 180);
			circlesServoLeft.write(circlesServoLeftRotation);
			circlesLeftLastActivityTime = millis();
		}

		// CIRCLES PROG 3
		int currentCLK_3 = digitalRead(CIRCLES_STEER_RIGHT_CLK);
		if (currentCLK_3 != circlesRightLastCLK && currentCLK_3 == LOW) {
			if (digitalRead(CIRCLES_STEER_RIGHT_DT) != currentCLK_3) {
				circlesRightDirection = 1;   // w prawo
			} else {
				circlesRightDirection = -1;  // w lewo
			}
		}
		if (circlesProg3DebounceCounter > 1) {
			circlesProg3DebounceCounter--;
			goto CirclesPhotoresistor;
		}
		else if (circlesProg3DebounceCounter == 1) {
			circlesProg3DebounceCounter--;
			goto CirclesProg3Debounce;
		}
		circlesRightLastCLK = currentCLK_3;
		if (digitalRead(CIRCLES_STEER_RIGHT_SW) == LOW && !circlesIsServoRightRunning) {
			circlesIsServoRightRunning = true;
			circlesRightStartTime = millis();
			circlesProg3DebounceCounter = 200;
			goto CirclesPhotoresistor;
			CirclesProg3Debounce:
			;
		}
		if (circlesIsServoRightRunning) {
			unsigned long elapsed_3 = millis() - circlesRightStartTime;
			if (circlesRightDirection == 1) {
				circlesServoRight.writeMicroseconds(1700); // prawo
			} 
			else {
				circlesServoRight.writeMicroseconds(1300); // lewo
			}
			if (elapsed_3 >= 4000) {
				circlesServoRight.writeMicroseconds(1500); // stop
				circlesIsServoRightRunning = false;
			}
		}
	}
	CirclesPhotoresistor:
	;

    unsigned int circlesAccuracy = analogRead(CIRCLES_LDR);
    if (circlesAccuracy >= CIRCLES_TARGET_ACCURACY) { // @TODO skalibrować wartość CIRCLES_TARGET_ACCURACY
        circlesHold++;
        if (circlesHold >= 500) { // 63 * 16ms > 1000ms
            return Status::SUCCESS;
        }
    }
    else {
        circlesHold--;
		if (circlesHold < 0) {
			circlesHold = 0;
		}
    }
    return Status::NEUTRAL;
}
#endif

void initBomb(){
    randomSeed(analogRead(A0)); // @NOTE pierwszy pin analogu jest zajęty tutaj, by generować liczby losowe
	
    Serial.begin(9600);
    resetTimer = 0;
    gameStatus = Status::NEUTRAL;
    buzzerMode = Buzzer::SILENT;

    ID = "";
    morseCodeLetters       = "";
    morseCodeLettersLength = 0;
    morseCodeLetterIndex   = 0;
    isMorsePaused          = false;

    #ifdef MODULE_INTERVAL
    intervalActivationTime = 0;
    intervalExplosionTime  = 0;
    hasIntervalActivated = false;
    intervalLEDState = false;
    #endif

    #ifdef MODULE_WIRES
    wiresCutCount = 0;
    #endif

    #ifdef MODULE_MELODY
    isMelodyPaused = false;
    isMelodySuccessful = false;
    isKeyPaused = false;
    isKeyCorrect = true;
    toneIndex = 0;
    keyIndex = 0;
    #endif

    #ifdef MODULE_LASER
    laserRotation = 90;
    laserHold     = 0;
    isLaserDone  = false;
    laserLastCLKState = 0;
    laserCurrCLKState = 0;
    #endif

    #ifdef MODULE_MAZE
    mazePlayerX = 0;
    mazePlayerY = 0;
    isMazeDone = false;
    mazeDebounceCounter = 0;
    #endif

    #ifdef MODULE_CIRCLES
    circlesHold = 0;
    areCirclesDone = false;
    circlesProg3DebounceCounter = 0;
    // CIRCLES PROG 1
    int circlesUpLastCLK;
    // CIRCLES PROG 2
    circlesLeftEncoderPos = 0;
    circlesLeftLastEncoded = 0;
    circlesServoLeftRotation = 90; // startowa pozycja serwa (program 2)
    circlesLeftLastActivityTime = 0;   // czas ostatniego ruchu enkodera (program 2)
    // CIRCLES PROG 3
    circlesIsServoRightRunning = false;
    circlesRightStartTime = 0;
    circlesRightDirection = 1; // 1 = prawo, -1 = lewo (program 3)
    circlesRightLastCLK = HIGH;
    #endif

    #ifdef MODULE_TIMER
    TIMER_SECONDS_LEFT = (TIME_TOTAL_MS / 1000);
    #endif
    TIME_MS = 0;
    currentTime = rememberedTime;
    rememberedTime = millis();


    #ifdef MODULE_TIMER
    // timer
    Wire.begin();
    timerSetBrightness(2);
    int minutesDigit1 = TIMER_SECONDS_LEFT / 600;
    int minutesDigit2 = (TIMER_SECONDS_LEFT / 60) % 10;
    int secondsDigit1 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) / 10;
    int secondsDigit2 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) % 10;
    timerShowDigits(minutesDigit1, minutesDigit2, secondsDigit1, secondsDigit2);
    #endif
    
    pinMode(BUZZER, OUTPUT);
    
    pinMode(MORSE_BTN, INPUT_PULLUP);
    
    #ifdef MODULE_INTERVAL
    pinMode(INTERVAL_LED, OUTPUT);
    pinMode(INTERVAL_BTN, INPUT_PULLUP);
    digitalWrite(INTERVAL_LED, LOW);
    #endif

    #ifdef MODULE_WIRES
    for (byte i = 0; i < WIRES_COUNT; i++) {
        pinMode(WIRES_BASE + i, INPUT_PULLUP);
    }
    #endif

    #ifdef MODULE_MELODY
    pinMode(MELODY_BTN, INPUT_PULLUP);
    #endif

    #ifdef MODULE_LASER
    pinMode(LASER_STEER_CLK, INPUT);
    pinMode(LASER_STEER_DT, INPUT);
    laserServo.attach(LASER_SERVO);
    //pinMode(LASER, OUTPUT);
    laserLastCLKState = digitalRead(LASER_STEER_CLK);
    #endif

    #ifdef MODULE_MAZE
    mazeTFT.initR(INITR_BLACKTAB); 
    mazeTFT.fillScreen(ST7735_BLACK);
    #endif
    
    #ifdef MODULE_CIRCLES
    // CIRCLES PROG 1
	pinMode(CIRCLES_STEER_UP_CLK, INPUT);
	pinMode(CIRCLES_STEER_UP_DT, INPUT);
	circlesServoUp.attach(CIRCLES_SERVO_UP);
	//circlesServoUp.write(circlesServoUpStop);
    circlesServoUp.writeMicroseconds(1300);
	circlesUpLastCLK = digitalRead(CIRCLES_STEER_UP_CLK);
	// CIRCLES PROG 2
	pinMode(CIRCLES_STEER_LEFT_CLK, INPUT_PULLUP);
	pinMode(CIRCLES_STEER_LEFT_DT, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(CIRCLES_STEER_LEFT_CLK), circlesProg2UpdateEncoder, CHANGE);
	attachInterrupt(digitalPinToInterrupt(CIRCLES_STEER_LEFT_DT), circlesProg2UpdateEncoder, CHANGE);
	circlesServoLeft.attach(CIRCLES_SERVO_LEFT);
	//circlesServoLeft.write(circlesServoLeftRotation);
    circlesServoLeft.writeMicroseconds(1400);
	circlesLeftLastActivityTime = millis();
	// CIRCLES PROG 3
	circlesServoRight.attach(CIRCLES_SERVO_RIGHT);
	pinMode(CIRCLES_STEER_RIGHT_CLK, INPUT);
	pinMode(CIRCLES_STEER_RIGHT_DT, INPUT);
	pinMode(CIRCLES_STEER_RIGHT_SW, INPUT_PULLUP);
	circlesServoRight.writeMicroseconds(1600); // stop na starcie
	circlesRightLastCLK = digitalRead(CIRCLES_STEER_RIGHT_CLK);
    #endif

    #ifdef MODULE_LED_STRIP
    strip.begin();
	//strip.clear();
    strip.show();
    #endif

    //digitalWrite(LASER,           HIGH);

    // generowanie rozgrywki
    ID = generateID();    
    generateMorseCode();
    #ifdef MODULE_WIRES
    generateWiresMask();
    #endif
    #ifdef MODULE_MELODY
    generateMelodyTones();
    #endif
    #ifdef MODULE_LASER
    laserServo.write(laserRotation);
    #endif
    #ifdef MODULE_CIRCLES
    //circlesServoUp.write(0);
    //circlesServoLeft.write(0);
    //circlesServoRight.write(0);
	// @TODO obrocic o 120 stopni wszystkie
    #endif

    #ifdef MODULE_EPAPER
    Epd epd;
    epd.Init();
    epd.Clear();
    unsigned char image[1500];
    Paint paint(image, 400, 28);
    paint.Clear(UNCOLORED);
	String partialID = "ID:" + ID.substring(0, 3);
    paint.DrawStringAt(140, 5, partialID.c_str(), &Font24, COLORED); // wyświetlanie ID   + ID[0] + ID[1] + ID[2]
    epd.Display_Partial(paint.GetImage(), 0, 130, 0 + paint.GetWidth(), 130 + paint.GetHeight());
    delay(1000);
    epd.Sleep();
    #endif

    #ifdef MODULE_CIRCLES
    delay(1000);
    circlesServoUp.writeMicroseconds(1500);
    circlesServoLeft.writeMicroseconds(1500);
    circlesServoRight.writeMicroseconds(1500);
    #endif
    delay(2000);
}

void setup(){
    initBomb();
}

void loop() {
    #ifdef MODULE_MELODY
    if (digitalRead(MELODY_BTN) == LOW && digitalRead(MORSE_BTN) == LOW) {
        resetTimer++;
        if (resetTimer >= 2000) {
            delay(2000); 
            initBomb();            
        }
    }
    else {
        resetTimer--;
        if (resetTimer < 0) {
            resetTimer = 0;
        }
    }
    #endif
    
    currentTime = millis();

    switch (gameStatus) {
        case Status::NEUTRAL: { // gra toczy się
            
            #ifdef MODULE_MELODY
            if (digitalRead(MELODY_BTN) == LOW) {
                rememberedTime = millis();
                buzzerMode = Buzzer::MELODY;
                noTone(BUZZER);
                toneIndex = 0;
                isMelodyPaused = false;
            }
            else
            #endif
            if (digitalRead(MORSE_BTN) == LOW) {
                rememberedTime = millis();
                buzzerMode = Buzzer::MORSE;
                noTone(BUZZER);
                morseCodeLetterIndex = 0;
                isMorsePaused = false;
            }
            else {
                switch (buzzerMode) {
                    case Buzzer::MORSE:    playMorse();  break;
                    #ifdef MODULE_MELODY
                    case Buzzer::MELODY:   playMelody(); break;
                    case Buzzer::KEYBOARD: playKeys();   break;
                    #endif
                    default: break;
                }
            }
            
            #ifdef MODULE_INTERVAL
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
            #endif


			#ifdef MODULE_WIRES
            Status wiresStatus;
			#endif
			#ifdef MODULE_MELODY
			Status melodyStatus;
			#endif
			#ifdef MODULE_LASER
			Status laserStatus;
			#endif
			#ifdef MODULE_MAZE
			Status mazeStatus;
			#endif
			#ifdef MODULE_CIRCLES
			Status circlesStatus;
			#endif
			
			
			
			
			#ifdef MODULE_LASER
			laserCurrCLKState = digitalRead(LASER_STEER_CLK);
			if (laserCurrCLKState != laserLastCLKState) {
				if (digitalRead(LASER_STEER_DT) != laserCurrCLKState) {
					laserRotation += 1;
				}
				else {
					laserRotation -= 1;
				}
				if (laserRotation > 180) { laserRotation = 180; }
				if (laserRotation < 0)   { laserRotation = 0;   }
				laserServo.write(laserRotation);
			}
			laserLastCLKState = laserCurrCLKState;
			laserStatus = Status::SUCCESS;
			if (!isLaserDone) {
				laserStatus = checkLaser();
				switch (laserStatus) {
					case Status::SUCCESS:
						isLaserDone = true;
                        laserRotation = 40;
                        laserServo.write(laserRotation);
						break;
					default:
						break;
				}
			}
			#endif
			
			#ifdef MODULE_MAZE
			mazeStatus = Status::SUCCESS;
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
			#endif
			
			#ifdef MODULE_CIRCLES
			circlesStatus = Status::SUCCESS;
			if (!areCirclesDone) {
				circlesStatus = checkCircles();
				switch (circlesStatus) {
					case Status::SUCCESS:
						areCirclesDone = true;
						circlesServoUp.writeMicroseconds(1500);
						circlesServoLeft.writeMicroseconds(1500);
						circlesServoRight.writeMicroseconds(1500);
						break;
					default:
						break;
				}
			}
			#endif
			
			
			
			

            if (TIME_MS % 1000 == 0) { // wykonuje tu co każdą sekundę
                #ifdef MODULE_TIMER
                TIMER_SECONDS_LEFT--;
                if(TIMER_SECONDS_LEFT >= 0){
                    int minutesDigit1 = TIMER_SECONDS_LEFT / 600;
                    int minutesDigit2 = (TIMER_SECONDS_LEFT / 60) % 10;
                    int secondsDigit1 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) / 10;
                    int secondsDigit2 = (TIMER_SECONDS_LEFT - ((minutesDigit1 * 10 + minutesDigit2) * 60)) % 10;
                    timerShowDigits(minutesDigit1, minutesDigit2, secondsDigit1, secondsDigit2);
                }
                #endif
				
				#ifdef MODULE_INTERVAL
                if (!hasIntervalActivated) {
                    if (false ||
                        #ifdef MODULE_WIRES
                        wiresStatus   == Status::SUCCESS || 
                        #endif
                        #ifdef MODULE_MELODY
                        melodyStatus  == Status::SUCCESS ||
                        #endif
                        #ifdef MODULE_LASER
                        laserStatus   == Status::SUCCESS ||
                        #endif
                        #ifdef MODULE_MAZE
                        mazeStatus    == Status::SUCCESS ||
                        #endif
                        #ifdef MODULE_CIRCLES
                        circlesStatus == Status::SUCCESS ||
                        #endif
                        false) {
                        hasIntervalActivated = true;
                    }
                }
                #endif
            }
            
            if (TIME_MS % 100 == 0) { // wykonuje tu co każde 100ms
                #ifdef MODULE_INTERVAL
                if (hasIntervalActivated && intervalActivationTime > INTERVAL_TOTAL_ACTIVATION_TIME) {
                    intervalLEDState = !intervalLEDState;
                    digitalWrite(INTERVAL_LED, intervalLEDState);
                }
                #endif
				
				#ifdef MODULE_WIRES
                wiresStatus = checkWires();
                switch (wiresStatus) {
                    case Status::FAILURE:
                        gameStatus = Status::FAILURE; // bomba wybucha
                        break;
                    default: break;
                }
                #endif
				
				#ifdef MODULE_MELODY
                melodyStatus = checkMelody();
                #endif
				
				#ifdef MODULE_LED_STRIP
                    strip.clear();
                    strip.setBrightness(50);
                    #ifdef MODULE_WIRES
                    if (wiresStatus   == Status::SUCCESS) { strip.setPixelColor(3, strip.Color(0, 255, 0)); }
                    #endif
                    #ifdef MODULE_MELODY
                    if (melodyStatus  == Status::SUCCESS) { strip.setPixelColor(4, strip.Color(0, 255, 0)); }
                    #endif
                    #ifdef MODULE_LASER
                    if (laserStatus   == Status::SUCCESS) { strip.setPixelColor(5, strip.Color(0, 255, 0)); }
                    #endif
                    #ifdef MODULE_MAZE
                    if (mazeStatus    == Status::SUCCESS) { strip.setPixelColor(6, strip.Color(0, 255, 0)); }
                    #endif
                    #ifdef MODULE_CIRCLES
                    if (circlesStatus == Status::SUCCESS) { strip.setPixelColor(7, strip.Color(0, 255, 0)); }
                    #endif
                    strip.show();
                #endif

                if (true &&
                    #ifdef MODULE_WIRES
                    wiresStatus   == Status::SUCCESS && 
                    #endif
                    #ifdef MODULE_MELODY
                    melodyStatus  == Status::SUCCESS &&
                    #endif
                    #ifdef MODULE_LASER
                    laserStatus   == Status::SUCCESS &&
                    #endif
                    #ifdef MODULE_MAZE
                    mazeStatus    == Status::SUCCESS &&
                    #endif
                    #ifdef MODULE_CIRCLES
                    circlesStatus == Status::SUCCESS &&
                    #endif
                    true) {
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
            #ifdef MODULE_EPAPER
            Epd epd;
            epd.Init();
            epd.Clear();
            unsigned char image[1500];
            Paint paint(image, 400, 28);
            paint.Clear(UNCOLORED);
            paint.DrawStringAt(50, 5, "Bomba rozbrojona"/*partialID.c_str()*/, &Font24, COLORED); // wyświetlanie ID   + ID[0] + ID[1] + ID[2]
            epd.Display_Partial(paint.GetImage(), 0, 130, 0 + paint.GetWidth(), 130 + paint.GetHeight());
            delay(1000);
            epd.Sleep();
            #endif
            
            gameStatus = Status::NONE;
            break;
        }
        case Status::FAILURE: { // bomba wybuchła
            /* @TODO buzzer wydaje głosy wybuchu, timer gasi się */
            #ifdef MODULE_EPAPER
            Epd epd;
            epd.Init();
            epd.Display(epd_bitmap_boom1bit);
            delay(1000);
            epd.Sleep();
            #endif
            
            gameStatus = Status::NONE;
            break;
        }
        default: break;
    }
}








