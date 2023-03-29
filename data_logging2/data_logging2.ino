//Include required libraries
//Google Sheet link---- https://script.google.com/u/0/home/projects/1YvN3NOM5-_bfwwdat09xY3o2hHzGp-sqPe9p_jNGbV2-9CPSFjZcXkVw/edit
//Deployment 17/03/2023
#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
// WiFi credentials
const char* ssid = "Manasi";         // change SSID
const char* password = "manasi.24";    // change password
// Google script ID and required credentials
//String GOOGLE_SCRIPT_ID = "AKfycbzLlxH3XT5lwav2IDrzi2fvK-Vlin2yoNkzmvfFT18koVuP8MaOkB5ppMsJ7NBxTrii1A";    // change Gscript ID
String GOOGLE_SCRIPT_ID = "AKfycbwRIdMjEqhbyCGFt_R2OyNtL_EUyzU29-vfzKrqKcI4Atq_QhQHEz9wL-923xzknhqj";    // change Gscript ID

int count1 = 0;
int count2 = 50;
void setup() {
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    static bool flag = false;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    char timeStringBuff[50]; //50 chars should be enough
    strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    String asString(timeStringBuff);
    asString.replace(" ", "-");
    Serial.print("Time:");
    Serial.println(asString);
    String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "date=" + asString + "&oxygen=" + String(count1) + + "&heartrate=" + String(count2);
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
  count1++;
  count2++;
  delay(1000);
}
