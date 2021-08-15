#include <stdio.h>
#include <stdint.h>
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <bl602_spi.h>
#include <bl602_gpio.h>
#include <bl602_glb.h>
#include <bl602_dma.h>

#include "tp_flow.h"

typedef struct tp_spi_config {
	uint8_t port;
	uint8_t spi_mode;
	uint32_t spi_speed;
    uint8_t miso;
    uint8_t mosi;
    uint8_t clk;
    uint8_t cs;
    uint8_t irq_pin;
} tp_spi_config_t;

typedef struct tp_spi_state_list {
	struct spi_slave_item tpf_item;
	DMA_LLI_Ctrl_Type lli[3];
	struct utils_list_hdr item;
#define TP_LIST_TYPE_HOLE            0
#define TP_LIST_TYPE_NOMAL           1
	uint8_t type;
	uint8_t reserve[3];
} tp_spi_state_list_t;

#define TPF_FRAME_NUM_MAX  TP_WINDOWS_NUM_MAX

typedef struct tp_spi_slave_ctx {
	struct tp_flow_ctx ctx;
	SemaphoreHandle_t tx_sem;
	TaskHandle_t tp_task_handle;

	struct utils_list rx_free_list;

	int8_t dma_tx_chan;
	int8_t dma_rx_chan;
	const tp_spi_config_t *config;

	tp_spi_state_list_t *tx_prev;
	tp_spi_state_list_t *tx_now;
	tp_spi_state_list_t tx_list[TPF_FRAME_NUM_MAX];
	tp_spi_state_list_t tx_loop[2];
	uint32_t tx_hole;
	uint8_t tx_loop_idx;

	tp_spi_state_list_t *rx_prev;
	tp_spi_state_list_t *rx_now;
	tp_spi_state_list_t rx_list[TPF_FRAME_NUM_MAX];
	tp_spi_state_list_t rx_loop[2];
	uint32_t rx_hole;
	uint8_t rx_loop_idx;
} tp_spi_slave_ctx_t;

int tp_spi_slave_init(tp_spi_slave_ctx_t *ctx, const tp_spi_config_t *config);

int tp_spi_slave_rx_free_item_get(tp_spi_slave_ctx_t *ctx);

int tp_spi_slave_send_asyn(tp_spi_slave_ctx_t *ctx, const uint8_t *buf, uint16_t length);

/* send sync */
int tp_spi_slave_send(tp_spi_slave_ctx_t *ctx, const uint8_t *buf, uint16_t length);

int tp_spi_slave_receive(tp_spi_slave_ctx_t *ctx, uint8_t *buf, uint16_t length);

int tp_spi_callback_set(tp_spi_slave_ctx_t *ctx, pfn_rx_handler_t cb, void *arg);

int tp_spi_slave_run(tp_spi_slave_ctx_t *ctx);

