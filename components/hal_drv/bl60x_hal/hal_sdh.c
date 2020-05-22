#include <stdio.h>
#include <hal/soc/sd.h>
#include "bl_sdh.h"
#include "hal_sdh.h"

static sd_card_t gSDCardInfo;

int32_t hal_sd_init(sd_dev_t *sd)
{
    static uint8_t sdh_init_called = 0;

    if (0 == sdh_init_called) {
#if USE_SD_DATABUF_WIDTH_4BIT
        SDH_Init(SDH_DATA_BUS_WIDTH_4BITS, &gSDCardInfo);
#else
        SDH_Init(SDH_DATA_BUS_WIDTH_1BIT, &gSDCardInfo);
#endif
        sdh_init_called = 1;
    }

    return 0;
}

int32_t hal_sd_stat_get(sd_dev_t *sd, hal_sd_stat *stat)
{
    *stat = SD_STAT_TRANSFER;
    return 0;
}

int32_t hal_sd_blks_read(sd_dev_t *sd, uint8_t *data, uint32_t blk_addr, uint32_t blks, uint32_t timeout)
{
    if (((uint32_t)data) & 0x7) {
        printf("[SDH] unaligned read address %p\r\n", data);
        return -1;
    }
    return SDH_ReadMultiBlocks(data, blk_addr, 512, blks);
}

int32_t hal_sd_blks_write(sd_dev_t *sd, uint8_t *data, uint32_t blk_addr, uint32_t blks, uint32_t timeout)
{
    if (((uint32_t)data) & 0x7) {
        printf("[SDH] unaligned write address %p\r\n", data);
        return -1;
    }
    return SDH_BlockWriteMultiBlocks(data, blk_addr, 512, blks);
}

int32_t hal_sd_info_get(sd_dev_t *sd, hal_sd_info_t *info)
{

    info->blk_nums = gSDCardInfo.blockCount;
    info->blk_size = gSDCardInfo.blockSize;

    return 0;
}

