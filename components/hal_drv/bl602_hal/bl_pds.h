#ifndef __BL_PDS_H__
#define __BL_PDS_H__


#include "bl602_glb.h"
#include "bl602_sec_eng.h"
#include "bl602_sf_cfg_ext.h"
#include "bl602_sflash_ext.h"
#include "bl602_romdriver.h"


void bl_pds_init(void);
void bl_pds_fastboot_cfg(uint32_t addr);
void bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);


#endif
