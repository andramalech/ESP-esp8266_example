// I N C L U D E S
//-------------------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "bme280.h"

// D E F I N E S
//-------------------------------------------------------------------------------------------------
#define BAUD_RATE     115200
#define dev_addr      0x76

// G L O B A L  V A R I A B L E S
//-------------------------------------------------------------------------------------------------
// WiFi/MQTT Config
char _SSID[] = "<2.4GHz WiFi SSID";
char _PSWD[] = "<2.4GHz WiFI SSID password";
char mqtt_server[] = "<mqtt_server_address";
const char* device_name = "Master Bedroom";

WiFiClient BedroomSensor;
PubSubClient client(BedroomSensor);

// BME280 variables
float pressure;               // Storing measured pressure reading
float temperature;            // Storing measured temperature reading
float humidity;               // Storing measured humidity reading

uint8_t chip_id;              // Storing BME280 Chip ID

char _temp_[10];
char _humid_[10];
char _pressure_[10];

BME280 sensor_bme280;
 
// S E T U P
//-------------------------------------------------------------------------------------------------
void setup() {
  Wire.begin();
  Serial.begin(BAUD_RATE);
  
  _wifi_setup();

  client.setServer(mqtt_server, 1883);
  reconnect();

  sensor_bme280.settings.i2c_addr = dev_addr;           // Set the device i2c address

  Serial.println("BME280 - Temperature, Humidity, and Pressure Sensor\n");
  Serial.print("Chip ID = 0x");
  Serial.print(sensor_bme280.begin_i2c(), HEX);
  Serial.println("\n");

  sensor_bme280.set_humid_osrs(bme280_osrs_two);
  sensor_bme280.set_temp_osrs(bme280_osrs_two);
  sensor_bme280.set_pres_osrs(bme280_osrs_two);
  sensor_bme280.set_mode(bme280_forced_mode);

  Serial.println("Settings");
  Serial.println("----------------------------------------------\r");
  Serial.print("CTRL HUM\t0x");
  Serial.print(sensor_bme280.read_reg(bme280_ctrl_hum_addr), HEX);
  Serial.print("\nCTRL MEAS\t0x");
  Serial.print(sensor_bme280.read_reg(bme280_ctrl_meas_addr), HEX);

  temperature = sensor_bme280.read_float_temp_f();
  pressure = sensor_bme280.read_float_pres();
  humidity = sensor_bme280.read_float_humidity();

  Serial.println("\n\nTemp\t\tHumidity\tPressure");
  Serial.println("----------------------------------------------\r");
  getData();
}

// L O O P
//-------------------------------------------------------------------------------------------------
void loop()
{
  delay(10e3);
  getData();
}

// F U N C T I O N S
//-------------------------------------------------------------------------------------------------
void getData()
{
  sensor_bme280.set_mode(bme280_forced_mode);
  
  temperature = sensor_bme280.read_float_temp_f();
  pressure = sensor_bme280.read_float_pres();
  humidity = sensor_bme280.read_float_humidity();

  dtostrf(temperature, 4, 1, _temp_);
  dtostrf(humidity, 4, 1, _humid_);
  dtostrf(pressure, 4, 1, _pressure_);

  client.publish("MasterBedroom/Temperature", _temp_);
  delay(100);
  client.publish("MasterBedroom/Humidity", _humid_);
  delay(100);
  client.publish("MasterBedroom/Pressure", _pressure_);
  delay(100);
  
  Serial.print(temperature);
  Serial.print(" ºF \t");
  Serial.print(humidity);
  Serial.print(" % \t");
  Serial.print(pressure);
  Serial.print(" hPa \t\n");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("MasterBedroom")) {
      Serial.println("MQTT Connected");
      client.subscribe("MasterBedroom/Temperature");
      client.subscribe("MasterBedroom/Humidity");
      client.subscribe("MasterBedroom/Pressure");
    }
  }
}

void _wifi_setup(void) {
  Serial.print("\nWiFi Connecting");
  WiFi.hostname(device_name);
  WiFi.begin(_SSID, _PSWD);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay(200);
    Serial.print(".");
  }

  Serial.print("\nWiFi Connected\n");
  Serial.print(WiFi.SSID());
  Serial.print("\t");
  Serial.print(WiFi.localIP());
  Serial.print("\n\n");
}