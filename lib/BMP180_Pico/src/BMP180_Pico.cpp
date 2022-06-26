#include "BMP180_Pico.h"

BMP180_Pico::BMP180_Pico(int bmp_sampling_mode)
{
	if (bmp_sampling_mode > 3 || bmp_sampling_mode < 0 ) {
		this->bmp_sampling_mode_ = 1;
	} else {
		this->bmp_sampling_mode_ = bmp_sampling_mode;
	}
	bmp_calib_bin_t *bmp_calib_table = new bmp_calib_bin_t;
	this->bmp_calibration = bmp_calib_table;
}

int BMP180_Pico::bmp_begin()
{
	if (!this->_bmp_get_calib_data(this->bmp_calibration)) return -1;
	return 1;
}

/********* BMP180_Pico::_bmp_get_calib_bin *******
*  Reads 22 bytes of factory recorded calibration data into the provided struct;
*  see p. 13 of the BMP180 datasheet (April 2013).
*   Inputs: bmp_calib_bin_t struct
*  Outputs: none
*/
int BMP180_Pico::_bmp_get_calib_data(bmp_calib_bin_t *calib_bin)
{
	const uint8_t calib_addr = { BMP_CALIB_R_ADDR };
	uint8_t calib_bytes[22] = {0};

	int16_t ret = 0;
	int16_t temp_raw = 0;
	int32_t X1 = 0;
	int32_t X2 = 0;
	int32_t B5 = 0;

	ret = i2c_write_blocking(i2c0, BMP_ADDR, &calib_addr, 1, true);
	if (ret < 0 ) return -1;
	ret = i2c_read_blocking(i2c0, BMP_ADDR, calib_bytes, 22, false);
	if (ret < 0 ) return -1;

	calib_bin->AC1 = ((calib_bytes[0] << 8) | calib_bytes[1]);
	calib_bin->AC2 = ((calib_bytes[2] << 8) | calib_bytes[3]);
	calib_bin->AC3 = ((calib_bytes[4] << 8) | calib_bytes[5]);
	calib_bin->AC4 = ((calib_bytes[6] << 8) | calib_bytes[7]);
	calib_bin->AC5 = ((calib_bytes[8] << 8) | calib_bytes[9]);
	calib_bin->AC6 = ((calib_bytes[10] << 8) | calib_bytes[11]);
	calib_bin->B1 = ((calib_bytes[12] << 8) | calib_bytes[13]);
	calib_bin->B2 = ((calib_bytes[14] << 8) | calib_bytes[15]);
	calib_bin->MB = ((calib_bytes[16] << 8) | calib_bytes[17]);
	calib_bin->MC = ((calib_bytes[18] << 8) | calib_bytes[19]);
	calib_bin->MD = ((calib_bytes[20] << 8) | calib_bytes[21]);

	temp_raw = _bmp_get_temp_raw();
	X1 = ( (temp_raw - calib_bin->AC6) * ( calib_bin->AC5 / (pow(2,15)) ) );
	X2 = ( ( calib_bin->MC * (pow(2,11)) ) / (X1 + calib_bin->MD) );
	B5 = X1 + X2;
	// Is pressure reading temperature compensated? (see pressure calib. coefficient B6)
	// This calib. function will need to run before each pressure reading.
	calib_bin->B5 = B5;

	return 1;
}

/********* BMP180_Pico::_bmp_get_temp_raw *******
*  Reads the uncalibrated temperature.
*   Inputs: none
*  Outputs: Raw 16 bit ADC reading from sensor.
* 
*  0xf6: adc_out_msb
*  0xf7: adc_out_lsb
*  0xf8: adc_out_xlsb
* 
*/
uint16_t BMP180_Pico::_bmp_get_temp_raw(void)
{
	uint8_t temp_parts[2] = {0};
	int32_t temp_raw = 0;
	const uint8_t temp_read_trig[2] = { BMP_READ_REQ_ADDR, BMP_TEMP_REQ_CMD };
	const uint8_t temp_read_addr = BMP_READ_ADDR;

	// initiate a reading of the temperature sensor
	i2c_write_blocking(i2c0, BMP_ADDR, temp_read_trig, 2, true );
	// delay 4.5ms
	sleep_us(bmp_oss_delay_table[0]);
	// fetch the reading
	i2c_write_blocking(i2c0, BMP_ADDR, &temp_read_addr, 1, true);
	i2c_read_blocking(i2c0, BMP_ADDR, temp_parts, BMP_TEMP_READ_LEN, false);

	temp_raw = ( temp_parts[0] << 8 ) + temp_parts[1];
	return temp_raw;

}

/********* BMP180_Pico::bmp_get_temperature *******
*  Calibrated temperature reading (public function)
*   Inputs: Uncalibrated temperature reading, device calibration struct. See p. 15 of
*           the BMP180 datasheet (April 2013).
*  Outputs: Calibrated temperature.
*/
float BMP180_Pico::bmp_get_temperature(void)
{
	uint32_t temp_raw = 0;
	float temp_cal = 0.0f;

	int32_t X1 = 0;
	int32_t X2 = 0;
	int32_t B5 = 0;
	
	temp_raw = this->_bmp_get_temp_raw();

	X1 = ( (temp_raw - this->bmp_calibration->AC6) * ( this->bmp_calibration->AC5 / (pow(2,15)) ) );
	X2 = ( (this->bmp_calibration->MC * (pow(2,11)) ) / (X1 + this->bmp_calibration->MD) );
	B5 = X1 + X2;
	temp_cal = (B5 + 8) / (pow(2,4));
	
	return temp_cal / 10.0f;

}

/********* BMP180_Pico::_bmp_get_pressure_raw *******
*  Uncalibrated pressure reading.
*   Inputs: hardware oversampling (0 = 1 sample @ 4.5ms; 1 = 2 samples @ 7.5ms;
*           2 = 4 @ 13.5ms; 3 = 8 @ 25.5ms)
*  Outputs: Raw 19 bit ADC reading from sensor memory.
*/
uint32_t BMP180_Pico::_bmp_get_press_raw(void)
{
	uint8_t press_parts[3] = {0};
	int32_t press_raw = 0;
	int press_req_cmd = this->bmp_sampling_mode_ << 6;
	const uint8_t press_read_trig[2] = { BMP_READ_REQ_ADDR, static_cast<uint8_t>(press_req_cmd) };
	const uint8_t press_read_addr = BMP_READ_ADDR;

	i2c_write_blocking(i2c0, BMP_ADDR, press_read_trig, 2, true);
	sleep_us(bmp_oss_delay_table[this->bmp_sampling_mode_]);
	i2c_write_blocking(i2c0, BMP_ADDR, &press_read_addr, 1, true);
	i2c_read_blocking(i2c0, BMP_ADDR, press_parts, BMP_PRESS_READ_LEN, false);
	press_raw = ( ( press_parts[0] << 16 ) + (press_parts[1] << 8) + press_parts[2] )
				 >> ( 8 - this->bmp_sampling_mode_ );
	return press_raw;

}

/********* BMP180_Pico::bmp_get_pressure *******
*  Calibrated presserature reading (public function)
*   Inputs: Uncalibrated presserature reading, device calibration struct. See p. 15 of
*           the BMP180 datasheet (April 2013), hardware oversampling oss
*           (see _bmp_get_press_raw())
*  Outputs: Calibrated absolute pressure in pascals.
*/
int32_t BMP180_Pico::bmp_get_pressure(void)
{
	int32_t press_raw = 0;
	int32_t press = 0;

	int32_t B6 = 0;
	int32_t X1 = 0;
	int32_t X2 = 0;
	int32_t X3 = 0;
	int32_t B3 = 0;
	uint32_t B4 = 0;
	uint32_t B7 = 0;

	press_raw = _bmp_get_press_raw();

	B6 = this->bmp_calibration->B5 - 4000;
	X1 = ( this->bmp_calibration->B2 * ( B6 * B6 / pow(2, 12) ) ) / pow(2, 11);
	X2 = this->bmp_calibration->AC2 * B6 / pow(2, 11);
	X3 = X1 + X2;

	B3 = (((this->bmp_calibration->AC1 * 4 + X3) << this->bmp_sampling_mode_) + 2) / 4;
	X1 = this->bmp_calibration->AC3 * B6 / pow(2, 13);
	X2 = ( this->bmp_calibration->B1 * ( B6 * B6 / pow(2, 12) ) ) / pow(2, 16);
	X3 = ((X1 + X2) + 2) / pow(2, 2);

	B4 = this->bmp_calibration->AC4 * (uint32_t)(X3 + 32768) / pow(2, 15);
	B7 = ((uint32_t)press_raw - B3) * (50000 >> this->bmp_sampling_mode_);
	if (B7 < 0x80000000UL)
	{
		press = (B7 * 2) / B4;
	}
	else
	{
		press = (B7 / B4) * 2;
	}

	X1 = (press / pow(2, 8)) * (press / pow(2, 8));
	X1 = (X1 * 3038) / pow(2, 16);
	X2 = (-7357 * press) / pow(2, 16);
	press = press + (X1 + X2 + 3791) / pow(2, 4);

	// Absolute pressure (relative to sea level)
	return press; // return in Pa

}