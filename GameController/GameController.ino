// ------  Static variables and pin assignments ------
// Shift register control pins
static int SER = 12; // data pin 
static int RCLK = 10; // register clock
static int SCLK = 11; // serial clock
static int OE = 9; // output enable

static int BUZZER = 8; // piezo
static int CONTROLLER_ONE = A0;
static int CONTROLLER_TWO = A1;

// ------- Reference numbers for game function --------
static int frameRate = 60;
static int frameTime = 1000/frameRate; // time inbetween frames.
static int resolution[2] = {32, 16};

// ------- Music tracks and sound effects --------
struct MusicTrack {
    unsigned int notes[100][2]; // each position in the array is one note which contains a frequency and duration.
    unsigned int length; // used to track when the song ends.
};

static struct MusicTrack STARTUP_JINGLE = {
    {
        {987, 200},
        {3951, 600}
    },
    2
};

static struct MusicTrack A1SCALE = {
    {
        {55, 500},
        {62, 500},
        {65, 500},
        {73, 500},
        {82, 500},
        {87, 500},
        {98, 500},
        {110, 500},
        {123, 500},
        {131, 500},
        {146, 500},
        {165, 500},
        {175, 500},
        {196, 500}
    },
    14
};

static struct MusicTrack A4SCALE = {
    {
        {440, 500},
        {493, 500},
        {523, 500},
        {587, 500},
        {659, 500},
        {698, 500},
        {784, 500},
        {880, 500},
        {988, 500},
        {1047, 500},
        {1174, 500},
        {1318, 500},
        {1396, 500},
        {1567, 500}
    },
    14
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
// Entities
unsigned int paddleOnePos = resolution[1] / 2;
unsigned int paddleTwoPos = resolution[1] / 2;
int ballPos[2] = {0, resolution[1] / 2};
// Pin Readings
int controllerOneSignal = 0;
int controllerTwoSignal = 0;
// Music tracking
bool musicActive = false;
struct MusicTrack* p_activeTrack;
int trackIndex = 0;

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

// Changed shift register outputs to match new values.
void setCounters(unsigned int playerOne, unsigned int playerTwo) {
    digitalWrite(OE, HIGH); // hide output while we are messing with it
    shiftOut(SER, SCLK, LSBFIRST, digitMappings[playerTwo]); // shift the second players score onto the first register
    shiftOut(SER, SCLK, LSBFIRST, digitMappings[playerOne]); // shift the first player's score onto first register, moving second player score to second register.
    digitalWrite(RCLK, HIGH);
    digitalWrite(RCLK, LOW);
    digitalWrite(OE, LOW); // show new values
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

// Sends a value over serial with an identifying character.
void sendWithID(int precision8, char id) {
    Serial.write(id);
    Serial.write(precision8);
}

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

    //startMusicTrack(&STARTUP_JINGLE);
    startMusicTrack(&A4SCALE);

    Serial.begin(115200);
}

void loop() {
    controllerOneSignal = analogRead(CONTROLLER_ONE);
    controllerTwoSignal = analogRead(CONTROLLER_TWO);
    paddleOnePos = map(controllerOneSignal, 0,1023, 0,9);
    paddleTwoPos = map(controllerTwoSignal, 0,1023, 0,9);
  
    // is it time to update displays?
    if (millis() > lastRefreshTime + frameTime) {
        setCounters(paddleOnePos, paddleTwoPos);
        
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
