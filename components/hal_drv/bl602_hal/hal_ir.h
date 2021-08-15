#ifndef __HAL_IR_H__
#define __HAL_IR_H__

int hal_ir_init_from_dts(uint32_t fdt, uint32_t dtb_offset);
int hal_irled_init(int chip_type);
int hal_irled_send_data(int data_num, uint32_t *buf);
#endif
