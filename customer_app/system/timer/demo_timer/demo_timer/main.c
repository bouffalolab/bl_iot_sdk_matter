#include <FreeRTOS.h>
#include <task.h>
#include <bl_sys_time.h>
#include <bl_timer.h>
#include <cli.h>

static void cmd_timer_us(char *buf, int len, int argc, char **argv)
{
    printf("Timer2 value is %lu\r\n",
            bl_timer_now_us()
    );
}

const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        { "timer_us", "check timer_us", cmd_timer_us},
};

static void _cli_init()
{
    /*Put CLI which needs to be init here*/
    bl_sys_time_cli_init();
}

static void proc_main_entry(void *pvParameters)
{
    _cli_init();

    vTaskDelete(NULL);
}

static void system_thread_init()
{
    /*nothing here*/
}

void main()
{
    system_thread_init();

    puts("[OS] proc_main_entry task...\r\n");
    xTaskCreate(proc_main_entry, (char*)"main_entry", 1024, NULL, 15, NULL);

}
