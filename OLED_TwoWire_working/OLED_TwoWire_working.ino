#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Define a new TwoWire object for the software I2C bus
TwoWire myWire(0);

// Create an SSD1306 object for the OLED display
Adafruit_SH1106G display(128, 64, &myWire, -1);
#define i2c_Address 0x3c
#define BLACK 0

void setup() {
  // Begin the software I2C bus with the desired pins
  myWire.begin(19, 23);

  // Initialize the OLED display
  display.begin(i2c_Address, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 16);
  display.print("Starting....");
  display.display();
  delay(1000);




}

void loop() {
  // Do nothing
  display.fillRect(0, 16, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background

  //  display.clearDisplay();

  for (int i = 0; i < 100; i++) {
    display.setCursor(0, 16);
    display.fillRect(0, 16, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background
    display.print(i);
    display.display();
    delay(100);

  }

    for (int i = 0; i < 100; i++) {
    display.setCursor(0, 0);
    display.fillRect(0, 0, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background
    display.print(i);
    display.display();
    delay(1000);

  }
}
