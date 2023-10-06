static int SER = 12;
static int RCLK = 11;
static int SCLK = 10;
static int OE = 9;

int counter = 0;

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

void setup() {
    pinMode(SER, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(OE, OUTPUT);

    digitalWrite(OE, LOW);
    digitalWrite(RCLK, LOW);
    shiftOut(SER, SCLK, MSBFIRST, 0b11111111);
    digitalWrite(RCLK, HIGH);
    delay(1);
    digitalWrite(RCLK, LOW);
}

void loop() {
    shiftOut(SER, SCLK, LSBFIRST, digitMappings[counter]);
    digitalWrite(RCLK, HIGH);
    delay(1);
    digitalWrite(RCLK, LOW);
    delay(500);
    counter++;
    if (counter > 9) {counter = 0;}
}
