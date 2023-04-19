//Libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <LiquidCrystal_I2C.h>


//Constants

#define REPORTING_PERIOD_MS     1000
#define ONE_WIRE_BUS 4

float heartrate;
float bloodoxygen;

//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
LiquidCrystal_I2C lcd(0x27, 16, 2);


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

void setup(void)
{
  Serial.begin(115200);

  //LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  Serial.println("Starting LCD");

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
  Wire.setClock(400000UL); // I tried changing the I2C_BUS_SPEED to 100Khz, it made no difference in the output values
  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  configureMax30100();


  //AD8232

  pinMode(40, INPUT); // Setup for leads off detection LO +
  pinMode(41, INPUT); // Setup for leads off detection LO -
  pinMode(A0, INPUT);

}


void loop(void)
{

  //DS18B20

  if (millis() - lastTempRequest >= 2000) // waited long enough??
  {
    float tempC = sensors.getTempC(tempDeviceAddress);
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempC));
    sensors.requestTemperatures();
    lastTempRequest = millis();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(tempC);
  }


  // MAX30100

  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      Serial.print("Heart rate:");
      heartrate = pox.getHeartRate();
      Serial.print(heartrate);
      Serial.print("bpm / SpO2:");
      bloodoxygen = pox.getSpO2();
      Serial.print(bloodoxygen);
      Serial.println("%");

      tsLastReport = millis(); 
      lcd.setCursor(0, 1);
      lcd.print("SpO2:");
      lcd.print(int(bloodoxygen));
      lcd.print(" Bpm:");
      lcd.print(int(heartrate));
  }


  //AD8232

  if (millis() - adLastReport > 10) {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
      Serial.println('!');
    }
    else {
      Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }



}
