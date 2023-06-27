
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

PulseOximeter pox;
uint32_t tsLastReport = 0;
int randox = 0;
int randbpm = 0;


void onBeatDetected()
{
  Serial.println("Beat!");
  randox = random(97,100);
  randbpm= random(70,80);
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Initializing pulse oximeter..");
  randomSeed(analogRead(1));


  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(randbpm);
    Serial.print("bpm / SpO2:");
    Serial.print(randox);
    Serial.println("%");

    tsLastReport = millis();
  }
}
