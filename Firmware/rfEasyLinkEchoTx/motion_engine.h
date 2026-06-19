#ifndef MOTION_ENGINE_H_
#define MOTION_ENGINE_H_

#include <stdbool.h>
#include <stdint.h>

#define MOTION_ENGINE_FLAG_MOTION          0x01U
#define MOTION_ENGINE_FLAG_SHOCK           0x02U
#define MOTION_ENGINE_FLAG_SEVERE_SHOCK    0x04U
#define MOTION_ENGINE_FLAG_DROP            0x08U
#define MOTION_ENGINE_FLAG_SUSTAINED_HIGHG 0x10U

typedef enum
{
    MOTION_ENGINE_STATE_STILL = 0,
    MOTION_ENGINE_STATE_MOVING
} motion_engine_state_t;

typedef struct
{
    motion_engine_state_t state;

    uint32_t motion_count;
    uint32_t shock_count;
    uint32_t severe_shock_count;
    uint32_t drop_count;
    uint32_t sustained_high_g_count;

    uint32_t movement_duration_s;
    uint32_t max_g_mg;
    uint32_t dynamic_mg;

    bool motion_event;
    bool shock_event;
    bool severe_shock_event;
    bool drop_event;
    bool sustained_high_g_event;
} motion_engine_status_t;

void motion_engine_init(void);
void motion_engine_update(int32_t x_mg,
                          int32_t y_mg,
                          int32_t z_mg,
                          uint32_t mag_mg,
                          uint32_t timestamp_s);
void motion_engine_get_status(motion_engine_status_t *status);

#endif /* MOTION_ENGINE_H_ */
