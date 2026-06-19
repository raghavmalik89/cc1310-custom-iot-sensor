# Firmware Structure

## Project Layout

The active Code Composer Studio project is:

```text
Firmware/rfEasyLinkEchoTx/
```

It is derived from TI's CC1310 LaunchPad `rfEasyLinkEchoTx` example and now contains the custom PCB sensor and motion-processing modules.

```text
Firmware/rfEasyLinkEchoTx/
  app_config.h
  board_pins.h
  i2c_bus.c/.h
  lis2dw12_port_ti.c/.h
  lis2dw12_driver.c/.h
  motion_engine.c/.h
  sensor_task.c/.h
  rfEasyLinkEchoTx.c
  easylink/
  smartrf_settings/
  third_party/st/
  targetConfigs/
```

## Application Modules

- `rfEasyLinkEchoTx.c`: application entry point, board initialization, TI-RTOS startup, sensor task creation, and example-derived EasyLink echo task.
- `sensor_task.c` / `sensor_task.h`: UART diagnostics and the polling workflow for LIS2DW12 initialization, identity verification, XYZ reads, conversion, motion-engine updates, and status output.
- `motion_engine.c` / `motion_engine.h`: polling-based motion state, threshold event flags, counters, movement duration, dynamic acceleration, and maximum magnitude.
- `app_config.h`: centralized compile-time accelerometer settings, motion thresholds, timing values, and UART diagnostic enablement.
- `board_pins.h`: custom PCB UART, I2C, and LIS2DW12 interrupt pin assignments.

## Sensor and Bus Layers

- `i2c_bus.c` / `i2c_bus.h`: reusable TI-RTOS I2C wrapper for raw and register-oriented transfers.
- `lis2dw12_port_ti.c` / `lis2dw12_port_ti.h`: adapter between the ST `stmdev_ctx_t` callbacks and the shared TI I2C bus wrapper.
- `lis2dw12_driver.c` / `lis2dw12_driver.h`: application-facing LIS2DW12 initialization, polling configuration, identity/status reads, raw XYZ reads, integer mg conversion, and magnitude calculation.
- `third_party/st/lis2dw12_reg.c` / `third_party/st/lis2dw12_reg.h`: platform-independent LIS2DW12 register access layer.

## RF and Platform Files

- `easylink/`: TI EasyLink implementation and API configuration.
- `smartrf_settings/`: SmartRF-generated radio settings.
- `CC1310_LAUNCHXL.c` / `CC1310_LAUNCHXL.h`: TI driver configuration adapted as the current board-support base.
- `Board.h`: board abstraction used by the application and TI drivers.
- `ccfg.c`: CC1310 customer configuration.
- `CC1310_LAUNCHXL_TIRTOS.cmd`: linker command file.
- `targetConfigs/`: CCS target configuration for the CC1310.

The RF task currently sends example-derived EasyLink payloads. No sensor measurements or motion events are encoded into RF packets.

## CCS Project Metadata

The following files support CCS import and should remain committed:

- `.project`
- `.cproject`
- `.ccsproject`
- `targetConfigs/CC1310F128.ccxml`
- `targetConfigs/readme.txt`

Generated build directories and local workspace state are excluded by the root `.gitignore`.

## Configuration

The implemented configuration framework is compile-time:

- Accelerometer address: `0x19`.
- Accelerometer sample setting: 25 Hz.
- Accelerometer full scale: +/-2 g.
- Motion, shock, severe-shock, drop, and sustained-high-g thresholds.
- Movement stop and event burst timing constants.
- Custom board DIO assignments.
- EasyLink and SmartRF PHY settings.

The framework does not currently provide runtime persistence, over-the-air configuration, or a command interface.

## Current Status

Completed and verified:

- Custom CC1310 PCB assembly and programming.
- UART, I2C, and EasyLink RF transmit operation.
- AHT20 detection at `0x38`.
- LIS2DW12 detection at `0x19`.
- LIS2DW12 `WHO_AM_I = 0x44` verification.
- XYZ polling.
- Integer mg conversion and magnitude calculation.
- Rotation validation.
- Motion engine implementation.
- Compile-time configuration framework.

Not yet implemented:

- LIS2DW12 interrupt handling on INT1/INT2.
- Event-driven wake-up.
- RF sensor packet format and transmission.
- Deep sleep.
- CR2450 power optimization.
