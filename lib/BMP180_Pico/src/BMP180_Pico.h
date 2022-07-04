#ifndef BMP180_PICO_H
#define BMP180_PICO_H

#include <math.h>
#include <stdio.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define BMP_ADDR 0x77
#define BMP_READ_REQ_ADDR 0xF4
#define BMP_TEMP_REQ_CMD 0x2E
#define BMP_PRESS_REQ_CMD 0x34
#define BMP_READ_ADDR 0xF6
#define BMP_TEMP_READ_LEN 2
#define BMP_PRESS_READ_LEN 3
#define BMP_CALIB_R_ADDR 0xAA
#define BMP_CALIB_READ_LEN 22 

typedef struct CalibrationData CalibrationData_t;
struct CalibrationData {
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

enum SamplingMode {
  kUltraLowPower = 0, // lowest power mode (fastest read delay)
  kStandard = 1,     // standard mode
  kHighRes = 2,     // high-res mode
  kUltraHighRes = 3   // ultra high res mode (longest read)
};

const int16_t kSamplingDelayTable[4] = {
    4500,
    7500,
    13500,
    25500
};

class BMP180_Pico {
  public:
   BMP180_Pico(const int sampling_mode = kStandard);
   int Begin(void);
   float GetTemperature(void);
   int32_t GetPressure(void);

  private:
   CalibrationData_t *calibration_data_ = nullptr;
   int sampling_mode_ = 0;
   
   int GetCalibrationData(CalibrationData_t *calib_table);
   uint16_t GetTemperatureRaw(void);
   uint32_t GetPressureRaw(void);
};

#endif