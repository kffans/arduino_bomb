
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
int tonesLength=3;
int g=0;
int czy_przerwa_key=0;

const int BUZZER=13;
unsigned long DOT_TIME=200;
unsigned long DASH_TIME=500;
unsigned long BREAK=1000;
unsigned long LONG_BREAK=1500;
const int BUTTON = 12;

unsigned long KEY_TIME=200;
unsigned long KEY_BREAK=200;

enum Mode {morse, keyboard, explosion};
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




bool buttonPressed()
{
  key = keypad.getKey();
  buttonState = digitalRead(BUTTON);
  if (buttonState == LOW || key != NO_KEY) 
  { 
       return 1;
  }
  else return 0;
}





void playKey()
{
  difference=currentTime-rememberedTime;
  if(czy_przerwa_key==0)
  tone(BUZZER,gameTones[g]);
  if(difference>=KEY_TIME && czy_przerwa_key==0)
  {
    noTone(BUZZER);
    rememberedTime=currentTime;
    czy_przerwa_key=1;
    Serial.println("gram");
  }
  
  if(czy_przerwa_key==1 && difference>=KEY_BREAK)
  {Serial.println("przerwa");
    if(g<tonesLength)
    g++;
    czy_przerwa_key=0;
    /*if(g==4)
    czy_przerwa_key=1;*/
  }
}



void playMorse ()
{ Serial.println(a);
  difference = currentTime-rememberedTime;
  if(czy_przerwa==1 && difference>=BREAK)
    {Serial.println("przerwa");
      rememberedTime=currentTime;
      czy_przerwa=0;
      difference = currentTime-rememberedTime;
    }

  if(letters[a]=='0' && czy_przerwa==0)
  {Serial.println("jestem w 0");
  
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
  {Serial.println("jestem w 1");
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
  {Serial.println("jestem w 2");
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
  /*
  Serial.print("current time:");
  Serial.println(currentTime);
  Serial.print("explosion time:");
  Serial.println(explosionTime);*/
  if(currentTime>=explosionTime)
  {
    currentMode = explosion;
    Serial.println("wybuchlo");
  }
  else if(buttonPressed())
  {
    currentMode = keyboard;
    Serial.println("weszlo");
    rememberedTime=millis();
    noTone(BUZZER);
    g=0;
    czy_przerwa_key=0;
  }
  if(currentMode==keyboard)
  {
    playKey();
  }
  else if (currentMode==morse) playMorse();

  delay(1);
}
