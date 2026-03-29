#include "dht_sensor.h"

void printDHTSensorInfo(DHT_Unified *dht)
{
    sensor_t sensor;
    dht->temperature().getSensor(&sensor);
    Serial.println(F("Temperature Sensor:"));
    Serial.print(F("  Unique ID: "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("  Type: "));
    Serial.println(sensor.type);
    Serial.print(F("  Max Value: "));
    Serial.print(sensor.max_value);
    Serial.print(F("  Min Value: "));
    Serial.print(sensor.min_value);
    Serial.print(F("  Resolution: "));
    Serial.print(sensor.resolution);
    Serial.print(F("  Min Delay: "));
    Serial.print(sensor.min_delay);

    dht->humidity().getSensor(&sensor);
    Serial.println(F("\nHumidity Sensor:"));
    Serial.print(F("  Unique ID: "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("  Type: "));
    Serial.println(sensor.type);
    Serial.print(F("  Max Value: "));
    Serial.print(sensor.max_value);
    Serial.print(F("  Min Value: "));
    Serial.print(sensor.min_value);
    Serial.print(F("  Resolution: "));
    Serial.print(sensor.resolution);
    Serial.print(F("  Min Delay: "));
    Serial.print(sensor.min_delay);
}

DHT_Sensor *getDHTSensorInfo(DHT_Unified *dht)
{

    // strncpy(sensor->name, "DHT22", sizeof(sensor->name) - 1);
    // sensor->version = 1;
    // sensor->sensor_id = 12345; // Unique ID for this sensor
    // sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
    // sensor->max_value = 125.0;
    // sensor->min_value = -40.0;
    // sensor->resolution = 0.1;
    // sensor->min_delay = 2000000; // Minimum delay between readings in microseconds (2 seconds)

    // JsonObject temp_sensor = sensors.add<JsonObject>();
    // temp_sensor["id"] = "temp_1";
    // temp_sensor["type"] = "temperature";
    // temp_sensor["unit"] = "celsius";
    // JsonArray temp_range = temp_sensor["range"].to<JsonArray>();
    // temp_range.add(-40);
    // temp_range.add(125);
    // temp_sensor["read_only"] = true;
    // JsonArray temp_modes = temp_sensor["sampling_modes"].to<JsonArray>();
    // temp_modes.add("push");
    // temp_modes.add("pull");

    // JsonObject hum_sensor = sensors.add<JsonObject>();
    // hum_sensor["id"] = "hum_1";
    // hum_sensor["type"] = "humidity";
    // hum_sensor["unit"] = "percent";
    // JsonArray hum_range = hum_sensor["range"].to<JsonArray>();
    // hum_range.add(0);
    // hum_range.add(100);
    // hum_sensor["read_only"] = true;
    // JsonArray hum_modes = hum_sensor["sampling_modes"].to<JsonArray>();
    // hum_modes.add("push");
    // hum_modes.add("pull");

    return nullptr;
}