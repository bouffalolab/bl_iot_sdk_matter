#include <bl606_timer.h>

#include "os_hal.h"
#include "bl_wdt.h"

static BL_Err_Type Timer_Watchdog_Case(TIMER_ID_Type timerId)
{
    WDT_Disable(timerId);
    WDT_Set_CLk_Src(timerId, TIMER_CLKSRC_1K);
    WDT_SetCompVaule(timerId, 60000);
    WDT_ResetCounterVaule(timerId);
    WDT_IntMask(timerId, WDT_INT, MASK);
    
/**
 ****************************************************************************************
 *
 * @file bl_wdt.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

    WDT_Enable(timerId);

    return SUCCESS;
}

int bl_wdt_init(void)
{
    Timer_Watchdog_Case(TIMER0_ID);
    return 0;
}
