static int CONTROLLER_ONE = A0;
static int CONTROLLER_TWO = A1;

int reading_one = 0;
int reading_two = 0;

void setup() {
  // put your setup code here, to run once:
    pinMode(CONTROLLER_ONE, INPUT);
    pinMode(CONTROLLER_TWO, INPUT);
    Serial.begin(115200);
}

void loop() {
    // put your main code here, to run repeatedly:
    reading_one = analogRead(CONTROLLER_ONE);
    reading_two = analogRead(CONTROLLER_TWO);

    Serial.print("A: ");
    Serial.print(reading_one);
    Serial.print("  B: ");
    Serial.println(reading_two);

    delay(100);
}
