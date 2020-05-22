#include <bl602_romdriver.h>

#include "bl_flash.h"

static struct {
    uint32_t magic;
    SPI_Flash_Cfg_Type flashCfg;
} boot2_flashCfg; //XXX Dont change the name of varaible, since we refer this boot2_partition_table in linker script

int bl_flash_erase(uint32_t addr, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == boot2_flashCfg.flashCfg.mid) {
        return -1;
    }
    RomDriver_XIP_SFlash_Erase_With_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            len
    );
    return 0;
}

int bl_flash_write(uint32_t addr, uint8_t *src, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == boot2_flashCfg.flashCfg.mid) {
        return -1;
    }

    RomDriver_XIP_SFlash_Write_With_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            src,
            len
    );
    return 0;
}

int bl_flash_read(uint32_t addr, uint8_t *dst, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == boot2_flashCfg.flashCfg.mid) {
        return -1;
    }

    RomDriver_XIP_SFlash_Read_With_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            dst,
            len
    );
    return 0;
}

static void _dump_flash_config()
{
    extern uint8_t __boot2_flashCfg_src;
    printf("======= FlashCfg magiccode @%p, code 0x%08lX =======\r\n",
            &__boot2_flashCfg_src,
            boot2_flashCfg.magic
    );
    printf("mid \t\t0x%X\r\n", boot2_flashCfg.flashCfg.mid);
    printf("clkDelay \t0x%X\r\n", boot2_flashCfg.flashCfg.clkDelay);
    printf("clkInvert \t0x%X\r\n", boot2_flashCfg.flashCfg.clkInvert);
    printf("sector size\t%uKBytes\r\n", boot2_flashCfg.flashCfg.sectorSize);
    printf("page size\t%uBytes\r\n", boot2_flashCfg.flashCfg.pageSize);
    puts("---------------------------------------------------------------\r\n");
}

int bl_flash_config_update(void)
{
    _dump_flash_config();

    return 0;
}
