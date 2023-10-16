#include <RGBmatrixPanel.h>
#include <math.h>

#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A2
#define C   A1

static int targetRefreshRate = 60; // Rate to refresh the display at in Hz.
static int frameTime = 1000 / targetRefreshRate;
static int paddleDimensions[2] = {1,4}; // x,y
//static int PI = 3.1415;

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Timers
unsigned long lastDrawnTime = millis();
unsigned long timeStamp = millis();

int paddleOnePos = 0;
int paddleTwoPos = 0;
// 0-31, 0-15 matched to matrix dimensions. Origin 0,0 is top left.
int ballPosition[2] = {0, 7}; 


void drawFrame(int paddleAPos){
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.fillRect(0, paddleAPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(7, 0, 0));
    matrix.fillRect(31, paddleBPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(0, 0, 7));
    matrix.drawPixel(ballPosition[0], ballPosition[1], matrix.Color333(0, 7, 0)); // Ball
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
  Serial.begin(115200);
}

void loop() {
    // Refresh the frame on time.
    if (millis() - lastDrawnTime > frameTime) {
        lastDrawnTime = millis();
        // TODO: read serial for new positions
        drawFrame(paddlePosition);
    }


    
  
}
