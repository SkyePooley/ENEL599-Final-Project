#include <RGBmatrixPanel.h>
#include <math.h>

#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A2
#define C   A1
#define POT A4

static int targetRefreshRate = 30; // Rate to refresh the physics at in Hz.
static int frameTime = 1000 / targetRefreshRate;
static int paddleDimensions[2] = {1,4}; // x,y
static float ballSpeed = 0.4;
//static int PI = 3.1415;

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Timers
unsigned long lastDrawnTime = millis();
unsigned long timeStamp = millis();

int paddlePosition = 0;
// 0-31, 0-15 matched to matrix dimensions. Origin 0,0 is top left.
float ballPosition[2] = {0, 7}; 
float ballDirection = 45; // Ball travel direction in degrees, 0 is straight up.

// Convert a direction in degrees to radians.
float degToRad(float degrees) {
    return degrees * PI / 180;
}

void drawFrame(int paddleAPos){
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    //matrix.fillRect(0, paddleAPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(7, 0, 0));
    matrix.drawPixel((int)ballPosition[0], (int)ballPosition[1], matrix.Color333(0, 7, 0));
}

float updateBall(float* ballPos, float ballDirection) {
    float radianDirection = degToRad(ballDirection);
    Serial.print("direction: ");
    Serial.println(ballDirection);
    
    float xOffset = ballSpeed * cos(radianDirection);
    float yOffset = ballSpeed * sin(radianDirection);
    Serial.print("Offsets: ");
    Serial.print(xOffset);
    Serial.print(", ");
    Serial.println(yOffset);

    ballPos[0] += xOffset;
    ballPos[1] += yOffset;

    // Constrain ball Y and bounce
    if (ballPos[1] < 0) { 
      ballPos[1] = 0;
      if (ballDirection < 180) { ballDirection = 135; }
      else { ballDirection = 225; }
    }
    else if (ballPos[1] > 15) { 
      ballPos[1] = 15; 
      if (ballDirection < 180) { ballDirection = 45; }
      else { ballDirection = 315; }
    }
    // Constrain ball X and bounce
    if (ballPos[0] < 0) { 
      ballPos[0] = 0;
      if (ballDirection < 270) { ballDirection = 135; }
      else { ballDirection = 45; }
    }
    else if (ballPos[0] > 31) { 
      ballPos[0] = 31; 
      if (ballDirection < 90) { ballDirection = 315; }
      else { ballDirection = 225; }
    }
    
    return ballDirection;
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
  Serial.begin(115200);
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
        lastDrawnTime = millis();
        ballDirection = updateBall(ballPosition, ballDirection);
        Serial.print(ballPosition[0]);
        Serial.print(", ");
        Serial.println(ballPosition[1]);
        drawFrame(paddlePosition);
    }


    
  
}
