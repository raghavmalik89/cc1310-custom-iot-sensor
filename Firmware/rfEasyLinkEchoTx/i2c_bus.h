#ifndef I2C_BUS_H_
#define I2C_BUS_H_

#include <stdint.h>

typedef enum
{
    I2C_BUS_OK = 0,
    I2C_BUS_ERR_ARG,
    I2C_BUS_ERR_OPEN,
    I2C_BUS_ERR_TRANSFER,
    I2C_BUS_ERR_NOT_READY
} i2c_bus_status_t;

i2c_bus_status_t i2c_bus_init(void);
i2c_bus_status_t i2c_bus_read_reg(uint8_t dev_addr,
                                  uint8_t reg_addr,
                                  uint8_t *data,
                                  uint16_t len);
i2c_bus_status_t i2c_bus_write_reg(uint8_t dev_addr,
                                   uint8_t reg_addr,
                                   const uint8_t *data,
                                   uint16_t len);
i2c_bus_status_t i2c_bus_write(uint8_t dev_addr,
                               const uint8_t *data,
                               uint16_t len);
i2c_bus_status_t i2c_bus_read(uint8_t dev_addr,
                              uint8_t *data,
                              uint16_t len);

#endif /* I2C_BUS_H_ */
