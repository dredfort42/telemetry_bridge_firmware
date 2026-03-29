#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4
#define DHTTYPE DHT22

typedef struct Sensor
{
    int sensor_id;
    int type;
    char unit[32];
    float max_value;
    float min_value;
    float resolution;
    unsigned long min_delay;
} Sensor;

typedef struct DHT_Sensor
{
    Sensor temperature;
    Sensor humidity;
} DHT_Sensor;

void printDHTSensorInfo(DHT_Unified *dht);
DHT_Sensor *getDHTSensorInfo(DHT_Unified *dht);

#endif // DHT_SENSOR_H