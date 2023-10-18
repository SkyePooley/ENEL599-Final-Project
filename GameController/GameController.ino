/*
    ENEL599 Final Project Game Controller code
    October 2023 - Skye Pooley - 22179237
    Plays PONG!
    This code works in tandem with the "Graphics controller code" on the other arduino.
    Both controllers are needed for the game to work.
    This controller handles:
        Input from the players
        Simulation of game objects
        Playing of sound effects and simple music
        Output to 7-segment displays to show player scores
        Transfer of game entity positions over serial to graphics controller arduino.
*/

// Labels for serial data flags, used to determine what a transmitted number refers to.
#define FLAG_PADDLE_A 0
#define FLAG_PADDLE_B 1
#define FLAG_BALL_X   2
#define FLAG_BALL_Y   3

// ------  Static variables and pin assignments ------
// Shift register control pins
static int SER = 12; // data pin 
static int RCLK = 10; // register clock
static int SCLK = 11; // serial clock
static int OE = 9; // output enable

static int BUZZER = 8; // piezo
static int CONTROLLER_ONE = A0; // player input
static int CONTROLLER_TWO = A1;

// ------- Reference numbers for game function --------
static int FRAME_RATE = 60;
static int FRAME_TIME = 1000/FRAME_RATE; // time inbetween frames/refreshes.
static int resolution[2] = {32, 16};
static int PADDLE_WIDTH = 4;
static float DEFAULT_SPEED[2] = {0.25, 0.35};
// the game input controller pins function both as button inputs and potentiometer inputs.
// When the input value is below this cutoff it should be treated as a button instead of a dial.
static int CONTROLLER_BUTTON_CUTOFF = 70;
static int WIN_SCORE = 3; // number of rounds that a player has to win in order to win the game.


// ------- Music tracks and sound effects --------
struct MusicTrack {
    unsigned int notes[100][2]; // each position in the array is one note which contains a frequency and duration.
    unsigned int length; // used to track when the song ends.
};

static struct MusicTrack STARTUP_JINGLE = {
    {
        {987, 100},
        {3951, 300}
    },
    2
};

static struct MusicTrack MISS = {
    {
        {600, 10},
        {850, 10},
        {400, 10},
        {250, 10},
        {400, 10},
        {250, 10},
        {400, 10},
        {250, 10},
        {100, 250}
    },
    11
};

static struct MusicTrack HIT = {
    {
        {600, 10},
        {850, 10},
        {600, 10},
        {850, 10},
        {600, 10},
        {950, 10},
    },
    8
};

static struct MusicTrack WIN = {
    {
        {600, 10},
        {850, 10},
        {600, 10},
        {850, 10},
        {850, 10},
        {1050, 10},
        {850, 10},
        {1050, 500},
        {600, 400},
        {1050, 800}
    },
    10
};

// Each bit in these bytes represents one segment of a 7-segment display. 
// The final bit is the decimal point which is not used.
// When referencing this array, the index number corresponds to the number displayed using that byte.
char digitMappings[10] = {
    0b11111100, //0
    0b01100000, //1
    0b11011010, //2
    0b11110010, //3
    0b01100110, //4
    0b10110110, //5
    0b10111110, //6
    0b11100000, //7
    0b11111110, //8
    0b11100110, //9
};

// ------  Dynamic Variables  --------
// Scores
unsigned int playerOneScore = 0;
unsigned int playerTwoScore = 0;

// Timers
unsigned long toneEndTime = 0; // records the time when a started tone should stop.
unsigned long lastRefreshTime = 0; // time that the frame was last refreshed.

bool gameActive = false; // determines whether the ball is moving, game needs to pause inbetween turns
int playerToServe = 1;
bool gameWon = false;

// Entities
unsigned int paddleOnePos = 0;
unsigned int paddleTwoPos = 0;
float ballPos[2] = {1.0, 0.0};
float ballVelocity[2] = {DEFAULT_SPEED[0], DEFAULT_SPEED[1]}; // amount to move in X,Y each refresh

// Pin Readings
int controllerOneSignal = 0;
int controllerTwoSignal = 0;

// Music tracking
bool musicActive = false;
struct MusicTrack* p_activeTrack; // currently playing music track
int trackIndex = 0; // what note in the track are we up to

// -------- Function Prototypes -------
void clearCounters();
void setCounters(unsigned int playerOne, unsigned int playerTwo);
void startTone(unsigned int frequency, unsigned int duration);
void startMusicTrack(struct MusicTrack* p_newTrack);
void sendWithID(int value5, int id3);
int updateBall(float* ballVelocity, float* ballPos, int paddleOnePos);


void setup() {
    pinMode(BUZZER, OUTPUT);
    pinMode(SER, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(CONTROLLER_ONE, INPUT);
    pinMode(CONTROLLER_TWO, INPUT);

    // Clear any floating rubbish on the registers
    clearCounters();
    startMusicTrack(&STARTUP_JINGLE);
    Serial.begin(115200);
}

void loop() {
    // ----- Handle Inputs ----- //
    controllerOneSignal = analogRead(CONTROLLER_ONE);
    controllerTwoSignal = analogRead(CONTROLLER_TWO);
    // if the controller signal is being used for the dial and not the button
    if (controllerOneSignal > CONTROLLER_BUTTON_CUTOFF)
        paddleOnePos = map(controllerOneSignal, CONTROLLER_BUTTON_CUTOFF,1023, 0,13); // map to screen space
    if (controllerTwoSignal > CONTROLLER_BUTTON_CUTOFF)
        paddleTwoPos = map(controllerTwoSignal, CONTROLLER_BUTTON_CUTOFF,1023, 0,13);
  
    // ----- Update Game on time ----- //
    if (millis() > lastRefreshTime + FRAME_TIME) {
        // If there is a current round happening
        if (gameActive) {
            // Update the ball position and handle point conditions.
            switch (updateBall(ballVelocity, ballPos, paddleOnePos)) {
                case 1:
                    startMusicTrack(&MISS);
                    playerOneScore++;
                    gameActive = false;
                    playerToServe = 1;
                    break;
                case 2:
                    startMusicTrack(&MISS);
                    playerTwoScore++;
                    gameActive = false;
                    playerToServe = 2;
                    break;
            }
        }
        // Game is between rounds but the game is not yet won
        else if (!gameWon) {
            switch (playerToServe) {
                case 1:
                    ballPos[1] = paddleOnePos + 1;
                    if (controllerOneSignal < 10) { gameActive = true; }
                    break;
                case 2:
                    ballPos[1] = paddleTwoPos + 1;
                    if (controllerTwoSignal < 10) { gameActive = true; }
                    break;
            }
        }

        if ((playerOneScore > 3 || playerTwoScore > 3) && !gameWon) {
            gameWon = true;
            startMusicTrack(&WIN);
        }

        // send entity positions to graphics controller
        sendWithID(paddleOnePos, FLAG_PADDLE_A);
        sendWithID(paddleTwoPos, FLAG_PADDLE_B);
        sendWithID((int)ballPos[0], FLAG_BALL_X);
        sendWithID((int)ballPos[1], FLAG_BALL_Y);

        // update 7-segment displays
        setCounters(playerOneScore, playerTwoScore);

        lastRefreshTime = millis();
    }

    // Do we need to play music?
    if (musicActive) {
        // check if a note should have stopped by now
        if (millis() > toneEndTime) {
            noTone(BUZZER); 
            trackIndex++;// move to the next note
            // are we at the end of the track
            if (trackIndex == p_activeTrack->length) {
                musicActive = false;
            }
            else {
                startTone(p_activeTrack->notes[trackIndex][0], p_activeTrack->notes[trackIndex][1]);
            }
        }
    }
}


// -------- Helper Functions --------
// Sets all shift register pins to off.
void clearCounters() {
    // clear shift registers
    digitalWrite(OE, HIGH); // Disable output
    digitalWrite(RCLK, LOW);
    shiftOut(SER, SCLK, MSBFIRST, 0b00000000); 
    shiftOut(SER, SCLK, MSBFIRST, 0b00000000);
    digitalWrite(RCLK, HIGH); // shift new data to output register.
    delay(1);
    digitalWrite(RCLK, LOW);
    digitalWrite(OE, HIGH); // Show output again
}

/* 
    Changed shift register outputs to match new values.
    Maximum value of nine.
*/
void setCounters(unsigned int playerOne, unsigned int playerTwo) {
    if (playerOne <= 9 && playerTwo <= 9) {
        digitalWrite(OE, HIGH); // hide output while we are messing with it
        shiftOut(SER, SCLK, LSBFIRST, digitMappings[playerTwo]); // shift the second players score onto the first register
        shiftOut(SER, SCLK, LSBFIRST, digitMappings[playerOne]); // shift the first player's score onto first register, moving second player score to second register.
        digitalWrite(RCLK, HIGH);
        digitalWrite(RCLK, LOW);
        digitalWrite(OE, LOW); // show new values
    }
}

/* 
Sends a value over serial with an identifying flag.
value5: value of the transmitted number, max 5 bit precision.
id3:    id flag of the number being send, max 3 bit precision.
*/
void sendWithID(int value5, int id3) {
    // shift the id flag to the three most significant bits, then join with the value.
    Serial.write((id3 << 5) ^ value5);
}

// starts a tone and sets the end time with duration added in milliseconds
void startTone(unsigned int frequency, unsigned int duration) {
    tone(BUZZER, frequency);
    toneEndTime = millis() + duration;
}

// Sets up the required variables to play a music track and starts the first note.
void startMusicTrack(struct MusicTrack* p_newTrack) {
    p_activeTrack = p_newTrack;
    musicActive = true;
    trackIndex = 0;
    startTone(p_activeTrack->notes[trackIndex][0], p_activeTrack->notes[trackIndex][1]);
}

/*
    Calculates the new position of the ball and checks for collisions with screen edges and paddles.
    returns 0 if the game continues,
    returns 1 if player one got a point and the game needs to pause,
    returns 2 if player two got a point and the game needs to pause.
*/
int updateBall(float* ballVelocity, float* ballPos, int paddleOnePos) {
    ballPos[0] += ballVelocity[0];
    ballPos[1] += ballVelocity[1];

    if (ballPos[1] < 0 || ballPos[1] > 15) {
        ballVelocity[1] *= -1;
    }

    if (ballPos[0] < 1) {
        if ((int)ballPos[1] >= paddleOnePos && paddleOnePos+PADDLE_WIDTH >= (int)ballPos[1]) {
            ballVelocity[0] *= -1;
            startMusicTrack(&HIT);
        }
        else {
            ballPos[0] = 30;
            ballVelocity[0] = DEFAULT_SPEED[0] * -1;
            ballVelocity[1] = DEFAULT_SPEED[1];
            return 2;
        }
    }
    else if (ballPos[0] > 31) {
        if ((int)ballPos[1] >= paddleTwoPos && paddleTwoPos+PADDLE_WIDTH >= (int)ballPos[1]) {
            ballVelocity[0] *= -1;
            startMusicTrack(&HIT);
        }
        else {
            ballPos[0] = 1;
            ballVelocity[0] = DEFAULT_SPEED[0];
            ballVelocity[1] = DEFAULT_SPEED[1] * -1;
            return 1;
        }
    }
    return 0;
}
