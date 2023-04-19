#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "TwoWire.h"


// Set the I2C address of the LCD
#define I2C_ADDR 0x3F

// Set the dimensions of the LCD
#define LCD_COLS 16
#define LCD_ROWS 2

// Set the pins for SDA and SCL
#define LCD_SDA_PIN 19
#define LCD_SCL_PIN 23

// Initialize the TwoWire object with the specified SDA and SCL pins
TwoWire Wire2(LCD_SDA_PIN, LCD_SCL_PIN);

// Initialize the LCD object with the TwoWire object and the I2C address and dimensions
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS, &Wire2);

void setup() {
  // Start the serial communication
  Serial.begin(9600);

  // Initialize the TwoWire object
  Wire2.begin();

  // Initialize the LCD object
  lcd.init();

  // Turn on the backlight
  lcd.backlight();

  // Clear the LCD screen
  lcd.clear();

  // Print a message on the LCD screen
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");
}

void loop() {
  // Do nothing in the loop
}
