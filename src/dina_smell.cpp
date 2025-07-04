#include <Arduino.h>

// #include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "Multichannel_Gas_GMXXX.h"
#include "seeed_bme680.h"
#include "sensirion_common.h"
#include "sgp30.h"

// WIFI
#define WIFI_SSID "DinaCam"
#define WIFI_PASS "bigheadsim"

// MQTT broker settings
#define MQTT_SERVER "192.168.8.174"
#define MQTT_PORT 1883
#define MQTT_USER "dinacam"
#define MQTT_PASS "bigheadsim"

#define MQTT_CLIENT "dinasmell-client"
#define MQTT_TOPIC_SMELL_temp MQTT_USER "/feeds/dinasmell.temp"
#define MQTT_TOPIC_SMELL_humd MQTT_USER "/feeds/dinasmell.humd"
#define MQTT_TOPIC_SMELL_pres MQTT_USER "/feeds/dinasmell.pres"
#define MQTT_TOPIC_SMELL_co2 MQTT_USER "/feeds/dinasmell.co2"
#define MQTT_TOPIC_SMELL_voc1 MQTT_USER "/feeds/dinasmell.voc1"
#define MQTT_TOPIC_SMELL_voc2 MQTT_USER "/feeds/dinasmell.voc2"
#define MQTT_TOPIC_SMELL_no2 MQTT_USER "/feeds/dinasmell.no2"
#define MQTT_TOPIC_SMELL_eth MQTT_USER "/feeds/dinasmell.eth"
#define MQTT_TOPIC_SMELL_co MQTT_USER "/feeds/dinasmell.co"

// Constants
#define BME680_I2C_ADDR uint8_t(0x76) // I2C address of BME680
#define PA_IN_KPA 1000.0              // Convert Pa to KPa

// Sampling timing
#define SAMPLING_PERIOD_MS 2000 // Sampling period (ms)

// Global objects
GAS_GMXXX<TwoWire> gas;               // Multichannel gas sensor v2
Seeed_BME680 bme680(BME680_I2C_ADDR);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect("DinaSmellClient", MQTT_USER, MQTT_PASS)) {
            // Connected
        } else {
            delay(5000);
        }
    }
}

void setup() {

    Serial.begin(115200);
    Wire.begin();

    setup_wifi();

    // Create a random client ID
    randomSeed(micros());
    String clientId = MQTT_CLIENT;
    clientId += String(random(0xffff), HEX);

    // Configure MQTT server
    client.setServer(MQTT_SERVER, MQTT_PORT);

    // Connect to MQTT server
    while (!client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS))
    {
        Serial.println("Failed to connect MQTT to mqtt server.");
        sleep(5);
    }
    Serial.print("Successfully connected to ");
    Serial.println(MQTT_SERVER);
    Serial.print("Client: ");
    Serial.println(clientId);

    // Initialize environmental sensor
    gas.begin(Wire, 0x08);

    // Initialize gas sensors
    while (!bme680.init())
    {
        Serial.println("Trying to initialize BME680...");
        delay(1000);
    }

    // Initialize VOC and eCO2 sensor
    while (sgp_probe() != STATUS_OK)
    {
        Serial.println("Trying to initialize SGP30...");
        delay(1000);
    }

    // ethyl alcohol and h2 initial read
    uint16_t sgp_eth;
    uint16_t sgp_h2;
    int16_t sgp_err = sgp_measure_signals_blocking_read(&sgp_eth, &sgp_h2);
    if (sgp_err != STATUS_OK)
    {
        Serial.println("Error: Could not read signal from SGP30");
        while (1)
            ;
    }
}

void read_smell_sensors()
{
    // Read from GM-X02b sensors (multichannel gas)
    const float gm_no2_v = gas.calcVol(gas.getGM102B());
    const float gm_eth_v = gas.calcVol(gas.getGM302B());
    const float gm_voc_v = gas.calcVol(gas.getGM502B());
    const float gm_co_v = gas.calcVol(gas.getGM702B());

    // Read BME680 environmental sensor
    if (bme680.read_sensor_data())
    {
        Serial.println("Error: Could not read from BME680");
        return;
    }

    // Read SGP30 sensor
    uint16_t sgp_tvoc = 0U;
    uint16_t sgp_co2 = 0U;
    const int16_t sgp_err = sgp_measure_iaq_blocking_read(&sgp_tvoc, &sgp_co2);
    if (sgp_err != STATUS_OK)
    {
        Serial.println("Error: Could not read from SGP30");
        return;
    }

    client.publish(MQTT_TOPIC_SMELL_temp, String(bme680.sensor_result_value.humidity).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_humd, String(bme680.sensor_result_value.humidity).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_pres, String(bme680.sensor_result_value.pressure / PA_IN_KPA).c_str(), true);

    client.publish(MQTT_TOPIC_SMELL_no2, String(gm_no2_v).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_eth, String(gm_eth_v).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_voc2, String(gm_voc_v).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_co, String(gm_co_v).c_str(), true);

    client.publish(MQTT_TOPIC_SMELL_co2, String(sgp_co2).c_str(), true);
    client.publish(MQTT_TOPIC_SMELL_voc1, String(sgp_tvoc).c_str(), true);

    Serial.println("MQTT published");

    // Print CSV data with timestamp
    Serial.print(bme680.sensor_result_value.temperature);
    // Serial.print(",");
    // Serial.print(bme680.sensor_result_value.humidity);
    Serial.print(",");
    Serial.print(bme680.sensor_result_value.pressure / PA_IN_KPA);
    // Serial.print(",");
    // Serial.print(sgp_co2);
    // Serial.print(",");
    // Serial.print(sgp_tvoc);
    // Serial.print(",");
    // Serial.print(gm_voc_v);
    // Serial.print(",");
    // Serial.print(gm_no2_v);
    // Serial.print(",");
    // Serial.print(gm_eth_v);
    // Serial.print(",");
    // Serial.print(gm_co_v);
    // Serial.println();

}

void loop() {
    const uint32_t now = millis();

    if (!client.connected()) {
        reconnect();
    }

    read_smell_sensors();

    // Wait just long enough for our sampling periodn hit our target frequency
    client.loop();
    while (millis() < now + SAMPLING_PERIOD_MS);
    {
        // wait
        client.loop();
    }

    delay(2000); // Publish every 2 seconds
}