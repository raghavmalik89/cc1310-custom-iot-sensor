# CC1310 Custom IoT Sensor

## Project Overview

Firmware for a custom sensor based on the Texas Instruments CC1310 and intended for CR2450 battery operation. The current application combines scheduled environmental sensing, interrupt-triggered motion observation, motion classification, UART diagnostics, and the TI EasyLink RF baseline.

Product direction includes environmental monitoring, motion logging, shock logging, drop logging, and transport monitoring.

## Hardware Overview

| Function | Device or pin |
|---|---|
| MCU | TI CC1310 |
| Temperature/humidity | AHT20 at I2C address `0x38` |
| Accelerometer | LIS2DW12 at I2C address `0x19` |
| UART RX | DIO2 |
| UART TX | DIO3 |
| I2C SCL | DIO4 |
| I2C SDA | DIO5 |
| LIS2DW12 INT2 | DIO14 |
| LIS2DW12 INT1 | DIO15 |

Verified interfaces are UART, I2C, and the existing EasyLink RF baseline. The LIS2DW12 identity has been verified with `WHO_AM_I = 0x44`.

## Repository Structure

```text
Docs/                         Architecture and firmware documentation
Firmware/rfEasyLinkEchoTx/    CCS project and firmware
  aht2x.*                     AHT20 measurement driver
  i2c_bus.*                   Shared TI I2C wrapper
  lis2dw12_driver.*           Accelerometer configuration and conversion
  lis2dw12_port_ti.*          ST register-driver TI adapter
  motion_engine.*             Motion classification and event state
  sensor_task.*               Scheduling, caching, interrupts, observations
  easylink/                   TI EasyLink implementation
  smartrf_settings/           RF configuration
  third_party/st/             LIS2DW12 register access layer
Hardware/                     Custom PCB assets
```

## Current Firmware State

Implemented and verified:

- UART diagnostics.
- I2C bus operation.
- EasyLink RF baseline operation.
- AHT20 detection and temperature/humidity measurements.
- LIS2DW12 detection and `WHO_AM_I = 0x44` verification.
- Accelerometer XYZ reads and integer mg conversion.
- Acceleration magnitude calculation.
- Configurable motion engine for movement, shock, severe shock, drop, and sustained high-G classification.
- Central application configuration for sensor schedules, thresholds, timing, and diagnostics.
- LIS2DW12 activity interrupt routed through INT1 to DIO15.
- Interrupt-triggered motion observation windows.
- Cached environmental measurements with age tracking.

The EasyLink task still uses example-derived payloads. Sensor and motion event packets are not implemented.

## Runtime Architecture

### Idle

Idle refers to the period when no motion observation window is active; deep sleep is not currently used.

- The AHT20 is polled on application schedules.
- Cached temperature is updated every 60 seconds.
- Cached humidity is updated every 300 seconds.
- The LIS2DW12 remains configured for activity detection.
- Continuous XYZ sampling is disabled outside an observation window.

### Movement

1. LIS2DW12 activity asserts INT1 on DIO15.
2. The interrupt starts or extends a 30-second observation window.
3. XYZ acceleration is sampled every 100 ms during the window.
4. Samples are converted to mg and acceleration magnitude.
5. `motion_engine` classifies motion, shock, severe shock, drop, and sustained high-G conditions.
6. The completed observation produces a UART summary.

The observation summary includes:

- Duration.
- Sample count.
- Final motion state and accumulated motion flags.
- Maximum acceleration magnitude (`maxG`, represented in mg).
- Cached temperature.
- Cached humidity.
- Environmental-data age.

### Future

- Deep sleep or standby integration.
- RF event packets carrying observation summaries.
- CR2450 battery optimization.
- Removal or reduction of bring-up diagnostics.

## Current Limitations

- Deep sleep is not implemented.
- Observation summaries are currently UART-only.
- RF event reporting is not implemented.
- Continuous debug instrumentation remains enabled for development.

## Build Environment

The firmware is a TI Code Composer Studio project using TI-RTOS, the SimpleLink CC13x0 SDK, and the TI ARM compiler. Import `Firmware/rfEasyLinkEchoTx/` into CCS with its TI-RTOS build dependency.
