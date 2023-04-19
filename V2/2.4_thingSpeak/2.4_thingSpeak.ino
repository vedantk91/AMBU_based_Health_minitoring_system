//Google Sheet link---- https://script.google.com/u/0/home/projects/1YvN3NOM5-_bfwwdat09xY3o2hHzGp-sqPe9p_jNGbV2-9CPSFjZcXkVw/edit
//Deployment 17/03/2023

//Libraries

#include "WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ThingSpeak.h>
#include <esp_task_wdt.h>


//#include "FreeRTOS"

//Constants

#define REPORTING_PERIOD_MS 100
#define ONE_WIRE_BUS 4
#define WDT_TIMEOUT 60 * 5

//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
TaskHandle_t wifiHandle;
WiFiClient client;



//Varialbes

unsigned long lastTempRequest = 0;  //DS18B20
unsigned long tsLastReport = 0;     //MAX30100
unsigned long adLastReport = 0;     //AD8232
unsigned long lastdataupdate = 0;
float temperature = 0.0;
float heartrate;
float bloodoxygen;
int numberOfDevices;
const char* ssid = "Manasi";
const char* password = "manasi.24";
const char* apiKey = "1DNTAUBVIAL7N0C6";
const long channelId = 2108287;


void onBeatDetected() {
  Serial.println("Beat!");
}

void configureMax30100() {
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
}



void setup() {

  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);

  Serial.begin(115200);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());


  //WIFI

  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  ThingSpeak.begin(client);


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

  pinMode(41, INPUT);  // Setup for leads off detection LO +
  pinMode(40, INPUT);  // Setup for leads off detection LO -
  pinMode(A0, INPUT);

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
  Serial.print("wifiHandle running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    esp_task_wdt_reset();

    // Data Logging

    if (millis() - lastdataupdate > 15000) {
      if (WiFi.status() == WL_CONNECTED) {
        ThingSpeak.setField(1, temperature);
        ThingSpeak.setField(2, heartrate);
        ThingSpeak.setField(3, bloodoxygen);
        //ThingSpeak.setField(4, number4);

        int x = ThingSpeak.writeFields(channelId, apiKey);
        if (x == 200) {
          Serial.println("Channel update successful.");
        } else {
          Serial.println("Problem updating channel. HTTP error code " + String(x));
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
    Serial.print("Temp C: ");
    Serial.print(temperature);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(temperature));
    sensors.requestTemperatures();
    lastTempRequest = millis();
  }


  // MAX30100

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    heartrate = pox.getHeartRate();
    Serial.print(heartrate);
    Serial.print("bpm / SpO2:");
    bloodoxygen = pox.getSpO2();
    Serial.print(bloodoxygen);
    Serial.println("%");

    tsLastReport = millis();
  }


  //AD8232

  if (millis() - adLastReport > 10) {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
      Serial.println('!');
    } else {
      Serial.println(analogRead(A0));
    }
    adLastReport = millis();
  }
}