#include "bl_timer.h"
#include "bl60x_timer.h"

static TIMER_CFG_Type timerCfg = {
    TIMER_CH0,                   /* timer channel 0 */
    TIMER_CLKSRC_FCLK,           /* timer clock source:bus clock */
    TIMER_PRELOAD_TRIG_COMP0,    /* reaload on comaparator 0  */
    TIMER_COUNT_PRELOAD,         /* preload when match occur */
    100000000-1,                 /* match value 0  */
    0,
    30000000,                    /* match value 1 */
    1,
    30000000,                    /* match value 2 */
    1,
    0,                           /* preload value */
    0,
};
static bl_timer_id_t timer_id;
static void * timer_arg;
static void (*bl_timer_cb)(bl_timer_id_t id, void *arg);

static void Timer_Match0_Cbf(void)
{
    bl_timer_cb(timer_id, timer_arg);
}

int bl_timer_init(bl_timer_id_t id, bl_timer_type_t type, uint32_t duration)
{
    TIMER_ID_Type timerId = (TIMER_ID_Type)id;
    uint64_t temp = duration;
    uint64_t matchValue = (uint64_t)(160 * temp) - 1;

    timerCfg.matchVal0_h = (matchValue >> 32);
    timerCfg.matchVal0 = (matchValue & 0xFFFFFFFF);
    if(type == BL_TIMER_TYPE_ONESHOT){
        timerCfg.countMode = TIMER_COUNT_FREERUN;
    }else{
        timerCfg.countMode = TIMER_COUNT_PRELOAD;
    }
    /* Disable all interrupt */
    TIMER_IntMask(timerId, timerCfg.timerCh,TIMER_INT_ALL, MASK);

    /* Disable timer before config */
    TIMER_Disable(timerId, timerCfg.timerCh);

    /* Timer init with default configuration */
    TIMER_Init(timerId, &timerCfg);
    return 0;
}

int bl_timer_start(bl_timer_id_t id, void (*cb)(bl_timer_id_t id, void *arg), void *arg)
{
    TIMER_ID_Type timerId = (TIMER_ID_Type)id;
    IRQn_Type timerIrqn=TIMER0_CH0_IRQn;

    /* Clear interrupt status*/
    TIMER_ClearIntStatus(timerId, timerCfg.timerCh, TIMER_COMP_ID_0);

    /* Enable timer match interrupt */
    TIMER_IntMask(timerId, timerCfg.timerCh, TIMER_INT_COMP_0, UNMASK);

    /* Install the interrupt callback function */
    Install_Int_Callback(timerIrqn, TIMER_INT_COMP_0, &Timer_Match0_Cbf);
    bl_timer_cb = cb;
    timer_id = id;
    timer_arg = arg;

    NVIC_EnableIRQ(timerIrqn);
    NVIC_SetPriority(TIMER0_CH0_IRQn, 7);

    /* Enable timer */
    TIMER_Enable(timerId, timerCfg.timerCh);
    return 0;
}

int bl_timer_stop(bl_timer_id_t id)
{
    TIMER_ID_Type timerId = (TIMER_ID_Type)id;
    TIMER_Disable(timerId, timerCfg.timerCh);
    return 0;
}
