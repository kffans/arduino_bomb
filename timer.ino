#include <Wire.h>

#define TIMER_ADDR       0x30 
#define TIMER_DISPLAY    0x00
#define TIMER_BRIGHTNESS 0x06

int timerSecondsLeft = 300;
int timeMs = 0;

void setup() {
    Wire.begin();
    timerSetBrightness(7);
}

void loop() {
    if(timeMs % 1000 == 0){
        timerSecondsLeft--;
        if(timerSecondsLeft >= 0){
            int minutesDigit1 = timerSecondsLeft / 600;
            int minutesDigit2 = (timerSecondsLeft / 60) % 10;
            int secondsDigit1 = (timerSecondsLeft - ((minutesDigit1 * 10 + minutesDigit2) * 60)) / 10;
            int secondsDigit2 = (timerSecondsLeft - ((minutesDigit1 * 10 + minutesDigit2) * 60)) % 10;

            timerShowDigits(minutesDigit1, minutesDigit2, secondsDigit1, secondsDigit2);
        }
    }
    timeMs += 1;
    delay(1);
}

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