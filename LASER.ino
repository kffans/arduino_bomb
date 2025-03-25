#include <Servo.h> 
int pos = 0; 
int obrot=0;
Servo my_servo; 

void setup() {
  pinMode(2,INPUT);
  pinMode(3,INPUT);
my_servo.attach(4);
  Serial.begin(9600);
}

void loop() { 
  int S1 = digitalRead(2); // domyslnie 1; ma wartosc 0 gdy kreci sie w lewo
  int S2 = digitalRead(3); // domyslnie 1; ma wartosc 0 gdy kreci sie w prawo
  //Serial.println(S1);
  Serial.println(obrot);
  if(S1==0)
    obrot+=5;
  if(S2==0)
    obrot-=5;

  if(obrot>180)
    obrot=180;
  if(obrot<0)
    obrot=0;
  
  my_servo.write(obrot); 
}