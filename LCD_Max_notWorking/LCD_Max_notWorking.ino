#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

TwoWire myWire(1);

// Define SDA and SCL pins for software I2C
//#define SDA_PIN 23
//#define SCL_PIN 19
LiquidCrystal_I2C lcd(0x27, 16, 2);


#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat!");
}


// Initialize SoftWire object with software I2C
//LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Start software I2C communication
  //  myWire.begin(SCL_PIN, SDA_PIN);
  myWire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  Serial.begin(9600);
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  Serial.println("Starting LCD");

  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);

}

void loop() {

  // Make sure to call update as fast as possible
  pox.update();
  //Serial.println(millis() - tsLastReport);
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    float heartrate = pox.getHeartRate();
    Serial.print("Heart rate:");
    Serial.print(heartrate);
    Serial.print("bpm / SpO2:");
    float bloodoxygen = pox.getSpO2();
    Serial.print(bloodoxygen);
    Serial.println("%");

    tsLastReport = millis();
    //Serial.println(tsLastReport);
    lcd.setCursor(0, 1);
    lcd.print("SpO2:");
    lcd.print(int(bloodoxygen));
    lcd.print(" Bpm:");
    lcd.print(int(heartrate));
  }
}
