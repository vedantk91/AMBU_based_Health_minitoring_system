#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//TwoWire myWire(0);

// Define SDA and SCL pins for software I2C
//#define SDA_PIN 23
//#define SCL_PIN 19
  LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize SoftWire object with software I2C
//LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Start software I2C communication
//  myWire.begin(SCL_PIN, SDA_PIN);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

 Serial.begin(9600);
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  Serial.println("Starting LCD");
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check1");
  Serial.println("Starting LCD1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check2");
  Serial.println("Starting LCD2");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check3");
  Serial.println("Starting LCD3");
  delay(1000);
}
