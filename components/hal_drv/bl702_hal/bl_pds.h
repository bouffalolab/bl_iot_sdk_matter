#ifndef __BL_PDS_H__
#define __BL_PDS_H__


#include "bl702_glb.h"
#include "bl702_sec_eng.h"
#include "bl702_sf_cfg.h"
#include "bl702_sflash.h"
#include "bl702_romdriver.h"


#define ATTR_PDS_SECTION                __attribute__((section(".pds_code")))


void bl_pds_init(void);
void bl_pds_fastboot_cfg(uint32_t addr);
void bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);


#endif
