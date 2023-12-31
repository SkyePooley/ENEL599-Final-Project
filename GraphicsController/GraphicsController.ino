/*
    ENEL599 Final Project Graphics Controller
    October 2023 - Skye Pooley - 22179237
    Companion to the "Game Controller" code on the other arduino.
    This code controls the RGB LED Matrix panel for the PONG project.
    The Adafruit RGBmatrixPanel library is used for shifting out data to the panel registers
    and selecting lines.
    https://github.com/adafruit/RGB-matrix-Panel
    Receives the position of entities to draw over serial.
*/

#include <RGBmatrixPanel.h>

// Control pins for the RGB panel
static int CLK = 8;
static int OE  = 9;
static int LAT = 10;
static int A   = A0;
static int B   = A1;
static int C   = A2;

// Flags for identifying incoming serial transmissions.
#define FLAG_PADDLE_A 0
#define FLAG_PADDLE_B 1
#define FLAG_BALL_X   2
#define FLAG_BALL_Y   3

static int targetRefreshRate = 60; // Rate to refresh the display at in Hz.
static int frameTime = 1000 / targetRefreshRate;
static int paddleDimensions[2] = {1,4}; // x,y

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Timer to update the video output at a regular rate
unsigned long lastDrawnTime = millis();

// Vertical position of the paddles
int paddleOnePos = 0;
int paddleTwoPos = 0;

// 0-31, 0-15 matched to matrix dimensions. Origin 0,0 is top left.
int ballPosition[2] = {0, 7}; 


void drawFrame(){
    matrix.fillScreen(matrix.Color333(0, 0, 0));// clear display
    // Draw paddles
    matrix.fillRect(0, paddleOnePos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(7, 0, 0));  // Player One
    matrix.fillRect(31, paddleTwoPos, paddleDimensions[0], paddleDimensions[1], matrix.Color333(0, 0, 7)); // Player Two
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
    // Receive data from control arduino
    while(Serial.available() > 0) {
        char receivedByte = Serial.read();
        char flag = (receivedByte & 0b11100000) >> 5;
        char value = receivedByte & 0b00011111;

        // Match flag to variable
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
  
    // Refresh the video output on time.
    if (millis() - lastDrawnTime > frameTime) {
        lastDrawnTime = millis();
        drawFrame();
    }
}
