#ifndef BMP180_PICO_H
#define BMP180_PICO_H

#include <math.h>
#include <stdio.h>
#include "hardware/i2c.h"

#define BMP_ADDR 0x77
#define BMP_READ_REQ_ADDR 0xF4
#define BMP_TEMP_REQ_CMD 0x2E
#define BMP_PRESS_REQ_CMD 0x34
#define BMP_READ_ADDR 0xF6
#define BMP_TEMP_READ_LEN 2
#define BMP_PRESS_READ_LEN 3
#define BMP_CALIB_R_ADDR 0xAA
#define BMP_CALIB_READ_LEN 22 

typedef struct bmp_calib_bin bmp_calib_bin_t;
struct bmp_calib_bin
{
	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;

	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;

	int32_t B5;
};

enum bmp_sampling_mode
{
	bmp_ultralowpower = 0, // lowest power mode (fastest read delay)
	bmp_standard = 1,	   // standard mode
	bmp_highress = 2,	   // high-res mode
	bmp_ultrahighres = 3   // ultra high res mode (longest read)
};

const int16_t bmp_oss_delay_table[4] = {
	4500,
	7500,
	13500,
	25500
};

class BMP180_Pico {
    public:
     BMP180_Pico(const int bmp_sampling_mode = bmp_standard);
	 int bmp_begin(void);
     float bmp_get_temperature(void);
	 int32_t bmp_get_pressure(void);

    private:
     bmp_calib_bin_t *bmp_calibration = nullptr;
	 int bmp_sampling_mode_ = 0;
	 
	 int _bmp_get_calib_data(bmp_calib_bin_t *calib_bin);
	 uint16_t _bmp_get_temp_raw(void);
	 uint32_t _bmp_get_press_raw();
};

#endif