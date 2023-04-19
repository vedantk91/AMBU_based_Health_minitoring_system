//Libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <LiquidCrystal_I2C.h>


//Constants

#define REPORTING_PERIOD_MS 1000
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
unsigned long motorLastReport = 0;
unsigned long lastForward = 0;
unsigned long lastReverse = 0;
unsigned long startTime = 0;
unsigned long currentTime = 0;
float temperature = 0.0;
int numberOfDevices;
bool lastState = 0;
int currentState;
int frequency;
int channel;
int resolution;
int temphighcounter = 0;
int tempCriticalCounter = 0;
int bpmHighCounter = 0;
int spoLowCounter = 0;
int spoCriticalCounter = 0;
int bpmLowCounter = 0;


//flags
bool motorFlag = 0;
bool SensitiveOxygen = 0;
bool CriticalOxygen = 0;
bool SensitiveBpm = 0;
bool CriticalBpm = 0;
bool SensitiveTemp = 0;
bool CriticalTemp = 0;
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

  //Buzzer
  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(18, channel);

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

    if (temperature >= 38 && temperature < 38.5) {
      temphighcounter++;
      if (temphighcounter >= 10) {
        SensitiveTemp = 1;
        CriticalTemp = 0;
        Serial.println("High Temperature");
      }
    }
    else if (temperature >= 38.5) {
      tempCriticalCounter++;
      if (tempCriticalCounter >= 10) {
        CriticalTemp = 1;
        SensitiveTemp = 0;
        Serial.println("Critical Temperature");
      }
    }
    else {
      Serial.println("Normal Healthy Temperature");
      tempCriticalCounter = 0;
      temphighcounter = 0;
      CriticalTemp = 0;
      SensitiveTemp = 0;
    }


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

    //buzzer_spo2 critical condition

    if (heartrate > 130) {
      bpmHighCounter++;
      if (bpmHighCounter >= 20) {
        SensitiveBpm = 1;
        CriticalBpm = 0;
        Serial.println("High BPM");
      }
    }

    else if (heartrate < 60) {
      bpmLowCounter++;
      if (bpmLowCounter = 20) {
        CriticalBpm = 1;
        SensitiveBpm = 0;
        bpmHighCounter = 0;
        Serial.println("Low BPM");
        motorFlag = 1;
      }
    }

    else {
      Serial.println("Normal BPM");
      SensitiveBpm = 0;
      CriticalBpm = 0;
      bpmLowCounter = 0;
      bpmHighCounter = 0;
    }

    if ((bloodoxygen < 93) && (bloodoxygen > 90)) {
      spoLowCounter++;
      if (spoLowCounter = 20) {
        SensitiveOxygen = 1;
        CriticalOxygen = 0;
        //      spoCriticalCounter = 0;
        Serial.println("Low SPO2");
      }
    }

    else if (bloodoxygen < 90) {
      spoCriticalCounter++;
      if (spoCriticalCounter >= 20) {
        CriticalOxygen = 1;
        SensitiveOxygen = 0;
        //      spoLowCounter = 0;
        Serial.println("Critical SPO2");
        motorFlag = 1;
      }
    }

    else if (bloodoxygen >= 93) {
      Serial.println("Normal SPO2");
      SensitiveOxygen = 0;
      CriticalOxygen = 0;
      spoCriticalCounter = 0;
      spoLowCounter = 0;
    }
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


  //Flag check

  if (((SensitiveOxygen == 1) || (SensitiveBpm == 1) || (SensitiveTemp == 1)) && ((CriticalOxygen == 0) &&  (CriticalBpm == 0))) {
    ledcWriteTone(channel, 400);
    Serial.println("Attention required");
  }


  else if ((CriticalOxygen == 1) || (CriticalBpm == 1)) {
    ledcWriteTone(channel, 800);
    motorFlag = 1;
    Serial.println("Critical Situation Ventilator will turn ON");

  }

  else if ((SensitiveOxygen == 0) && (SensitiveBpm == 0) && (SensitiveTemp == 0) && (CriticalOxygen == 0) && (CriticalBpm == 0))
  { ledcWriteTone(channel, 0);
    Serial.println("Healthy Situation");

  }

  // Button Check

  currentState = digitalRead(BUTTON_PIN);
  //Serial.println(currentState);

  if (lastState == 0 && currentState == 1) {
    motorFlag = !motorFlag;
  }

  //  }
  lastState = currentState;
  ventilator();

}



void ventilator() {
  //  if(millis() - motorLastReport > 5000)
  //  {
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

  }
  else if (motorFlag == 0) {
    Serial.println("motor stopped");
    digitalWrite(AIN1, LOW);  //Motor A Rotate AntiClockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 0);
  }
  //motorLastReport= millis();
}
