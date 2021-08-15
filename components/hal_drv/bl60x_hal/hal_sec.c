#include <cli.h>

#include "bl_sec.h"
#include "hal_sec.h"

static void test_cmd_trng(char *buf, int len, int argc, char **argv);
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        { "test_trng", "Test TRNG", test_cmd_trng},                                       
};                                                                                   

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
static void test_cmd_trng(char *buf, int len, int argc, char **argv)
{
    bl_sec_test();
}

int hal_sec_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));          
    return 0;
}
