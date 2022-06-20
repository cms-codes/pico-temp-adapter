#include <vector>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_Si7021.h"
#include "Adafruit_BMP085.h"
#include "temp.h"
#include "temp.tpp"
#include "connector.h"

// default "Wire" object: SDA = GP4, SCL = GP5, I2C0 peripheral
// used for sensor2 by default
// our new wire object used for sensor1:
#define WIRE1_SDA 6
#define WIRE1_SCL 7
arduino::MbedI2C Wire1(WIRE1_SDA, WIRE1_SCL);

// create the driver objects
Adafruit_Si7021 sensor1 = Adafruit_Si7021(&Wire1);
Adafruit_BMP085 sensor2;

// create the adapters
ConnectorAdaSi7021 sensor1_conn = ConnectorAdaSi7021(&sensor1);
ConnectorAdaBMP085 sensor2_conn = ConnectorAdaBMP085(&sensor2);
ConnectorAdaSi7021 *sensor1_conn_ptr = &sensor1_conn;
ConnectorAdaBMP085 *sensor2_conn_ptr = &sensor2_conn;

// create the abstract device object
Temp<ConnectorAdaSi7021> temp1 = Temp<ConnectorAdaSi7021>(sensor1_conn_ptr);
Temp<ConnectorAdaBMP085> temp2 = Temp<ConnectorAdaBMP085>(sensor2_conn_ptr);

// vector of temperature sensors
std::vector<Sensor*> temp_sensors;

void setup() {
  Serial.begin(115200);
  while(!Serial)
  ;
  if (!sensor1.begin()) {
    while (true)
      Serial.println("Did not find Si7021 sensor!");
  }
  if (!sensor2.begin()) {
    while (true)
      Serial.println("Did not find BMP085 sensor!");
  }
  temp_sensors.push_back(&temp1);
  temp_sensors.push_back(&temp2);
}

void loop() {
  for (auto &sensor : temp_sensors) {
    sensor->Update();
    Serial.print(sensor->Get_value(), 2);
    Serial.println(" C");
    
  }
  delay(2000);

}
