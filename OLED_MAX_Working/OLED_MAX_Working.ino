#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "MAX30100_PulseOximeter.h"


#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat!");
}

// Define a new TwoWire object for the software I2C bus
TwoWire myWire(1);

// Create an SSD1306 object for the OLED display
Adafruit_SH1106G display(128, 64, &myWire, -1);
//#define i2c_Address 0x3c

void setup() {
  // Begin the software I2C bus with the desired pins
  Serial.begin(115200);

  myWire.begin(19, 23);

  // Initialize the OLED display
  display.begin(0x3C, 0x3C);
  display.clearDisplay();



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
  // Do nothing
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
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print("SpO2:");
    display.println(int(bloodoxygen));
    display.println();
    display.print(" Bpm:");
    display.println(int(heartrate));
    display.display();
      display.clearDisplay();
  }

}
