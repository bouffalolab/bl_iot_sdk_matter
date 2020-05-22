#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bl_uart.h>
#include "demo.h"

static const char *ci_table_step_init[] = CI_CASE_TABLE_STEP1;
static const char *ci_table_step_log[] = CI_CASE_TABLE_STEP2;
static const char *ci_table_step_end[] = CI_CASE_TABLE_STEP3;

void log_step(const char *step[2])
{
    printf("%s   %s\r\n", step[0], step[1]);
}

void helloworld(void)
{
    log_step(ci_table_step_init);
    log_step(ci_table_step_log);
    log_step(ci_table_step_end);
}

void user_vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName )
{
    /*empty*/
}

void user_vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
        xPortGetFreeHeapSize()
    );  
    /*empty*/
}

void user_vApplicationIdleHook(void)
{
    __asm volatile(
            "   wfi     "
    );  
    /*empty*/
}

void bfl_main(void)
{
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);    
    helloworld();
}

