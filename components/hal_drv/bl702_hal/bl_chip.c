#include <stdint.h>
#include <string.h>

#include "bl_chip.h"

static uint32_t _risc_isa_info(void)
{
    volatile uint32_t misa = 0;

    __asm volatile( "csrr %0, misa" : "=r"( misa ) );

    return misa;
}


int bl_chip_info(char *info)
{
    uint32_t misa;
    int i;

    misa = _risc_isa_info();

    /*Get base ISA*/
    i = (misa >> 30);
    switch (i) {
        case 1:
        {
            memcpy(info, "RV32", 4);
            info += 4;
        }
        break;
        case 2:
        {
            memcpy(info, "RV64", 4);
            info += 4;
        }
        break;
        case 3:
        {
            memcpy(info, "RV128", 5);
            info += 5;
        }
        break;
        default:
        {
            memcpy(info, "RVxx", 4);
            info += 4;
        }
        break;
    }

    /*add switch*/
    *(info++) = '-';

    /*add feature set*/
    for (i = 0; i < 26; i++) {
        if (misa & (1 << i)) {
            /*Feature bit is set*/
            *(info++) = ('A' + i);
        }
    }
    *info = '\0';

    return 0;
}

static const char bannder_shadow_bl702[] = {
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2,
  0x96, 0x88, 0xe2, 0x95, 0x97, 0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0x20,
  0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0x20,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2,
  0x96, 0x88, 0xe2, 0x95, 0x97, 0x20, 0x0d, 0x0a, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95,
  0x94, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0x20, 0x20, 0xe2, 0x95, 0x9a, 0xe2, 0x95,
  0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x95, 0x91, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x90, 0xe2,
  0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0xe2, 0x95,
  0x9a, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0x0d, 0x0a, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96,
  0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe2,
  0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d, 0xe2, 0x96, 0x88, 0xe2, 0x96,
  0x88, 0xe2, 0x95, 0x91, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d, 0x0d, 0x0a, 0xe2, 0x96,
  0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x95, 0x97, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0x20,
  0x20, 0x20, 0x20, 0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d,
  0x20, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94,
  0xe2, 0x95, 0x9d, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0xe2, 0x96, 0x88, 0xe2,
  0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95,
  0x9d, 0x20, 0x0d, 0x0a, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d, 0xe2, 0x96, 0x88, 0xe2,
  0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96,
  0x88, 0xe2, 0x95, 0x97, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x91, 0x20, 0x20, 0xe2,
  0x95, 0x9a, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96,
  0x88, 0xe2, 0x96, 0x88, 0xe2, 0x95, 0x94, 0xe2, 0x95, 0x9d, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88,
  0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2, 0x96, 0x88, 0xe2,
  0x95, 0x97, 0x0d, 0x0a, 0xe2, 0x95, 0x9a, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90,
  0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x9d, 0x20, 0xe2, 0x95, 0x9a, 0xe2, 0x95, 0x90,
  0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2,
  0x95, 0x9d, 0xe2, 0x95, 0x9a, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x9d, 0x20, 0x20, 0x20, 0xe2, 0x95,
  0x9a, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90,
  0xe2, 0x95, 0x9d, 0x20, 0xe2, 0x95, 0x9a, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90,
  0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x90, 0xe2, 0x95, 0x9d, 0x0a, 0x0d, 0x00
};

int bl_chip_banner(const char **banner)
{
    *banner = bannder_shadow_bl702;
    return 0;
}

int bl_chip_memory_ram(int *num, unsigned int addr[], unsigned int size[], char desc[][6])
{
    if (*num < 3) {
        /*only one block memory*/
        return -1;
    }
    *num = 3;

extern uint8_t _ld_ram_size0, _ld_ram_addr0;
extern uint8_t _ld_ram_size1, _ld_ram_addr1;
extern uint8_t _ld_ram_size2, _ld_ram_addr2;
    addr[0] = (unsigned int)&_ld_ram_addr0;
    size[0] = (unsigned int)&_ld_ram_size0;
    strcpy(desc[0], "flash");
    addr[1] = (unsigned int)&_ld_ram_addr1;
    size[1] = (unsigned int)&_ld_ram_size1;
    strcpy(desc[1], "tcm");
    addr[2] = (unsigned int)&_ld_ram_addr2;
    size[2] = (unsigned int)&_ld_ram_size2;
    strcpy(desc[2], "wifi");
    return 0;
}
