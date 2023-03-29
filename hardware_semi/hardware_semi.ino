//Libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

//Constants

#define REPORTING_PERIOD_MS 1000
#define ONE_WIRE_BUS 4
#define AIN1 25
#define AIN2 26
#define PWMA 32
#define PIN5 5
#define pin2 2

//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;

//Varialbes

unsigned long lastTempRequest = 0;
unsigned long tsLastReport = 0;
unsigned long adLastReport = 0;
float temperature = 0.0;
int numberOfDevices;


void onBeatDetected() {
  Serial.println("Beat!");
}

void configureMax30100() {
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void setup(void) {
  Serial.begin(115200);

  //DS18B20

  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  Serial.print("Locating temperature sensors...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" sensors.");
  sensors.getAddress(tempDeviceAddress, 0);
  Serial.print("Found device ");
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();

  //MAX30100

  Serial.print("Initializing pulse oximeter..");
  Wire.setClock(400000UL);  // I tried changing the I2C_BUS_SPEED to 100Khz, it made no difference in the output values
  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
  }
  configureMax30100();


  //AD8232

  pinMode(40, INPUT);  // Setup for leads off detection LO +
  pinMode(41, INPUT);  // Setup for leads off detection LO -
  pinMode(A0, INPUT);


  //TB6612FNG

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PIN5, HIGH);  //just for debugging
  pinMode(pin2, INPUT);
}


void loop(void) {

  //DS18B20

  if (millis() - lastTempRequest >= 2000)  // waited long enough??
  {
    float tempC = sensors.getTempC(tempDeviceAddress);
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempC));
    sensors.requestTemperatures();
    lastTempRequest = millis();
  }


  // MAX30100

  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    tsLastReport = millis();
  }


  //AD8232

  if (millis() - adLastReport > 10) {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
      Serial.println('!');
    } else {
      Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }


  // TB6612FNG

  if (pin2 == HIGH) {
    Serial.println("motor started");
    digitalWrite(AIN1, HIGH);  //Motor A Rotate Clockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 255);
        
  } else {
    Serial.println("motor stopped");
    digitalWrite(AIN1, LOW);  //Motor A Rotate AntiClockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 0);
    
  }
  
}
