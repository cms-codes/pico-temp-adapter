#include "BMP180_Pico.h"

/********* BMP180_Pico::BMP180_Pico *******
*  Constructor initializes pressure sampling mode and calibration datatype.
*/
BMP180_Pico::BMP180_Pico(int sampling_mode) {
  if (sampling_mode > 3 || sampling_mode < 0 ) {
    this->sampling_mode_ = 1;
  } else {
    this->sampling_mode_ = sampling_mode;
  }
  CalibrationData_t *calib_table = new CalibrationData_t;
  this->calibration_data_ = calib_table;
}

/********* BMP180_Pico::Begin *******
*  Reads in calibration data from device EEPROM (Public function).
*  Outputs: 1 on success, -1 for failure
*/
int BMP180_Pico::Begin() {
  if (!this->GetCalibrationData(this->calibration_data_)) return -1; // calibration retrieval fails
  return 1;
}

/********* BMP180_Pico::GetCalibrationData *******
*  Reads 22 bytes of factory recorded calibration data into the provided datatype;
*  see p. 13 of the BMP180 datasheet (April 2013).
*   Inputs: CalibrationData_t datatype
*  Outputs: 1 on success, -1 on failure
*/
int BMP180_Pico::GetCalibrationData(CalibrationData_t *calib_table) {
  const uint8_t calib_addr = { BMP_CALIB_R_ADDR };
  uint8_t calib_bytes[22] = {0};

  int16_t ret = 0;
  int16_t temp_raw = 0;
  int32_t X1 = 0;
  int32_t X2 = 0;
  int32_t B5 = 0;

  // if the i2c api returns an error, report failure
  ret = i2c_write_blocking(i2c0, BMP_ADDR, &calib_addr, 1, true);
  if (ret < 0 ) return -1;
  ret = i2c_read_blocking(i2c0, BMP_ADDR, calib_bytes, 22, false);
  if (ret < 0 ) return -1;

  // report failure when reading invalid data
  if (calib_bytes[0] == 0 || calib_bytes[0] == 0xff) return -1;

  calib_table->AC1 = ((calib_bytes[0] << 8) | calib_bytes[1]);
  calib_table->AC2 = ((calib_bytes[2] << 8) | calib_bytes[3]);
  calib_table->AC3 = ((calib_bytes[4] << 8) | calib_bytes[5]);
  calib_table->AC4 = ((calib_bytes[6] << 8) | calib_bytes[7]);
  calib_table->AC5 = ((calib_bytes[8] << 8) | calib_bytes[9]);
  calib_table->AC6 = ((calib_bytes[10] << 8) | calib_bytes[11]);
  calib_table->B1 = ((calib_bytes[12] << 8) | calib_bytes[13]);
  calib_table->B2 = ((calib_bytes[14] << 8) | calib_bytes[15]);
  calib_table->MB = ((calib_bytes[16] << 8) | calib_bytes[17]);
  calib_table->MC = ((calib_bytes[18] << 8) | calib_bytes[19]);
  calib_table->MD = ((calib_bytes[20] << 8) | calib_bytes[21]);

  temp_raw = GetTemperatureRaw();
  X1 = ( (temp_raw - calib_table->AC6) * ( calib_table->AC5 / (pow(2,15)) ) );
  X2 = ( ( calib_table->MC * (pow(2,11)) ) / (X1 + calib_table->MD) );
  B5 = X1 + X2;
  // Is pressure reading temperature compensated? (see pressure calib. coefficient B6)
  // This calib. function needs to run before each pressure reading.
  calib_table->B5 = B5;

  return 1;
}

/********* BMP180_Pico::GetTemperatureRaw *******
*  Reads the uncalibrated temperature.
*   Inputs: none
*  Outputs: Raw 16 bit ADC reading from sensor.
* 
*  0xf6: adc_out_msb
*  0xf7: adc_out_lsb
*  0xf8: adc_out_xlsb
* 
*/
uint16_t BMP180_Pico::GetTemperatureRaw(void) {
  uint8_t temp_parts[2] = {0};
  int32_t temp_raw = 0;
  const uint8_t temp_read_trig[2] = { BMP_READ_REQ_ADDR, BMP_TEMP_REQ_CMD };
  const uint8_t temp_read_addr = BMP_READ_ADDR;
  
  // initiate a reading of the temperature sensor
  i2c_write_blocking(i2c0, BMP_ADDR, temp_read_trig, 2, true );
  // delay 4.5ms
  busy_wait_us(kSamplingDelayTable[0]);
  // fetch the reading
  i2c_write_blocking(i2c0, BMP_ADDR, &temp_read_addr, 1, true);
  i2c_read_blocking(i2c0, BMP_ADDR, temp_parts, BMP_TEMP_READ_LEN, false);

  temp_raw = ( temp_parts[0] << 8 ) + temp_parts[1];
  return temp_raw;

}

/********* BMP180_Pico::GetTemperature *******
*  Calibrated temperature reading (public function)
*   Inputs: None.
*  Outputs: Calibrated temperature.
*/
float BMP180_Pico::GetTemperature(void) {
  uint32_t temp_raw = 0;
  float temp_calibrated = 0.0f;

  int32_t X1 = 0;
  int32_t X2 = 0;
  int32_t B5 = 0;
  
  temp_raw = this->GetTemperatureRaw();

  X1 = ( (temp_raw - this->calibration_data_->AC6) * 
       ( this->calibration_data_->AC5 / (pow(2,15)) ) );

  X2 = ( (this->calibration_data_->MC * (pow(2,11)) ) / 
       (X1 + this->calibration_data_->MD) );

  B5 = X1 + X2;
  temp_calibrated = (B5 + 8) / (pow(2,4));
  
  return temp_calibrated / 10.0f;
}

/********* BMP180_Pico::GetPressureRaw *******
*  Uncalibrated pressure reading.
*   Inputs: None.
*  Outputs: Raw pressure reading from sensor memory. Length is dependent on
*           sampling mode.
*/
uint32_t BMP180_Pico::GetPressureRaw(void) {
  uint8_t press_parts[3] = {0};
  int32_t press_raw = 0;
  int press_req_cmd = this->sampling_mode_ << 6;
  const uint8_t press_read_trigger[2] = { 
    BMP_READ_REQ_ADDR, 
    static_cast<uint8_t>(press_req_cmd) 
  };
  const uint8_t press_read_addr = BMP_READ_ADDR;

  i2c_write_blocking(i2c0, BMP_ADDR, press_read_trigger, 2, true);
  busy_wait_us(kSamplingDelayTable[this->sampling_mode_]);
  i2c_write_blocking(i2c0, BMP_ADDR, &press_read_addr, 1, true);
  i2c_read_blocking(i2c0, BMP_ADDR, press_parts, BMP_PRESS_READ_LEN, false);

  press_raw = ( ( press_parts[0] << 16 ) + (press_parts[1] << 8) + press_parts[2] )
         >> ( 8 - this->sampling_mode_ );
  return press_raw;

}

/********* BMP180_Pico::GetPressure *******
*  Calibrated presserature reading (public function)
*   Inputs: None.
*  Outputs: Calibrated absolute pressure in pascals.
*/
int32_t BMP180_Pico::GetPressure(void) {
  int32_t press_raw = 0;
  int32_t press_calibrated = 0;

  int32_t B6 = 0;
  int32_t X1 = 0;
  int32_t X2 = 0;
  int32_t X3 = 0;
  int32_t B3 = 0;
  uint32_t B4 = 0;
  uint32_t B7 = 0;

  press_raw = GetPressureRaw();

  B6 = this->calibration_data_->B5 - 4000;
  X1 = ( this->calibration_data_->B2 * ( B6 * B6 / pow(2, 12) ) ) / pow(2, 11);
  X2 = this->calibration_data_->AC2 * B6 / pow(2, 11);
  X3 = X1 + X2;

  B3 = (((this->calibration_data_->AC1 * 4 + X3) << this->sampling_mode_) + 2) / 4;
  X1 = this->calibration_data_->AC3 * B6 / pow(2, 13);
  X2 = ( this->calibration_data_->B1 * ( B6 * B6 / pow(2, 12) ) ) / pow(2, 16);
  X3 = ((X1 + X2) + 2) / pow(2, 2);

  B4 = this->calibration_data_->AC4 * (uint32_t)(X3 + 32768) / pow(2, 15);
  B7 = ((uint32_t)press_raw - B3) * (50000 >> this->sampling_mode_);
  if (B7 < 0x80000000UL) {
    press_calibrated = (B7 * 2) / B4;
  }
  else {
    press_calibrated = (B7 / B4) * 2;
  }

  X1 = (press_calibrated / pow(2, 8)) * (press_calibrated / pow(2, 8));
  X1 = (X1 * 3038) / pow(2, 16);
  X2 = (-7357 * press_calibrated) / pow(2, 16);
  press_calibrated = press_calibrated + (X1 + X2 + 3791) / pow(2, 4);

  // Absolute pressure (relative to sea level)
  return press_calibrated; // return in Pa

}