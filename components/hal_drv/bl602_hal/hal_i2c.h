#ifndef __HAL_I2C_H__
#define __HAL_I2C_H__

#include <bl_i2c.h>

#define EV_I2C_END_INT       0
#define EV_I2C_TXF_INT       1
#define EV_I2C_RXF_INT       3
#define EV_I2C_FER_INT       4
#define EV_I2C_ARB_INT       5 
#define EV_I2C_NAK_INT       6
#define EV_I2C_UNKNOW_INT    0xff

int hal_i2c_init(int i2cx, int freq);
int hal_i2c_read_block(int address, char *data, int length, int subaddr_len, int subaddr);
int hal_i2c_write_block(int address, const char *data, int length, int subaddr_len, int subaddr);
int hal_i2c_write_no_block(int address, const char *data, int length, int subaddr_len, int subaddr);
int hal_i2c_read_no_block(int address, const char *data, int length, int subaddr_len, int subaddr);

int i2c_transfer_msgs_block(i2c_msg_t *pstmsg, int num, int support_ins);
void i2c_msgs_process(i2c_msg_t *pstmsg);
void i2c_insert_msgs_process(i2c_msg_t *pstmsg);

#endif
