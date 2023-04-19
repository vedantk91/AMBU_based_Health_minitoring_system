//Google Sheet link---- https://script.google.com/u/0/home/projects/1YvN3NOM5-_bfwwdat09xY3o2hHzGp-sqPe9p_jNGbV2-9CPSFjZcXkVw/edit
//Deployment 17/03/2023

//Libraries

#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
//#include "FreeRTOS"

//Constants

#define REPORTING_PERIOD_MS 1000
#define ONE_WIRE_BUS 4

//Objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
TaskHandle_t wifiHandle;
TaskHandle_t hardwareHandle;

//Varialbes

unsigned long lastTempRequest = 0;  //DS18B20
unsigned long tsLastReport = 0;     //MAX30100
unsigned long adLastReport = 0;     //AD8232
unsigned long lastdataupdate = 0;
float temperature = 0.0;
float heartrate;
float bloodoxygen;
int numberOfDevices;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
const char* ssid = "OnePlus9";      // WIFI SSID
const char* password = "leena123";  // WIFI Password
//String GOOGLE_SCRIPT_ID = "AKfycbwRIdMjEqhbyCGFt_R2OyNtL_EUyzU29-vfzKrqKcI4Atq_QhQHEz9wL-923xzknhqj";    // Gscript ID
String GOOGLE_SCRIPT_ID = "AKfycbzigZVMHqNCZE-73yzYZf4E2TiLE7UR_fYZ0qPzhEb1FewGsCuGHEF9Pn6gFx2HcaOl";  // Sensors only


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

  //Time

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    hardware,        /* Task function. */
    "hardware",      /* name of task. */
    10000,           /* Stack size of task */
    NULL,            /* parameter of the task */
    1,               /* priority of the task */
    &hardwareHandle, /* Task handle to keep track of created task */
    0);              /* pin task to core 0 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    wifiPart,    /* Task function. */
    "wifiPart",  /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &wifiHandle, /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);





}



void hardware(void* pvParameters) {
  Serial.print("hardwareHandle running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {

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
}

void wifiPart(void* pvParameters) {
  Serial.print("wifiHandle running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    // Data Logging

    if (millis() - lastdataupdate > 1000) {
      if (WiFi.status() == WL_CONNECTED) {
        static bool flag = false;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
          Serial.println("Failed to obtain time");
          return;
        }
        char timeStringBuff[50];  //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
        String asString(timeStringBuff);
        asString.replace(" ", "-");
        Serial.print("Time:");
        Serial.println(asString);
        String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "date=" + asString + "&temperature=" + String(temperature) + "&heartrate=" + String(heartrate) + "&bloodoxygen=" + String(bloodoxygen);
        Serial.print("POST data to spreadsheet:");
        Serial.println(urlFinal);
        HTTPClient http;
        http.begin(urlFinal.c_str());
        //http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        Serial.print("HTTP Status Code: ");
        Serial.println(httpCode);
        //---------------------------------------------------------------------
        //getting response from google sheet
        String payload;
        if (httpCode > 0) {
          payload = http.getString();
          Serial.println("Payload: " + payload);
        }
        //---------------------------------------------------------------------
        http.end();
      }
      lastdataupdate = millis();
    }
  }
}


void loop() {
}
