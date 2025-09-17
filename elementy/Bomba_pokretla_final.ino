#include <Servo.h>

// --- Definicje i zmienne z programu 1 ---
#define CIRCLES_SERVO_UP 43  // Servo1 (program 1)
#define CIRCLES_STEER_UP_CLK 24  // Enkoder1A (program 1)
#define CIRCLES_STEER_UP_DT  26  // Enkoder1B (program 1)
#define CIRCLES_SERVO_LEFT  45     // (program 2)
#define CIRCLES_STEER_LEFT_CLK  2   // (program 2)
#define CIRCLES_STEER_LEFT_DT  3    // (program 2)
#define CIRCLES_SERVO_RIGHT 44
#define CIRCLES_STEER_RIGHT_CLK 22   // kanał A (CLK) (program 3)
#define CIRCLES_STEER_RIGHT_DT 23    // kanał B (DT) (program 3)
#define CIRCLES_STEER_RIGHT_SW 25
// CIRCLES PROG 1
Servo circlesServoUp;
const int circlesServoUpStop = 90;     // zatrzymanie (program 1)
const int circlesServoUpSteerRight = 100;   // ruch w prawo (program 1)
const int circlesServoUpSteerLeft  = 80;    // ruch w lewo (program 1)
const int circlesUpStepDegree = 2;        // krok obrotu (program 1)
const float circlesUpDegreesPerMs = 0.1f;  // ile stopni na 1 ms (program 1)
const int circlesUpMoveTime = circlesUpStepDegree / circlesUpDegreesPerMs;  // czas ruchu dla kroku (program 1)
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
int circlesRiightDirection = 1; // 1 = prawo, -1 = lewo (program 3)
int circlesRightLastCLK = HIGH;



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

void setup() 
{
	// CIRCLES PROG 1
	pinMode(CIRCLES_STEER_UP_CLK, INPUT);
	pinMode(CIRCLES_STEER_UP_DT, INPUT);
	circlesServoUp.attach(CIRCLES_SERVO_UP);
	circlesServoUp.write(circlesServoUpStop);
	circlesUpLastCLK = digitalRead(CIRCLES_STEER_UP_CLK);

	// CIRCLES PROG 2
	pinMode(CIRCLES_STEER_LEFT_CLK, INPUT_PULLUP);
	pinMode(CIRCLES_STEER_LEFT_DT, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(CIRCLES_STEER_LEFT_CLK), circlesProg2UpdateEncoder, CHANGE);
	attachInterrupt(digitalPinToInterrupt(CIRCLES_STEER_LEFT_DT), circlesProg2UpdateEncoder, CHANGE);
	circlesServoLeft.attach(CIRCLES_SERVO_LEFT);
	circlesServoLeft.write(circlesServoLeftRotation);
	circlesLeftLastActivityTime = millis();

	// CIRCLES PROG 3
	circlesServoRight.attach(CIRCLES_SERVO_RIGHT);
	pinMode(CIRCLES_STEER_RIGHT_CLK, INPUT);
	pinMode(CIRCLES_STEER_RIGHT_DT, INPUT);
	pinMode(CIRCLES_STEER_RIGHT_SW, INPUT_PULLUP);
	circlesServoRight.writeMicroseconds(1500); // stop na starcie
	circlesRightLastCLK = digitalRead(CIRCLES_STEER_RIGHT_CLK);
}

void loop() 
{
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
				circlesRiightDirection = 1;   // w prawo
			} else {
				circlesRiightDirection = -1;  // w lewo
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
			if (circlesRiightDirection == 1) {
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
}
