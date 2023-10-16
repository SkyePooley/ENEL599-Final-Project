#include <RGBmatrixPanel.h>
#include <math.h>

#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2

#define FLAG_PADDLE_A 0
#define FLAG_PADDLE_B 1
#define FLAG_BALL_X   2
#define FLAG_BALL_Y   3

static int targetRefreshRate = 60; // Rate to refresh the display at in Hz.
static int frameTime = 1000 / targetRefreshRate;
static int paddleDimensions[2] = {1,4}; // x,y

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Timers
unsigned long lastDrawnTime = millis();
unsigned long timeStamp = millis();

int paddleOnePos = 0;
int paddleTwoPos = 0;
// 0-31, 0-15 matched to matrix dimensions. Origin 0,0 is top left.
int ballPosition[2] = {0, 7}; 


void drawFrame(){
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.fillRect(0, paddleOnePos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(7, 0, 0));
    matrix.fillRect(31, paddleTwoPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(0, 0, 7));
    //matrix.fillRect(31, 0, 1, 16, matrix.Color333(0, 0, 7));
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
    while(Serial.available() > 0) {
        char receivedByte = Serial.read();
        char flag = (receivedByte & 0b11100000) >> 5;
        char value = receivedByte & 0b00011111;
      
        switch(flag) {
          case FLAG_PADDLE_A:
            paddleOnePos = value;
            break;
          case FLAG_PADDLE_B:
            paddleTwoPos = value;
            break;
          case FLAG_BALL_X:
            ballPosition[0] = value;
            break;
          case FLAG_BALL_Y:
            ballPosition[1] = value;
            break;
        }
    }
  
    // Refresh the frame on time.
    if (millis() - lastDrawnTime > frameTime) {
        lastDrawnTime = millis();
        drawFrame();
    }


    
  
}
