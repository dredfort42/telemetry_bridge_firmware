#ifndef DIGEST_H
#define DIGEST_H

#include <WiFi.h>    // For WiFi connectivity
#include <WiFiUdp.h> // For UDP communication
#include <ArduinoJson.h>
#include <WString.h>

#define DIGEST_PORT 9999
#define DIGEST_SERVER_TYPE "TelemetryBridge"

void receiveDigestPackets(bool isConnected);
void registerDevice(bool *isDeviceRegistered);
void sendData(String *payload);

#endif // DIGEST_H