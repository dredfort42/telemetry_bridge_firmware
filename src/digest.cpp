#include "network.h"

WiFiUDP udp;

// // Port for server digest
// int digestPort = 9999;
// char digestServerType[64] = "TelemetryBridge";

// Buffer for incoming UDP packets
char incomingPacket[1024];

// Flag to indicate if a valid digest has been received
bool isDigestReceived = false;

// Variables to store server information from digest
char serverType[64];
char serverIP[16];
int serverPort;

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

    if (strcmp(serverType, DIGEST_SERVER_TYPE) != 0)
        return false;

    return true;
}

// Receive UDP packets containing server digest
void receiveDigestPackets(bool isConnected)
{
    if (!isConnected)
    {
        isDigestReceived = false; // Reset flag if not connected
        return;
    }

    if (isDigestReceived)
        return;

    static bool udpInitialized = false;
    if (!udpInitialized)
    {
        if (udp.begin(DIGEST_PORT))
        {
            Serial.printf("UDP listening on port %d\n", DIGEST_PORT);
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
void registerDevice(bool *isDeviceRegistered)
{
    if (!isDeviceRegistered)
        return;

    if (!isDigestReceived)
    {
        *isDeviceRegistered = false; // Ensure device is marked as unregistered
        return;
    }

    if (*isDeviceRegistered)
        return;

    WiFiClient client;
    String url = "/register";
    String host = String(serverIP);
    int port = serverPort;

    // Prepare JSON payload
    JsonDocument json;

    // Device Info
    JsonObject device_info = json["device_info"].to<JsonObject>();
    device_info["vendor"] = "dredfort42";
    device_info["model"] = "iot-ctrl-v0";
    device_info["farmvare"] = "1.2.3";
    device_info["ip"] = WiFi.localIP().toString();
    device_info["port"] = "";
    device_info["mac"] = WiFi.macAddress();

    // Capabilities
    JsonObject capabilities = json["capabilities"].to<JsonObject>();

    // Sensors
    JsonArray sensors = capabilities["sensors"].to<JsonArray>();

    JsonObject temp_sensor = sensors.add<JsonObject>();
    temp_sensor["id"] = "temp_1";
    temp_sensor["type"] = "temperature";
    temp_sensor["unit"] = "celsius";
    JsonArray temp_range = temp_sensor["range"].to<JsonArray>();
    temp_range.add(-40);
    temp_range.add(125);
    temp_sensor["read_only"] = true;
    JsonArray temp_modes = temp_sensor["sampling_modes"].to<JsonArray>();
    temp_modes.add("push");
    temp_modes.add("pull");

    JsonObject hum_sensor = sensors.add<JsonObject>();
    hum_sensor["id"] = "hum_1";
    hum_sensor["type"] = "humidity";
    hum_sensor["unit"] = "percent";
    JsonArray hum_range = hum_sensor["range"].to<JsonArray>();
    hum_range.add(0);
    hum_range.add(100);
    hum_sensor["read_only"] = true;
    JsonArray hum_modes = hum_sensor["sampling_modes"].to<JsonArray>();
    hum_modes.add("push");
    hum_modes.add("pull");

    // Actuators
    JsonArray actuators = capabilities["actuators"].to<JsonArray>();

    JsonObject relay = actuators.add<JsonObject>();
    relay["id"] = "relay_1";
    relay["type"] = "relay";
    JsonArray relay_commands = relay["commands"].to<JsonArray>();
    relay_commands.add("on");
    relay_commands.add("off");
    JsonArray relay_state = relay["state"].to<JsonArray>();
    relay_state.add("on");
    relay_state.add("off");

    // JsonObject motor = actuators.add<JsonObject>();
    // motor["id"] = "motor_1";
    // motor["type"] = "motor";
    // JsonArray motor_commands = motor["commands"].to<JsonArray>();
    // motor_commands.add("start");
    // motor_commands.add("stop");
    // motor_commands.add("set_speed");
    // JsonObject motor_params = motor["params"].to<JsonObject>();
    // JsonObject speed_param = motor_params["speed"].to<JsonObject>();
    // speed_param["min"] = 0;
    // speed_param["max"] = 3000;
    // speed_param["unit"] = "rpm";

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

    int retries = 3;
    while (retries > 0)
    {
        if (client.connect(host.c_str(), port))
        {
            client.print(request);

            unsigned long timeout = millis();
            bool success = false;
            while (client.connected() && millis() - timeout < 5000)
            {
                if (client.available())
                {
                    String line = client.readStringUntil('\n');
                    Serial.println(line);
                    if (line.startsWith("HTTP/1.1 200"))
                    {
                        *isDeviceRegistered = true;
                        success = true;
                        break;
                    }
                }
            }
            client.stop();
            if (success)
                return;
        }
        else
        {
            Serial.printf("Connection failed to %s:%d, retries left: %d\n", host.c_str(), port, retries - 1);
        }
        retries--;
        delay(1000); // Wait before retrying
    }
}

void sendData(String *payload, bool *isDeviceRegistered)
{
    WiFiClient client;
    String url = "/data";
    String host = String(serverIP);
    int port = serverPort;

    // Build HTTP POST request
    String request =
        "POST " + url + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Content-Type: application/json\r\n" +
        "Content-Length: " + String(payload->length()) + "\r\n" +
        "Connection: close\r\n\r\n" +
        *payload;

    if (client.connect(host.c_str(), port))
    {
        client.print(request);

        unsigned long timeout = millis();
        bool success = false;
        while (client.connected() && millis() - timeout < 5000)
        {
            if (client.available())
            {
                String line = client.readStringUntil('\n');
                Serial.println(line);
                if (line.startsWith("HTTP/1.1 200"))
                {
                    success = true;
                    break;
                }
            }
        }
        client.stop();
        if (success)
            return;
    }

    Serial.printf("Send data failed to %s:%d\n", host.c_str(), port);
    isDigestReceived = false; // Mark digest as invalid to trigger re-registration
}