
#define AIN1 32
//#define BIN1 7
#define AIN2 33
//#define BIN2 8
#define PWMA 25
//#define PWMB 6
//#define STBY 26


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PWMA,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  

}

void loop() {
  // put your main code here, to run repeatedly:
   Serial.println("Forward");
   digitalWrite(AIN1,HIGH); //Motor A Rotate Clockwise
   digitalWrite(AIN2,LOW);
   
   delay(3000);

   Serial.println("Reverse");
   digitalWrite(AIN1,LOW); //Motor A Rotate AntiClockwise
   digitalWrite(AIN2,HIGH);
   delay(3000);   


}
