// Motor Driver Connections
const int motorPin1 = 25; // Motor Driver Input 1
const int motorPin2 = 26; // Motor Driver Input 2
int PWMA = 32;

// Motor Rotation Time Variables
unsigned long startTime = 0;
unsigned long currentTime = 0;
bool forward = true; // Indicates whether the motor is currently rotating forward or reverse

void setup() {
  // Set motor driver pins as output
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // Check if it's time to change the motor direction
  currentTime = millis();
  if (forward && (currentTime - startTime >= 10000)) {
    forward = false;
    startTime = currentTime;
    Serial.println("debug1");
  } else if (!forward && (currentTime - startTime >= 4000)) {
    forward = true;
    startTime = currentTime;
    Serial.println("debug2");

  }

  // Rotate the motor in the current direction
  if (forward) {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(PWMA, 255);

    Serial.println("Forward");

  } else {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(PWMA, 255);
    Serial.println("Reverse");

  }
  delay(500);
}
