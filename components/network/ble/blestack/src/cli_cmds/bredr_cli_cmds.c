/** @file
 * @brief Bluetooth BR/EDR shell module
 *
 * Provide some BR/EDR commands that can be useful to applications.
 */

#include <stdlib.h>
#include <string.h>
#include <byteorder.h>
#include <bluetooth.h>
#include <hci_host.h>
#include <conn.h>
#include <l2cap.h>

#include "cli.h"


static void bredr_discoverable(char *p_write_buffer, int write_buffer_len, int argc, char **argv);
static void bredr_connectable(char *p_write_buffer, int write_buffer_len, int argc, char **argv);


const struct cli_command bredr_cmd_set[] STATIC_CLI_CMD_ATTRIBUTE = {
    {"bredr_discoverable", "", bredr_discoverable},
    {"bredr_connectable", "", bredr_connectable},

};

static void bredr_discoverable(char *p_write_buffer, int write_buffer_len, int argc, char **argv)
{
    int err;
    uint8_t action;
    
    if(argc != 2){
        printf("Number of parameters is not correct\n");
        return;
    }

    get_uint8_from_string(&argv[1], &action);
    
    if (action == 1) {
        err = bt_br_set_discoverable(true);
    } else if (action == 0) {
        err = bt_br_set_discoverable(false);
    } else {
        printf("Arg1 is invalid\n");
        return;
    }

    if (err) {
        printf("BR/EDR set discoverable failed, (err %d)\n", err);
    } else {
    	printf("BR/EDR set discoverable done.\n");
    }
}

static void bredr_connectable(char *p_write_buffer, int write_buffer_len, int argc, char **argv)
{
    int err;
    uint8_t action;
    
    if(argc != 2){
        printf("Number of parameters is not correct\n");
        return;
    }

    get_uint8_from_string(&argv[1], &action);
    
    if (action == 1) {
        err = bt_br_set_connectable(true);
    } else if (action == 0) {
        err = bt_br_set_connectable(false);
    } else {
        printf("Arg1 is invalid\n");
        return;
    }

    if (err) {
        printf("BR/EDR set connectable failed, (err %d)\n", err);
    } else {
    	printf("BR/EDR set connectable done.\n");
    }
}


int bredr_cli_register(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //aos_cli_register_commands(bredr_cmd_set, sizeof(bredr_cmd_set)/sizeof(bredr_cmd_set[0]));
    return 0;
}
		       
