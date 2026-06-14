#include "sensor_task.h"

#include <stdio.h>
#include <string.h>

#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include "Board.h"
#include "app_config.h"
#include "i2c_bus.h"
#include "lis2dw12_driver.h"
#include "third_party/st/lis2dw12_reg.h"

#define SENSOR_TASK_STACK_SIZE    1024
#define SENSOR_TASK_PRIORITY      1

static Task_Struct sensorTask;
static Task_Params sensorTaskParams;
static uint8_t sensorTaskStack[SENSOR_TASK_STACK_SIZE];

static UART_Handle sensorUart;

static void sensor_uart_open(void)
{
    UART_Params uartParams;

    UART_init();
    UART_Params_init(&uartParams);
    uartParams.baudRate = 115200;
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;

    sensorUart = UART_open(Board_UART0, &uartParams);
}

static void sensor_print(const char *text)
{
#if APP_UART_DEBUG_ENABLE
    if ((sensorUart != NULL) && (text != NULL))
    {
        UART_write(sensorUart, text, strlen(text));
    }
#else
    (void)text;
#endif
}

static void sensor_printf(const char *fmt, int32_t a, int32_t b, int32_t c, int32_t d)
{
    char line[96];
    int len;

    len = snprintf(line, sizeof(line), fmt, a, b, c, d);
    if (len > 0)
    {
        sensor_print(line);
    }
}

static void sensorTaskFnx(UArg arg0, UArg arg1)
{
    lis2dw12_driver_config_t accelCfg;
    lis2dw12_raw_t raw;
    lis2dw12_accel_mg_t accel;
    uint8_t whoami;
    uint8_t status;

    (void)arg0;
    (void)arg1;

    sensor_uart_open();
    sensor_print("CC1310 LIS2DW12 test boot\r\n");

    if (i2c_bus_init() != I2C_BUS_OK)
    {
        sensor_print("ERR: I2C init failed\r\n");
        return;
    }
    sensor_print("I2C init OK\r\n");

    accelCfg.i2c_addr = APP_ACCEL_I2C_ADDR;
    accelCfg.full_scale_g = APP_ACCEL_FULL_SCALE_G;
    accelCfg.odr_hz = APP_ACCEL_SAMPLE_HZ;

    if (lis2dw12_driver_init(&accelCfg) != LIS2DW12_DRIVER_OK)
    {
        sensor_print("ERR: LIS2DW12 driver init failed\r\n");
        return;
    }

    sensor_printf("LIS2DW12 detected at 0x%02x\r\n", APP_ACCEL_I2C_ADDR, 0, 0, 0);

    if (lis2dw12_driver_read_id(&whoami) != LIS2DW12_DRIVER_OK)
    {
        sensor_print("ERR: LIS2DW12 WHO_AM_I read failed\r\n");
        return;
    }

    if (whoami != LIS2DW12_ID)
    {
        sensor_printf("ERR: LIS2DW12 unexpected WHO_AM_I = 0x%02x\r\n", whoami, 0, 0, 0);
        return;
    }
    sensor_printf("LIS2DW12 WHO_AM_I = 0x%02x OK\r\n", whoami, 0, 0, 0);

    if (lis2dw12_driver_configure_basic() != LIS2DW12_DRIVER_OK)
    {
        sensor_print("ERR: LIS2DW12 config failed\r\n");
        return;
    }
    sensor_print("LIS2DW12 config OK\r\n");

    while (1)
    {
        if (lis2dw12_driver_get_status(&status) != LIS2DW12_DRIVER_OK)
        {
            sensor_print("ERR: LIS2DW12 status read failed\r\n");
        }
        else if ((status & LIS2DW12_STATUS_DRDY) == 0U)
        {
            sensor_print("LIS2DW12 status: data not ready\r\n");
        }

        if (lis2dw12_driver_read_raw(&raw) != LIS2DW12_DRIVER_OK)
        {
            sensor_print("ERR: LIS2DW12 XYZ read failed\r\n");
        }
        else if (lis2dw12_driver_read_mg(&accel) != LIS2DW12_DRIVER_OK)
        {
            sensor_print("ERR: LIS2DW12 XYZ read failed\r\n");
        }
        else
        {
            sensor_printf("ACC raw: x=%d y=%d z=%d\r\n", raw.x, raw.y, raw.z, 0);
            sensor_printf("ACC mg:  x=%d y=%d z=%d mag=%u\r\n",
                          accel.x_mg,
                          accel.y_mg,
                          accel.z_mg,
                          (int32_t)accel.magnitude_mg);
        }

        Task_sleep(1000000 / Clock_tickPeriod);
    }
}

void sensorTask_init(void)
{
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = SENSOR_TASK_STACK_SIZE;
    sensorTaskParams.priority = SENSOR_TASK_PRIORITY;
    sensorTaskParams.stack = sensorTaskStack;

    Task_construct(&sensorTask, sensorTaskFnx, &sensorTaskParams, NULL);
}
