#ifndef __HAL_UART_H__
#define __HAL_UART_H__
int vfs_uart_init(uint32_t fdt, uint32_t dtb_uart_offset);
int hal_uart_data_notify(int number, int dir);
#endif
