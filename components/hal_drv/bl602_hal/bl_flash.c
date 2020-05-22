#include <bl602_romdriver.h>

#include "bl_flash.h"

static SPI_Flash_Cfg_Type flashCfg;

int bl_flash_erase(uint32_t addr, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == flashCfg.mid) {
        return -1;
    }
    RomDriver_XIP_SFlash_Erase_With_Lock(
            &flashCfg,
            addr,
            len
    );
    return 0;
}

int bl_flash_write(uint32_t addr, uint8_t *src, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == flashCfg.mid) {
        return -1;
    }

    RomDriver_XIP_SFlash_Write_With_Lock(
            &flashCfg,
            addr,
            src,
            len
    );
    return 0;
}

int bl_flash_read(uint32_t addr, uint8_t *dst, int len)
{
    /*We assume mid zeor is illegal*/
    if (0 == flashCfg.mid) {
        return -1;
    }

    RomDriver_XIP_SFlash_Read_With_Lock(
            &flashCfg,
            addr,
            dst,
            len
    );
    return 0;
}

static void _dump_flash_config(void *config)
{
    SPI_Flash_Cfg_Type *_flashCfg = (SPI_Flash_Cfg_Type*)((uint8_t*)config + 4);

    printf("======= FlashCfg magiccode @%p, code 0x%08lX =======\r\n",
            _flashCfg,
            *(uint32_t*)config
    );
    printf("mid 0x%X\r\n", _flashCfg->mid);
    printf("clkDelay 0x%X\r\n", _flashCfg->clkDelay);
    printf("clkInvert 0x%X\r\n", _flashCfg->clkInvert);
}

int bl_flash_config_update(void *config)
{
    _dump_flash_config(config);
    memcpy(&flashCfg, ((uint8_t*)config) + 4, sizeof(flashCfg));

    return 0;
}
