# CC1310 LIS2DW12 Accelerometer Firmware Agent

You are developing firmware for a custom TI CC1310 IoT sensor board using SimpleLink SDK, TI-RTOS, and CCS.

This is not Arduino.
This is not ESP32.
Do not write Arduino-style code.

## Base Project

Use the existing `rfEasyLinkEchoTx` project as the firmware base.

## Confirmed Hardware

Custom PCB pinout:

```c
UART_TX = DIO3
UART_RX = DIO2

I2C_SCL = DIO4
I2C_SDA = DIO5

LIS2DW12_INT1 = DIO15
LIS2DW12_INT2 = DIO14
```

Verified hardware status:

```text
Board flashes via JTAG.
UART debug works.
RF transmit example works.
I2C hardware works.
AHT20 ACKs at 0x38.
LIS2DW12 responds at 0x19.
LIS2DW12 WHO_AM_I register 0x0F returns 0x44.
```

Known probe result:

```text
AHT20 0x38 ACK status=0x18
LIS2DW12 0x18 NACK WHO=0x00
LIS2DW12 0x19 ACK WHO=0x44
```

Therefore, do not waste time re-debugging basic I2C hardware. The task is firmware development.

## Current Scope

Develop only the LIS2DW12 accelerometer firmware.

Do not implement:

* AHT20 driver
* RF packet format
* motion engine
* deep sleep
* final README

Those will be done later after accelerometer operation is confirmed.

## Architecture Goal

Use a clean, professional structure:

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

## Design Principle

Use STMicroelectronics’ official LIS2DW12 platform-independent driver as the register-level driver if practical.

Use the same architecture pattern as Zephyr:

```text
ST lis2dw12_reg.c/.h
        ↑
stmdev_ctx_t
        ↑
TI transport adapter
        ↑
TI I2C_transfer()
```

The TI transport adapter must connect ST’s `stmdev_ctx_t` read/write callbacks to the project’s `i2c_bus.c`.

## Layer Responsibilities

### `i2c_bus.c`

Responsible for TI-RTOS I2C handling only.

It should provide:

```c
i2c_bus_init()
i2c_bus_read_reg()
i2c_bus_write_reg()
i2c_bus_write()
i2c_bus_read()
```

It may use TI APIs such as:

```c
I2C_open()
I2C_transfer()
```

### `lis2dw12_port_ti.c`

Responsible for adapting TI I2C to ST’s driver context.

It should provide:

```c
lis2dw12_ti_ctx_init()
lis2dw12_ti_read_reg()
lis2dw12_ti_write_reg()
```

This file should not contain product logic.

### `lis2dw12_driver.c`

Responsible for the clean application-facing accelerometer API.

It should provide:

```c
lis2dw12_driver_init()
lis2dw12_driver_read_id()
lis2dw12_driver_configure_basic()
lis2dw12_driver_read_raw()
lis2dw12_driver_read_mg()
lis2dw12_driver_get_status()
```

### `sensor_task.c`

Responsible for calling the driver and printing UART diagnostics.

UART prints should not be inside the low-level ST driver.

## Driver Rules

The driver must be:

* Generic
* Reusable
* Clearly commented
* Defensive
* Testable
* Low-risk
* Suitable for battery-powered firmware later

Do not hardcode board pins inside the LIS2DW12 driver.

Do not put motion thresholds inside the LIS2DW12 driver.

Do not put RF packet fields inside the LIS2DW12 driver.

Do not mix app logic with register transport code.

## Error Handling

Use explicit return codes.

No silent failure.

Every public function should validate arguments.

Every I2C transfer failure should propagate a meaningful error.

Suggested status enum:

```c
typedef enum
{
    LIS2DW12_DRIVER_OK = 0,
    LIS2DW12_DRIVER_ERR_ARG,
    LIS2DW12_DRIVER_ERR_I2C,
    LIS2DW12_DRIVER_ERR_ID,
    LIS2DW12_DRIVER_ERR_CONFIG,
    LIS2DW12_DRIVER_ERR_NOT_READY
} lis2dw12_driver_status_t;
```

## UART Diagnostic Style

Keep UART diagnostics readable:

```text
CC1310 accelerometer firmware boot
I2C init OK
LIS2DW12 detected at 0x19
LIS2DW12 WHO_AM_I = 0x44 OK
LIS2DW12 basic config OK
ACC raw: x=..., y=..., z=...
ACC mg:  x=..., y=..., z=..., |g|=...
```

Error examples:

```text
ERR: I2C init failed
ERR: LIS2DW12 WHO_AM_I read failed
ERR: LIS2DW12 unexpected WHO_AM_I = 0x00
ERR: LIS2DW12 basic config failed
ERR: LIS2DW12 XYZ read failed
```

## Milestones

### Milestone 1 — Clean driver skeleton

Create files and compile successfully.

No advanced features.

### Milestone 2 — WHO_AM_I through final driver path

Read WHO_AM_I using:

```text
sensor_task.c
 -> lis2dw12_driver.c
 -> lis2dw12_port_ti.c
 -> i2c_bus.c
 -> TI I2C_transfer()
```

Expected result:

```text
LIS2DW12 WHO_AM_I = 0x44 OK
```

### Milestone 3 — Basic accelerometer configuration

Configure a known polling mode.

Use conservative settings:

* Low-power or low-noise mode suitable for later CR2450 operation.
* ±2 g full scale initially.
* 12.5 Hz or 25 Hz output data rate.
* X/Y/Z enabled.

### Milestone 4 — Raw XYZ polling

Read raw X/Y/Z values.

Print raw values over UART.

### Milestone 5 — mg conversion

Convert raw readings to mg based on selected full-scale and mode.

Print:

```text
ACC mg: x=..., y=..., z=..., |g|=...
```

When board is stationary, total acceleration should be approximately 1000 mg.

### Milestone 6 — Stability test

Print readings periodically.

Confirm:

* No I2C failure
* No stuck zeros
* Resting magnitude near 1 g
* Axis signs change when board is rotated
* UART output remains readable

## Current Instruction

Implement accelerometer firmware only.

Do not add interrupts yet.

Do not add motion classification yet.

Do not add RF packet changes yet.

Do not write README yet.

Once accelerometer operation is confirmed, README will be written separately.
