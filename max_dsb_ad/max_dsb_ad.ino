
// Sample of using Async reading of Dallas Temperature Sensors
//
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 36

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;

int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;                    //??
float temperature = 0.0;
int  idle = 0;
//
// SETUP
//
// Callback routine is executed when a pulse is detected
void onBeatDetected() {
  Serial.println("Beat!");
}
void configureMax30100() {
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Dallas Temperature Control Library - Async Demo");
  Serial.print("Library Version: ");
  Serial.println(DALLASTEMPLIBVERSION);
  Serial.println("\n");

  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  delayInMillis = 750 / (1 << (12 - resolution));     //??  
  lastTempRequest = millis();

  //pinMode(13, OUTPUT);

  Serial.print("Initializing pulse oximeter..");
  Wire.setClock(400000UL); // I tried changing the I2C_BUS_SPEED to 100Khz, it made no difference in the output values
  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  configureMax30100();


  //ad8232
  pinMode(40, INPUT); // Setup for leads off detection LO +
  pinMode(41, INPUT); // Setup for leads off detection LO -
  pinMode(A0,INPUT);

}





void loop(void)
{

  if (millis() - lastTempRequest >= 2000) // waited long enough??
  {
    //digitalWrite(13, LOW);
    Serial.print(" Temperature: ");
    temperature = sensors.getTempCByIndex(0);
    Serial.println(temperature, resolution - 8);
    Serial.print("  Resolution: ");
    Serial.println(resolution);
    Serial.print("Idle counter: ");     //??
    Serial.println(idle);                //??
    Serial.println();

    idle = 0;

    // immediately after fetching the temperature we request a new sample
    // in the async modus
    // for the demo we let the resolution change to show differences
    resolution++;
    if (resolution > 12) resolution = 9;

    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - resolution));    //??
    lastTempRequest = millis();
  }

  //digitalWrite(13, HIGH);
  // we can do usefull things here
  // for the demo we just count the idle time in millis
  delay(1);
  idle++;

  pox.update();

  // Grab the updated heart rate and SpO2 levels

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    tsLastReport = millis();
  }


   //ad8232

  if((digitalRead(40) == 1)||(digitalRead(41) == 1)){
     Serial.println('!');
  }
  else{
    //send the value of analog input 0:
    Serial.println(analogRead(A0));
  }
//Wait for a bit to keep serial data from saturating
  delay(10);


 

}