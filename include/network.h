#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>    // For WiFi connectivity
#include <WiFiUdp.h> // For UDP communication
#include <ArduinoJson.h>
#include <WString.h>

#include "secrets.h" // WIFI_SSID and WIFI_PASSWORD

#define DIGEST_PORT 9999
#define DIGEST_SERVER_TYPE "TelemetryBridge"

void connectToWiFi(bool *isWiFiConnected);

void receiveDigestPackets(bool isConnected);
void registerDevice(bool *isDeviceRegistered);
void sendData(String *payload, bool *isDeviceRegistered);

#endif // NETWORK_H