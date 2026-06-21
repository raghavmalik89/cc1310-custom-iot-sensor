# Task: UART-Only Bring-up for CC1310 Custom PCB

## Purpose

Bring up UART debug output on the custom CC1310 IoT sensor PCB using the existing TI SimpleLink / CCS `rfEasyLinkEchoTx` project.

This is Stage 1 only.

The goal is to confirm:

1. The custom PCB boots.
2. UART works on the correct custom PCB pins.
3. LaunchPad LED dependency is removed, disabled, or guarded.
4. EasyLink RF structure remains intact.

## Repository / Workspace Context

Base workspace:

```text
C:\Users\ragha\ti\Workspaces\CC1310\Easylink
```

Primary CCS project:

```text
rfEasyLinkEchoTx
```

Use `rfEasyLinkEchoTx` as the firmware bring-up base.

## Platform Rules

This is a TI CC1310 SimpleLink / CCS / TI-RTOS firmware project.

This is not:

* Arduino
* ESP32
* ESP-IDF
* PlatformIO

Do not introduce Arduino-style code, ESP32 assumptions, or non-CCS restructuring.

## Authoritative Custom PCB Pinout

Use only this pinout.

| Function      | Custom PCB DIO |
| ------------- | -------------: |
| UART_RX       |           DIO2 |
| UART_TX       |           DIO3 |
| I2C_SCL       |           DIO4 |
| I2C_SDA       |           DIO5 |
| LIS2DW12_INT1 |          DIO15 |
| LIS2DW12_INT2 |          DIO14 |

## Critical Pinout Rules

Do not use old LaunchPad assumptions.

Incorrect mappings:

| Function | Do Not Use |
| -------- | ---------: |
| UART_RX  |       DIO1 |
| UART_TX  |       DIO2 |
| I2C_SCL  |       DIO3 |
| I2C_SDA  |       DIO4 |

Correct UART mapping:

```text
UART RX = DIO2
UART TX = DIO3
```

Correct future I2C mapping:

```text
I2C SCL = DIO4
I2C SDA = DIO5
```

Accelerometer interrupt mapping:

```text
LIS2DW12 INT1 = DIO15
LIS2DW12 INT2 = DIO14
```

Do not confuse DIO15 and DIO14.

## Important Board Constraint

The custom PCB has no fitted LEDs.

Therefore:

* Do not use LED blink as debug.
* Do not rely on LaunchPad LED symbols for runtime status.
* Remove, disable, or guard LaunchPad LED code.
* Use UART debug output instead.

## Required Workflow

### Step 1: Inspect Only

Before editing, inspect the project and report:

1. Current `rfEasyLinkEchoTx` structure.
2. Board files used by `rfEasyLinkEchoTx`.
3. Where UART pins are defined.
4. Where I2C pins are defined, if present.
5. Where LED definitions are declared.
6. Where LED definitions are used.
7. Whether LED or button mappings conflict with the custom PCB pinout.
8. Exact files proposed for editing.

Do not edit until the inspection is complete and exact edit files are listed.

### Step 2: Make Minimal Edits Only

After inspection, make the smallest CCS-friendly changes required for UART-only bring-up.

Allowed edits:

* Board pin mapping files.
* UART configuration files.
* `rfEasyLinkEchoTx` application source file, only if needed for UART prints.
* LED code guards/removal if required for build or runtime.

Do not make broad refactors.

## Stage 1 Implementation Requirements

Configure UART as:

| Parameter    | Value  |
| ------------ | ------ |
| RX           | DIO2   |
| TX           | DIO3   |
| Baud         | 115200 |
| Data bits    | 8      |
| Parity       | None   |
| Stop bits    | 1      |
| Flow control | None   |

Add UART boot output:

```text
CC1310 custom PCB boot
UART OK
```

Add periodic UART alive output:

```text
Alive
```

The alive message should print periodically and should not disturb the existing EasyLink callback architecture.

## Do Not Add in This Pass

Do not add:

* I2C initialization
* I2C scanning
* AHT20 driver
* LIS2DW12 driver
* LIS2DW12 interrupt handling
* Sensor reading
* Telemetry protocol changes
* RF PHY changes
* RF packet format changes
* Power optimization
* Sleep mode changes
* New project structure
* New RTOS architecture

## Preserve

Preserve:

* Existing `rfEasyLinkEchoTx` base project.
* Existing EasyLink RF logic unless explicitly required for LED removal.
* Existing EasyLink callback style.
* CCS project compatibility.

## Projects Not to Touch

Do not modify:

* `rfEasyLinkEchoRx`
* `rfPacketTx`
* `ATH2x_ENS160_I2C`
* `tirtos_builds_CC1310_LAUNCHXL_release_ccs`, unless inspection shows it is directly required by the active build and you clearly state why before editing.

## Expected UART Output

Serial terminal should show:

```text
CC1310 custom PCB boot
UART OK
Alive
Alive
Alive
```

## Hardware Verification

Connect USB-UART as follows:

| USB-UART Adapter | Custom PCB     |
| ---------------- | -------------- |
| TX               | UART_RX / DIO2 |
| RX               | UART_TX / DIO3 |
| GND              | GND            |

Serial terminal settings:

```text
Baud: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

## After Editing, Report

After implementation, report clearly:

1. Files inspected.
2. Files changed.
3. Exact UART pin mapping confirmed.
4. LED code removed, disabled, or guarded.
5. Confirmation that I2C was not added.
6. Confirmation that sensor drivers were not added.
7. Confirmation that RF PHY/settings were not changed.
8. Expected UART output.
9. Hardware verification steps.
10. Any unresolved assumptions or risks.

## Completion Criteria

This task is complete only when:

1. Firmware builds in CCS.
2. UART is configured for RX = DIO2 and TX = DIO3.
3. Boot messages are added.
4. Periodic `Alive` message is added.
5. LED dependency is removed, disabled, or guarded.
6. No I2C or sensor code is added.
7. EasyLink RF structure remains intact.

## Final Instruction

Keep the change small, reviewable, and suitable for custom PCB bring-up.

Do not combine this with the next I2C stage.
