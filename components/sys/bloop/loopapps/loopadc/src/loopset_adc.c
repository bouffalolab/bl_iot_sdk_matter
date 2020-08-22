#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils_debug.h>
#include <utils_log.h>
#include <blog.h>
#include <looprt.h>
#include <loopset.h>
#include <bl_adc.h>
#include <loopset_adc.h>
#include <aos/yloop.h>

#define ADC_PRIORITY   5

struct loop_evt_handler_holder {
    const struct loop_evt_handler handler;
};

struct adc_trigger_item {
    int oneshot;
    int exist;
    int status;
    int event_type;
    uint32_t volt;
    struct loop_timer timer;
};

struct adc_trigger_item adc_item;

static int _adc_bloop_evt(struct loop_ctx *loop, const struct loop_evt_handler *handler, uint32_t *bitmap_evt, uint32_t *evt_type_map)
{
    uint32_t map = *evt_type_map;
redo:
    if (map & EVT_MAP_ADC_CREATER) {
        map &= (~EVT_MAP_ADC_CREATER);
        if (adc_item.oneshot == 0) {
            looprt_timer_register(&adc_item.timer);
        } else { 
        }
    }else if (map & EVT_MAP_ADC_TRIGGER) {
        map &= (~EVT_MAP_ADC_TRIGGER);
        aos_post_event(EV_ADC, adc_item.event_type, adc_item.volt);
    } else if (map & EVT_MAP_ADC_TIMER) {
        map &= (~EVT_MAP_ADC_TIMER);
        /* do nothing */ 
    } else {
    }

    if (map) {
        goto redo;
    }

    *evt_type_map = 0;
    return 0;
}

static int _adc_bloop_msg(struct loop_ctx *loop, const struct loop_evt_handler *handler, struct loop_msg *msg)
{
    blog_debug("[ADC] [MSG] called with msg info\r\n");

    return 0;
}

int loopapp_adc_hook_on_looprt(void)
{
    static const struct loop_evt_handler_holder _adc_bloop_handler_holder = {
        .handler = {
            .name = "ADC Trigger",
            .evt = _adc_bloop_evt,
            .handle = _adc_bloop_msg,
        },
    };

    return looprt_handler_register(&_adc_bloop_handler_holder.handler, ADC_PRIORITY);
}

static void _cb_adc_trigger(struct loop_ctx *loop, struct loop_timer *timer, void *arg)
{
    if (adc_item.status == 1) {
        start_adc_data_collect();
    } else {
        /* adc stop, do nothing */
    }

    return;
}

void loopapp_adc_create(int oneshot , int sampling_ms)
{
    if (adc_item.exist == 1) {
        blog_error("error: can not init adc more than once \r\n");
        return;
    }

    memset(&adc_item, 0, sizeof(struct adc_trigger_item));

    if (oneshot == 0) {
        adc_item.oneshot = 0;
        adc_item.exist = 0;
        adc_item.volt = 0;
        adc_item.status = 0;
        bloop_timer_init(&adc_item.timer, 0);
        bloop_timer_repeat_enable(&adc_item.timer);
        bloop_timer_configure(&adc_item.timer, sampling_ms,
            _cb_adc_trigger,
            &adc_item,
            ADC_PRIORITY,
            EVT_MAP_ADC_TIMER
        );
    } else {
        adc_item.oneshot = 1;
        adc_item.exist = 0;
    }
    
    return;
}

void loopapp_adc_process(int flag)
{
    if (flag == 0) {
        if (adc_item.exist == 0) {
            adc_item.exist = 1;
            adc_item.status = 1;
            looprt_evt_notify_async(ADC_PRIORITY, EVT_MAP_ADC_CREATER);
        } else {
            adc_item.status = 1;
        }
    } else {
        adc_item.status = 0;
    }
        
    return;
}

void loopapp_adc_trigger(uint32_t volt, int event_type)
{
    adc_item.volt = volt;
    adc_item.event_type = event_type;
    looprt_evt_notify_async_fromISR(ADC_PRIORITY, EVT_MAP_ADC_TRIGGER);

    return;
}
