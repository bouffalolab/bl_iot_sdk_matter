#include <stdio.h>
#include <stdint.h>
#include <utils_list.h>
#include "ringbuf.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

enum SPI_DATA_HEADER_TYPE_E {
    SPI_DATA_HEADER_TYPE_CMD = 0,
    SPI_DATA_HEADER_TYPE_DAT,
    SPI_DATA_HEADER_TYPE_EVT,
};

enum SPI_DATA_HEADER_SUBTYPE_CMD_E {
    SPI_DATA_HEADER_TYPE_CMD_QUEUE_SEND = 0,

    /*Throughput test*/
    SPI_DATA_HEADER_TYPE_CMD_THROUGHPUT,

    /*General cmd*/
    SPI_DATA_HEADER_TYPE_CMD_WIFI_SCAN,
    /*STA related cmd*/
    SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_ENABLE,
    SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_CONNECT,
    SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_DISCONNECT,
    SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_PS,
    SPI_DATA_HEADER_TYPE_CMD_WIFI_STA_DISABLE,

    /*AP related cmd*/
    SPI_DATA_HEADER_TYPE_CMD_WIFI_AP_START,
    SPI_DATA_HEADER_TYPE_CMD_WIFI_AP_STOP,

    /*OTA related cmd*/
    SPI_DATA_HEADER_TYPE_CMD_OTA_START,
    SPI_DATA_HEADER_TYPE_CMD_OTA_DATA_PIECE,
    SPI_DATA_HEADER_TYPE_CMD_OTA_STOP,
    
    /*Warning must kept in the tail*/
    SPI_DATA_HEADER_TYPE_CMD_MAX,
};

typedef void (*pfn_rx_handler_t)(void *arg, void *buf, size_t length);

struct tp_status {          // 16 byte
    uint32_t    magic;      //"BLSK"
    uint8_t     version[2]; //bigger number, newer version
    uint8_t     windows;
    uint8_t     reserved[5];
    uint32_t    seq;        //updated by DMA link on every success received frame
};

#define TP_FRAME_SIZE       4096
#define TP_WINDOWS_NUM_MAX  4

struct tp_flow_data {
	uint32_t   length;
//	union {
//		uint16_t ack:1;
//		uint16_t cmd:16;
//	};
    uint8_t    payload_data[TP_FRAME_SIZE - 8 - 16 - 2*sizeof(struct tp_status)];
    uint32_t   data_crc;
};

/* 4096 - 16 bytes */
struct tpf_frame {
    uint32_t _dummy[4];
    struct tp_status status;
    struct tp_flow_data tfd;
//    struct status s_status;
};

struct spi_slave_item {
    struct utils_list_hdr item;
    uint32_t    used_by_dma;//set by DMA when current item is filled by data
    struct ringbuf rbuf;

    struct tpf_frame *tpf_f;
};

struct tp_flow_ctx {
    struct tp_status common_status;
    struct tp_status loop_status;
    uint32_t    seq;//update by DMA link after data is transfered
    struct {
        uint32_t    used_by_dma; //source for set used_by_dma in spi
    } rx_dummy_pool;
    struct utils_list rx_item_list;
    struct utils_list tx_item_list;
	SemaphoreHandle_t run_sem;
    pfn_rx_handler_t rx_handler;
    void *rx_arg;
};

typedef int (*cmd_handler_t)(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx, struct tp_flow_data *frame);

int tp_flow_init(struct tp_flow_ctx *ctx);

int tp_flow_item_init(struct tp_flow_ctx *ctx, struct spi_slave_item *tx, struct spi_slave_item *rx);

struct spi_slave_item *tp_flow_tx_item_get(struct tp_flow_ctx *ctx);

int tp_flow_task(struct tp_flow_ctx *ctx);

int tp_flow_item_push(struct tp_flow_ctx *ctx, struct spi_slave_item *item_rx);

void tp_flow_item_windows_set(struct spi_slave_item *item, int windows);

int tp_flow_data_put(struct tp_flow_ctx *ctx, struct spi_slave_item *item, const uint8_t *buf, uint32_t length);

int tp_flow_data_flush(struct tp_flow_ctx *ctx, struct spi_slave_item *item);

int tp_flow_callback_set(struct tp_flow_ctx *ctx, pfn_rx_handler_t cb, void *arg);

int tp_flow_data_clear(struct tp_flow_ctx *ctx, struct spi_slave_item *item);

