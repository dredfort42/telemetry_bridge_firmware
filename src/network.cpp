#include "network.h" // For WiFi connectivity and digest handling

// connect to WiFi network
void connectToWiFi(bool *isWiFiConnected)
{
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.print(".");
        delay(200);
        *isWiFiConnected = false;
    }

    if (WiFi.status() == WL_CONNECTED && !*isWiFiConnected)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println();
        Serial.println("WiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        // WiFi.printDiag(Serial);
        *isWiFiConnected = true;
    }
}