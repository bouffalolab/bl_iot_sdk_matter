#include <stdio.h>
#include <stdint.h>
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include <hosal_dma.h>
#include "tp_spi_slave.h"
#include "bl_gpio.h"

static inline tp_spi_state_list_t *_state_item_next(tp_spi_state_list_t *item)
{
	return utils_container_of((DMA_LLI_Ctrl_Type *)item->lli[2].nextLLI,
             struct tp_spi_state_list, lli[0]);
}

static inline void _state_item_add_tail(tp_spi_state_list_t *list, tp_spi_state_list_t *item)
{
	list->lli[2].nextLLI = (uint32_t)&item->lli[0];
}

static inline tp_spi_state_list_t *_state_item_trav(tp_spi_state_list_t *item, tp_spi_state_list_t *t)
{
    tp_spi_state_list_t *p = NULL;
    tp_spi_state_list_t *l = item;

    while (l != t) {
        p = l;
        l = _state_item_next(l);
    }
    return p;
}

static inline tp_spi_state_list_t *_item_free_get(tp_spi_slave_ctx_t *ctx)
{
	tp_spi_state_list_t *item;

    if (utils_list_is_empty(&ctx->rx_free_list)) {
        return NULL;
    }
    item = (tp_spi_state_list_t *)utils_container_of(utils_list_pick(&ctx->rx_free_list), struct tp_spi_state_list, item);
    if (item->tpf_item.used_by_dma != 0) {
        return NULL;
    }
    return (tp_spi_state_list_t *)utils_container_of(utils_list_pop_front(&ctx->rx_free_list), struct tp_spi_state_list, item);
}

static inline tp_spi_state_list_t *_state_free_item_get(tp_spi_slave_ctx_t *slave_ctx, tp_spi_state_list_t *now_loop, tp_spi_state_list_t *loop)
{
	tp_spi_state_list_t *h = NULL;
	tp_spi_state_list_t *t = NULL;
	tp_spi_state_list_t *elm = NULL;

	while (1) {
		elm = _item_free_get(slave_ctx);
		if (elm == NULL) {
			break;
		}
		if (!h) {
			h = elm;
			t = h;
			continue;
		}
		if (t) {
			_state_item_add_tail(t, elm);
			t = elm;
		}
	}
//	for (i = 0; i < TPF_FRAME_NUM_MAX; i++) {
//		if ((!h) && (!slave_ctx->rx_list[i].tpf_item.used_by_dma)) {
//			h = &slave_ctx->rx_list[i];
//			t = h;
//			continue;
//		}
//		if (t && (!slave_ctx->rx_list[i].tpf_item.used_by_dma)) {
//			_state_item_add_tail(t, &slave_ctx->rx_list[i]);
//			t = &slave_ctx->rx_list[i];
//		}
//	}
	if (t) {
		_state_item_add_tail(t, loop);
	}
	return h;
}

static inline uint32_t _state_item_seq_newest(tp_spi_slave_ctx_t *ctx)
{
    int i;
    uint32_t max = 0;
    for (i = 0; i < TPF_FRAME_NUM_MAX; i++) {
       if (ctx->rx_list[i].tpf_item.tpf_f->status.seq > max) {
           max = ctx->rx_list[i].tpf_item.tpf_f->status.seq;
       }
    }
    return max;
}

static __attribute__((section(".tcm_code"))) void _spi_dma_rx_irq(void *p_arg, uint32_t flag)
{
	tp_spi_slave_ctx_t *slave_ctx = (tp_spi_slave_ctx_t *)p_arg;
	tp_spi_state_list_t *next, *now_loop, *next_loop;

	if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
		printf("spi_dma_rx error\r\n");
		return;
	}

	/* now item is not black hole */
	tp_flow_item_push(&slave_ctx->ctx, &slave_ctx->rx_now->tpf_item);

//	if (slave_ctx->rx_now != &slave_ctx->rx_loop[slave_ctx->rx_loop_idx]) {
//		//FIXME:!!!
//	}

    now_loop  = &slave_ctx->rx_loop[slave_ctx->rx_loop_idx];
    next_loop = &slave_ctx->rx_loop[!slave_ctx->rx_loop_idx];

    next = _state_free_item_get(slave_ctx, now_loop, next_loop);
	if (next) {
		_state_item_add_tail(next_loop, next_loop);

        if (slave_ctx->rx_now->type == TP_LIST_TYPE_HOLE) {

		    _state_item_add_tail(now_loop, next);

        } else if (_state_item_next(slave_ctx->rx_now) != now_loop) {
            
            /* add lli to now_loop prev */
            tp_spi_state_list_t *l = _state_item_trav(slave_ctx->rx_now, now_loop);
            if (l) {
                _state_item_add_tail(l, next);
            	slave_ctx->rx_loop_idx = !slave_ctx->rx_loop_idx;
            }
        }
	}
	if (slave_ctx->rx_now->type != TP_LIST_TYPE_HOLE) {
		utils_list_push_back(&slave_ctx->rx_free_list, &slave_ctx->rx_now->item);
	}
    /* update prev and now lli  */
	slave_ctx->rx_prev = slave_ctx->rx_now;
	slave_ctx->rx_now = _state_item_next(slave_ctx->rx_now);
}

static __attribute__((section(".tcm_code"))) void _spi_dma_tx_irq(void *p_arg, uint32_t flag)
{
	tp_spi_slave_ctx_t *slave_ctx = (tp_spi_slave_ctx_t *)p_arg;
	BaseType_t xHigherPriorityTaskWoken;

	if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
		printf("spi_dma_tx error\r\n");
		return;
	}

	if (!slave_ctx->tx_now->tpf_item.used_by_dma) {
		return;
	}
	tp_flow_data_clear(&slave_ctx->ctx, &slave_ctx->tx_now->tpf_item);

	/* update prev and now lli  */
	slave_ctx->tx_prev = slave_ctx->tx_now;
	slave_ctx->tx_now = _state_item_next(slave_ctx->tx_now);

	xSemaphoreGiveFromISR(slave_ctx->tx_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static int _spi_hw_init(tp_spi_slave_ctx_t *ctx)
{
    SPI_CFG_Type spicfg;
    SPI_ClockCfg_Type clockcfg;
    SPI_FifoCfg_Type fifocfg;
    SPI_ID_Type spi_id; //TODO change SPI_ID_Type

    uint8_t clk_div;    
    spi_id = ctx->config->port;

#if 0
     *2  --->  20 Mhz
     *5  --->  8  Mhz
     *6  --->  6.66 Mhz
     *10 --->  4 Mhz
     * */
    clk_div = (uint8_t)(40000000 / ctx->config->spi_speed);
    GLB_Set_SPI_CLK(ENABLE, 0);
    clockcfg.startLen = clk_div;
    clockcfg.stopLen = clk_div;
    clockcfg.dataPhase0Len = clk_div;
    clockcfg.dataPhase1Len = clk_div;
    clockcfg.intervalLen = clk_div;
    SPI_ClockConfig(spi_id, &clockcfg);
#endif

    /* Set SPI clock */
    GLB_Set_SPI_CLK(ENABLE, 0);

    /* spi config */
    spicfg.deglitchEnable = DISABLE;
    spicfg.continuousEnable = ENABLE;
    spicfg.byteSequence = SPI_BYTE_INVERSE_BYTE0_FIRST,
    spicfg.bitSequence = SPI_BIT_INVERSE_MSB_FIRST,
    spicfg.frameSize = SPI_FRAME_SIZE_32;

    if (ctx->config->spi_mode == 0) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (ctx->config->spi_mode == 1) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (ctx->config->spi_mode == 2) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    } else if (ctx->config->spi_mode == 3) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    } else {
        blog_error("node support polar_phase \r\n");
        return -1;
    }

    SPI_Init(0, &spicfg);

    SPI_Disable(spi_id, SPI_WORK_MODE_SLAVE);

    SPI_IntMask(spi_id, SPI_INT_ALL, MASK);
    SPI_IntMask(spi_id, SPI_INT_FIFO_ERROR, UNMASK);

    /* fifo */
    fifocfg.txFifoThreshold = 1;
    fifocfg.rxFifoThreshold = 1;
    fifocfg.txFifoDmaEnable = ENABLE;
    fifocfg.rxFifoDmaEnable = ENABLE;
    SPI_FifoConfig(spi_id, &fifocfg);

    return 0;
}

static int _spi_lli_list_init(tp_spi_slave_ctx_t *ctx)
{
    uint32_t i = 0;
    uint32_t count;
    volatile struct DMA_Control_Reg dmactrl;

    count = TPF_FRAME_NUM_MAX;

    // tx hole
    dmactrl.SBSize = DMA_BURST_SIZE_1;
    dmactrl.DBSize = DMA_BURST_SIZE_1;
    dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
    dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
    dmactrl.Prot = 0;
    dmactrl.SLargerD = 0;
    dmactrl.I = 0;
    dmactrl.SI = DMA_MINC_DISABLE;
    dmactrl.DI = DMA_MINC_DISABLE;
    dmactrl.TransferSize = TP_FRAME_SIZE / 2 / 4;
    ctx->tx_loop[0].lli[0].srcDmaAddr  = (uint32_t)&ctx->tx_hole;
    ctx->tx_loop[0].lli[0].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
    ctx->tx_loop[0].lli[0].dmaCtrl     = dmactrl;
    ctx->tx_loop[0].lli[0].nextLLI     = (uint32_t)&ctx->tx_loop[0].lli[2];

    ctx->tx_loop[1].lli[0].srcDmaAddr  = (uint32_t)&ctx->tx_hole;
    ctx->tx_loop[1].lli[0].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
    ctx->tx_loop[1].lli[0].dmaCtrl     = dmactrl;
    ctx->tx_loop[1].lli[0].nextLLI     = (uint32_t)&ctx->tx_loop[1].lli[2];
    // ctx->tx_loop.lli[1] reserve
    dmactrl.I = 1;
    ctx->tx_loop[0].lli[2].srcDmaAddr  = (uint32_t)&ctx->tx_hole;
    ctx->tx_loop[0].lli[2].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
    ctx->tx_loop[0].lli[2].dmaCtrl     = dmactrl;
    ctx->tx_loop[0].lli[2].nextLLI     = (uint32_t)&ctx->tx_loop[0].lli[0];
    ctx->tx_loop[0].type = TP_LIST_TYPE_HOLE;

    ctx->tx_loop[1].lli[2].srcDmaAddr  = (uint32_t)&ctx->tx_hole;
    ctx->tx_loop[1].lli[2].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
    ctx->tx_loop[1].lli[2].dmaCtrl     = dmactrl;
    ctx->tx_loop[1].lli[2].nextLLI     = (uint32_t)&ctx->tx_loop[1].lli[0];
    ctx->tx_loop[1].type = TP_LIST_TYPE_HOLE;
    //tp_flow_item_init(&ctx->tx_loop.tpf_item);

    // rx hole
    dmactrl.SBSize = DMA_BURST_SIZE_1;
    dmactrl.DBSize = DMA_BURST_SIZE_1;
    dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
    dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
    dmactrl.Prot = 0;
    dmactrl.SLargerD = 0;
    dmactrl.I = 0;
    dmactrl.SI = DMA_MINC_DISABLE;
    dmactrl.DI = DMA_MINC_DISABLE;
    dmactrl.TransferSize = TP_FRAME_SIZE / 2 / 4;
    ctx->rx_loop[0].lli[0].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
    ctx->rx_loop[0].lli[0].destDmaAddr = (uint32_t)&ctx->rx_hole;
    ctx->rx_loop[0].lli[0].dmaCtrl     = dmactrl;
    ctx->rx_loop[0].lli[0].nextLLI     = (uint32_t)&ctx->rx_loop[0].lli[2];

    ctx->rx_loop[1].lli[0].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
    ctx->rx_loop[1].lli[0].destDmaAddr = (uint32_t)&ctx->rx_hole;
    ctx->rx_loop[1].lli[0].dmaCtrl     = dmactrl;
    ctx->rx_loop[1].lli[0].nextLLI     = (uint32_t)&ctx->rx_loop[1].lli[2];
    // ctx->rx_loop.lli[1] reserve

    dmactrl.I = 1;
    ctx->rx_loop[0].lli[2].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
    ctx->rx_loop[0].lli[2].destDmaAddr = (uint32_t)&ctx->rx_hole;
    ctx->rx_loop[0].lli[2].dmaCtrl     = dmactrl;
    ctx->rx_loop[0].lli[2].nextLLI     = (uint32_t)&ctx->rx_loop[0].lli[0];
    ctx->rx_loop[0].type = TP_LIST_TYPE_HOLE;

    ctx->rx_loop[1].lli[2].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
    ctx->rx_loop[1].lli[2].destDmaAddr = (uint32_t)&ctx->rx_hole;
    ctx->rx_loop[1].lli[2].dmaCtrl     = dmactrl;
    ctx->rx_loop[1].lli[2].nextLLI     = (uint32_t)&ctx->rx_loop[1].lli[0];
    ctx->rx_loop[1].type = TP_LIST_TYPE_HOLE;
    //tp_flow_item_init(&ctx->rx_loop.tpf_item);

    for (i = 0; i < count; i++) {
        ctx->tx_list[i].type = TP_LIST_TYPE_NOMAL;
        ctx->rx_list[i].type = TP_LIST_TYPE_NOMAL;
        tp_flow_item_init(&ctx->ctx, &ctx->tx_list[i].tpf_item, &ctx->rx_list[i].tpf_item);
        //tp_flow_item_init(&ctx->rx_list[i].tpf_item);

        dmactrl.SBSize = DMA_BURST_SIZE_1;
        dmactrl.DBSize = DMA_BURST_SIZE_1;
        dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.Prot = 0;
        dmactrl.SLargerD = 0;
        dmactrl.I = 0;
        dmactrl.SI = DMA_MINC_ENABLE;
        dmactrl.DI = DMA_MINC_DISABLE;

        /* tx lli config */
        dmactrl.TransferSize = sizeof(struct tpf_frame) / 4;
        ctx->tx_list[i].lli[0].srcDmaAddr  = (uint32_t)ctx->tx_list[i].tpf_item.tpf_f->_dummy; // last s_status
        ctx->tx_list[i].lli[0].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
        ctx->tx_list[i].lli[0].dmaCtrl     = dmactrl;
        ctx->tx_list[i].lli[0].nextLLI     = (uint32_t)&ctx->tx_list[i].lli[2];

#if 0
        dmactrl.TransferSize = sizeof(struct tp_flow_data) / 4;
        ctx->tx_list[i].lli[1].srcDmaAddr  = (uint32_t)&ctx->tx_list[i].tpf_item.tpf_f->tfd;
        ctx->tx_list[i].lli[1].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
        ctx->tx_list[i].lli[1].dmaCtrl     = dmactrl;
        ctx->tx_list[i].lli[1].nextLLI     = (uint32_t)&ctx->tx_list[i].lli[2];
#endif
        dmactrl.I = 1;
        dmactrl.TransferSize = sizeof(struct tp_status) / 4;
        ctx->tx_list[i].lli[2].srcDmaAddr  = (uint32_t)&ctx->rx_list[i].tpf_item.tpf_f->status; // now m_status
        ctx->tx_list[i].lli[2].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
        ctx->tx_list[i].lli[2].dmaCtrl     = dmactrl;
        if (i != count - 1) {
            ctx->tx_list[i].lli[2].nextLLI = (uint32_t)&ctx->tx_list[i + 1].lli[0];
        } else {
            ctx->tx_list[i].lli[2].nextLLI = (uint32_t)&ctx->tx_list[0].lli[0];
        }

        dmactrl.SBSize = DMA_BURST_SIZE_1;
        dmactrl.DBSize = DMA_BURST_SIZE_1;
        dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.Prot = 0;
        dmactrl.SLargerD = 0;
        dmactrl.I = 0;
        dmactrl.SI = DMA_MINC_DISABLE;
        dmactrl.DI = DMA_MINC_ENABLE;

        /* rx lli config */
        dmactrl.TransferSize = sizeof(struct tpf_frame) / 4;
        ctx->rx_list[i].lli[0].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
        ctx->rx_list[i].lli[0].destDmaAddr = (uint32_t)ctx->rx_list[i].tpf_item.tpf_f->_dummy; // now m_status
        ctx->rx_list[i].lli[0].dmaCtrl     = dmactrl;
        ctx->rx_list[i].lli[0].nextLLI     = (uint32_t)&ctx->rx_list[i].lli[2];

#if 0
        dmactrl.TransferSize = sizeof(struct tp_flow_data) / 4;
        ctx->rx_list[i].lli[1].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
        ctx->rx_list[i].lli[1].destDmaAddr = (uint32_t)&ctx->rx_list[i].tpf_item.tpf_f->tfd;
        ctx->rx_list[i].lli[1].dmaCtrl     = dmactrl;
        ctx->rx_list[i].lli[1].nextLLI     = (uint32_t)&ctx->rx_list[i].lli[2];
#endif
        dmactrl.I = 1;
        dmactrl.TransferSize = sizeof(struct tp_status) / 4;
        ctx->rx_list[i].lli[2].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
        ctx->rx_list[i].lli[2].destDmaAddr = (uint32_t)&ctx->tx_list[i].tpf_item.tpf_f->status; // last s_status
        ctx->rx_list[i].lli[2].dmaCtrl     = dmactrl;
        if (i != count - 1) {
            ctx->rx_list[i].lli[2].nextLLI = (uint32_t)&ctx->rx_list[i + 1].lli[0];
        } else {
            ctx->rx_list[i].lli[2].nextLLI = (uint32_t)&ctx->rx_loop[0].lli[0];
        }
    }
    ctx->rx_now = &ctx->rx_list[0];
    ctx->tx_now = &ctx->tx_list[0];

    return 0;
}

static int _spi_dma_init(tp_spi_slave_ctx_t *ctx)
{
    int ret;
    DMA_LLI_Cfg_Type txllicfg;
    DMA_LLI_Cfg_Type rxllicfg;

    ctx->dma_rx_chan = hosal_dma_chan_request(0);
    if (ctx->dma_rx_chan < 0) {
        return -1;
    }
    ctx->dma_tx_chan = hosal_dma_chan_request(0);
    if (ctx->dma_tx_chan < 0) {
        return -1;
    }

    txllicfg.dir = DMA_TRNS_M2P;
    txllicfg.srcPeriph = DMA_REQ_NONE;
    txllicfg.dstPeriph = DMA_REQ_SPI_TX;
    rxllicfg.dir = DMA_TRNS_P2M;
    rxllicfg.srcPeriph = DMA_REQ_SPI_RX;
    rxllicfg.dstPeriph = DMA_REQ_NONE;
    ret = _spi_lli_list_init(ctx);
    if (ret < 0) {
        blog_error("init lli failed. \r\n");
        return -1;
    }

	DMA_LLI_Init(ctx->dma_rx_chan, &rxllicfg);
	DMA_LLI_Update(ctx->dma_rx_chan,(uint32_t)&ctx->rx_list[0].lli[0]);
	hosal_dma_irq_callback_set(ctx->dma_rx_chan, _spi_dma_rx_irq, (void *)ctx);
	hosal_dma_chan_start(ctx->dma_rx_chan);

	DMA_LLI_Init(ctx->dma_tx_chan, &txllicfg);
	DMA_LLI_Update(ctx->dma_tx_chan, (uint32_t)&ctx->tx_list[0].lli[0]);
	hosal_dma_irq_callback_set(ctx->dma_tx_chan, _spi_dma_tx_irq, (void *)ctx);
	hosal_dma_chan_start(ctx->dma_tx_chan);

    SPI_Enable(ctx->config->port, SPI_WORK_MODE_SLAVE);

    return 0;
}

static void _spi_gpio_init(const tp_spi_config_t *config)
{
    uint8_t gpiopins[4];

    gpiopins[0] = config->cs;  //pin cs
    gpiopins[1] = config->clk;
    gpiopins[2] = config->mosi;
    gpiopins[3] = config->miso;

    GLB_GPIO_Func_Init(GPIO_FUN_SPI, gpiopins, sizeof(gpiopins)/sizeof(gpiopins[0]));
    GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_SLAVE);
}

static int _slave_send_by_notify(tp_spi_slave_ctx_t *ctx, const uint8_t *buf, uint16_t length)
{
	int ret;
	ret = tp_spi_slave_send_asyn(ctx, buf, length);
	if (ret == 0) {
		return 0;
	}
	bl_gpio_output_set(ctx->config->irq_pin, 0);
	bl_gpio_output_set(ctx->config->irq_pin, 1);
	return ret;
}

static void _throughput_task(void *p_arg)
{
	tp_spi_slave_ctx_t *ctx = (tp_spi_slave_ctx_t *)p_arg;
 
    while (1) {
        tp_spi_slave_run(ctx);
    }
}

int tp_spi_slave_rx_free_item_get(tp_spi_slave_ctx_t *ctx)
{
	int i, free_cnt = 0;
	for (i = 0; i < TPF_FRAME_NUM_MAX; i++) {
		if (!ctx->rx_list[i].tpf_item.used_by_dma) {
			free_cnt++;
		}
	}
	return free_cnt;
}

int tp_spi_slave_send_asyn(tp_spi_slave_ctx_t *ctx, const uint8_t *buf, uint16_t length)
{
	uint32_t len = 0;
	struct spi_slave_item *tx = NULL;
	struct tp_spi_state_list *tx_list;

	do {

		while (1) {
			tx = tp_flow_tx_item_get(&ctx->ctx);
			if (!tx) {
				xSemaphoreTake(ctx->tx_sem, (uint32_t)(0xffffffff));
			} else {
				break;
			}
		}

		tx_list = (struct tp_spi_state_list *)utils_container_of(tx, struct tp_spi_state_list, tpf_item);

		//hosal_dma_chan_stop(ctx->dma_tx_chan);
		//SPI_ClrTxFifo(ctx->config->port);
		len += tp_flow_data_put(&ctx->ctx, tx, buf + len, length - len);
		tp_flow_item_windows_set(tx, tp_spi_slave_rx_free_item_get(ctx));
		tp_flow_data_flush(&ctx->ctx, tx);
		//DMA_LLI_Update(ctx->dma_tx_chan,(uint32_t)&tx_list->lli[0]);
		//hosal_dma_chan_start(ctx->dma_tx_chan);

	} while (len != length);

	return len;
}

/* send sync */
int tp_spi_slave_send(tp_spi_slave_ctx_t *ctx, const uint8_t *buf, uint16_t length)
{
	return _slave_send_by_notify(ctx, buf, length);
}

int tp_spi_callback_set(tp_spi_slave_ctx_t *ctx, pfn_rx_handler_t cb, void *arg)
{
	return tp_flow_callback_set(&ctx->ctx, cb, arg);
}

int tp_spi_slave_run(tp_spi_slave_ctx_t *ctx)
{
	if (tp_spi_slave_rx_free_item_get(ctx) == TPF_FRAME_NUM_MAX) {
//		bl_gpio_output_set(ctx->config->irq_pin, 0);
//		bl_gpio_output_set(ctx->config->irq_pin, 1);
	}
	return tp_flow_task(&ctx->ctx);
}

int tp_spi_slave_init(tp_spi_slave_ctx_t *ctx, const tp_spi_config_t *config)
{
	memset(ctx, 0, sizeof(tp_spi_slave_ctx_t));

    ctx->config = config;
    ctx->dma_tx_chan = -1;
    ctx->dma_rx_chan = -1;
    ctx->rx_loop_idx = 0;
    ctx->tx_loop_idx = 0;

    tp_flow_init(&ctx->ctx);
    utils_list_init(&ctx->rx_free_list);

    bl_gpio_enable_output(config->irq_pin, 1, 0);
    bl_gpio_output_set(ctx->config->irq_pin, 1);

    ctx->tx_sem = xSemaphoreCreateBinary();
    if (ctx->tx_sem == NULL) {
    	return -1;
    }

    _spi_gpio_init(config);
    _spi_hw_init(ctx);
    _spi_dma_init(ctx);

    xTaskCreate(_throughput_task, (char*)"throughput", 1024 / 4, ctx, 15, &ctx->tp_task_handle);

    return 0;
}
