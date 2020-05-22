#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <cli.h>

#include <bl_uart.h>
#include <bl_chip.h>
#include <bl_wifi.h>
#include <bl_sec.h>
#include <bl_cks.h>
#include <bl_irq.h>
#include <bl_dma.h>
#include <hal_uart.h>
#include <hal_sys.h>
#include <hal_gpio.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <looprt.h>
#include <loopset.h>
#include <bl_sys_time.h>
#include <fdt.h>
#include <libfdt.h>

#include <utils_log.h>
#include <libfdt.h>
#include <blog.h>

extern uint8_t _heap_start;
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegions[] =
{
        { &_heap_start,  (unsigned int) &_heap_size}, //set on runtime
        { &_heap_wifi_start, (unsigned int) &_heap_wifi_size },
        { NULL, 0 }, /* Terminates the array. */
        { NULL, 0 } /* Terminates the array. */
};

void user_vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName )
{
    puts("Stack Overflow checked\r\n");
    while (1) {
        /*empty here*/
    }
}

void user_vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
        xPortGetFreeHeapSize()
    );
//    while (1) {
//        /*empty here*/
//    }
}

void user_vApplicationIdleHook(void)
{
    __asm volatile(
            "   wfi     "
    );
    /*empty*/
}

static void _cli_init()
{
    /*Put CLI which needs to be init here*/
    bl_sys_time_cli_init();
}

static int get_dts_addr(const char *name, uint32_t *start, uint32_t *off)
{
    uint32_t addr = hal_board_get_factory_addr();
    const void *fdt = (const void *)addr;
    uint32_t offset;

    if (!name || !start || !off) {
        return -1;
    }

    offset = fdt_subnode_offset(fdt, 0, name);
    if (offset <= 0) {
       log_error("%s NULL.\r\n", name);
       return -1;
    }

    *start = (uint32_t)fdt;
    *off = offset;

    return 0;
}

static void aos_loop_proc(void *pvParameters)
{
    int fd_console;
    uint32_t fdt = 0, offset = 0;

    vfs_init();
    vfs_device_init();

    /* uart */
    if (0 == get_dts_addr("uart", &fdt, &offset)) {
        vfs_uart_init(fdt, offset);
    }

    aos_loop_init();

    fd_console = aos_open("/dev/ttyS0", 0);
    if (fd_console >= 0) {
        printf("Init CLI with event Driven\r\n");
        aos_cli_init(0);
        aos_poll_read_fd(fd_console, aos_cli_event_cb_read_get(), (void*)0x12345678);
        _cli_init();
    }

    aos_loop_run();

    puts("------------------------------------------\r\n");
    puts("+++++++++Critical Exit From Loop++++++++++\r\n");
    puts("******************************************\r\n");
    vTaskDelete(NULL);
}

void user_vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void user_vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void user_vAssertCalled(void)
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
    while( ulSetTo1ToExitFunction != 1 ) {
        __asm volatile( "NOP" );
    }
}

static void _dump_boot_info(void)
{
    char chip_feature[40];
    const char *banner;

    puts("Booting BL602 Chip...\r\n");

    /*Display Banner*/
    if (0 == bl_chip_banner(&banner)) {
        puts(banner);
    }
    puts("\r\n");
    /*Chip Feature list*/
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
    puts("RISC-V Core Feature:");
    bl_chip_info(chip_feature);
    puts(chip_feature);
    puts("\r\n");

    puts("Build Version: ");
    puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");
    puts("Build Date: ");
    puts(__DATE__);
    puts("\r\n");
    puts("Build Time: ");
    puts(__TIME__);
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");

}

static void system_init(void)
{
    blog_init();
    bl_sec_init();
    bl_sec_test();
    bl_dma_init();
    bl_irq_init();
    hal_boot2_init();

    /* board config is set after system is init*/
    hal_board_cfg(0);
}

static void system_thread_init()
{
    /*nothing here*/
}

static void __update_rom_api(void)
{
    struct romapi_freertos_map *romapi_freertos;

    romapi_freertos = hal_sys_romapi_get();

    romapi_freertos->vApplicationIdleHook = user_vApplicationIdleHook;
    romapi_freertos->vApplicationGetIdleTaskMemory = user_vApplicationGetIdleTaskMemory;
    romapi_freertos->vApplicationStackOverflowHook = user_vApplicationStackOverflowHook;
    romapi_freertos->vApplicationGetTimerTaskMemory = user_vApplicationGetTimerTaskMemory;
    romapi_freertos->vApplicationMallocFailedHook = user_vApplicationMallocFailedHook;
    romapi_freertos->vAssertCalled = user_vAssertCalled;

    hal_sys_romapi_update(romapi_freertos);
}

#define HEAP_TEST_UNIT_SIZE 1024

static void __heap_test_fun (void *p_arg)
{
    int counts, words, piece;
    uint32_t *ptr, *p_heap_addr;
    uint32_t  ptr_piece_num;
    uint8_t   test_ok;

    TaskHandle_t *p_handle;

    ptr_piece_num = xPortGetFreeHeapSize() / HEAP_TEST_UNIT_SIZE + 1;
    p_heap_addr = pvPortMalloc(ptr_piece_num * 4);

    if (p_heap_addr == NULL) {
        blog_info("mem not enough\r\n");
        p_handle = (TaskHandle_t *)p_arg;
        vTaskDelete(*p_handle);
        return;
	}

    counts = 0;
    ptr = NULL;
    while ((ptr = pvPortMalloc(HEAP_TEST_UNIT_SIZE)) != NULL) {

        if (counts == 0) {
            blog_info("ptr_start = %p\r\n", ptr);
        }

        blog_info("ptr = %p\r\n", ptr);
        for (words = 0; words < HEAP_TEST_UNIT_SIZE / 4; words++) {
            *(ptr + words) = (uint32_t)ptr + words;
        }

        p_heap_addr[counts] = (uint32_t)ptr;
        counts++;
    }
    blog_info("ptr_end = %p\r\n", p_heap_addr[counts - 1]);
    blog_info("%d pieces of test mem, %d bytes each\r\n", counts - 1, HEAP_TEST_UNIT_SIZE);

    test_ok = 1;
    for (piece = 0; piece < counts; piece++) {
        ptr = (uint32_t *)p_heap_addr[piece];
        for (words = 0; words < HEAP_TEST_UNIT_SIZE / 4; words++) {
            if (*(ptr + words) != (uint32_t)ptr + words) {
                blog_info("\r\n-------------%p error----------\r\n", ptr);
                test_ok = 0;
            }
        }
        vPortFree(ptr);
    }
    vPortFree(p_heap_addr);

    if (test_ok == 1) {
        blog_info("mem heap test ok!\r\n");
    } else {
        blog_info("mem heap test failed!\r\n");
    }

    p_handle = (TaskHandle_t *)p_arg;
    vTaskDelete(*p_handle);
}

static void cmd_heap_test(char *buf, int len, int argc, char **argv)
{
    static StackType_t heap_test_stack[1024];
    static StaticTask_t heap_test_task;
    static TaskHandle_t task_handle;

    puts("[OS] start heap test task...\r\n");
    task_handle = xTaskCreateStatic(__heap_test_fun,
                                    (char*)"test",
                                    1024,
                                    &task_handle,
                                    20,
                                    heap_test_stack,
                                    &heap_test_task);
}

const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        { "heap_test", "Heap Test", cmd_heap_test},
};
void bfl_main(void)
{
    static StaticTask_t aos_loop_proc_task;
    static StackType_t aos_loop_proc_stack[1024];

    /*Init UART In the first place*/
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
    puts("Starting bl602 now....\r\n");

    __update_rom_api();
    _dump_boot_info();

    vPortDefineHeapRegions(xHeapRegions);

    system_init();
    system_thread_init();

    puts("[OS] Starting aos_loop_proc task...\r\n");
    xTaskCreateStatic(aos_loop_proc, (char*)"event_loop", 1024, NULL, 15, aos_loop_proc_stack, &aos_loop_proc_task);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();
}
