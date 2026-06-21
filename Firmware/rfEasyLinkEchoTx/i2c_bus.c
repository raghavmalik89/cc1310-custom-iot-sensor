#include "i2c_bus.h"

#include <stdbool.h>

#include <ti/drivers/I2C.h>

#include "Board.h"

static I2C_Handle i2cHandle;
static bool i2cReady;

i2c_bus_status_t i2c_bus_init(void)
{
    I2C_Params params;

    if (i2cReady)
    {
        return I2C_BUS_OK;
    }

    I2C_init();
    I2C_Params_init(&params);
    params.bitRate = I2C_400kHz;

    i2cHandle = I2C_open(Board_I2C0, &params);
    if (i2cHandle == NULL)
    {
        return I2C_BUS_ERR_OPEN;
    }

    i2cReady = true;
    return I2C_BUS_OK;
}

i2c_bus_status_t i2c_bus_read_reg(uint8_t dev_addr,
                                  uint8_t reg_addr,
                                  uint8_t *data,
                                  uint16_t len)
{
    I2C_Transaction transaction;

    if ((data == NULL) || (len == 0U))
    {
        return I2C_BUS_ERR_ARG;
    }

    if (!i2cReady || (i2cHandle == NULL))
    {
        return I2C_BUS_ERR_NOT_READY;
    }

    transaction.slaveAddress = dev_addr;
    transaction.writeBuf = &reg_addr;
    transaction.writeCount = 1;
    transaction.readBuf = data;
    transaction.readCount = len;

    return I2C_transfer(i2cHandle, &transaction) ? I2C_BUS_OK : I2C_BUS_ERR_TRANSFER;
}

i2c_bus_status_t i2c_bus_write_reg(uint8_t dev_addr,
                                   uint8_t reg_addr,
                                   const uint8_t *data,
                                   uint16_t len)
{
    I2C_Transaction transaction;
    uint8_t txBuf[9];
    uint16_t i;

    if ((data == NULL) || (len == 0U) || (len > (sizeof(txBuf) - 1U)))
    {
        return I2C_BUS_ERR_ARG;
    }

    if (!i2cReady || (i2cHandle == NULL))
    {
        return I2C_BUS_ERR_NOT_READY;
    }

    txBuf[0] = reg_addr;
    for (i = 0; i < len; i++)
    {
        txBuf[i + 1U] = data[i];
    }

    transaction.slaveAddress = dev_addr;
    transaction.writeBuf = txBuf;
    transaction.writeCount = len + 1U;
    transaction.readBuf = NULL;
    transaction.readCount = 0;

    return I2C_transfer(i2cHandle, &transaction) ? I2C_BUS_OK : I2C_BUS_ERR_TRANSFER;
}

i2c_bus_status_t i2c_bus_write(uint8_t dev_addr,
                               const uint8_t *data,
                               uint16_t len)
{
    I2C_Transaction transaction;

    if ((data == NULL) || (len == 0U))
    {
        return I2C_BUS_ERR_ARG;
    }

    if (!i2cReady || (i2cHandle == NULL))
    {
        return I2C_BUS_ERR_NOT_READY;
    }

    transaction.slaveAddress = dev_addr;
    transaction.writeBuf = (void *)data;
    transaction.writeCount = len;
    transaction.readBuf = NULL;
    transaction.readCount = 0;

    return I2C_transfer(i2cHandle, &transaction) ? I2C_BUS_OK : I2C_BUS_ERR_TRANSFER;
}

i2c_bus_status_t i2c_bus_read(uint8_t dev_addr,
                              uint8_t *data,
                              uint16_t len)
{
    I2C_Transaction transaction;

    if ((data == NULL) || (len == 0U))
    {
        return I2C_BUS_ERR_ARG;
    }

    if (!i2cReady || (i2cHandle == NULL))
    {
        return I2C_BUS_ERR_NOT_READY;
    }

    transaction.slaveAddress = dev_addr;
    transaction.writeBuf = NULL;
    transaction.writeCount = 0;
    transaction.readBuf = data;
    transaction.readCount = len;

    return I2C_transfer(i2cHandle, &transaction) ? I2C_BUS_OK : I2C_BUS_ERR_TRANSFER;
}

I2C_Handle i2c_bus_get_handle(void)
{
    return i2cReady ? i2cHandle : NULL;
}
