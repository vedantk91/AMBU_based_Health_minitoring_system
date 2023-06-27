void setup() {
// initialize the serial communication:
Serial.begin(9600);
pinMode(1, INPUT); // Setup for leads off detection LO +
pinMode(3, INPUT); // Setup for leads off detection LO -
pinMode(A0,INPUT);
}
 
void loop() {
 
if((digitalRead(40) == 1)||(digitalRead(41) == 1)){
Serial.println('!');
}
else{
// send the value of analog input 0:
Serial.println(analogRead(A0));
}
//Wait for a bit to keep serial data from saturating
delay(100);
}
