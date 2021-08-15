#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__
int vfs_gpio_init(void);
int vfs_gpio_fdt_init(uint32_t fdt, uint32_t dtb_gpio_offset);
#endif