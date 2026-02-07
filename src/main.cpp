#include "secrets.h"          // WIFI_SSID and WIFI_PASSWORD
#include "digest.h"           // For digest parsing and server info
#include <WiFi.h>             // For WiFi connectivity
#include <Wire.h>             // For I2C communication
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_SSD1306.h> // OLED driver
#include <ArduinoJson.h>

// Define display dimensions (common for 0.96")
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define I2C address (usually 0x3C or 0x3D)
#define OLED_RESET -1       // No reset pin for I2C, or specific GPIO if used
#define SCREEN_ADDRESS 0x3C // Check your display if this doesn't work

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi connection status
bool isConnected = false;

bool isDeviceRegistered = false;

// connect to WiFi network
void connectToWiFi()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print(".");
    delay(200);
    isConnected = false;
  }

  if (WiFi.status() == WL_CONNECTED && !isConnected)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println();
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // WiFi.printDiag(Serial);
    isConnected = true;
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize I2C (using default ESP32 pins 21/22)
  Wire.begin();

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
      ; // Don't proceed if display fails
  }

  // Clear the buffer.
  display.clearDisplay();

  // Set text properties
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Loading...");

  // Show the buffer on the screen
  display.display();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("\nConnecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Clear the buffer.
  display.clearDisplay();
  delay(200);
}

float temperature = 0.1;
float humidity = 0.2;

void loop()
{
  connectToWiFi();
  receiveDigestPackets(isConnected);
  registerDevice(&isDeviceRegistered);

  display.clearDisplay();

  if (isDeviceRegistered)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.setCursor(0, 0);
    display.println("CONNECTED");

    // Send data to the server
    // Prepare JSON payload
    JsonDocument json;
    json["mac"] = WiFi.macAddress();
    json["temperature_c"] = temperature;
    json["humidity_percent"] = humidity;
    json["timestamp"] = millis();

    String payload;
    serializeJson(json, payload);

    sendData(&payload);

    temperature += 0.11;
    humidity += 0.12;
  }

  display.setTextSize(1); // Normal 1:1 pixel scale

  display.setCursor(0, 20);
  display.printf("SSID: ");
  display.setCursor(40, 20);
  display.printf("%s", WiFi.SSID().c_str());

  display.setCursor(0, 30);
  display.printf("CHNL: ");
  display.setCursor(40, 30);
  display.printf("%d\n", WiFi.channel());

  display.setCursor(0, 40);
  display.printf("RSSI: ");
  display.setCursor(40, 40);
  display.printf("%d dBm\n", WiFi.RSSI());

  display.setCursor(0, 50);
  display.printf("IP: ");
  display.setCursor(40, 50);
  display.printf("%s\n", WiFi.localIP().toString().c_str());

  display.display();

  if (isConnected)
  {
    // Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());

    delay(1000);
  }
}