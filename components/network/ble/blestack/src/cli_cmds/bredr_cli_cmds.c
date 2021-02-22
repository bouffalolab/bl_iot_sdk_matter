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

static void bredr_write_eir(char *p_write_buffer, int write_buffer_len, int argc, char **argv);
static void bredr_discoverable(char *p_write_buffer, int write_buffer_len, int argc, char **argv);
static void bredr_connectable(char *p_write_buffer, int write_buffer_len, int argc, char **argv);


const struct cli_command bredr_cmd_set[] STATIC_CLI_CMD_ATTRIBUTE = {
    {"bredr_eir", "", bredr_write_eir},
    {"bredr_connectable", "", bredr_connectable},
    {"bredr_discoverable", "", bredr_discoverable},

};

static void bredr_write_eir(char *p_write_buffer, int write_buffer_len, int argc, char **argv)
{
    int err;
    char *name = "Bouffalolab-bl606p-classic-bt";
    uint8_t rec = 1;
    uint8_t data[32] = {0};

    data[0] = 30;
    data[1] = 0x09;
    memcpy(data+2, name, strlen(name));

    for(int i = 0; i < strlen(name); i++)
    {
        printf("0x%02x ", data[2+i]);
    }
    printf("\n");

    err = bt_br_write_eir(rec, data);
    if (err) {
        printf("BR/EDR write EIR failed, (err %d)\n", err);
    } else {
        printf("BR/EDR write EIR done.\n");
    }
}

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
		       
