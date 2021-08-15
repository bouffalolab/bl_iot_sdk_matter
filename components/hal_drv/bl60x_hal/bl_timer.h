#ifndef __BL_TIMER_H__
#define __BL_TIMER_H__
#include <stdint.h>

typedef enum {
    BL_TIMER_ID_0,
    BL_TIMER_ID_1,
} bl_timer_id_t;

typedef enum {
    BL_TIMER_TYPE_ONESHOT,
    BL_TIMER_TYPE_REPEAT
} bl_timer_type_t;

/*duration's unit is us*/
int bl_timer_init(bl_timer_id_t id, bl_timer_type_t type, uint32_t duration);

/*cb will be called with timer id and arg*/
int bl_timer_start(bl_timer_id_t id, void (*cb)(bl_timer_id_t id, void *arg), void *arg);

/*stop maybe called in callback*/
int bl_timer_stop(bl_timer_id_t id);
#endif
