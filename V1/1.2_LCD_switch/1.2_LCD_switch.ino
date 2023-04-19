//Libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <LiquidCrystal_I2C.h>


//Constants

#define REPORTING_PERIOD_MS     1000
#define ONE_WIRE_BUS 4
#define BUTTON_PIN 23
#define AIN1 25
#define AIN2 26
#define PWMA 32



//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Varialbes

float heartrate;
float bloodoxygen;
unsigned long lastTempRequest = 0;
unsigned long tsLastReport = 0;
unsigned long adLastReport = 0;
unsigned long motorLastReport =0;
float temperature = 0.0;
int numberOfDevices;
int lastState = HIGH;
int currentState;
bool motorFlag = 0;


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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
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


  //tb6612fng setup part
  pinMode(PWMA,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT); 
}


void loop(void)
{

  //DS18B20

  if (millis() - lastTempRequest >= 2000) // waited long enough??
  {
    float temperature = sensors.getTempC(tempDeviceAddress);
    Serial.print("Temp C: ");
    Serial.print(temperature);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(temperature));
    sensors.requestTemperatures();
    lastTempRequest = millis();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
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
      //Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }

  // TB6612FNG

  currentState = digitalRead(BUTTON_PIN);
  //Serial.println(currentState);

  if (lastState == LOW && currentState == HIGH)
  {
    motorFlag = ! motorFlag;
  }
//  if(millis() - motorLastReport > 5000)
//  {
  if (motorFlag == 1) {

    Serial.println("motor started");
    digitalWrite(AIN1, HIGH);  //Motor A Rotate Clockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 255);
  }
  else if (motorFlag == 0) {
    Serial.println("motor stopped");
    digitalWrite(AIN1, LOW);  //Motor A Rotate AntiClockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 0);
  }
//motorLastReport= millis();
//  }
  lastState = currentState;

}
