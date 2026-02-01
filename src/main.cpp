#include "secrets.h"          // WIFI_SSID and WIFI_PASSWORD
#include <WiFi.h>             // For WiFi connectivity
#include <WiFiUdp.h>          // For UDP communication
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
WiFiUDP udp;

// WiFi connection status
bool isConnected = false;

// Port for server digest
int digestPort = 9999;
char digestServerType[64] = "TelemetryBridge";

// Buffer for incoming UDP packets
char incomingPacket[1024];

// Flag to indicate if a valid digest has been received
bool isDigestReceived = false;

// Variables to store server information from digest
char serverType[64];
char serverIP[16];
int serverPort;

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

// Parse the received UDP packet as JSON digest
bool parseDigest(const char *packet)
{
  JsonDocument digest;

  DeserializationError error = deserializeJson(digest, packet);
  if (error)
    return false;

  Serial.println(F("Response:"));
  Serial.println(digest["type"].as<const char *>());
  Serial.println(digest["ip"].as<const char *>());
  Serial.println(digest["port"].as<const int>());

  strncpy(serverType, digest["type"].as<const char *>(), sizeof(serverType) - 1);
  strncpy(serverIP, digest["ip"].as<const char *>(), sizeof(serverIP) - 1);
  serverPort = digest["port"].as<const int>();

  if (strcmp(serverType, digestServerType) != 0)
    return false;

  return true;
}

// Receive UDP packets containing server digest
void receiveDigestPackets()
{
  if (!isConnected || isDigestReceived)
    return;

  static bool udpInitialized = false;
  if (!udpInitialized)
  {
    if (udp.begin(digestPort))
    {
      Serial.printf("UDP listening on port %d\n", digestPort);
      udpInitialized = true;
    }
    else
    {
      Serial.println("Failed to start UDP");
      return;
    }
  }

  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0)
    {
      incomingPacket[len] = '\0';
      Serial.printf("Received UDP packet: %s\n", incomingPacket);
      isDigestReceived = parseDigest(incomingPacket);
    }
  }
}

// Register device with the server via HTTP POST
void registerDevice()
{
  if (!isDigestReceived || isDeviceRegistered)
    return;

  WiFiClient client;
  String url = "/register";
  String host = String(serverIP);
  int port = serverPort;

  // Prepare JSON payload
  JsonDocument json;
  json["mac"] = WiFi.macAddress();
  json["timestamp"] = millis();

  String payload;
  serializeJson(json, payload);

  // Build HTTP POST request
  String request =
      "POST " + url + " HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "Content-Type: application/json\r\n" +
      "Content-Length: " + String(payload.length()) + "\r\n" +
      "Connection: close\r\n\r\n" +
      payload;

  if (client.connect(host.c_str(), port))
  {
    client.print(request);

    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 3000)
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        if (line.startsWith("HTTP/1.1 200"))
        {
          isDeviceRegistered = true;
          break;
        }
      }
    }
    client.stop();
  }
}

void loop()
{
  connectToWiFi();
  receiveDigestPackets();
  registerDevice();

  display.clearDisplay();

  if (isDeviceRegistered)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.setCursor(0, 0);
    display.println("CONNECTED");
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