#include <RGBmatrixPanel.h>
#include <math.h>

#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A2
#define C   A1
#define POT A4

static int targetRefreshRate = 1;
static int frameTime = 1000 / targetRefreshRate;
static int paddleDimensions[2] = {1,4}; // x,y
static int ballSpeed = 0.5;
//static int PI = 3.1415;

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Timers
unsigned long lastDrawnTime = millis();
unsigned long timeStamp = millis();

int paddlePosition = 0;
float ballPosition[2] = {0, 0}; // 0-31, 0-15
double ballDirection = 135; // Ball travel direction in degrees, 0 is straight up.

// Convert a direction in degrees to radians.
double degToRad(double degrees) {
    return degrees * PI / 180;
}

void drawFrame(int paddleAPos){
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.fillRect(0, paddleAPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(7, 0, 0));
    matrix.drawPixel((int)ballPosition[0], (int)ballPosition[1], matrix.Color333(0, 7, 0));
}

void updateBall(float* ballPos, double ballDirection) {
    if (ballDirection > 90)
        ballDirection -= 90;
    if (ballDirection > 180)
        ballDirection -= 90;
    if (ballDirection > 270)
        ballDirection -= 90;


    float xOffset = ballSpeed * sin(degToRad(ballDirection));
    float yOffset = ballSpeed + cos(degToRad(ballDirection));

    ballPos[0] += xOffset;
    ballPos[1] += yOffset;
}

void setup() {
  pinMode(OE, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(CLK, OUTPUT);

  matrix.begin();
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  pinMode(POT, INPUT);
}

void loop() {
    if (millis() - timeStamp > 200) {
        paddlePosition ++;
        if (paddlePosition > 15)
            paddlePosition = 0;
        timeStamp = millis();
    }

    // Refresh the frame on time.
    if (millis() - lastDrawnTime > frameTime) {
        updateBall(ballPosition, ballDirection);
        drawFrame(paddlePosition);
    }


    
  
}
