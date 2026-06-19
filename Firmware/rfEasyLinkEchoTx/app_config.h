#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#define APP_UART_DEBUG_ENABLE     1

#define APP_ACCEL_I2C_ADDR        0x19
#define APP_ACCEL_SAMPLE_HZ       25
#define APP_ACCEL_FULL_SCALE_G    2

#define MOTION_DYNAMIC_THRESHOLD_MG        250
#define SHOCK_THRESHOLD_MG                3000
#define SEVERE_SHOCK_THRESHOLD_MG         6000
#define DROP_FREEFALL_THRESHOLD_MG         300
#define SUSTAINED_HIGH_G_THRESHOLD_MG     1500
#define MOVEMENT_STOP_TIME_S                30
#define EVENT_BURST_SAMPLE_TIME_S             3

#endif /* APP_CONFIG_H_ */
