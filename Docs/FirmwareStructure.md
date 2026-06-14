# Firmware Structure

## Project

The active firmware project is:

```text
Firmware/rfEasyLinkEchoTx/
```

It is a TI Code Composer Studio project derived from the CC1310 LaunchPad TI EasyLink echo transmitter example.

## Important Source Areas

- `rfEasyLinkEchoTx.c`: application entry point, board initialization, TI-RTOS startup, EasyLink echo task, and sensor task startup.
- `sensor_task.c` / `sensor_task.h`: UART debug setup, I2C initialization, LIS2DW12 detection at `0x19`, `WHO_AM_I = 0x44` verification, and ongoing accelerometer polling development.
- `i2c_bus.c` / `i2c_bus.h`: small wrapper around TI I2C driver operations.
- `lis2dw12_driver.c` / `lis2dw12_driver.h`: project-level LIS2DW12 driver work in progress.
- `lis2dw12_port_ti.c` / `lis2dw12_port_ti.h`: TI I2C adapter layer for the LIS2DW12 register interface.
- `third_party/st/lis2dw12_reg.c` / `third_party/st/lis2dw12_reg.h`: LIS2DW12 register-level access layer.
- `smartrf_settings/`: SmartRF-generated radio configuration used by EasyLink.
- `easylink/`: TI EasyLink abstraction and configuration files used by the RF example baseline.
- `targetConfigs/`: CCS target configuration for the CC1310.

## CCS Project Files

These files are part of the importable CCS project and should be committed:

- `.project`
- `.cproject`
- `.ccsproject`
- `targetConfigs/CC1310F128.ccxml`
- `targetConfigs/readme.txt`

## Files To Commit

Commit these categories:

- Root documentation: `README.md`, `Docs/Architecture.md`, and `Docs/FirmwareStructure.md`.
- Firmware source and headers: `*.c`, `*.h`, and linker command files such as `*.cmd`.
- CCS project import metadata: `.project`, `.cproject`, `.ccsproject`, and `targetConfigs/`.
- RF configuration source under `smartrf_settings/`.
- EasyLink source and headers included with the project.
- Third-party source files that are intentionally vendored in the firmware tree.
- Hardware design files under `Hardware/` when they are added and are intended for publication.

## Files Not To Commit

Do not commit these categories:

- `Debug/` and `Release/` build output directories.
- Object files and dependency files such as `*.obj`, `*.o`, `*.d`, and `*.d_raw`.
- Linker and image outputs such as `*.map`, `*.out`, `*.hex`, `*.bin`, and `*_linkInfo.xml`.
- Generated CCS make fragments such as `makefile`, `objects.mk`, `sources.mk`, `subdir_rules.mk`, and `subdir_vars.mk`.
- Local CCS/Eclipse workspace state such as `.settings/` and `.metadata/`.
- Local clangd indexes and generated compile databases such as `.clangd/` and `compile_commands.json`.
- Temporary editor or operating-system files such as `*.tmp`, `*.bak`, `*.swp`, `.DS_Store`, and `Thumbs.db`.
- Local automation notes under `.codex/`.

## Current Firmware Status

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
