We are developing firmware for a custom TI CC1310 IoT sensor board using SimpleLink SDK, TI-RTOS, and CCS.

This is not Arduino and not ESP32.

Base project:
Use `rfEasyLinkEchoTx` as the firmware base.

Confirmed custom PCB pinout:

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
AHT20 is at 0x38.
LIS2DW12 is at 0x19.
LIS2DW12 WHO_AM_I register 0x0F returns 0x44.
```

Known hardware probe:

```text
Probe 0: AHT20 0x38 ACK status=0x18
Probe 0: LIS2DW12 0x18 NACK WHO=0x00, 0x19 ACK WHO=0x44
Probe 1: AHT20 0x38 ACK status=0x18
Probe 1: LIS2DW12 0x18 NACK WHO=0x00, 0x19 ACK WHO=0x44
Probe 2: AHT20 0x38 ACK status=0x18
Probe 2: LIS2DW12 0x18 NACK WHO=0x00, 0x19 ACK WHO=0x44
```

Task:
Implement only the LIS2DW12 accelerometer firmware path.

Do not implement AHT20 yet.
Do not implement RF packet changes yet.
Do not implement motion classification yet.
Do not implement interrupts yet.
Do not write README yet.

Architecture required:

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

Use STMicroelectronics’ official LIS2DW12 platform-independent driver if available in the repository or add it under:

```text
third_party/st/
```

Keep the ST driver unchanged where possible.

Use the Zephyr-style architecture:

```text
ST lis2dw12_reg.c/.h
        ↑
stmdev_ctx_t
        ↑
lis2dw12_port_ti.c
        ↑
i2c_bus.c
        ↑
TI I2C_transfer()
```

Implementation requirements:

1. Create `board_pins.h` for custom PCB pins.
2. Create `app_config.h` for app-level accelerometer settings.
3. Create or clean up `i2c_bus.h/.c` as the TI-RTOS I2C wrapper.
4. Create `lis2dw12_port_ti.h/.c` to connect ST `stmdev_ctx_t` read/write callbacks to `i2c_bus`.
5. Create `lis2dw12_driver.h/.c` as the clean app-facing LIS2DW12 API.
6. Update the main task or `sensor_task.c` to:

   * initialize I2C
   * initialize LIS2DW12 driver
   * read WHO_AM_I
   * configure basic polling mode
   * read raw XYZ
   * read converted mg values
   * print clear UART diagnostics

Required UART output style:

```text
CC1310 LIS2DW12 test boot
I2C init OK
LIS2DW12 detected at 0x19
LIS2DW12 WHO_AM_I = 0x44 OK
LIS2DW12 config OK
ACC raw: x=... y=... z=...
ACC mg:  x=... y=... z=... mag=...
```

Error output style:

```text
ERR: I2C init failed
ERR: LIS2DW12 WHO_AM_I read failed
ERR: LIS2DW12 unexpected WHO_AM_I = 0x00
ERR: LIS2DW12 config failed
ERR: LIS2DW12 XYZ read failed
```

Driver quality requirements:

* Plain C.
* CCS-compatible.
* TI-RTOS-compatible.
* Explicit return codes.
* Defensive argument checking.
* No silent failures.
* No UART prints inside the ST low-level driver.
* No board pins inside the generic LIS2DW12 driver.
* No motion thresholds inside the LIS2DW12 driver.
* No RF packet logic inside the LIS2DW12 driver.
* Avoid floating point unless necessary.
* Use clear comments for register configuration.
* Keep code professional and maintainable.

Use these known constants:

```c
#define LIS2DW12_I2C_ADDR        0x19
#define LIS2DW12_WHO_AM_I_REG    0x0F
#define LIS2DW12_WHO_AM_I_VALUE  0x44
```

Initial accelerometer settings:

```text
Full scale: ±2 g
ODR: 25 Hz if convenient, otherwise nearest simple supported value
Mode: simple polling mode
FIFO: disabled
Interrupts: disabled
```

Expected validation:
When the board is stationary, the acceleration magnitude should be approximately 1000 mg.

Implement this in small, reviewable changes.

After implementation, summarize:

* files added
* files modified
* how data flows from `sensor_task.c` to TI I2C
* expected UART output
* any assumptions made
