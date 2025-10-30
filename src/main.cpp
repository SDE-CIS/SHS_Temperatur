#include <SPI.h>
#include <WiFiNINA.h>
#include <rgb_lcd.h>
#include "Grove_Temperature_And_Humidity_Sensor.h"
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <config.h>

// Server URL and port

// Functions
void updateLCDColor(float temperature, float lowTemp, float normalTemp, float highTemp);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void setup_wifi();
void connectToMqtt();

// Create instances
rgb_lcd lcd;
#define DHTTYPE DHT11
#define FAN_PIN 4

DHT sensor(7, DHTTYPE);
const int tempCalibration = -2; // Temperature calibration value

// Settings
struct Settings
{
  float tooLow = 15;
  float justRight = 20;
  float tooHigh = 25;
  bool fanOn = false;
  bool permFanOff = true;
  String settingTopic = "123/temperatur/stue/1/settings";
} deviceSettings;

// Create SSL espClient instance
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Fan Delays
unsigned long lastTimeFanSpun = millis();
unsigned long delayBetweenOnOff = 10000;
int fanSpeed = 0;

void setup()
{
  Serial.begin(9600); // Start serial communication

  // Initialize the LCD
  lcd.begin(16, 2);        // Set the LCD size to 16 columns and 2 rows
  lcd.setRGB(0, 128, 255); // Set the RGB backlight color

  // Initialize the fan
  pinMode(FAN_PIN, OUTPUT);

  // Initialize the sensor
  sensor.begin();

  // Setup Wifi
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(mqttCallback);
  connectToMqtt();
  client.subscribe(deviceSettings.settingTopic.c_str());


  delay(1000);
  lcd.clear(); // Clear the LCD screen

  // Set the time
  timeClient.begin();
}

void loop()
{
  timeClient.update();                            // Update the time
  long current_epoch = timeClient.getEpochTime(); // Get the current epoch time
  float temp_hum_val[2] = {0};
  if(!client.connected()){
    connectToMqtt();
  }
  client.loop();

  if (!sensor.readTempAndHumidity(temp_hum_val))
  {
    float temperature = temp_hum_val[1] + tempCalibration; // Read temperature
    float humidity = temp_hum_val[0];    // Read humidity

    updateLCDColor(temperature, deviceSettings.tooLow, deviceSettings.justRight, deviceSettings.tooHigh);
    lcd.clear();                  // Clear the LCD screen
    // Display values on LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(humidity);
    lcd.print(" %");

    JsonDocument jsonDoc;
    jsonDoc["temperatur"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["fanOn"] = deviceSettings.fanOn;
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    client.publish(MQTT_TOPIC, jsonString.c_str());

    // Turn on fan if temperature is too high
    unsigned long timeNow = millis();
    if (temperature <= deviceSettings.justRight)
    {
      if(timeNow - lastTimeFanSpun > delayBetweenOnOff){
        fanSpeed = 0; // Fan off
        deviceSettings.fanOn = false;
      }
    }
    else if (temperature >= deviceSettings.tooHigh)
    {
      fanSpeed = 255; // Fan at full speed
      lastTimeFanSpun = timeNow;
      deviceSettings.fanOn = true;
    }
    else
    {
      // Map the temperature range between tooLow and tooHigh to the PWM range (0-255)
      fanSpeed = map(temperature, deviceSettings.tooLow, deviceSettings.tooHigh, 0, 255);
      lastTimeFanSpun = timeNow;
      deviceSettings.fanOn = true;
    }
    // Set the fan speed
    if(deviceSettings.permFanOff){
      analogWrite(FAN_PIN, 0);
    } else if (!deviceSettings.permFanOff){
      analogWrite(FAN_PIN, fanSpeed);
    }
  }
  else
  {
    Serial.println("Error reading temperature and humidity.");
  }
}

void updateLCDColor(float temperature, float lowTemp, float normalTemp, float highTemp)
{
  int red = 0, green = 0, blue = 0;

  // Below lowTemp → Blue
  if (temperature <= lowTemp)
  {
    red = 0;
    green = 0;
    blue = 255;
  }
  // Between lowTemp and normalTemp → Green
  else if (temperature > lowTemp && temperature <= normalTemp)
  {
    // Smooth gradient from blue to green
    float ratio = (temperature - lowTemp) / (normalTemp - lowTemp);
    red = 0;
    green = (int)(255 * ratio);
    blue = (int)(255 * (1.0 - ratio));
  }
  // Between normalTemp and highTemp → Yellow to Red
  else if (temperature > normalTemp && temperature <= highTemp)
  {
    // Smooth gradient from green (normal) to red (high)
    float ratio = (temperature - normalTemp) / (highTemp - normalTemp);
    red = (int)(255 * ratio);
    green = (int)(255 * (1.0 - ratio));
    blue = 0;
  }
  // Above highTemp → Red
  else
  {
    red = 255;
    green = 0;
    blue = 0;
  }

  lcd.setRGB(red, green, blue);
}


void setup_wifi(){
  Serial.println("Connecting to WiFi...");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void connectToMqtt(){
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect(MQTT_TOPIC, MQTT_USER, MQTT_PASS)) {
      Serial.println("MQTT connected.");
    }
    else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length){
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println("Received MQTT message on topic: " + String(topic));
  Serial.println("Message: " + message);

  if(String(topic) == deviceSettings.settingTopic) {
    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, message);

    if (error)
    {
      Serial.println("Failed to parse JSON");
      Serial.println(error.c_str());
      return;
    }  

    if(jsonDoc["maxTemp"] && jsonDoc["normalTemp"] && jsonDoc["lowTemp"]){
      deviceSettings.tooHigh = jsonDoc["maxTemp"];
      deviceSettings.justRight = jsonDoc["normalTemp"];
      deviceSettings.tooLow = jsonDoc["lowTemp"];
    }
    if(jsonDoc.containsKey("permFanOff")){
      deviceSettings.permFanOff = jsonDoc["permFanOff"];
    }


  }

}