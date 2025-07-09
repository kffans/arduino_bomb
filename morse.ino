
#include <Keypad.h>
#include <string.h>
#include "pitches.h"

const uint8_t ROWS = 3;
const uint8_t COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' }
}; 

uint8_t rowPins[ROWS] = { 14, 15, 16 }; // Piny wierszy
uint8_t colPins[COLS] = { 17, 18, 19 }; // Piny kolumn

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //inicjalizacja klawiatury
char key; 

const int gameTones[] = { NOTE_E5, NOTE_CS4, NOTE_F6}; //4 tony które trzeba wcisnąć na klawiaturze
const int Keys[]={'5','2','6'}; //numery klawiszy ktore trzeba wcisnac
const int KeyboardTones[3][3]= { //pelna klawiatura dzwiekow
                          {NOTE_C1,  NOTE_CS4, NOTE_D4 },
                          {NOTE_DS5, NOTE_E5,  NOTE_F6 },
                          {NOTE_FS6, NOTE_G7,  NOTE_GS7}
                          }; 

int tonesLength=3;
int tone_number=0;
int czy_przerwa_key=0;

int czy_przerwa_melody=0;
int key_number=0;
int czy_poprawny_key=0;

int koniec_dzwieku=0;

const int BUZZER=7;
unsigned long DOT_TIME=150;
unsigned long DASH_TIME=500;
unsigned long BREAK=600;
unsigned long LONG_BREAK=1600;
const int BUTTON = 8;

unsigned long KEY_TIME=500;
unsigned long KEY_BREAK=500;
unsigned long WAITING_TIME=5000;

enum Mode {morse, keyboard, melody, explosion};
Mode currentMode = morse;

unsigned long currentTime = 0;
unsigned long rememberedTime = millis();
unsigned long difference = 0;
unsigned long explosionTime = 500000;

int letter_number=0;
int tryb=0;
String letters = "01210002";//._ _... AB
const int lettersLength = letters.length(); 
int buttonState=0;
int czy_przerwa=0;



void playKeys()
{
  difference=currentTime-rememberedTime;
  if(difference>=WAITING_TIME)//po 5 sekundach braku aktywnosci wracam do morse'a
  {
    currentMode=morse;
    key_number=0;
  }
  key=keypad.getKey();
  if(key != NO_KEY)//po wcisnieciu klawisza
  { 
    rememberedTime=millis();
    switch(key)//gram dzwiek przypisany do klawisza
    {
      case '1': tone(BUZZER, KeyboardTones[0][0]);Serial.println("klik 1");break;
      case '2': tone(BUZZER, KeyboardTones[0][1]);Serial.println("klik 2");break;
      case '3': tone(BUZZER, KeyboardTones[0][2]);Serial.println("klik 3");break;
      case '4': tone(BUZZER, KeyboardTones[1][0]);Serial.println("klik 4");break;
      case '5': tone(BUZZER, KeyboardTones[1][1]);Serial.println("klik 5");break;
      case '6': tone(BUZZER, KeyboardTones[1][2]);Serial.println("klik 6");break;
      case '7': tone(BUZZER, KeyboardTones[2][0]);Serial.println("klik 7");break;
      case '8': tone(BUZZER, KeyboardTones[2][1]);Serial.println("klik 8");break;
      case '9': tone(BUZZER, KeyboardTones[2][2]);Serial.println("klik 9");break;
      default : break;
    }
    delay(200);
    czy_przerwa_key=1;
    if(key!=Keys[key_number])//sprawdzenie poprawnosci
    {
      Serial.println("klawisz sie nie zgadza");
      Serial.println(key);
      Serial.println((char)Keys[key_number]);
      czy_poprawny_key=1;
      
    }
    key_number++;
    Serial.print("k=");
    Serial.println(key_number);
    if(key_number==3 && czy_poprawny_key==0)//gdy 4 klawisze wcisniecie sprawdzenie czy nie bylo pomylki
    {
      //cos sie dzieje gdy wygrana
      Serial.println("wygrana!");
      noTone(BUZZER);
      currentMode=morse;
      key_number=0;
    }
    else if(key_number==3 && czy_poprawny_key==1)//gdy zdarzyla sie pomylka mozna wpisac kod ponownie
    {
      Serial.println("zle!");
      key_number=0;
      czy_poprawny_key = 0;
    }
  }
  difference=currentTime-rememberedTime;
  if(difference>=KEY_TIME && czy_przerwa_key==1)//gdy minie 200ms koncze grac dzwiek
  {
    noTone(BUZZER);
    czy_przerwa_key=0;
  }

}

void playMelody()
{
  difference=currentTime-rememberedTime;
  if(czy_przerwa_melody==0) //kiedy 0 gra dzwiek
  tone(BUZZER,gameTones[tone_number]);
  if(difference>=KEY_TIME && czy_przerwa_melody==0) //gdy minie czas dzwieku wylacz buzzer
  {
    noTone(BUZZER);
    rememberedTime=currentTime;
    czy_przerwa_melody=1;
    Serial.println("gram");
  }
  else
  if(czy_przerwa_melody==1 && difference>=KEY_BREAK) //gdy minie czas przerwy zagraj nastepny dzwiek
  {Serial.println("przerwa");
    if(tone_number<tonesLength)
    tone_number++;
    czy_przerwa_melody=0;
    rememberedTime=currentTime;
  }
  if(tone_number==tonesLength)
  {
    currentMode=keyboard;
    tone_number=0;
  }

}

void playMorse ()
{ 
  difference = currentTime-rememberedTime;
  if(czy_przerwa==1 && difference>=BREAK)
    {
      rememberedTime=currentTime;
      czy_przerwa=0;
      difference = currentTime-rememberedTime;
    }

  if(letters[letter_number]=='0' && czy_przerwa==0)
  {
    tone(BUZZER, 1000);
    if(difference >= DOT_TIME)
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(letter_number<lettersLength-1)
      letter_number++;
      else letter_number=0;
      czy_przerwa=1;
    }
  }
  if(letters[letter_number]=='1' && czy_przerwa==0)
  {
    tone(BUZZER, 1000);
    if(difference >= DASH_TIME )
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(letter_number<lettersLength-1)
      letter_number++;
      else letter_number=0;
      czy_przerwa=1;
    }
  }
  if(letters[letter_number]=='2' && czy_przerwa==0)
  {
    noTone(BUZZER);
    if(difference >= LONG_BREAK)
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(letter_number<lettersLength-1)
      letter_number++;
      else letter_number=0;
      czy_przerwa=1;
    }
  }  
}
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON,INPUT_PULLUP);
}

void loop() {
  currentTime=millis();
  buttonState = digitalRead(BUTTON);
  if(currentTime>=explosionTime)
  {
    currentMode = explosion;
    Serial.println("wybuchlo");
  }
  else if(buttonState == LOW)
  {
    currentMode = melody;
    Serial.println("weszlo");
    rememberedTime=millis();
    noTone(BUZZER);
    tone_number=0;
    czy_przerwa_melody=0;
  }
  else
  if(currentMode==melody)
  {
    playMelody();
  }
  else
  if(currentMode==keyboard)
  {
    playKeys();
  }
  else if (currentMode==morse) playMorse();

  delay(1);
}
