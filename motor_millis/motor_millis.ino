// define the pins for motor driver
int in1Pin = 25;
int in2Pin = 26;
int PWMA = 32;

// define the speed for the motor
int motorSpeed = 255;

// define the time duration for each direction
unsigned long forwardDuration = 10000; // 10 seconds
unsigned long reverseDuration = 4000; // 4 seconds

// define the start time for each direction
unsigned long forwardStartTime=0;
unsigned long reverseStartTime=10000;

void setup() {
  // set the motor driver pins as output
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(PWMA, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  // check if it's time to start moving forward
  if (millis() - forwardStartTime >= forwardDuration) {
    // stop the motor
    digitalWrite(in1Pin, 0);
    digitalWrite(in2Pin, 0);
    Serial.println("motor is stopped 1");
    // set the start time for reverse direction
    reverseStartTime = millis();
  }
  // check if it's time to start moving in reverse direction
   if (millis() - reverseStartTime >= reverseDuration && millis() - forwardStartTime >= forwardDuration) {
    // stop the motor
    digitalWrite(in1Pin, 0);
    digitalWrite(in2Pin, 0);
    Serial.println("motor is stopped 2 ");
    // set the start time for forward direction
    forwardStartTime = millis();
  }
  // move the motor forward
  else if (millis() - reverseStartTime >= 0) {
    Serial.println("Motor in direction Reverse");
    digitalWrite(in1Pin, 0);
    digitalWrite(in2Pin, 1);
    digitalWrite(PWMA, motorSpeed);
  }
  // move the motor in reverse direction
  else {
    Serial.println("Motor in direction Forward");
    digitalWrite(in1Pin, 1);
    digitalWrite(in2Pin, 0);
    digitalWrite(PWMA, motorSpeed);
  }
}
    
