#include "digest.h"

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
    if (!isConnected || isDigestReceived)
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
    if (!isDigestReceived || (isDeviceRegistered && *isDeviceRegistered))
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
                    *isDeviceRegistered = true;
                    break;
                }
            }
        }
        client.stop();
    }
}

void sendData(String *payload)
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
        while (client.connected() && millis() - timeout < 3000)
        {
            if (client.available())
            {
                String line = client.readStringUntil('\n');
                Serial.println(line);
                if (line.startsWith("HTTP/1.1 200"))
                    break;
            }
        }
        client.stop();
    }
}