#include "third_party/st/lis2dw12_reg.h"

int32_t lis2dw12_read_reg(const stmdev_ctx_t *ctx,
                          uint8_t reg,
                          uint8_t *data,
                          uint16_t len)
{
    if ((ctx == 0) || (ctx->read_reg == 0) || (data == 0) || (len == 0U))
    {
        return -1;
    }

    return ctx->read_reg(ctx->handle, reg, data, len);
}

int32_t lis2dw12_write_reg(const stmdev_ctx_t *ctx,
                           uint8_t reg,
                           const uint8_t *data,
                           uint16_t len)
{
    if ((ctx == 0) || (ctx->write_reg == 0) || (data == 0) || (len == 0U))
    {
        return -1;
    }

    return ctx->write_reg(ctx->handle, reg, data, len);
}

int32_t lis2dw12_device_id_get(const stmdev_ctx_t *ctx, uint8_t *buff)
{
    return lis2dw12_read_reg(ctx, LIS2DW12_WHO_AM_I, buff, 1);
}

int32_t lis2dw12_status_get(const stmdev_ctx_t *ctx, uint8_t *buff)
{
    return lis2dw12_read_reg(ctx, LIS2DW12_STATUS, buff, 1);
}

int32_t lis2dw12_acceleration_raw_get(const stmdev_ctx_t *ctx, int16_t *data)
{
    uint8_t buff[6];
    int32_t ret;

    if (data == 0)
    {
        return -1;
    }

    ret = lis2dw12_read_reg(ctx, LIS2DW12_OUT_X_L, buff, sizeof(buff));
    if (ret != 0)
    {
        return ret;
    }

    data[0] = (int16_t)((uint16_t)buff[1] << 8 | buff[0]);
    data[1] = (int16_t)((uint16_t)buff[3] << 8 | buff[2]);
    data[2] = (int16_t)((uint16_t)buff[5] << 8 | buff[4]);

    return 0;
}
