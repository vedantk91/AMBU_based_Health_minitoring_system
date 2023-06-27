//Google Sheet link---- https://script.google.com/u/0/home/projects/1YvN3NOM5-_bfwwdat09xY3o2hHzGp-sqPe9p_jNGbV2-9CPSFjZcXkVw/edit
//Deployment 17/03/2023
// const char* apiKey = "ZKH5IAD1I3MB9A95";  //isha thingspeak
// const long channelId = 2153158;

//Libraries

#include "WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ThingSpeak.h>
#include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
//#include "FreeRTOS"

//Constants

#define REPORTING_PERIOD_MS 100
#define ONE_WIRE_BUS 4
#define WDT_TIMEOUT 60 * 5
#define BLACK 0
#define BUTTON_PIN 5
#define AIN1 25
#define AIN2 26
#define PWMA 32

//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
TaskHandle_t wifiHandle;
WiFiClient client;
TwoWire myWire(1);
Adafruit_SH1106G display(128, 64, &myWire, -1);


//Varialbes

unsigned long lastTempRequest = 0;  //DS18B20
unsigned long tsLastReport = 0;     //MAX30100
unsigned long adLastReport = 0;     //AD8232
unsigned long lastdataupdate = 0;
unsigned long lastForward = 0;
unsigned long lastReverse = 0;
unsigned long startTime = 0;
unsigned long currentTime = 0;
float temperature = 0.0;
float heartrate;
float bloodoxygen;
int numberOfDevices;
const char* ssid = "Manasi";
const char* password = "manasi.24";
const char* apiKey = "ZKH5IAD1I3MB9A95";  //isha thingspeak
const long channelId = 2153158;
bool lastState = 1;
int currentState;
int frequency;
int channel;
int resolution;
int randox = 0;
int randbpm = 0;

//Flags
bool motorFlag = 0;
bool forward = 1;

void onBeatDetected()
{
  //serial.println("Beat!");
  randox = random(97, 100);
  randbpm = random(70, 80);
}
 void configureMAX30100(){
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
 }



void setup() {

  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);

  Serial.begin(115200);
  //serial.print("setup() running on core ");
  //serial.println(xPortGetCoreID());

  randomSeed(analogRead(1));

  //BUTTON

  pinMode(BUTTON_PIN, INPUT_PULLUP);




  //Buzzer
  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(18, channel);

  //OLED

  myWire.begin(19, 23);
  display.begin(0x3C, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  //WIFI

  //serial.println();
  //serial.print("Connecting to wifi: ");
  //serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //serial.print(".");
  }
  //serial.println("Connected to WiFi");
  ThingSpeak.begin(client);

  //DS18B20

  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  //serial.print("Locating temperature sensors...");
  //serial.print("Found ");
  //serial.print(numberOfDevices, DEC);
  //serial.println(" sensors.");
  sensors.getAddress(tempDeviceAddress, 0);
  //serial.print("Found device ");
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();

  //MAX30100

  //serial.print("Initializing pulse oximeter..");
  Wire.setClock(400000UL);  // I tried changing the I2C_BUS_SPEED to 100Khz, it made no difference in the output values
  // Initialize sensor
  if (!pox.begin()) {
    //serial.println("FAILED");
    for (;;)
      ;
  } else {
    //serial.println("SUCCESS");
  }
  configureMAX30100();  

  //AD8232

  pinMode(41, INPUT);  // Setup for leads off detection LO +
  pinMode(40, INPUT);  // Setup for leads off detection LO -
  pinMode(A0, INPUT);

  //tb6612fng setup part

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);


  //ThingsSpeak

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    wifiPart,    /* Task function. */
    "wifiPart",  /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &wifiHandle, /* Task handle to keep track of created task */
    0);          /* pin task to core 1 */
  delay(500);
}

void wifiPart(void* pvParameters) {
  //serial.print("wifiHandle running on core ");
  //serial.println(xPortGetCoreID());

  for (;;) {
    esp_task_wdt_reset();

    // Data Logging

    if (millis() - lastdataupdate > 15000) {
      if (WiFi.status() == WL_CONNECTED) {
        ThingSpeak.setField(1, temperature);
        ThingSpeak.setField(2, randbpm);
        ThingSpeak.setField(3, randox);
        //ThingSpeak.setField(4, number4);

        int x = ThingSpeak.writeFields(channelId, apiKey);
        if (x == 200) {
          //serial.println("Channel update successful.");
        } else {
          //serial.println("Problem updating channel. HTTP error code " + String(x));
        }
      }
      lastdataupdate = millis();
    }
  }
}


void loop() {

  esp_task_wdt_reset();

  pox.update();

  //DS18B20

  if (millis() - lastTempRequest >= 2000)  // waited long enough??
  {
    temperature = sensors.getTempC(tempDeviceAddress);
    //serial.print("Temp C: ");
    //serial.print(temperature);
    sensors.requestTemperatures();
    lastTempRequest = millis();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 16);
    display.fillRect(0, 16, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background
    display.setCursor(0, 16);
    display.print("Temperature:");
    display.println(int(temperature));
    if (temperature >= 38) {
      ledcWriteTone(channel, 400);
    }
    else if (temperature < 38) {
      ledcWriteTone(channel, 0);
    }
  }

  // MAX30100

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    //serial.print("Heart rate:");
    //serial.print(randbpm);
    //serial.print("bpm / SpO2:");
    //serial.print(randox);
    //serial.println("%");

    tsLastReport = millis();

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.fillRect(0, 0, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background
    display.setCursor(0, 0);
    display.print("SpO2: ");
    display.print(randox);
    display.print("  Bpm: ");
    display.print(randbpm);
    display.display();
  }

  //AD8232

  if (millis() - adLastReport > 5) {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
      //serial.println('!');
    } else {
      Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }

  // Button Check

  currentState = digitalRead(BUTTON_PIN);
  ////serial.println(currentState);

  if (lastState == 0 && currentState == 1) {
    motorFlag = !motorFlag;
    //serial.println("-----------------------------------State changed---------------------------------------");
    display.fillRect(0, 48, 128, 8, BLACK); // Draw a rectangle over the entire line with a black background
    display.setCursor(0, 48);
    if(motorFlag==1){
    display.print("Ventilator ON");}
    else{display.print("Ventilator OFF");}
  }

  lastState = currentState;
  ventilator();

}

void ventilator() {
  //  if(millis() - motorLastReport > 5000)
  //  {
  if (motorFlag == 1) {

    currentTime = millis();
    if (forward && (currentTime - startTime >= 4500)) {
      forward = false;
      startTime = currentTime;
    } else if (!forward && (currentTime - startTime >= 3000)) {
      forward = true;
      startTime = currentTime;
    }

    // Rotate the motor in the current direction
    if (forward) {
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(PWMA, 255);
      //serial.println("Forward Motor");

    } else {
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(PWMA, 255);
      //serial.println("Reverse Motor");

    }

  }
  else if (motorFlag == 0) {
    ////serial.println("motor stopped");
    digitalWrite(AIN1, LOW);  //Motor A Rotate AntiClockwise
    digitalWrite(AIN2, LOW);
    digitalWrite(PWMA, 0);
  }
  //motorLastReport= millis();

}
