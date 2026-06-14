# CC1310 Custom IoT Sensor

Firmware for a custom low-power IoT sensor node based on the Texas Instruments CC1310 sub-1 GHz wireless MCU. The current repository is focused on board bring-up, UART diagnostics, I2C sensor communication, and early accelerometer integration using the TI EasyLink example as the RF application base.

## Hardware Overview

The target hardware is a custom PCB using:

- TI CC1310 wireless MCU for the application processor and sub-1 GHz radio.
- AHT20 temperature and humidity sensor detected on the I2C bus at `0x38`.
- ST LIS2DW12 low-power accelerometer detected on the I2C bus at `0x19`.
- UART debug output for bring-up and verification.

The firmware project is currently organized around the CC1310 LaunchPad/TI-RTOS board support files while the custom PCB interface is brought up incrementally.

## Firmware Overview

The firmware is based on TI's `rfEasyLinkEchoTx` example and retains EasyLink support for sub-1 GHz radio bring-up. A sensor task initializes UART diagnostics, opens the I2C bus, detects the LIS2DW12 at `0x19`, verifies `WHO_AM_I = 0x44`, and supports ongoing LIS2DW12 driver and polling development.

EasyLink remains present as the RF abstraction layer, but application-specific RF sensor packets are still planned work.

## Current Status

Completed:

- UART
- I2C
- AHT20 detection at `0x38`
- LIS2DW12 detection at `0x19`
- LIS2DW12 `WHO_AM_I = 0x44` verification

In Progress:

- LIS2DW12 driver
- Accelerometer polling

Planned:

- Motion engine
- RF sensor packets
- LIS2DW12 interrupts
- Low power operation

## Repository Layout

```text
Docs/
  Architecture.md
  FirmwareStructure.md
Firmware/
  rfEasyLinkEchoTx/
    CCS project, board support, EasyLink files, sensor task, and sensor drivers
Hardware/
  Reserved for custom PCB design files and hardware documentation
```

## Build Environment

The firmware is a TI Code Composer Studio project for the CC1310 and TI-RTOS/SimpleLink SDK environment. The project metadata needed for CCS import should be committed with the firmware:

- `Firmware/rfEasyLinkEchoTx/.project`
- `Firmware/rfEasyLinkEchoTx/.cproject`
- `Firmware/rfEasyLinkEchoTx/.ccsproject`
- `Firmware/rfEasyLinkEchoTx/targetConfigs/`

Generated `Debug/` and `Release/` outputs are intentionally ignored and should be rebuilt locally.

## Publication Notes

Commit source, project metadata, RF settings, target configuration, documentation, and hardware design files when they are added. Do not commit generated build output, compiler intermediates, local CCS workspace metadata, editor caches, or local automation notes.
