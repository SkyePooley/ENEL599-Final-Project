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
static int OE = 9; // output enable
static int SER_PIN = 12;
static int SCLK_PIN = 11;
static int RCLK_PIN = 10;
static int OE_PIN = 9;
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

/*
    Keeps track of the player scores and manages the 7-segment displays.
    SER_PIN:    pin which carries the serial data to the shift registers
    SCLK_PIN:   pin which carries the serial clock 
    RCLK_PIN:   pin chich carries the register clock to move data to the output registers
    OE_PIN:     pin which enables and disables the register outputs, active low.
*/
class ScoreCounter {
    private:
        // Each bit in these bytes represents one segment of a 7-segment display. 
        // The final bit is the decimal point which is not used.
        // When referencing this array, the index number corresponds to the number displayed using that byte.
        const unsigned char digitMappings[10] = {
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
        int SER;
        int SCLK;
        int RCLK;
        int OE;
        int scores[2];
    public:
        ScoreCounter(int SER_PIN, int SCLK_PIN, int RCLK_PIN, int OE_PIN) {
            SER = SER_PIN;
            SCLK = SCLK_PIN;
            RCLK = RCLK_PIN;
            OE = OE_PIN;
            scores[0] = 0;
            scores[1] = 0;
        }

        /* Increase the first player's score unless they have already reached the winning score.
           Updates the 7-segment displays with the new value. */
        void incrementPlayerOneScore() {
            if (scores[0] < WIN_SCORE) {
                scores[0]++;
            }
            refresh();
        }

        // Increase the second player's score unless they have already reached the winning score.
        void incrementPlayerTwoScore() {
            if (scores[1] < WIN_SCORE) {
                scores[1]++;
            }
            refresh();
        }

        int getPlayerOneScore() {
            return scores[0];
        }

        int getPlayerTwoScore() {
            return scores[1];
        }

        // Clears all data in the shift registers setting all segments to off.
        void clearRegisters() {
            // clear shift registers
            digitalWrite(OE, HIGH); // Disable output
            digitalWrite(RCLK, LOW);
            shiftOut(SER, SCLK, MSBFIRST, 0b00000000); 
            shiftOut(SER, SCLK, MSBFIRST, 0b00000000);
            digitalWrite(RCLK, HIGH); // shift new data to output register.
            delay(1);
            digitalWrite(RCLK, LOW);
            digitalWrite(OE, LOW); // Show output again
        }
        /* 
            Chang shift register outputs to match new values.
            Maximum value of nine.
        */
        void refresh() {
            if (scores[0] <= 9 && scores[1] <= 9) {
                digitalWrite(OE, HIGH); // hide output while we are messing with it
                // shift the second players score onto the first register
                shiftOut(SER, SCLK, LSBFIRST, digitMappings[scores[0]]); 
                // shift the first player's score onto first register, moving second player score to second register.
                shiftOut(SER, SCLK, LSBFIRST, digitMappings[scores[1]]); 
                digitalWrite(RCLK, HIGH);
                digitalWrite(RCLK, LOW);
                digitalWrite(OE, LOW); // show new values
            }
        }
        /*
            Set the counters back to zero.
        */
        void reset() {
            clearRegisters();
            scores[0] = 0;
            scores[1] = 0;
            refresh();
        }
};


// ------  Dynamic Variables  --------
// Scores
ScoreCounter scoreCounters(SER_PIN, SCLK_PIN, RCLK_PIN, OE_PIN);

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
void startTone(unsigned int frequency, unsigned int duration);
void startMusicTrack(struct MusicTrack* p_newTrack);
void sendWithID(int value5, int id3);
int updateBall(float* ballVelocity, float* ballPos, int paddleOnePos);


void setup() {
    pinMode(CONTROLLER_ONE, INPUT);
    pinMode(CONTROLLER_TWO, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(SER_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);
    pinMode(SCLK_PIN, OUTPUT);
    pinMode(OE_PIN, OUTPUT);

    scoreCounters.refresh();
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
                    scoreCounters.incrementPlayerOneScore();
                    gameActive = false;
                    playerToServe = 1;
                    break;
                case 2:
                    startMusicTrack(&MISS);
                    scoreCounters.incrementPlayerTwoScore();
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

        if ((scoreCounters.getPlayerOneScore() > 3 || scoreCounters.getPlayerTwoScore() > 3) && !gameWon) {
            gameWon = true;
            startMusicTrack(&WIN);
        }

        // send entity positions to graphics controller
        sendWithID(paddleOnePos, FLAG_PADDLE_A);
        sendWithID(paddleTwoPos, FLAG_PADDLE_B);
        sendWithID((int)ballPos[0], FLAG_BALL_X);
        sendWithID((int)ballPos[1], FLAG_BALL_Y);

        // update 7-segment displays
        //setCounters(playerOneScore, playerTwoScore);

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
