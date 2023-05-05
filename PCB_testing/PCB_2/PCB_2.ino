//Libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


//Constants

#define REPORTING_PERIOD_MS 100
#define ONE_WIRE_BUS 4
#define BUTTON_PIN 5
#define AIN1 25
#define AIN2 26
#define PWMA 32
#define BLACK 0


//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
TwoWire myWire(1);
Adafruit_SH1106G display(128, 64, &myWire, -1);



//Varialbes

float heartrate;
float bloodoxygen;
unsigned long lastTempRequest = 0;
unsigned long tsLastReport = 0;
unsigned long adLastReport = 0;
unsigned long motorLastReport = 0;
unsigned long lastForward = 0;
unsigned long lastReverse = 0;
unsigned long startTime = 0;
unsigned long currentTime = 0;
float temperature = 0.0;
int numberOfDevices;
int lastState = HIGH;
int currentState;
bool motorFlag = 0;
bool forward = 1;


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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  //OLED

  myWire.begin(19, 23);
  display.begin(0x3C, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();


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


  //tb6612fng setup part
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
}


void loop(void) {

  //DS18B20

  if (millis() - lastTempRequest >= 2000)  // waited long enough??
  {
    float temperature = sensors.getTempC(tempDeviceAddress);
    Serial.print("Temp C: ");
    Serial.print(temperature);
    sensors.requestTemperatures();
    lastTempRequest = millis();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 16);
    display.fillRect(0, 16, 128, 8, BLACK);  // Draw a rectangle over the entire line with a black background
    display.setCursor(0, 16);
    display.print("Temperature: ");
    display.println(int(temperature));
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

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.fillRect(0, 0, 128, 8, BLACK);  // Draw a rectangle over the entire line with a black background
    display.setCursor(0, 0);
    display.print("SpO2: ");
    display.print(int(bloodoxygen));
    display.print("  Bpm: ");
    display.print(int(heartrate));
    display.display();
  }


  //AD8232

  if (millis() - adLastReport > 10) {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
      Serial.println('!');
    } else {
      //Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }

  // TB6612FNG

  currentState = digitalRead(BUTTON_PIN);
  //Serial.println(currentState);

  if (lastState == LOW && currentState == HIGH) {
    motorFlag = !motorFlag;
    Serial.println("------------------------state changed--------------------------------");
  }
  
  lastState = currentState;

  if (motorFlag == 1) {

    currentTime = millis();
    if (forward && (currentTime - startTime >= 10000)) {
      forward = false;
      startTime = currentTime;
    } else if (!forward && (currentTime - startTime >= 4000)) {
      forward = true;
      startTime = currentTime;
    }

    // Rotate the motor in the current direction
    if (forward) {
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(PWMA, 255);
      Serial.println("Forward Motor");

    } else {
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(PWMA, 255);
      Serial.println("Reverse Motor");
    }

  } else if (motorFlag == 0) {
    Serial.println("motor stopped");
    digitalWrite(AIN1, LOW);  //Motor A Rotate AntiClockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 0);
  }
}
