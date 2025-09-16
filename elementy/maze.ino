
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include<SPI.h> 

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128

#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8

#define JOY_X A1
#define JOY_Y A2
#define JOY_BUTTON A3


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

//uint16_t        Display_Text_Color         = 0x0000;
//uint16_t        Display_Backround_Color    = 0xFFFF;


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int playerX = 0;
int playerY = 0;
int czyZmiana=0;

void setup() {

  Serial.begin(9600);

  pinMode(JOY_BUTTON, INPUT_PULLUP);
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST7735_BLACK);
}

void loop() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  Serial.print("xVal="); Serial.println(xVal);
  Serial.print("yVal="); Serial.println(yVal);
  


  int dx = 0;
  int dy = 0;

  if (xVal < 300) dx = 1;   // w prawo
  if (xVal > 700) dx = -1;  // w lewo
  if (yVal < 300) dy = 1;   // w dół
  if (yVal > 700) dy = -1;  // w górę

  int newX = playerX + dx;
  int newY = playerY + dy;

  if(newX!=playerX || newY!=playerY) czyZmiana=1;

  // sprawdź granice i ścianę
  if (newX >= 0 && newX < mazeWidth && newY >= 0 && newY < mazeHeight && maze[newY][newX] == 0) {
    playerX = newX;
    playerY = newY;
    delay(200); // prosty debounce
  }
  if (maze[newY][newX]==2){
    playerX = newX;
    playerY = newY;
    delay(200);
    drawEnd();
  }

  
  if(czyZmiana==1){
  drawMaze();
  drawPlayer();
  drawFinish();
  czyZmiana=0;}
  //Serial.println("czyZmiana=",czyZmiana);
}

void drawMaze() {
  tft.fillScreen(ST7735_BLACK);
  for (int y = 0; y < mazeHeight; y++) {
    for (int x = 0; x < mazeWidth; x++) {
      if (maze[y][x] == 1) {
        tft.fillRect(x * 16, y * 16, 16, 16, ST77XX_WHITE); // ściana
      }
    }
  }
}
void drawPlayer() {
  // Gracz = biały kwadrat 8x8 px w środku pola
  tft.fillRect(playerX * 16 + 4, playerY * 16 + 4, 8, 8, ST77XX_WHITE);
}

void drawFinish(){
  //wyjscie - pusty bialy kwadracik
  tft.drawRect(3*16 + 4, 8*16 + 4, 8, 8, ST77XX_WHITE);
}

void drawEnd(){
  tft.fillScreen(ST7735_BLACK);
  //wyjscie - pusty bialy kwadracik
  tft.setTextSize(3);
  tft.write("Wygrana!");
  while(1);
}