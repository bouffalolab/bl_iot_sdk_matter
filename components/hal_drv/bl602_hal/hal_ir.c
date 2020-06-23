/*
 * Copyright (c) 2020 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <fdt.h>
#include <libfdt.h>
#include <blog.h>
#include <loopset.h>

#include "hal_ir.h"
#include "bl_ir.h"

#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))

int hal_ir_init_from_dts(uint32_t fdt_input, uint32_t dtb_offset)
{
    int offset1 = 0;
    const void *fdt = (const void *)fdt_input;

    const uint32_t *addr_prop = 0;
    int lentmp = 0;
    const char *result = 0;
    int countindex = 0;
    int ctrltype = 0;

    uint8_t pin = 0;
    uint16_t interval = NULL;
   
    addr_prop = fdt_getprop(fdt, dtb_offset, "ctrltype", &lentmp);
    if (addr_prop == NULL) {
        log_info("do not find ctrltype \r\n");
    } else {
        ctrltype = BL_FDT32_TO_U32(addr_prop, 0);
        log_info("ctrltype == %d \r\n", ctrltype);
    }

    offset1 = fdt_subnode_offset(fdt, dtb_offset, "rx");
    if (0 >= offset1) {
        log_info("ir rx NULL.\r\n");
    } else {
        countindex = fdt_stringlist_count(fdt, offset1, "status");
        if (countindex != 1) {
            log_info("ir rx status_countindex = %d NULL.\r\n", countindex);
        } else {
            result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
            if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
                log_info("ir rx status = %s\r\n", result);
            } else {
                /* set id */
                addr_prop = fdt_getprop(fdt, offset1, "pin", &lentmp);
                if (addr_prop == NULL) {
                    log_info("ir rx pin NULL.\r\n");
                } else {
                    pin = BL_FDT32_TO_U32(addr_prop, 0);
                    log_info("pin == %d \r\n", pin);
                    addr_prop = fdt_getprop(fdt, offset1, "interval", &lentmp);
                    if (addr_prop == NULL) {
                        log_info("ir rx interval NULL.\r\n");
                    } else {
                        interval = BL_FDT32_TO_U32(addr_prop, 0);
                        log_info("add here pin = %d, interval = %d\r\n", pin, interval);
                    }
                }
            }
        }
    }
   
    //TODO clean out ctrltype
    bl_ir_init(pin, ctrltype);

    return 0;
}
