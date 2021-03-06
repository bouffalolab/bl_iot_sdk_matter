#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <bl702_dma.h>

#include <FreeRTOS.h>
#include <task.h>

#include "bl_dma.h"
#include "bl_irq.h"

#include <blog.h>

#define DMA_CHANNEL_OFFSET              0x100
#define DMA_Get_Channel(ch)             (DMA_BASE+DMA_CHANNEL_OFFSET+(ch)*0x100)

/*please also change NVIC_SetPriority of DMA channel*/
#define DMA_DEFAULT_CHANNEL_COPY        (DMA_CH0)

static struct utils_list dma_copy_list;

int bl_dma_int_clear(int ch)
{
    uint32_t tmpVal;
    uint32_t intClr;
    /* Get DMA register */
    uint32_t DMAChs = DMA_BASE;

    tmpVal = BL_RD_REG(DMAChs, DMA_INTTCSTATUS);
    if((BL_GET_REG_BITS_VAL(tmpVal, DMA_INTTCSTATUS) & (1 << ch)) != 0) {
        /* Clear interrupt */
        tmpVal = BL_RD_REG(DMAChs, DMA_INTTCCLEAR);
        intClr = BL_GET_REG_BITS_VAL(tmpVal, DMA_INTTCCLEAR);
        intClr |= (1 << ch);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, DMA_INTTCCLEAR, intClr);
        BL_WR_REG(DMAChs, DMA_INTTCCLEAR, tmpVal);
    }

    tmpVal = BL_RD_REG(DMAChs, DMA_INTERRORSTATUS);
    if((BL_GET_REG_BITS_VAL(tmpVal, DMA_INTERRORSTATUS) & (1 << ch)) != 0) {
        /*Clear interrupt */
        tmpVal = BL_RD_REG(DMAChs, DMA_INTERRCLR);
        intClr = BL_GET_REG_BITS_VAL(tmpVal, DMA_INTERRCLR);
        intClr |= (1 << ch);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, DMA_INTERRCLR, intClr);
        BL_WR_REG(DMAChs, DMA_INTERRCLR, tmpVal);
    }

    return 0;
}

void bl_dma_update_memsrc(uint8_t ch, uint32_t src, uint32_t len)
{
    uint32_t tmpVal;
    /* Get channel register */
    uint32_t DMAChs = DMA_Get_Channel(ch);

    /* Check the parameters */
    CHECK_PARAM(IS_DMA_CHAN_TYPE(ch));

    /* config channel config*/
    BL_WR_REG(DMAChs, DMA_SRCADDR, src);

    tmpVal = BL_RD_REG(DMAChs, DMA_CONTROL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, DMA_TRANSFERSIZE, len);
    BL_WR_REG(DMAChs, DMA_CONTROL, tmpVal);
}

void bl_dma_update_memdst(uint8_t ch, uint32_t dst, uint32_t len)
{
    uint32_t tmpVal;
    /* Get channel register */
    uint32_t DMAChs = DMA_Get_Channel(ch);

    /* Check the parameters */
    CHECK_PARAM(IS_DMA_CHAN_TYPE(ch));

    /* config channel config*/
    BL_WR_REG(DMAChs, DMA_DSTADDR, dst);

    tmpVal = BL_RD_REG(DMAChs, DMA_CONTROL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, DMA_TRANSFERSIZE, len);
    BL_WR_REG(DMAChs, DMA_CONTROL, tmpVal);
}

static void inline _dma_copy_trigger(struct bl_dma_item *first)
{
    //blog_info("------ DMA Trigger\r\n");
    DMA_LLI_Update(DMA_DEFAULT_CHANNEL_COPY, (uint32_t)&(first->src));
    DMA_Channel_Enable(DMA_DEFAULT_CHANNEL_COPY);
}

void bl_dma_copy(struct bl_dma_item *first)
{
    taskENTER_CRITICAL();
    if (utils_list_is_empty(&dma_copy_list)) {
        _dma_copy_trigger(first);
    }
    utils_list_push_back(&dma_copy_list, &(first->item));
    taskEXIT_CRITICAL();
}

void bl_dma_IRQHandler(void)
{
    struct bl_dma_item *first;

    //blog_info("------ Clear DMA now\r\n");
    bl_dma_int_clear(DMA_DEFAULT_CHANNEL_COPY);
    first = (struct bl_dma_item*)utils_list_pop_front(&dma_copy_list);
    if (NULL == first) {
        blog_info("[INT] ASSERT here for empty chain\r\n");
        while (1) {
        }
    }
    if (first->cb) {
        first->cb(first->arg);
    }

    /*prepare NEXT Transfer*/
    first = (struct bl_dma_item*)utils_list_pick(&dma_copy_list);
    if (first) {
        _dma_copy_trigger(first);
    }
}

void bl_dma_init(void)
{
    //FIXME use DMA_CH4 as channel copy
    DMA_Chan_Type dmaCh = DMA_DEFAULT_CHANNEL_COPY;
    DMA_LLI_Cfg_Type lliCfg =
    {
        DMA_TRNS_M2M,
        DMA_REQ_NONE,
        DMA_REQ_NONE,
    };
    utils_list_init(&dma_copy_list);

    DMA_Enable();
    DMA_IntMask(dmaCh, DMA_INT_ALL, MASK);
    DMA_IntMask(dmaCh, DMA_INT_TCOMPLETED, UNMASK);
    DMA_IntMask(dmaCh, DMA_INT_ERR, UNMASK);
    DMA_LLI_Init(dmaCh, &lliCfg);
    bl_irq_register(DMA_ALL_IRQn, bl_dma_IRQHandler);
    bl_irq_enable(DMA_ALL_IRQn);
}

static void _cb_cmd(void *arg)
{
    struct bl_dma_item *first;

    first = (struct bl_dma_item*)arg;
    blog_info("[DMA] [TEST] Callback is working, arg is %p\r\n", arg);
    first->arg = NULL;
}

static void _dma_test_case1(void)
{
    struct bl_dma_item *first;
    uint32_t *src;
    uint32_t *dst;
    const int size = 68;

    first = pvPortMalloc(sizeof(struct bl_dma_item));
    src = pvPortMalloc(size);
    dst = pvPortMalloc(size);

    blog_info("[TEST] [DMA] first %p, src %p, dst %p\r\n",
            first,
            src,
            dst
    );

    memset(first, 0, sizeof(struct bl_dma_item));
    memset(src, 1, size);
    memset(dst, 0xFF, size);
    first->src = (uint32_t)src;
    first->dst = (uint32_t)dst;
    first->next = 0;
    first->ctrl =  BL_DMA_ITEM_CTRL_MAGIC_IRQ | (size >> 2);
    first->cb = _cb_cmd;
    first->arg = first;

    bl_dma_copy(first);

    /*Arg will be set in callback*/
    while (first->arg) {
        vTaskDelay(2);
    }

    vPortFree((void*)first->src);
    vPortFree((void*)first->dst);
    vPortFree(first);
}

void bl_dma_test(void)
{
    _dma_test_case1();
}
