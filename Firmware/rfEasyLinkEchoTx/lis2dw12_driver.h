#ifndef LIS2DW12_DRIVER_H_
#define LIS2DW12_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    LIS2DW12_DRIVER_OK = 0,
    LIS2DW12_DRIVER_ERR_ARG,
    LIS2DW12_DRIVER_ERR_I2C,
    LIS2DW12_DRIVER_ERR_ID,
    LIS2DW12_DRIVER_ERR_CONFIG,
    LIS2DW12_DRIVER_ERR_NOT_READY
} lis2dw12_driver_status_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} lis2dw12_raw_t;

typedef struct
{
    int32_t x_mg;
    int32_t y_mg;
    int32_t z_mg;
    uint32_t magnitude_mg;
} lis2dw12_accel_mg_t;

typedef struct
{
    uint8_t i2c_addr;
    uint8_t full_scale_g;
    uint16_t odr_hz;
} lis2dw12_driver_config_t;

lis2dw12_driver_status_t lis2dw12_driver_init(const lis2dw12_driver_config_t *cfg);
lis2dw12_driver_status_t lis2dw12_driver_read_id(uint8_t *whoami);
lis2dw12_driver_status_t lis2dw12_driver_configure_basic(void);
lis2dw12_driver_status_t lis2dw12_driver_read_raw(lis2dw12_raw_t *raw);
lis2dw12_driver_status_t lis2dw12_driver_read_mg(lis2dw12_accel_mg_t *accel);
lis2dw12_driver_status_t lis2dw12_driver_get_status(uint8_t *status);
lis2dw12_driver_status_t lis2dw12_enable_activity_interrupt(void);
lis2dw12_driver_status_t lis2dw12_clear_activity_interrupt(void);

#endif /* LIS2DW12_DRIVER_H_ */
