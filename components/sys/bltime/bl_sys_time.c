#include <FreeRTOS.h>
#include <task.h>

#include <bl_sys_time.h>

static uint64_t epoch_time = 0; //in ms
static uint64_t time_synced = 0; //in ms

static uint32_t init_tick_rtos = 0;
static uint32_t init_tick_rtc = 0;
static int sync_init = 0;

void bl_sys_time_update(uint64_t epoch)
{
    time_synced = xTaskGetTickCount();
    epoch_time = epoch;
}

int bl_sys_time_get(uint64_t *epoch)
{
    int ms_diff;

    if (0 == epoch_time) {
        return -1;
    }

    //TODO we should use RTC hardware to track time
    ms_diff = ((int32_t)xTaskGetTickCount()) - ((int32_t)time_synced);
    *epoch = epoch_time + ms_diff;

    return 0;
}

void bl_sys_time_sync_init(void)
{
    taskENTER_CRITICAL();
    init_tick_rtos = xTaskGetTickCount();
    init_tick_rtc = bl_rtc_get_timestamp_ms() & 0xFFFFFFFF;
    taskEXIT_CRITICAL();
    
    sync_init = 1;
}

int bl_sys_time_sync_state(uint32_t *xTicksToJump)
{
    uint32_t currTickRtos;
    uint32_t currTickRtc;
    uint32_t deltaTickRtos;
    uint32_t deltaTickRtc;
    
    if(!sync_init){
        return -1;
    }
    
    taskENTER_CRITICAL();
    currTickRtos = xTaskGetTickCount();
    currTickRtc = bl_rtc_get_timestamp_ms() & 0xFFFFFFFF;
    taskEXIT_CRITICAL();
    
    deltaTickRtos = currTickRtos - init_tick_rtos;
    deltaTickRtc = currTickRtc - init_tick_rtc;
    
    if(deltaTickRtc > deltaTickRtos){
        *xTicksToJump = deltaTickRtc - deltaTickRtos;
        return 1;
    }else{
        return 0;
    }
}

uint32_t bl_sys_time_sync(void)
{
    uint32_t xTicksToJump = 0;
    
    if(bl_sys_time_sync_state(&xTicksToJump) > 0){
        vTaskStepTickSafe(xTicksToJump);
    }
    
    return xTicksToJump;
}
