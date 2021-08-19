#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include "tp_flow.h"
#include "tp_check.h"
#include "utils_log.h"

#define TP_FLOW_STATE_MAGIC      0x424C534B // "BLSK"
#define TP_FLOW_VERSION          0x0102

static int cmd_handler_throughput(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx, struct tp_flow_data *header)
{
    /*calculate the throughput*/
    return 0;
}

static const cmd_handler_t cmd_handlers[] =
{
    [SPI_DATA_HEADER_TYPE_CMD_QUEUE_SEND]           = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_THROUGHPUT]           = cmd_handler_throughput,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_SCAN]            = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_ENABLE]      = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_CONNECT]     = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_DISCONNECT]  = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_PS]          = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_DISABLE]     = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_AP_START]        = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_WIFI_AP_STOP]         = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_OTA_START]            = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_OTA_DATA_PIECE]       = NULL,
    [SPI_DATA_HEADER_TYPE_CMD_OTA_STOP]             = NULL,
};

static int _check_data(struct tp_flow_data *f_data)
{
	uint32_t check_sum = tp_check((uint8_t *)f_data, f_data->length + 4);
	if (check_sum == f_data->data_crc) {
		return 1;
	}
	blog_error("check error 0x%x recved 0x%x\r\n", check_sum, f_data->data_crc);
	log_buf((uint8_t*)f_data, 128);
    return 0;
}

static void _check_data_set(struct tp_flow_data *f_data)
{
    f_data->data_crc = tp_check((uint8_t *)f_data, f_data->length + 4);
    blog_info("length %ld, crc set 0x%08x\r\n", f_data->length, f_data->data_crc);
}

static int tp_rx_handle_cmd(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx, struct tp_flow_data *data)
{
    int release_now = 0;
    uint8_t cmd;

    cmd = 0;
    if (cmd < SPI_DATA_HEADER_TYPE_CMD_MAX) {
        /*legal cmds handler*/
    	if (cmd_handlers[cmd]) {
    		release_now = cmd_handlers[cmd](ctx, item_rx, data);
    	}
    } else {
        /*XXX should we ASSERT or warnning here?*/
    }

    static int recv_cnt = 0;
    recv_cnt += sizeof(*item_rx->tpf_f);

    //blog_buf((uint8_t *)item_rx->tpf_f, sizeof(*item_rx->tpf_f));
    blog_info("rx handle seq %ld\r\n", item_rx->tpf_f->status.seq);

    if (_check_data(&item_rx->tpf_f->tfd)) {
        // blog_info("[OK] check received,check sum %d\r\n", item_rx->tpf_f->tfd.data_crc);
    	if (ctx->rx_handler && data->length) {
    		ctx->rx_handler(ctx->rx_arg, data->payload_data, data->length);
    	}
    } else {
        //blog_info("[%04d] received, check sum %d\r\n", item_rx->tpf_f->tfd.length);
    }

    return release_now;
}

static void tp_rx_handle(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx)
{
    int should_item_release_now = 0;
    struct tp_flow_data *data;
    struct tp_status *status;

    if (item_rx->tpf_f == NULL) {
        // blog_info("[LOOP] frame\r\n");
    	blog_info("LOOP\r\n");
        goto __exit;
    }

    data = (struct tp_flow_data*) &item_rx->tpf_f->tfd;
    status = &item_rx->tpf_f->status;

    if (status->magic != (uint32_t)TP_FLOW_STATE_MAGIC) {
    	blog_info("rx magic %08x\r\n", item_rx->tpf_f->status.magic);
    	//FIXME: magic error!!!
    }

    // blog_info("tp_rx_handle seq %d\r\n", status->seq);

    /*handle data or CMD*/
    should_item_release_now = tp_rx_handle_cmd(ctx, item_rx, data);

__exit:
    item_rx->used_by_dma = 0;

    if (should_item_release_now) {
        tp_flow_item_push(ctx, item_rx);
    }

    return;
}

void tp_flow_item_windows_set(struct spi_slave_item *item, int windows)
{
    item->tpf_f->status.windows = windows;
}

int tp_flow_item_push(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx)
{
	BaseType_t xHigherPriorityTaskWoken;

	taskENTER_CRITICAL();
	item_rx->used_by_dma = 1;
	utils_list_push_back(&ctx->rx_item_list, &item_rx->item);
	taskENTER_CRITICAL();

	xSemaphoreGiveFromISR(ctx->run_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return 0;
}

struct spi_slave_item *tp_flow_tx_item_get(struct tp_flow_ctx *ctx)
{
    if (utils_list_is_empty(&ctx->tx_item_list)) {
        return NULL;
    }
    return utils_container_of(utils_list_pop_front(&ctx->tx_item_list), struct spi_slave_item, item);
}

int tp_flow_data_put(struct tp_flow_ctx *ctx, struct spi_slave_item *item, const uint8_t *buf, uint32_t length)
{
	int ret;

	taskENTER_CRITICAL();
	ret = ringbuf_put(&item->rbuf, buf, length);
	taskENTER_CRITICAL();

	if (!item->used_by_dma) {
		utils_list_extract(&ctx->tx_item_list, &item->item);
		//item->tpf_f->status.seq = ctx->seq;
	}
	item->tpf_f->tfd.length += ret;
	_check_data_set(&item->tpf_f->tfd);

	item->used_by_dma = 1;
	return ret;
}

int tp_flow_data_flush(struct tp_flow_ctx *ctx, struct spi_slave_item *item)
{
	uint16_t length;

	taskENTER_CRITICAL();
	length = ringbuf_get_size(&item->rbuf) - ringbuf_data_len(&item->rbuf);
	taskENTER_CRITICAL();

	if (length == 0) {
		return 0;
	}

	taskENTER_CRITICAL();
    while (length--) {
        ringbuf_putchar(&item->rbuf, 0x00);
    }
	taskENTER_CRITICAL();

	return 0;
}

int tp_flow_callback_set(struct tp_flow_ctx *ctx, pfn_rx_handler_t cb, void *arg)
{
	ctx->rx_handler = cb;
	ctx->rx_arg = arg;
	return 0;
}

int tp_flow_data_get(struct tp_flow_ctx *ctx, struct spi_slave_item *item, uint8_t *buf, uint32_t length)
{
	int ret;

	taskENTER_CRITICAL();
	ret = ringbuf_get(&item->rbuf, buf, length);
	taskENTER_CRITICAL();
	return ret;
}

int tp_flow_data_clear(struct tp_flow_ctx *ctx, struct spi_slave_item *item)
{
	taskENTER_CRITICAL();
	ringbuf_reset(&item->rbuf);
	item->tpf_f->tfd.length = 0;
	item->used_by_dma = 0;
//	utils_list_push_back(&ctx->tx_item_list, &item->item);
	taskENTER_CRITICAL();
	memset(&item->tpf_f->tfd, 0, sizeof(item->tpf_f->tfd));

	return 0;
}

int tp_flow_task(struct tp_flow_ctx *ctx)
{
    struct spi_slave_item *item;

	xSemaphoreTake(ctx->run_sem, (uint32_t)(0xffffffff));
    taskENTER_CRITICAL();
    while (!utils_list_is_empty(&ctx->rx_item_list)) {
        item = utils_container_of(utils_list_pick(&ctx->rx_item_list), struct spi_slave_item, item);
        if (!item->used_by_dma) {
            /*current item is Not used by DMA*/
            break;
        }
        /*current item is already used, so pop out from list*/
        item = utils_container_of(utils_list_pop_front(&ctx->rx_item_list), struct spi_slave_item, item);
        taskEXIT_CRITICAL();

        /*Handle item now, it may take sometime*/
        tp_rx_handle(ctx, item);

        taskENTER_CRITICAL();
    }
    taskEXIT_CRITICAL();
    return 0;
}

int tp_flow_item_init(struct tp_flow_ctx *ctx, struct spi_slave_item *tx, struct spi_slave_item *rx)
{
	tx->used_by_dma = 0;
	tx->tpf_f = calloc(sizeof(struct tpf_frame), 1);
	if (tx->tpf_f == NULL) {
		blog_info("item_init failed !\r\n");
		return -1;
	}
	tx->tpf_f->status.magic      = TP_FLOW_STATE_MAGIC;
	tx->tpf_f->status.version[0] = TP_FLOW_VERSION >> 8;
	tx->tpf_f->status.version[1] = TP_FLOW_VERSION & 0xff;
	tx->tpf_f->status.windows    = TP_WINDOWS_NUM_MAX;
	tx->tpf_f->status.seq        = 0;
	ringbuf_init(&tx->rbuf, tx->tpf_f->tfd.payload_data, sizeof(tx->tpf_f->tfd.payload_data));
	utils_list_push_back(&ctx->tx_item_list, &tx->item);

	rx->used_by_dma = 0;
	rx->tpf_f = calloc(sizeof(struct tpf_frame), 1);
	if (rx->tpf_f == NULL) {
		blog_info("item_init failed !\r\n");
		return -1;
	}
	return 0;
}

int tp_flow_init(struct tp_flow_ctx *ctx)
{
 
    ctx->run_sem = xSemaphoreCreateBinary();
    if (ctx->run_sem == NULL) {
    	return -1;
    }

	ctx->seq = 0;
	utils_list_init(&ctx->rx_item_list);
	utils_list_init(&ctx->tx_item_list);
	return 0;
}
