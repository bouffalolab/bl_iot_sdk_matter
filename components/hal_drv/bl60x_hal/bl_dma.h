#ifndef __BL_DMA_H__
#define __BL_DMA_H__

#include <utils_list.h>

#define BL_DMA_ITEM_CTRL_MAGIC_IRQ           (0x8c49b000)
#define BL_DMA_ITEM_CTRL_MAGIC_NOIRQ         (0x0c49b000)
#define BL_DMA_ITEM_CTRL_MAGIC_BYTE_IRQ      (0x8c01b000)
#define BL_DMA_ITEM_CTRL_MAGIC_BYTE_NOIRQ    (0x0c01b000)
#define BL_DMA_ITEM_CTRL_MAGIC_IRQ_CLR       (0x7FFFFFFF)
#define BL_DMA_ITEM_CTRL_MAGIC_IRQ_SET       (0x80000000)

struct bl_dma_item {
    struct utils_list_hdr item;
    void (*cb)(void *arg);
    void *arg;
    /*the following fields is for hardware access, strict aligment and memory is required*/
    uint32_t src;
    uint32_t dst;
    uint32_t next;
    uint32_t ctrl;
};


int bl_dma_int_clear(int ch);
void bl_dma_update_memsrc(uint8_t ch, uint32_t src, uint32_t len);
void bl_dma_update_memdst(uint8_t ch, uint32_t dst, uint32_t len);
void bl_dma_copy(struct bl_dma_item *item);
void bl_dma_init(void);

#endif
