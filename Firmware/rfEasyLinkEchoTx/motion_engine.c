#include "motion_engine.h"

#include <limits.h>
#include <string.h>

#include "app_config.h"

#define MOTION_ENGINE_GRAVITY_MG 1000U

static motion_engine_status_t motionStatus;
static uint32_t movementStartTimestampS;
static uint32_t belowThresholdStartTimestampS;
static bool belowThresholdTimerActive;
static bool previousShock;
static bool previousSevereShock;
static bool previousDrop;

static void increment_count(uint32_t *count)
{
    if (*count != UINT32_MAX)
    {
        (*count)++;
    }
}

void motion_engine_init(void)
{
    memset(&motionStatus, 0, sizeof(motionStatus));
    motionStatus.state = MOTION_ENGINE_STATE_STILL;

    movementStartTimestampS = 0U;
    belowThresholdStartTimestampS = 0U;
    belowThresholdTimerActive = false;
    previousShock = false;
    previousSevereShock = false;
    previousDrop = false;
}

void motion_engine_update(int32_t x_mg,
                          int32_t y_mg,
                          int32_t z_mg,
                          uint32_t mag_mg,
                          uint32_t timestamp_s)
{
    bool motionDetected;

    (void)x_mg;
    (void)y_mg;
    (void)z_mg;

    if (mag_mg >= MOTION_ENGINE_GRAVITY_MG)
    {
        motionStatus.dynamic_mg = mag_mg - MOTION_ENGINE_GRAVITY_MG;
    }
    else
    {
        motionStatus.dynamic_mg = MOTION_ENGINE_GRAVITY_MG - mag_mg;
    }

    if (mag_mg > motionStatus.max_g_mg)
    {
        motionStatus.max_g_mg = mag_mg;
    }

    motionDetected = motionStatus.dynamic_mg >= MOTION_DYNAMIC_THRESHOLD_MG;
    motionStatus.motion_event = motionDetected;
    motionStatus.shock_event = mag_mg >= SHOCK_THRESHOLD_MG;
    motionStatus.severe_shock_event = mag_mg >= SEVERE_SHOCK_THRESHOLD_MG;
    motionStatus.drop_event = mag_mg <= DROP_FREEFALL_THRESHOLD_MG;
    motionStatus.sustained_high_g_event =
        mag_mg >= SUSTAINED_HIGH_G_THRESHOLD_MG;

    if (motionDetected)
    {
        if (motionStatus.state == MOTION_ENGINE_STATE_STILL)
        {
            motionStatus.state = MOTION_ENGINE_STATE_MOVING;
            movementStartTimestampS = timestamp_s;
            increment_count(&motionStatus.motion_count);
        }

        belowThresholdTimerActive = false;
    }
    else if (motionStatus.state == MOTION_ENGINE_STATE_MOVING)
    {
        if (!belowThresholdTimerActive)
        {
            belowThresholdStartTimestampS = timestamp_s;
            belowThresholdTimerActive = true;
        }
        else if ((timestamp_s - belowThresholdStartTimestampS) >=
                 MOVEMENT_STOP_TIME_S)
        {
            motionStatus.state = MOTION_ENGINE_STATE_STILL;
            motionStatus.movement_duration_s = 0U;
            belowThresholdTimerActive = false;
        }
    }

    if (motionStatus.state == MOTION_ENGINE_STATE_MOVING)
    {
        motionStatus.movement_duration_s = timestamp_s - movementStartTimestampS;
    }
    else
    {
        motionStatus.movement_duration_s = 0U;
    }

    if (motionStatus.shock_event && !previousShock)
    {
        increment_count(&motionStatus.shock_count);
    }

    if (motionStatus.severe_shock_event && !previousSevereShock)
    {
        increment_count(&motionStatus.severe_shock_count);
    }

    if (motionStatus.drop_event && !previousDrop)
    {
        increment_count(&motionStatus.drop_count);
    }

    if (motionStatus.sustained_high_g_event)
    {
        increment_count(&motionStatus.sustained_high_g_count);
    }

    previousShock = motionStatus.shock_event;
    previousSevereShock = motionStatus.severe_shock_event;
    previousDrop = motionStatus.drop_event;
}

void motion_engine_get_status(motion_engine_status_t *status)
{
    if (status != NULL)
    {
        *status = motionStatus;
    }
}
