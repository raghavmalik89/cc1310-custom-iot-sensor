#ifndef LIS2DW12_REG_H_
#define LIS2DW12_REG_H_

#include <stdint.h>

#define LIS2DW12_I2C_ADD_L       0x18
#define LIS2DW12_I2C_ADD_H       0x19

#define LIS2DW12_WHO_AM_I        0x0F
#define LIS2DW12_ID              0x44

#define LIS2DW12_CTRL1           0x20
#define LIS2DW12_CTRL2           0x21
#define LIS2DW12_CTRL6           0x25
#define LIS2DW12_STATUS          0x27
#define LIS2DW12_OUT_X_L         0x28

#define LIS2DW12_STATUS_DRDY     0x01

typedef int32_t (*stmdev_write_ptr)(void *, uint8_t, const uint8_t *, uint16_t);
typedef int32_t (*stmdev_read_ptr)(void *, uint8_t, uint8_t *, uint16_t);

typedef struct
{
    stmdev_write_ptr write_reg;
    stmdev_read_ptr read_reg;
    void *handle;
} stmdev_ctx_t;

int32_t lis2dw12_read_reg(const stmdev_ctx_t *ctx,
                          uint8_t reg,
                          uint8_t *data,
                          uint16_t len);
int32_t lis2dw12_write_reg(const stmdev_ctx_t *ctx,
                           uint8_t reg,
                           const uint8_t *data,
                           uint16_t len);
int32_t lis2dw12_device_id_get(const stmdev_ctx_t *ctx, uint8_t *buff);
int32_t lis2dw12_status_get(const stmdev_ctx_t *ctx, uint8_t *buff);
int32_t lis2dw12_acceleration_raw_get(const stmdev_ctx_t *ctx, int16_t *data);

#endif /* LIS2DW12_REG_H_ */
