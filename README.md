# CC1310 Custom IoT Sensor

## Project Overview

Firmware for a custom battery-powered IoT sensor built around the Texas Instruments CC1310 sub-1 GHz wireless MCU. The project uses TI-RTOS, the SimpleLink CC13x0 SDK, and TI EasyLink as its current RF foundation.

The present firmware validates the custom PCB, polls a LIS2DW12 accelerometer, converts samples to engineering units, and feeds them into a configurable motion engine. Application-specific RF sensor packets and low-power wake behavior are not yet implemented.

## Hardware Overview

The assembled custom PCB includes:

- TI CC1310 wireless MCU.
- AHT20 temperature and humidity sensor detected over I2C at `0x38`.
- ST LIS2DW12 low-power accelerometer detected over I2C at `0x19`.
- UART on DIO3 (TX) and DIO2 (RX).
- I2C on DIO4 (SCL) and DIO5 (SDA).
- LIS2DW12 interrupt connections on DIO15 (INT1) and DIO14 (INT2).

Verified hardware operation:

- The custom PCB is assembled and programmable.
- UART diagnostics work.
- EasyLink RF transmission works using the existing example-derived RF task.
- I2C communication works.
- The LIS2DW12 returns `WHO_AM_I = 0x44`.

The interrupt pins are defined in the board configuration, but interrupt handling and event-driven wake-up are not implemented.

## Repository Structure

```text
Docs/
  Architecture.md          Firmware architecture and implemented boundaries
  FirmwareStructure.md     Firmware module and repository file guide
Firmware/
  rfEasyLinkEchoTx/        CCS project, application, drivers, motion engine,
                           EasyLink, RF settings, and board support
Hardware/                  Reserved for custom PCB files and images
```

No hardware images are currently present under `Hardware/`.

## Current Firmware Architecture

The application starts two TI-RTOS tasks:

- Sensor task: initializes UART and I2C, verifies the LIS2DW12, configures polling, reads XYZ acceleration, performs mg and magnitude conversion, updates the motion engine, and prints diagnostics.
- EasyLink echo task: retains the example-derived asynchronous RF transmit/receive flow used to verify radio operation.

The sensor path is layered:

```text
sensor_task
    |
    +-- motion_engine
    |
    +-- lis2dw12_driver
            |
            +-- lis2dw12_port_ti
                    |
                    +-- i2c_bus
                            |
                            +-- TI I2C driver
```

Configuration is centralized through:

- `app_config.h` for UART enablement, accelerometer settings, and motion thresholds/timing.
- `board_pins.h` for custom PCB UART, I2C, and LIS2DW12 interrupt pin assignments.

## Current Development Status

Completed and verified:

- Custom CC1310 PCB assembly and programming.
- UART operation.
- EasyLink RF transmit operation.
- I2C operation.
- AHT20 detection at `0x38`.
- LIS2DW12 detection at `0x19`.
- LIS2DW12 `WHO_AM_I = 0x44` verification.
- LIS2DW12 XYZ polling.
- Integer mg conversion.
- Acceleration magnitude calculation.
- Rotation validation.
- Threshold-based motion engine.
- Compile-time configuration framework.

The motion engine currently processes polled magnitude data and tracks motion state, event flags, counts, movement duration, dynamic acceleration, and maximum measured magnitude. It does not use LIS2DW12 hardware interrupts.

## Roadmap

- Implement LIS2DW12 interrupt handling.
- Add event-driven wake-up.
- Define and transmit application-specific RF sensor packets.
- Add deep-sleep operation.
- Optimize firmware and hardware behavior for CR2450 operation.

## Build Environment

The firmware is a TI Code Composer Studio project targeting the CC1310 with TI-RTOS and the SimpleLink CC13x0 SDK. Import `Firmware/rfEasyLinkEchoTx/` into CCS together with its TI-RTOS build dependency.

CCS project metadata and target configuration are retained in the repository. Generated `Debug/` and `Release/` outputs are ignored and should be rebuilt locally.
