// Pre Deployment
// Google Sheet link---- https://docs.google.com/spreadsheets/d/1SeHCbwECzW-PCDrUTJRjPD4zlu2xblFqK190mxi6c8U/edit#gid=0
// Deployment 17/03/2023

// Libaries

#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Constants

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
const char *ssid = "Manasi"; // WiFi credentials - Vedant Laptop
const char *password = "manasi.24";
// String GOOGLE_SCRIPT_ID = "AKfycbzLlxH3XT5lwav2IDrzi2fvK-Vlin2yoNkzmvfFT18koVuP8MaOkB5ppMsJ7NBxTrii1A";    // Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycbyxkSjINgfvgcDliGRtJ8-xyyw5TyXZilUIIxYY7Ac01wrI5tcGKVk2gT3Pzof_ueWNUQ";
#define ONE_WIRE_BUS 36
#define AIN1 25
#define AIN2 26
#define PWMA 32

// Variables

int count = 0;
int resolution = 12;
unsigned long lastTempRequest = 0;
int delayInMillis = 0;
float temperature = 0.0;
int idle = 0;
uint32_t tsLastReport = 0;
unsigned long lastValueAD = 0;

// objects

PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;

void onBeatDetected()
{
  Serial.println("Beat!");
}

void configureMax30100()
{
  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // DS18B20

  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  delayInMillis = 750 / (1 << (12 - resolution));

  // MAX300100

  Serial.print("Initializing pulse oximeter..");
  Wire.setClock(400000UL); // I tried changing the I2C_BUS_SPEED to 100Khz, it made no difference in the output values
  // Initialize sensor
  if (!pox.begin())
  {
    Serial.println("FAILED");
    for (;;)
      ;
  }
  else
  {
    Serial.println("SUCCESS");
  }

  configureMax30100();

  // AD8232

  pinMode(40, INPUT); // Setup for leads off detection LO +
  pinMode(41, INPUT); // Setup for leads off detection LO -
  pinMode(A0, INPUT);

  // TB6612

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
}
void loop()
{

  if (millis() - lastTempRequest >= 2000) // waited long enough??
  {
    // digitalWrite(13, LOW);
    Serial.print(" Temperature: ");
    temperature = sensors.getTempCByIndex(0);
    Serial.println(temperature, resolution - 8);
    Serial.print("  Resolution: ");
    Serial.println(resolution);
    Serial.print("Idle counter: "); //??
    Serial.println(idle);           //??
    Serial.println();

    idle = 0;

    // immediately after fetching the temperature we request a new sample
    // in the async modus
    // for the demo we let the resolution change to show differences
    resolution++;
    if (resolution > 12)
      resolution = 9;

    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - resolution)); //??
    lastTempRequest = millis();
  }
  idle++;

  // max30100

  pox.update();

  // Grab the updated heart rate and SpO2 levels

  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    tsLastReport = millis();
  }

  // ad8232

  if (millis() - lastValueAD >= 10)
  {
    if ((digitalRead(40) == 1) || (digitalRead(41) == 1))
    {
      Serial.println('!');
    }
    else
    {
      // send the value of analog input 0:
      Serial.println(analogRead(A0));
    }

    lastValueAD = millis();
  }

  // tb6612fng

  // put condition here for push button
  Serial.println("Forward");
  digitalWrite(AIN1, HIGH); // Motor A Rotate Clockwise
  digitalWrite(AIN2, LOW);
  digitalWrite(PWMA, 255);
  delay(3000);

  Serial.println("Reverse");
  digitalWrite(AIN1, LOW); // Motor A Rotate AntiClockwise
  digitalWrite(AIN2, HIGH);
  digitalWrite(PWMA, 255);
  delay(3000);

  if (WiFi.status() == WL_CONNECTED)
  {
    static bool flag = false;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      return;
    }
    char timeStringBuff[50]; // 50 chars should be enough
    strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    String asString(timeStringBuff);
    asString.replace(" ", "-");
    Serial.print("Time:");
    Serial.println(asString);
    String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "date=" + asString + "&temperature=" + String(temperature) + "&heartrate" + String(pox.getHeartRate()) + "&SPo2" + String(pox.getSpO2());
    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    // http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    // getting response from google sheet
    String payload;
    if (httpCode > 0)
    {
      payload = http.getString();
      Serial.println("Payload: " + payload);
    }
    //---------------------------------------------------------------------
    http.end();
  }
  count++;
}