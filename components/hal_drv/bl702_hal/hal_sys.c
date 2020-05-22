#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hal_sys.h"

void hal_reboot()
{
    /*TODO empty now*/
}

void hal_poweroff()
{
    /*TODO empty now*/
}

static uint32_t user_clz(uint32_t priorities)
{
    return __builtin_clz(priorities);
}

struct romapi_freertos_map* hal_sys_romapi_get(void)
{
    extern uint8_t __global_pointer_head$;
    uint32_t *gp_data_start = (uint32_t*)(&__global_pointer_head$);
    struct romapi_freertos_map* romapi_freertos;

    puts("  Configuring Version 1.0 ROM API...\r\n");

    /*clear bss/common section in GP area*/
    memset(&__global_pointer_head$, 0, 0x4A0);
    romapi_freertos = (struct romapi_freertos_map*) (((uint8_t*)gp_data_start) + 0x45c);

    /* *pTrapNetCounter = &TrapNetCounter */
    *(gp_data_start + 0) = (uint32_t)((uint8_t*)(gp_data_start) + 0x5C);
    /* *pullNextTime = &ullNextTime */
    *(gp_data_start + 1) = (uint32_t)((uint8_t*)(gp_data_start) + 0x60);
    /* uxTimerIncrementsForOneTick */
    *(gp_data_start + 2) = 2000 * 1000 / 1000;//Use 2M on fpga

    return romapi_freertos;
}

void hal_sys_romapi_update(struct romapi_freertos_map *romapi_freertos)
{
    extern void interrupt_entry(uint32_t mcause);
    extern void exception_entry(uint32_t mcause, uint32_t mepc, uint32_t mtval);
    extern uint8_t __freertos_irq_stack_top;

    romapi_freertos->rtos_strlen_ptr = strlen;
    romapi_freertos->rtos_sprintf = sprintf;
    romapi_freertos->rtos_memset_ptr = memset;
    romapi_freertos->exception_entry_ptr = exception_entry;
    romapi_freertos->rtos_clz = user_clz;
    romapi_freertos->rtos_memcpy_ptr = memcpy;
    romapi_freertos->rtos_strcpy_ptr = strcpy;
    romapi_freertos->interrupt_entry_ptr = interrupt_entry;
    romapi_freertos->xISRStackTop = &__freertos_irq_stack_top;

}
