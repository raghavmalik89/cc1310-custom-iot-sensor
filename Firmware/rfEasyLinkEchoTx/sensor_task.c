#include "sensor_task.h"

#include <stdio.h>
#include <string.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/PIN.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include "Board.h"
#include "aht2x.h"
#include "app_config.h"
#include "board_pins.h"
#include "i2c_bus.h"
#include "lis2dw12_driver.h"
#include "motion_engine.h"
#include "third_party/st/lis2dw12_reg.h"

#define SENSOR_TASK_STACK_SIZE    1280
#define SENSOR_TASK_PRIORITY      1
#define SENSOR_TASK_PERIOD_MS     100U

static Task_Struct sensorTask;
static Task_Params sensorTaskParams;
static uint8_t sensorTaskStack[SENSOR_TASK_STACK_SIZE];

static UART_Handle sensorUart;

typedef struct
{
    int32_t temperatureTenths;
    int32_t humidityTenths;
    uint32_t temperatureTick;
    uint32_t humidityTick;
    bool temperatureValid;
    bool humidityValid;
} environmental_cache_t;

typedef enum
{
    OBS_IDLE = 0,
    OBS_ACTIVE
} observation_state_t;

typedef struct
{
    observation_state_t state;
    uint32_t startTick;
    uint32_t endTick;
    uint32_t lastSampleTick;
    uint32_t interruptCount;
    uint32_t sampleCount;
    uint32_t maxG;
    uint8_t flags;
    motion_engine_state_t latestMotionState;
#if APP_ACCEL_DEBUG_PRINT_ENABLE
    uint32_t lastDebugPrintTick;
#endif
} motion_observation_t;

#if APP_ACCEL_ACTIVITY_INT_ENABLE
static PIN_Handle accelInt1PinHandle;
static PIN_State accelInt1PinState;
static volatile uint32_t accelInt1Count;
static volatile bool accelInt1Pending;

static PIN_Config accelInt1PinTable[] = {
    BOARD_LIS2DW12_INT1_DIO | PIN_INPUT_EN | PIN_NOPULL | PIN_HYSTERESIS,
    PIN_TERMINATE
};

static void accel_int1_callback(PIN_Handle handle, PIN_Id pinId)
{
    (void)handle;
    (void)pinId;

    accelInt1Count++;
    accelInt1Pending = true;
}

static bool accel_int1_gpio_init(void)
{
    PIN_Status pinStatus;

    accelInt1Count = 0U;
    accelInt1Pending = false;

    accelInt1PinHandle = PIN_open(&accelInt1PinState, accelInt1PinTable);
    if (accelInt1PinHandle == NULL)
    {
        return false;
    }

    pinStatus = PIN_registerIntCb(accelInt1PinHandle, accel_int1_callback);
    if (pinStatus != PIN_SUCCESS)
    {
        return false;
    }

#if APP_ACCEL_INT1_ACTIVE_HIGH
    pinStatus = PIN_setInterrupt(accelInt1PinHandle,
                                 BOARD_LIS2DW12_INT1_DIO | PIN_IRQ_POSEDGE);
#else
    pinStatus = PIN_setInterrupt(accelInt1PinHandle,
                                 BOARD_LIS2DW12_INT1_DIO | PIN_IRQ_NEGEDGE);
#endif

    return pinStatus == PIN_SUCCESS;
}
#endif

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

static uint8_t motion_status_flags(const motion_engine_status_t *motion)
{
    uint8_t flags = 0U;

    if (motion == NULL)
    {
        return 0U;
    }

    if (motion->motion_event)
    {
        flags |= MOTION_ENGINE_FLAG_MOTION;
    }
    if (motion->shock_event)
    {
        flags |= MOTION_ENGINE_FLAG_SHOCK;
    }
    if (motion->severe_shock_event)
    {
        flags |= MOTION_ENGINE_FLAG_SEVERE_SHOCK;
    }
    if (motion->drop_event)
    {
        flags |= MOTION_ENGINE_FLAG_DROP;
    }
    if (motion->sustained_high_g_event)
    {
        flags |= MOTION_ENGINE_FLAG_SUSTAINED_HIGHG;
    }

    return flags;
}

static void sensor_print_motion(const motion_engine_status_t *motion)
{
    char line[144];
    const char *stateText;
    uint8_t flags;
    int len;

    if (motion == NULL)
    {
        return;
    }

    stateText = (motion->state == MOTION_ENGINE_STATE_MOVING) ?
                "MOVING" : "STILL";
    flags = motion_status_flags(motion);

    len = snprintf(line,
                   sizeof(line),
                   "MOTION: state=%s motion=%lu shock=%lu severe=%lu "
                   "drop=%lu highG=%lu dur=%lu maxG=%lu flags=0x%02x\r\n",
                   stateText,
                   (unsigned long)motion->motion_count,
                   (unsigned long)motion->shock_count,
                   (unsigned long)motion->severe_shock_count,
                   (unsigned long)motion->drop_count,
                   (unsigned long)motion->sustained_high_g_count,
                   (unsigned long)motion->movement_duration_s,
                   (unsigned long)motion->max_g_mg,
                   flags);
    if (len > 0)
    {
        sensor_print(line);
    }
}

static void sensor_print_accel(const lis2dw12_accel_mg_t *accel,
                               uint32_t dynamicMg)
{
    char line[96];
    int len;

    if (accel == NULL)
    {
        return;
    }

    len = snprintf(line,
                   sizeof(line),
                   "ACC mg:  x=%ld y=%ld z=%ld mag=%lu dyn=%lu\r\n",
                   (long)accel->x_mg,
                   (long)accel->y_mg,
                   (long)accel->z_mg,
                   (unsigned long)accel->magnitude_mg,
                   (unsigned long)dynamicMg);
    if (len > 0)
    {
        sensor_print(line);
    }
}

static int32_t float_to_tenths(float value)
{
    float scaled = value * 10.0f;

    return (int32_t)(scaled + ((scaled >= 0.0f) ? 0.5f : -0.5f));
}

static void sensor_format_fixed_tenths(int32_t valueTenths,
                                       const char **sign,
                                       uint32_t *whole,
                                       uint32_t *fraction)
{
    uint32_t magnitude;

    *sign = (valueTenths < 0) ? "-" : "";
    magnitude = (uint32_t)((valueTenths < 0) ?
                          -valueTenths : valueTenths);
    *whole = magnitude / 10U;
    *fraction = magnitude % 10U;
}

static void sensor_print_environment(const environmental_cache_t *cache)
{
    char line[64];
    const char *tempSign;
    const char *humiditySign;
    uint32_t tempWhole;
    uint32_t tempFraction;
    uint32_t humidityWhole;
    uint32_t humidityFraction;
    int len;

    if (cache == NULL)
    {
        return;
    }

    sensor_format_fixed_tenths(cache->temperatureTenths,
                               &tempSign,
                               &tempWhole,
                               &tempFraction);
    sensor_format_fixed_tenths(cache->humidityTenths,
                               &humiditySign,
                               &humidityWhole,
                               &humidityFraction);

    len = snprintf(line,
                   sizeof(line),
                   "AHT20: temp=%s%lu.%luC rh=%s%lu.%lu%%\r\n",
                   tempSign,
                   (unsigned long)tempWhole,
                   (unsigned long)tempFraction,
                   humiditySign,
                   (unsigned long)humidityWhole,
                   (unsigned long)humidityFraction);
    if (len > 0)
    {
        sensor_print(line);
    }
}

static uint32_t sensor_ticks_to_ms(uint32_t ticks)
{
    return (uint32_t)(((uint64_t)ticks * Clock_tickPeriod) / 1000U);
}

static uint32_t sensor_ms_to_ticks(uint32_t milliseconds)
{
    return (uint32_t)((((uint64_t)milliseconds * 1000U) +
                       Clock_tickPeriod - 1U) / Clock_tickPeriod);
}

static bool sensor_tick_reached(uint32_t nowTick, uint32_t targetTick)
{
    return (int32_t)(nowTick - targetTick) >= 0;
}

static void sensor_print_observation_summary(const motion_observation_t *observation,
                                             const environmental_cache_t *cache)
{
    char line[160];
    const char *stateText;
    uint32_t nowTick;
    uint32_t durationMs;
    uint32_t tempAgeMs;
    uint32_t humidityAgeMs;
    uint32_t envAgeMs;
    bool environmentValid;
    int len;

    stateText = (observation->latestMotionState == MOTION_ENGINE_STATE_MOVING) ?
                "MOVING" : "STILL";
    nowTick = Clock_getTicks();
    durationMs = sensor_ticks_to_ms(nowTick - observation->startTick);
    tempAgeMs = sensor_ticks_to_ms(nowTick - cache->temperatureTick);
    humidityAgeMs = sensor_ticks_to_ms(nowTick - cache->humidityTick);
    envAgeMs = (tempAgeMs > humidityAgeMs) ? tempAgeMs : humidityAgeMs;
    environmentValid = cache->temperatureValid &&
                       cache->humidityValid &&
                       (envAgeMs <= APP_ENV_CACHE_INVALID_AGE_MS);

    if (environmentValid)
    {
        const char *tempSign;
        const char *humiditySign;
        uint32_t tempWhole;
        uint32_t tempFraction;
        uint32_t humidityWhole;
        uint32_t humidityFraction;

        sensor_format_fixed_tenths(cache->temperatureTenths,
                                   &tempSign,
                                   &tempWhole,
                                   &tempFraction);
        sensor_format_fixed_tenths(cache->humidityTenths,
                                   &humiditySign,
                                   &humidityWhole,
                                   &humidityFraction);

        len = snprintf(line,
                       sizeof(line),
                       "OBS: done duration=%lus samples=%lu state=%s flags=0x%02x "
                       "maxG=%lu temp=%s%lu.%luC rh=%s%lu.%lu%% env_age=%lus\r\n",
                       (unsigned long)(durationMs / 1000U),
                       (unsigned long)observation->sampleCount,
                       stateText,
                       observation->flags,
                       (unsigned long)observation->maxG,
                       tempSign,
                       (unsigned long)tempWhole,
                       (unsigned long)tempFraction,
                       humiditySign,
                       (unsigned long)humidityWhole,
                       (unsigned long)humidityFraction,
                       (unsigned long)(envAgeMs / 1000U));
    }
    else
    {
        len = snprintf(line,
                       sizeof(line),
                       "OBS: done duration=%lus samples=%lu state=%s flags=0x%02x "
                       "maxG=%lu temp=NA rh=NA env_age=NA\r\n",
                       (unsigned long)(durationMs / 1000U),
                       (unsigned long)observation->sampleCount,
                       stateText,
                       observation->flags,
                       (unsigned long)observation->maxG);
    }

    if (len > 0)
    {
        sensor_print(line);
    }
}

static void sensor_observation_start_or_extend(motion_observation_t *observation,
                                               uint32_t interruptCount,
                                               uint32_t nowTick)
{
    motion_engine_status_t motion;
    uint32_t observationTicks =
        sensor_ms_to_ticks(APP_MOTION_OBSERVATION_WINDOW_MS);
    uint32_t sampleTicks =
        sensor_ms_to_ticks(APP_MOTION_OBSERVATION_SAMPLE_PERIOD_MS);

    if (observation->state == OBS_IDLE)
    {
        motion_engine_init();
        motion_engine_get_status(&motion);
        observation->state = OBS_ACTIVE;
        observation->startTick = nowTick;
        observation->lastSampleTick = nowTick - sampleTicks;
        observation->sampleCount = 0U;
        observation->maxG = 0U;
        observation->flags = 0U;
        observation->latestMotionState = motion.state;
#if APP_ACCEL_DEBUG_PRINT_ENABLE
        observation->lastDebugPrintTick = nowTick;
#endif
        sensor_printf("OBS: start int_count=%u\r\n",
                      (int32_t)interruptCount,
                      0,
                      0,
                      0);
    }
    else
    {
        sensor_printf("OBS: extend int_count=%u\r\n",
                      (int32_t)interruptCount,
                      0,
                      0,
                      0);
    }

    observation->interruptCount = interruptCount;
    observation->endTick = nowTick + observationTicks;

    if (lis2dw12_clear_activity_interrupt() != LIS2DW12_DRIVER_OK)
    {
        sensor_print("ERR: LIS2DW12 activity interrupt clear failed\r\n");
    }
}

static void sensor_observation_process(motion_observation_t *observation,
                                       const environmental_cache_t *cache,
                                       uint32_t nowTick)
{
    lis2dw12_accel_mg_t accel;
    motion_engine_status_t motion;
    uint32_t timestampS;
    uint32_t sampleTicks =
        sensor_ms_to_ticks(APP_MOTION_OBSERVATION_SAMPLE_PERIOD_MS);
#if APP_ACCEL_DEBUG_PRINT_ENABLE
    uint32_t debugPrintTicks =
        sensor_ms_to_ticks(APP_ACCEL_DEBUG_PRINT_PERIOD_MS);
#endif

    if (observation->state != OBS_ACTIVE)
    {
        return;
    }

    timestampS = sensor_ticks_to_ms(nowTick) / 1000U;

    if (sensor_tick_reached(nowTick,
                            observation->lastSampleTick + sampleTicks))
    {
        observation->lastSampleTick = nowTick;

        if (lis2dw12_driver_read_mg(&accel) == LIS2DW12_DRIVER_OK)
        {
            motion_engine_update(accel.x_mg,
                                 accel.y_mg,
                                 accel.z_mg,
                                 accel.magnitude_mg,
                                 timestampS);
            motion_engine_get_status(&motion);

            observation->sampleCount++;
            observation->flags |= motion_status_flags(&motion);
            observation->latestMotionState = motion.state;
            if (accel.magnitude_mg > observation->maxG)
            {
                observation->maxG = accel.magnitude_mg;
            }

#if APP_ACCEL_DEBUG_PRINT_ENABLE
            if (sensor_tick_reached(nowTick,
                                    observation->lastDebugPrintTick +
                                    debugPrintTicks))
            {
                observation->lastDebugPrintTick = nowTick;
                sensor_print_accel(&accel, motion.dynamic_mg);
                sensor_print_motion(&motion);
            }
#endif
        }
    }

    if (sensor_tick_reached(nowTick, observation->endTick))
    {
        /*
         * Clear any latched activity raised during observation so a later
         * movement can produce a fresh rising edge on DIO15.
         */
        if (lis2dw12_clear_activity_interrupt() != LIS2DW12_DRIVER_OK)
        {
            sensor_print("ERR: LIS2DW12 activity interrupt clear failed\r\n");
        }

        sensor_print_observation_summary(observation, cache);
        observation->state = OBS_IDLE;
    }
}

static void sensorTaskFnx(UArg arg0, UArg arg1)
{
    AHT2x aht2x;
    AHT2x_Status ahtStatus;
    lis2dw12_driver_config_t accelCfg;
    environmental_cache_t environmentCache = {0};
    motion_observation_t observation;
    uint32_t tempElapsedMs = 0U;
    uint32_t humidityElapsedMs = 0U;
    bool ahtReady = false;
    uint8_t whoami;

    (void)arg0;
    (void)arg1;

    memset(&observation, 0, sizeof(observation));
    observation.state = OBS_IDLE;

    sensor_uart_open();
    sensor_print("CC1310 LIS2DW12 test boot\r\n");

    if (i2c_bus_init() != I2C_BUS_OK)
    {
        sensor_print("ERR: I2C init failed\r\n");
        return;
    }
    sensor_print("I2C init OK\r\n");

    /*
     * AHT20 uses scheduled polling only. LIS2DW12 INT1 is the independent
     * event-wake path; environmental sensing must not use that interrupt.
     */
    AHT2x_init(&aht2x, i2c_bus_get_handle(), AHT2X_I2C_ADDRESS);
    ahtStatus = AHT2x_ensureCalibrated(&aht2x);
    if (ahtStatus == AHT2X_STATUS_OK)
    {
        ahtReady = true;
        sensor_print("AHT20 scheduled polling OK\r\n");
    }
    else
    {
        sensor_print("ERR: AHT20 init/calibration failed\r\n");
    }

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

#if APP_ACCEL_ACTIVITY_INT_ENABLE
    if (lis2dw12_enable_activity_interrupt() != LIS2DW12_DRIVER_OK)
    {
        sensor_print("ERR: LIS2DW12 activity interrupt config failed\r\n");
        return;
    }
    sensor_print("LIS2DW12 activity interrupt OK\r\n");

    if (!accel_int1_gpio_init())
    {
        sensor_print("ERR: INT1 GPIO DIO15 interrupt init failed\r\n");
        return;
    }
    sensor_print("INT1 GPIO DIO15 interrupt OK\r\n");
#endif

    motion_engine_init();

    /*
     * Regular accelerometer polling was useful for bring-up only. For battery
     * operation, XYZ sampling is restricted to the observation window started
     * by LIS2DW12 INT1. The LIS2DW12 remains configured internally to detect
     * activity while the MCU is not sampling acceleration.
     */
    while (1)
    {
        bool tempDue;
        bool humidityDue;
        uint32_t nowTick = Clock_getTicks();

#if APP_ACCEL_ACTIVITY_INT_ENABLE
        if (accelInt1Pending)
        {
            uint32_t eventCount;
            UInt hwiKey;

            hwiKey = Hwi_disable();
            accelInt1Pending = false;
            eventCount = accelInt1Count;
            Hwi_restore(hwiKey);

            sensor_observation_start_or_extend(&observation,
                                               eventCount,
                                               nowTick);
        }
#endif

        sensor_observation_process(&observation,
                                   &environmentCache,
                                   nowTick);

        tempElapsedMs += SENSOR_TASK_PERIOD_MS;
        humidityElapsedMs += SENSOR_TASK_PERIOD_MS;
        tempDue = tempElapsedMs >= APP_TEMP_SAMPLE_PERIOD_MS;
        humidityDue = humidityElapsedMs >= APP_HUMIDITY_SAMPLE_PERIOD_MS;

        if (ahtReady && (tempDue || humidityDue))
        {
            float sampledTemperatureC;
            float sampledHumidityPercent;

            if (tempDue)
            {
                tempElapsedMs = 0U;
            }
            if (humidityDue)
            {
                humidityElapsedMs = 0U;
            }

            ahtStatus = AHT2x_readTemperatureHumidity(&aht2x,
                                                      &sampledTemperatureC,
                                                      &sampledHumidityPercent);
            if (ahtStatus == AHT2X_STATUS_OK)
            {
                uint32_t sampleTick = Clock_getTicks();

                if (tempDue)
                {
                    environmentCache.temperatureTenths =
                        float_to_tenths(sampledTemperatureC);
                    environmentCache.temperatureTick = sampleTick;
                    environmentCache.temperatureValid = true;
                }

                /*
                 * The AHT20 conversion returns both channels. The application
                 * retains RH only on its slower schedule, except for the first
                 * successful sample used to seed readable diagnostics.
                 */
                if (humidityDue || !environmentCache.humidityValid)
                {
                    environmentCache.humidityTenths =
                        float_to_tenths(sampledHumidityPercent);
                    environmentCache.humidityTick = sampleTick;
                    environmentCache.humidityValid = true;
                }

                if (tempDue &&
                    environmentCache.temperatureValid &&
                    environmentCache.humidityValid)
                {
                    sensor_print_environment(&environmentCache);
                }
            }
            else
            {
                sensor_print("ERR: AHT20 measurement failed\r\n");
            }
        }

        Task_sleep((SENSOR_TASK_PERIOD_MS * 1000U) / Clock_tickPeriod);
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
