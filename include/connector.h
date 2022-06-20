#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <Adafruit_Si7021.h>
#include <Adafruit_BMP085.h>

class ConnectorAdaSi7021 {
    public:
     ConnectorAdaSi7021(Adafruit_Si7021 *sensor_ptr) : sensor(sensor_ptr) {};
     float Read_temp();
    private:
     Adafruit_Si7021 *sensor = nullptr;

};

class ConnectorAdaBMP085 {
    public:
     ConnectorAdaBMP085(Adafruit_BMP085 *sensor_ptr) : sensor(sensor_ptr) {};
     float Read_temp();
    private:
     Adafruit_BMP085 *sensor = nullptr;

};

#endif