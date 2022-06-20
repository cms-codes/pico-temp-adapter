#include "connector.h"

float ConnectorAdaSi7021::Read_temp() {
    return this->sensor->readTemperature();
}

float ConnectorAdaBMP085::Read_temp() {
    return this->sensor->readTemperature();
}