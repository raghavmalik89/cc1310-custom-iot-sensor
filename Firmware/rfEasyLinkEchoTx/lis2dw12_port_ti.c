#include "lis2dw12_port_ti.h"

#include "i2c_bus.h"

static uint8_t lis2dw12PortI2cAddr;

int32_t lis2dw12_ti_read_reg(void *handle,
                             uint8_t reg,
                             uint8_t *bufp,
                             uint16_t len)
{
    uint8_t i2cAddr;

    if ((handle == 0) || (bufp == 0) || (len == 0U))
    {
        return -1;
    }

    i2cAddr = *(uint8_t *)handle;
    return (i2c_bus_read_reg(i2cAddr, reg, bufp, len) == I2C_BUS_OK) ? 0 : -1;
}

int32_t lis2dw12_ti_write_reg(void *handle,
                              uint8_t reg,
                              const uint8_t *bufp,
                              uint16_t len)
{
    uint8_t i2cAddr;

    if ((handle == 0) || (bufp == 0) || (len == 0U))
    {
        return -1;
    }

    i2cAddr = *(uint8_t *)handle;
    return (i2c_bus_write_reg(i2cAddr, reg, bufp, len) == I2C_BUS_OK) ? 0 : -1;
}

void lis2dw12_ti_ctx_init(stmdev_ctx_t *ctx, uint8_t i2c_addr)
{
    if (ctx == 0)
    {
        return;
    }

    lis2dw12PortI2cAddr = i2c_addr;
    ctx->read_reg = lis2dw12_ti_read_reg;
    ctx->write_reg = lis2dw12_ti_write_reg;
    ctx->handle = &lis2dw12PortI2cAddr;
}
