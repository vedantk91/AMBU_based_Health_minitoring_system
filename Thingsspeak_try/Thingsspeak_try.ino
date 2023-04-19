#include <WiFi.h>
#include <ThingSpeak.h>

// Replace with your network credentials
const char* ssid = "Manasi";
const char* password = "manasi.24";

// Replace with your ThingSpeak API key and channel ID
const char* apiKey = "1DNTAUBVIAL7N0C6";
const long channelId = 2108287;

// Define the analog input pin for the AD8232 sensor
//const int analogPin = A0;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak library
  ThingSpeak.begin(client);
}

void loop() {
//  // Read data from the AD8232 sensor
//  int sensorValue = analogRead(analogPin);
//
//  // Convert sensor value to a voltage between 0 and 3.3V
//  float voltage = sensorValue * (3.3 / 4096.0);

  int randomNumber = random(4096);
  // print the random number to the serial monitor
  Serial.println(randomNumber);

  // Send data to ThingSpeak
  ThingSpeak.setField(1, randomNumber);
  int response = ThingSpeak.writeFields(channelId, apiKey);

  if (response == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Error sending data to ThingSpeak");
  }

  delay(1000); // Wait 10 seconds before sending the next reading
}
