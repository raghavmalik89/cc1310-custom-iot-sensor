#include "lis2dw12_driver.h"

#include "app_config.h"
#include "lis2dw12_port_ti.h"
#include "third_party/st/lis2dw12_reg.h"

#define LIS2DW12_CTRL1_ODR_25HZ_LP_MODE1      0x30U
#define LIS2DW12_CTRL2_IF_ADD_INC             0x04U
#define LIS2DW12_CTRL2_BDU                    0x08U
#define LIS2DW12_CTRL6_FS_2G                  0x00U

#define LIS2DW12_RAW_16BIT_2G_DENOM           32768L
#define LIS2DW12_RAW_16BIT_2G_NUM             2000L

#define LIS2DW12_REG_CTRL3                     0x22U
#define LIS2DW12_REG_CTRL4_INT1_PAD_CTRL       0x23U
#define LIS2DW12_REG_WAKE_UP_THS               0x34U
#define LIS2DW12_REG_WAKE_UP_DUR               0x35U
#define LIS2DW12_REG_WAKE_UP_SRC               0x38U
#define LIS2DW12_REG_CTRL7                     0x3FU

#define LIS2DW12_CTRL3_PP_OD                   0x20U
#define LIS2DW12_CTRL3_LIR                     0x10U
#define LIS2DW12_CTRL3_H_LACTIVE               0x08U
#define LIS2DW12_CTRL4_INT1_WU                 0x20U
#define LIS2DW12_CTRL7_INTERRUPTS_ENABLE       0x20U

#define LIS2DW12_WAKE_UP_THS_MASK              0x3FU
#define LIS2DW12_WAKE_UP_DUR_MASK              0x60U
#define LIS2DW12_WAKE_UP_DUR_SHIFT             5U
#define LIS2DW12_WAKE_UP_DUR_MAX               3U

static stmdev_ctx_t lis2dw12Ctx;
static lis2dw12_driver_config_t lis2dw12Cfg;
static bool lis2dw12Initialized;
static bool lis2dw12Configured;

static uint32_t isqrt_u64(uint64_t value)
{
    uint64_t bit = (uint64_t)1 << 62;
    uint64_t result = 0;

    while (bit > value)
    {
        bit >>= 2;
    }

    while (bit != 0U)
    {
        if (value >= result + bit)
        {
            value -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }

        bit >>= 2;
    }

    return (uint32_t)result;
}

static int32_t raw_to_mg(int16_t raw)
{
    return ((int32_t)raw * LIS2DW12_RAW_16BIT_2G_NUM) / LIS2DW12_RAW_16BIT_2G_DENOM;
}

static int32_t read_register(uint8_t reg, uint8_t *value)
{
    return lis2dw12_read_reg(&lis2dw12Ctx, reg, value, 1);
}

static int32_t write_register(uint8_t reg, uint8_t value)
{
    return lis2dw12_write_reg(&lis2dw12Ctx, reg, &value, 1);
}

lis2dw12_driver_status_t lis2dw12_driver_init(const lis2dw12_driver_config_t *cfg)
{
    if ((cfg == 0) || (cfg->i2c_addr == 0U))
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    if ((cfg->full_scale_g != 2U) || (cfg->odr_hz != 25U))
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    lis2dw12Cfg = *cfg;
    lis2dw12_ti_ctx_init(&lis2dw12Ctx, cfg->i2c_addr);
    lis2dw12Initialized = true;
    lis2dw12Configured = false;

    return LIS2DW12_DRIVER_OK;
}

lis2dw12_driver_status_t lis2dw12_driver_read_id(uint8_t *whoami)
{
    if (whoami == 0)
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    if (!lis2dw12Initialized)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    return (lis2dw12_device_id_get(&lis2dw12Ctx, whoami) == 0) ?
           LIS2DW12_DRIVER_OK : LIS2DW12_DRIVER_ERR_I2C;
}

lis2dw12_driver_status_t lis2dw12_driver_configure_basic(void)
{
    uint8_t value;

    if (!lis2dw12Initialized)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    value = LIS2DW12_CTRL2_IF_ADD_INC | LIS2DW12_CTRL2_BDU;
    if (lis2dw12_write_reg(&lis2dw12Ctx, LIS2DW12_CTRL2, &value, 1) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    value = LIS2DW12_CTRL6_FS_2G;
    if (lis2dw12_write_reg(&lis2dw12Ctx, LIS2DW12_CTRL6, &value, 1) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    /* 25 Hz output data rate, +/-2 g full scale, low-power polling mode. */
    value = LIS2DW12_CTRL1_ODR_25HZ_LP_MODE1;
    if (lis2dw12_write_reg(&lis2dw12Ctx, LIS2DW12_CTRL1, &value, 1) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    lis2dw12Configured = true;
    (void)lis2dw12Cfg;
    return LIS2DW12_DRIVER_OK;
}

lis2dw12_driver_status_t lis2dw12_driver_read_raw(lis2dw12_raw_t *raw)
{
    int16_t data[3];

    if (raw == 0)
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    if (!lis2dw12Configured)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    if (lis2dw12_acceleration_raw_get(&lis2dw12Ctx, data) != 0)
    {
        return LIS2DW12_DRIVER_ERR_I2C;
    }

    raw->x = data[0];
    raw->y = data[1];
    raw->z = data[2];

    return LIS2DW12_DRIVER_OK;
}

lis2dw12_driver_status_t lis2dw12_driver_read_mg(lis2dw12_accel_mg_t *accel)
{
    lis2dw12_raw_t raw;
    int64_t magSq;

    if (accel == 0)
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    if (lis2dw12_driver_read_raw(&raw) != LIS2DW12_DRIVER_OK)
    {
        return LIS2DW12_DRIVER_ERR_I2C;
    }

    accel->x_mg = raw_to_mg(raw.x);
    accel->y_mg = raw_to_mg(raw.y);
    accel->z_mg = raw_to_mg(raw.z);

    magSq = ((int64_t)accel->x_mg * accel->x_mg) +
            ((int64_t)accel->y_mg * accel->y_mg) +
            ((int64_t)accel->z_mg * accel->z_mg);
    accel->magnitude_mg = isqrt_u64((uint64_t)magSq);

    return LIS2DW12_DRIVER_OK;
}

lis2dw12_driver_status_t lis2dw12_driver_get_status(uint8_t *status)
{
    if (status == 0)
    {
        return LIS2DW12_DRIVER_ERR_ARG;
    }

    if (!lis2dw12Initialized)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    return (lis2dw12_status_get(&lis2dw12Ctx, status) == 0) ?
           LIS2DW12_DRIVER_OK : LIS2DW12_DRIVER_ERR_I2C;
}

lis2dw12_driver_status_t lis2dw12_enable_activity_interrupt(void)
{
    uint32_t fullScaleMg;
    uint32_t threshold;
    uint32_t duration;
    uint8_t value;

    if (!lis2dw12Configured)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    fullScaleMg = (uint32_t)lis2dw12Cfg.full_scale_g * 1000U;
    threshold = ((uint32_t)APP_ACCEL_ACTIVITY_THRESHOLD_MG * 64U +
                 fullScaleMg - 1U) / fullScaleMg;
    if (threshold == 0U)
    {
        threshold = 1U;
    }
    else if (threshold > LIS2DW12_WAKE_UP_THS_MASK)
    {
        threshold = LIS2DW12_WAKE_UP_THS_MASK;
    }

    duration = ((uint32_t)APP_ACCEL_ACTIVITY_DURATION_MS *
                lis2dw12Cfg.odr_hz + 999U) / 1000U;
    if (duration > LIS2DW12_WAKE_UP_DUR_MAX)
    {
        duration = LIS2DW12_WAKE_UP_DUR_MAX;
    }

    /*
     * WAKE_UP_THS (0x34): threshold in FS/64 steps.
     * WAKE_UP_DUR (0x35): duration in ODR periods.
     * CTRL3 (0x22): push-pull, latched, configured interrupt polarity.
     * CTRL4_INT1_PAD_CTRL (0x23): route wake-up recognition to INT1.
     * CTRL7 (0x3F): enable embedded-function interrupts using HP-filter data.
     */
    if (read_register(LIS2DW12_REG_WAKE_UP_THS, &value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }
    value = (value & (uint8_t)~LIS2DW12_WAKE_UP_THS_MASK) |
            (uint8_t)threshold;
    if (write_register(LIS2DW12_REG_WAKE_UP_THS, value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    if (read_register(LIS2DW12_REG_WAKE_UP_DUR, &value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }
    value = (value & (uint8_t)~LIS2DW12_WAKE_UP_DUR_MASK) |
            (uint8_t)(duration << LIS2DW12_WAKE_UP_DUR_SHIFT);
    if (write_register(LIS2DW12_REG_WAKE_UP_DUR, value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    if (read_register(LIS2DW12_REG_CTRL3, &value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }
    value &= (uint8_t)~LIS2DW12_CTRL3_PP_OD;
    value |= LIS2DW12_CTRL3_LIR;
#if APP_ACCEL_INT1_ACTIVE_HIGH
    value &= (uint8_t)~LIS2DW12_CTRL3_H_LACTIVE;
#else
    value |= LIS2DW12_CTRL3_H_LACTIVE;
#endif
    if (write_register(LIS2DW12_REG_CTRL3, value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    if (read_register(LIS2DW12_REG_CTRL4_INT1_PAD_CTRL, &value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }
    value |= LIS2DW12_CTRL4_INT1_WU;
    if (write_register(LIS2DW12_REG_CTRL4_INT1_PAD_CTRL, value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    if (read_register(LIS2DW12_REG_CTRL7, &value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }
    value |= LIS2DW12_CTRL7_INTERRUPTS_ENABLE;
    if (write_register(LIS2DW12_REG_CTRL7, value) != 0)
    {
        return LIS2DW12_DRIVER_ERR_CONFIG;
    }

    return lis2dw12_clear_activity_interrupt();
}

lis2dw12_driver_status_t lis2dw12_clear_activity_interrupt(void)
{
    uint8_t source;

    if (!lis2dw12Configured)
    {
        return LIS2DW12_DRIVER_ERR_NOT_READY;
    }

    return (read_register(LIS2DW12_REG_WAKE_UP_SRC, &source) == 0) ?
           LIS2DW12_DRIVER_OK : LIS2DW12_DRIVER_ERR_I2C;
}
