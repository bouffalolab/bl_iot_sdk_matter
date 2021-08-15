#ifndef __BL_SYS_H__
#define __BL_SYS_H__
#define BL_SYS_HBN_SRC_GPIO_ID_10 (10)
int bl_sys_reset_por(void);
int bl_sys_mem_info(char *buffer, int total);
int bl_sys_hbn(int source);
int bl_cpu0_ipc_enable(void);
void bl_sys_cpu1_cache_enable(void);
#endif
