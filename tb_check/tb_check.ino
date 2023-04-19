#define AIN1 25
//#define BIN1 7
#define AIN2 26
//#define BIN2 8
#define PWMA 32
//#define PWMB 6
//#define STBY 26

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Forward");
  digitalWrite(AIN1, LOW);  //Motor A Rotate Clockwise
  digitalWrite(AIN2, HIGH);
  digitalWrite(PWMA, 255);

  delay(10000);

  Serial.println("Reverse");
  digitalWrite(AIN1, HIGH);  //Motor A Rotate AntiClockwise
  digitalWrite(AIN2, LOW);
  digitalWrite(PWMA, 255);

  delay(4000);
}