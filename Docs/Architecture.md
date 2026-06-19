# Architecture

## System Overview

The project runs on an assembled custom CC1310 PCB with an AHT20 environmental sensor and LIS2DW12 accelerometer on a shared I2C bus. UART, I2C, and RF transmission have been verified on the hardware.

```text
Custom CC1310 PCB
  |
  +-- UART diagnostics
  |
  +-- I2C bus
  |     |
  |     +-- AHT20 at 0x38
  |     |
  |     +-- LIS2DW12 at 0x19
  |
  +-- LIS2DW12 INT1/INT2 wiring
  |     |
  |     +-- DIO15 / DIO14 (not yet handled by firmware)
  |
  +-- Sub-1 GHz radio through EasyLink
```

The AHT20 is verified at detection level only. An AHT20 measurement driver is not present.

## Runtime Architecture

`main()` initializes the board and starts two TI-RTOS tasks.

### Sensor Task

The sensor task:

1. Opens the UART at 115200 baud.
2. Initializes the shared I2C bus at 400 kHz.
3. Initializes the LIS2DW12 driver for address `0x19`, 25 Hz, and +/-2 g.
4. Verifies `WHO_AM_I = 0x44`.
5. Configures low-power polling mode.
6. Polls raw XYZ samples.
7. Converts XYZ values to mg using integer arithmetic.
8. Calculates acceleration magnitude.
9. Updates the motion engine.
10. Prints acceleration and motion diagnostics over UART.

The task currently polls once per second. It does not wait on a sensor interrupt.

### EasyLink Task

The EasyLink task retains the asynchronous transmit/receive echo behavior from the TI example base. This path has verified RF transmission, but its payload is still example data. It is not an application-specific sensor packet implementation.

## Sensor Software Layers

```text
sensor_task.c
  Application sequencing and UART diagnostics
        |
        +-- motion_engine.c
        |     Threshold classification and motion state
        |
        +-- lis2dw12_driver.c
              Device configuration, raw reads, mg conversion, magnitude
                    |
                    +-- lis2dw12_port_ti.c
                          ST driver context to TI I2C adapter
                                |
                                +-- i2c_bus.c
                                      TI-RTOS I2C wrapper
                                            |
                                            +-- I2C_transfer()
```

The ST-compatible register layer under `third_party/st/` remains independent of application policy and UART output.

## Motion Engine

The implemented motion engine consumes polled acceleration magnitude and maintains:

- Still and moving states.
- Dynamic acceleration relative to 1 g.
- Motion, shock, severe-shock, drop, and sustained-high-g flags.
- Event counters.
- Movement duration.
- Maximum observed magnitude.

Thresholds and timing values come from `app_config.h`. The current implementation is software classification over polled samples; it is not interrupt-driven and does not provide event-driven wake-up.

## Configuration Framework

Configuration is compile-time and split by responsibility:

- `app_config.h`: UART diagnostics, LIS2DW12 I2C address, sample rate, full scale, motion thresholds, and timing constants.
- `board_pins.h`: custom PCB UART, I2C, INT1, and INT2 DIO assignments.
- `smartrf_settings/`: generated radio configuration used by EasyLink.
- `easylink/easylink_config.*`: EasyLink API and PHY configuration.

This is a centralized compile-time configuration framework. There is no runtime configuration storage or remote configuration protocol.

## Verified Boundaries

Implemented and verified:

- UART and I2C operation.
- Example-derived EasyLink RF transmission.
- AHT20 detection at `0x38`.
- LIS2DW12 detection at `0x19` and identity verification.
- XYZ polling, mg conversion, magnitude calculation, and rotation validation.
- Polling-based motion engine and compile-time configuration.

Not implemented:

- LIS2DW12 interrupt handling.
- Event-driven wake-up.
- Application-specific RF sensor packets.
- Deep sleep.
- CR2450 power optimization.
