
#include <Keypad.h>
#include <string.h>
#include "pitches.h"

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

uint8_t colPins[COLS] = { 5, 4, 3, 2 }; // Pins connected to C1, C2, C3, C4
uint8_t rowPins[ROWS] = { 9, 8, 7, 6 }; // Pins connected to R1, R2, R3, R4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char key;

const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};
const int Keys[]={'1','5','3','2'};
const int KeyboardTones[5][3]= {
                          {NOTE_G3, NOTE_G5, NOTE_E4},
                          {NOTE_A4, NOTE_C4, NOTE_A4},
                          {NOTE_E1, NOTE_E2, NOTE_E3},
                          {NOTE_D1, NOTE_G1, NOTE_G3},
                          {0,NOTE_G1,0}
                          };
int tonesLength=4;
int g=0;
int czy_przerwa_key=0;

int czy_przerwa_melody=0;
int k=0;
int czy_poprawny_key=0;

int koniec_dzwieku=0;

const int BUZZER=13;
unsigned long DOT_TIME=200;
unsigned long DASH_TIME=500;
unsigned long BREAK=1000;
unsigned long LONG_BREAK=1500;
const int BUTTON = 12;

unsigned long KEY_TIME=500;
unsigned long KEY_BREAK=500;
unsigned long WAITING_TIME=5000;

enum Mode {morse, keyboard, melody, explosion};
Mode currentMode = morse;

unsigned long currentTime = 0;
unsigned long rememberedTime = millis();
unsigned long difference = 0;
unsigned long explosionTime = 500000;

int a=0;
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
    k=0;
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
      case '0': tone(BUZZER, KeyboardTones[3][1]);Serial.println("klik 0");break;
      default : break;
    }
    czy_przerwa_key=1;
    if(key!=Keys[k])//sprawdzenie poprawnosci
    {
      Serial.println("klawisz sie nie zgadza");
      Serial.println(key);
      Serial.println((char)Keys[k]);
      czy_poprawny_key=1;
      
    }
    k++;
    Serial.print("k=");
    Serial.println(k);
    if(k==4 && czy_poprawny_key==0)//gdy 4 klawisze wcisniecie sprawdzenie czy nie bylo pomylki
    {
      //cos sie dzieje gdy wygrana
      Serial.println("wygrana!");
      currentMode=morse;
      k=0;
    }
    else if(k==4 && czy_poprawny_key==1)//gdy zdarzyla sie pomylka mozna wpisac kod ponownie
    {
      Serial.println("zle!");
      k=0;
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
  if(czy_przerwa_melody==0)
  tone(BUZZER,gameTones[g]);
  if(difference>=KEY_TIME && czy_przerwa_melody==0)
  {
    noTone(BUZZER);
    rememberedTime=currentTime;
    czy_przerwa_melody=1;
    Serial.println("gram");
  }
  else
  if(czy_przerwa_melody==1 && difference>=KEY_BREAK)
  {Serial.println("przerwa");
    if(g<tonesLength)
    g++;
    czy_przerwa_melody=0;
    rememberedTime=currentTime;
  }
  if(g==tonesLength)
  {
    currentMode=keyboard;
    g=0;
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

  if(letters[a]=='0' && czy_przerwa==0)
  {
    tone(BUZZER, 1000);
    if(difference >= DOT_TIME)
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(a<lettersLength-1)
      a++;
      else a=0;
      czy_przerwa=1;
    }
  }
  if(letters[a]=='1' && czy_przerwa==0)
  {
    tone(BUZZER, 1000);
    if(difference >= DASH_TIME )
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(a<lettersLength-1)
      a++;
      else a=0;
      czy_przerwa=1;
    }
  }
  if(letters[a]=='2' && czy_przerwa==0)
  {
    noTone(BUZZER);
    if(difference >= LONG_BREAK)
    {
      rememberedTime=currentTime;
      noTone(BUZZER);
      if(a<lettersLength-1)
      a++;
      else a=0;
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
    g=0;
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
