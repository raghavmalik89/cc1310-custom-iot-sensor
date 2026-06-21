# LIS2DW12 CC1310 Driver Development Skill

## Purpose

Build a clean LIS2DW12 accelerometer driver for a TI CC1310 custom board using SimpleLink SDK, TI-RTOS, and CCS.

The goal is to make accelerometer operation reliable before adding motion logic, RF packets, or low-power sleep.

## Known Hardware Facts

```c
#define BOARD_UART_TX_DIO        3
#define BOARD_UART_RX_DIO        2

#define BOARD_I2C_SCL_DIO        4
#define BOARD_I2C_SDA_DIO        5

#define BOARD_LIS2DW12_INT1_DIO  15
#define BOARD_LIS2DW12_INT2_DIO  14
```

LIS2DW12 facts:

```c
#define LIS2DW12_I2C_ADDR        0x19
#define LIS2DW12_WHO_AM_I_REG    0x0F
#define LIS2DW12_WHO_AM_I_VALUE  0x44
```

Hardware has already confirmed:

```text
LIS2DW12 at 0x19 responds with WHO_AM_I = 0x44.
```

## Development Strategy

Use a layered design.

```text
Application task
    ↓
Clean LIS2DW12 app-facing driver
    ↓
TI port adapter
    ↓
Shared I2C bus wrapper
    ↓
TI-RTOS I2C_transfer()
```

## File Layout

Recommended files:

```text
app_config.h
board_pins.h

i2c_bus.h
i2c_bus.c

third_party/st/lis2dw12_reg.h
third_party/st/lis2dw12_reg.c

lis2dw12_port_ti.h
lis2dw12_port_ti.c

lis2dw12_driver.h
lis2dw12_driver.c

sensor_task.c
```

## Important Rule

Keep the ST driver generic and unchanged where possible.

Do not edit `third_party/st/lis2dw12_reg.c` unless absolutely necessary.

Put TI-specific code in:

```text
lis2dw12_port_ti.c
```

Put product-level accelerometer API in:

```text
lis2dw12_driver.c
```

Put UART diagnostics in:

```text
sensor_task.c
```

## `board_pins.h`

Use this for board pin definitions only:

```c
#ifndef BOARD_PINS_H_
#define BOARD_PINS_H_

#define BOARD_UART_TX_DIO        3
#define BOARD_UART_RX_DIO        2

#define BOARD_I2C_SCL_DIO        4
#define BOARD_I2C_SDA_DIO        5

#define BOARD_LIS2DW12_INT1_DIO  15
#define BOARD_LIS2DW12_INT2_DIO  14

#endif
```

## `app_config.h`

Use this for application-level settings only:

```c
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#define APP_UART_DEBUG_ENABLE            1

#define APP_ACCEL_I2C_ADDR               0x19
#define APP_ACCEL_SAMPLE_HZ              25

#define APP_ACCEL_FULL_SCALE_G           2

#endif
```

Do not put register addresses here.

## I2C Bus API

Create a shared I2C wrapper.

```c
typedef enum
{
    I2C_BUS_OK = 0,
    I2C_BUS_ERR_ARG,
    I2C_BUS_ERR_OPEN,
    I2C_BUS_ERR_TRANSFER,
    I2C_BUS_ERR_NOT_READY
} i2c_bus_status_t;
```

Required functions:

```c
i2c_bus_status_t i2c_bus_init(void);

i2c_bus_status_t i2c_bus_read_reg(uint8_t dev_addr,
                                  uint8_t reg_addr,
                                  uint8_t *data,
                                  uint16_t len);

i2c_bus_status_t i2c_bus_write_reg(uint8_t dev_addr,
                                   uint8_t reg_addr,
                                   const uint8_t *data,
                                   uint16_t len);
```

## TI Port Adapter

Create a port adapter compatible with ST’s `stmdev_ctx_t`.

Required functions:

```c
int32_t lis2dw12_ti_read_reg(void *handle,
                             uint8_t reg,
                             uint8_t *bufp,
                             uint16_t len);

int32_t lis2dw12_ti_write_reg(void *handle,
                              uint8_t reg,
                              const uint8_t *bufp,
                              uint16_t len);

void lis2dw12_ti_ctx_init(stmdev_ctx_t *ctx, uint8_t i2c_addr);
```

Expected behavior:

* Return `0` on success.
* Return negative value on failure.
* Use `i2c_bus_read_reg()` and `i2c_bus_write_reg()`.
* Store or pass the I2C address cleanly.
* Do not print UART messages here.

## Application-Facing Driver API

Create `lis2dw12_driver.h`.

```c
#ifndef LIS2DW12_DRIVER_H_
#define LIS2DW12_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

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

#endif
```

## Basic Configuration Requirement

Configure the LIS2DW12 into a simple known polling mode.

Recommended initial settings:

```text
I2C address: 0x19
Full scale: ±2 g
ODR: 25 Hz
Mode: low-power or low-noise mode
Interrupts: disabled for now
FIFO: disabled for now
```

The goal is stable polling, not advanced event detection.

## XYZ Reading Requirement

Read acceleration registers through ST driver functions where possible.

Convert raw values to mg only after confirming the selected full-scale and mode.

Static board expectation:

```text
sqrt(x^2 + y^2 + z^2) ≈ 1000 mg
```

Use integer math where practical.

Avoid floating point unless clearly necessary.

## UART Diagnostics

UART diagnostics belong in the task layer.

Good output:

```text
CC1310 LIS2DW12 test boot
I2C init OK
LIS2DW12 WHO_AM_I = 0x44 OK
LIS2DW12 config OK
ACC raw: x=123 y=-45 z=16200
ACC mg:  x=8 y=-3 z=998 mag=1001
```

Bad output:

```text
read failed
error
0
```

Always identify what failed.

## Do Not Implement Yet

Do not implement these until polling is confirmed:

* INT1 interrupt on DIO15
* drop detection
* shock detection
* movement duration
* sustained high-G classification
* RF packet integration
* deep sleep
* README

## Quality Checklist

Before finishing, check:

* Code compiles in CCS.
* Headers have include guards.
* Public APIs validate pointers.
* I2C failure is propagated.
* No UART prints inside ST low-level driver.
* Board pins are not inside generic driver.
* Magic numbers are replaced with named constants.
* Driver does not assume AHT20 exists.
* Driver does not modify RF logic.
* Static acceleration magnitude can be checked from UART.
