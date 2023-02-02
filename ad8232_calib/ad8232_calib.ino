void setup() {
// initialize the serial communication:
Serial.begin(9600);
pinMode(25, INPUT); // Setup for leads off detection LO +
pinMode(26, INPUT); // Setup for leads off detection LO -
pinMode(A0,INPUT);
}
 
void loop() {
 
//if((digitalRead(10) == 1)||(digitalRead(11) == 1)){
//Serial.println('!');
//}
//else{
// send the value of analog input 0:
Serial.println(analogRead(A0));
//}
//Wait for a bit to keep serial data from saturating
delay(10);
}
