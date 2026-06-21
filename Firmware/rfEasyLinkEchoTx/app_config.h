#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#define APP_UART_DEBUG_ENABLE     1

#define APP_ACCEL_I2C_ADDR        0x19
#define APP_ACCEL_SAMPLE_HZ       25
#define APP_ACCEL_FULL_SCALE_G    2

/*
 * Optional ACC/MOTION diagnostics are emitted only during an active motion
 * observation window. They remain bring-up diagnostics and will be removed
 * or reduced further during battery optimization.
 */
#define APP_ACCEL_DEBUG_PRINT_ENABLE          1
#define APP_ACCEL_DEBUG_PRINT_PERIOD_MS   5000U

/*
 * AHT20 environmental sensing is scheduled polling only. Temperature is
 * sampled every 60 seconds for now; slow-changing humidity is retained on a
 * five-minute application schedule. A future RF policy may transmit averaged
 * temperature once per minute, and a future low-power policy may use a timer
 * wake for these environmental samples.
 */
#define APP_TEMP_SAMPLE_PERIOD_MS        60000U
#define APP_HUMIDITY_SAMPLE_PERIOD_MS   300000U

/*
 * INT1 starts or extends an observation window used to capture shock, drop,
 * high-G, and movement duration after activity begins. This is awake-mode
 * preparation for future low-power operation. Accelerometer XYZ sampling is
 * disabled while the observation state is idle.
 */
#define APP_MOTION_OBSERVATION_WINDOW_MS        30000U
#define APP_MOTION_OBSERVATION_SAMPLE_PERIOD_MS   100U
#define APP_ENV_CACHE_INVALID_AGE_MS    600000U

/*
 * Bring-up/debug sensitivity: 250 mg for about 80 ms (2 samples) at 25 Hz.
 * Keep this proven bench setting for now. Tune the final product threshold
 * later, likely within 125 mg to 250 mg, with a duration of 1 to 3 samples
 * based on battery use and required event behavior.
 * INT1 is configured as an active-high, push-pull, latched output.
 */
#define APP_ACCEL_ACTIVITY_INT_ENABLE       1
#define APP_ACCEL_ACTIVITY_THRESHOLD_MG   250
#define APP_ACCEL_ACTIVITY_DURATION_MS      80
#define APP_ACCEL_INT1_ACTIVE_HIGH           1

#define MOTION_DYNAMIC_THRESHOLD_MG        250
#define SHOCK_THRESHOLD_MG                3000
#define SEVERE_SHOCK_THRESHOLD_MG         6000
#define DROP_FREEFALL_THRESHOLD_MG         300
#define SUSTAINED_HIGH_G_THRESHOLD_MG     1500
#define MOVEMENT_STOP_TIME_S                30

#endif /* APP_CONFIG_H_ */
