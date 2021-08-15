#ifndef __HAL_BOOT2_H__
#define __HAL_BOOT2_H__
#include <stdint.h>
#include "bl_boot2.h"

void hal_boot2_set_ptable_opt(pPtTable_Flash_Erase erase, pPtTable_Flash_Write write);
int hal_boot2_partition_bus_addr_active(const char *name, uint32_t *addr);
int hal_boot2_partition_bus_addr(const char *name, uint32_t *addr0, uint32_t *addr1, int *active);
int hal_boot2_partition_addr_active(const char *name, uint32_t *addr);
int hal_boot2_partition_addr(const char *name, uint32_t *addr0, uint32_t *addr1, int *active);
uint8_t hal_boot2_get_active_partition(void);
int hal_boot2_get_active_entries(PtTable_Entry_Type type, PtTable_Entry_Config *ptEntry);
int hal_boot2_update_ptable(PtTable_Entry_Config *ptEntry);
int hal_boot2_dump(void);
int hal_boot2_init(void);

#endif
