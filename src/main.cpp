#include "network.h"    // For WiFi connectivity and digest handling
#include "dht_sensor.h" // For sensor reading and information

// WiFi connection status
bool isWiFiConnected = false;

// Device registration status
bool isDeviceRegistered = false;

// DHT sensor instance
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("\nConnecting to WiFi...\n");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Configure time with NTP
  Serial.printf("Configuring time with NTP...\n");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  delay(200);

  Serial.println("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 60)
  {
    delay(1000);
    now = time(nullptr);
    attempts++;
  }
  struct tm timeinfo = *localtime(&now);
  Serial.printf("\nCurrent time: ");
  Serial.println(asctime(&timeinfo));

  dht.begin();
  printDHTSensorInfo(&dht);
}

float temperature = 0.1;
float humidity = 0.2;

void loop()
{
  connectToWiFi(&isWiFiConnected);
  receiveDigestPackets(isWiFiConnected);
  registerDevice(&isDeviceRegistered);

  if (isDeviceRegistered)
  {

    // Send data to the server
    // Prepare JSON payload
    JsonDocument json;
    json["mac"] = WiFi.macAddress();
    json["timestamp"] = time(nullptr); // Send timestamp in milliseconds

    JsonArray sensors = json["sensors"].to<JsonArray>();

    JsonObject temp_sensor = sensors.add<JsonObject>();
    temp_sensor["id"] = "temp_1";
    temp_sensor["value"] = temperature;
    temp_sensor["dimension"] = "celsius";

    JsonObject hum_sensor = sensors.add<JsonObject>();
    hum_sensor["id"] = "hum_1";
    hum_sensor["value"] = humidity;
    hum_sensor["dimension"] = "percent";

    JsonArray actuators = json["actuators"].to<JsonArray>();

    JsonObject relay = actuators.add<JsonObject>();
    relay["id"] = "relay_1";
    relay["state"] = "off";

    String payload;
    serializeJson(json, payload);

    sendData(&payload, &isDeviceRegistered);

    temperature += 0.11;
    humidity += 0.12;
  }

  if (isWiFiConnected)
  {
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());

    delay(1000);
  }
}