# Architecture

## Purpose

This project brings up a custom CC1310-based IoT sensor node with local sensor validation first. RF sensor packet transmission, sensor interrupts, and low-power behavior are planned architecture areas. The current architecture separates the TI EasyLink RF example path from a sensor task used for UART and I2C bring-up.

## High-Level Blocks

```text
Custom PCB
  |
  +-- CC1310 MCU
        |
        +-- UART diagnostics
        |
        +-- I2C bus
        |     |
        |     +-- AHT20 temperature/humidity sensor at 0x38
        |     |
        |     +-- LIS2DW12 accelerometer at 0x19
        |
        +-- EasyLink RF stack
```

## Firmware Tasks

The application starts TI-RTOS BIOS after board initialization. Two application paths are present:

- Sensor bring-up task: opens UART, initializes I2C, detects the LIS2DW12 at `0x19`, verifies `WHO_AM_I = 0x44`, and supports ongoing driver and polling development.
- EasyLink echo task: preserves the TI EasyLink transmit/receive example behavior used as the RF baseline.

## Current Data Flow

```text
LIS2DW12 -> I2C bus wrapper -> LIS2DW12 driver -> sensor task -> UART log
```

The AHT20 has been detected at `0x38` during bring-up, but this repository should not be treated as having a completed AHT20 measurement driver unless one is added explicitly.

RF sensor packets are planned. The current EasyLink code is still example-oriented and should be treated as the transport baseline, not as the final sensor packet protocol.

## Hardware Roles

- CC1310: main MCU, TI-RTOS host, I2C controller, UART debug endpoint, and sub-1 GHz radio.
- AHT20: I2C temperature/humidity sensor currently validated at detection level at `0x38`.
- LIS2DW12: I2C accelerometer currently detected at `0x19`, with `WHO_AM_I = 0x44` verified and driver/polling development in progress.
- Custom PCB: target product hardware for integrating the sensors and radio design.

## Boundaries

Implemented and documented behavior is limited to the current bring-up state. Motion classification, RF sensor payloads, LIS2DW12 interrupts, and low-power operation are planned architecture areas and are not documented here as completed features.
