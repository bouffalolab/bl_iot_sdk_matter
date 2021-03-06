/**
 ****************************************************************************************
 *
 * @file bl_tx.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <netif/etharp.h>

#include "bl_tx.h"
#include "bl_irqs.h"
#include "bl_utils.h"
#include "os_hal.h"


struct utils_list tx_list_bl;

int internel_cal_size_tx_hdr = sizeof(struct bl_txhdr);

TaskHandle_t taskHandle_output = NULL;
extern struct bl_hw wifi_hw;
static struct bl_hw *bl_hw_static = &wifi_hw;

void bl_tx_push(struct bl_hw *bl_hw, struct bl_txhdr *txhdr)
{
    volatile struct hostdesc *host;
    uint32_t* p = txhdr->p;

    host = &(ipc_host_txdesc_get(bl_hw->ipc_env)->host);
    ASSERT_ERR(host);//TODO protect when host is NULL

    {
        u8 *src, *dst;
        int i;
        dst = (typeof(dst))host;
        src = (typeof(src))&txhdr->host;
        for (i = 0; i < sizeof(*host) / sizeof(*src); i++) {
            *dst++ = *src++;
        }
    }

    ipc_host_txdesc_push(bl_hw->ipc_env, p);
    bl_hw->stats.cfm_balance++;
}

#define TXHDR_HODLER_LEN (8)
#define TXHDR_HODLER_MSK (0x7)
struct bl_txhdr *(txhdr_hodler[8]);
uint32_t txhdr_pos_r = 0;
uint32_t txhdr_pos_w = 0;

void bl_tx_resend()
{
    taskENTER_CRITICAL();
    while (txhdr_pos_r != txhdr_pos_w) {
        if (NULL == ipc_host_txdesc_get(bl_hw_static->ipc_env)) {
            break;
        }
        txhdr_hodler[txhdr_pos_r & TXHDR_HODLER_MSK]->status.value = 0;
#if 0
        printf("Push back %p\r\n", txhdr_hodler[txhdr_pos_r & TXHDR_HODLER_MSK]);
#endif
        bl_tx_push(bl_hw_static, txhdr_hodler[txhdr_pos_r & TXHDR_HODLER_MSK]);
        txhdr_pos_r++;
    }
    taskEXIT_CRITICAL();
}

void bl_tx_try_flush()
{
    struct bl_txhdr *txhdr;

    taskENTER_CRITICAL();
    while (ipc_host_txdesc_get(bl_hw_static->ipc_env)) {
        txhdr = (struct bl_txhdr*)utils_list_pop_front(&tx_list_bl);
        if (NULL == txhdr) {
            break;
        }
        bl_tx_push(bl_hw_static, txhdr);
    }
    taskEXIT_CRITICAL();
}

static bool tx_retry_needed(struct bl_txhdr *txhdr)
{
#if TX_SW_RETRY_POLICY_ENABLE
    const uint32_t round_limit = TX_SW_RETRY_ROUND_LIMIT; // to kill a warning

    if (txhdr->sw_retry_stat.retrying) {
        if((uint32_t)os_tick_get() - txhdr->sw_retry_stat.first_retry_tick_ms >= TX_SW_RETRY_TIMEOUT_MS) {
            return false;
        }
    } else {
        txhdr->sw_retry_stat.retrying = 1;
        txhdr->sw_retry_stat.retry_times = 0;
        txhdr->sw_retry_stat.first_retry_tick_ms = os_tick_get();
    }
    if (txhdr->sw_retry_stat.retry_times >= round_limit) {
        return false;
    }
    txhdr->sw_retry_stat.retry_times++;
#endif
    return true;
}

int bl_txdatacfm(void *pthis, void *host_id)
{
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
int update_tx_pbuf_free_cnt_to_scratch_reg(void);
#endif
#define RETRY_LIMIT_REACHED_BIT (1 << 16)
    struct pbuf *p = (struct pbuf*)host_id;
    struct bl_txhdr *txhdr;
    union bl_hw_txstatus bl_txst;

    txhdr = (struct bl_txhdr*)(((uint32_t)p->payload) + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload));


    /* Read status in the TX control header */
    bl_txst = txhdr->status;

    if (bl_txst.value == 0) {
        return -1;
    }
    if (bl_txst.value & RETRY_LIMIT_REACHED_BIT) {
#if 0
        printf("TX STATUS %08lX", bl_txst.value);
        printf(" Retry reached %p:%lu:%lu", txhdr, txhdr_pos_r, txhdr_pos_w);
#endif
        /*we don't pbuf_free here, because we will resend this packet*/
        if (((txhdr_pos_w + 1) & TXHDR_HODLER_MSK) != (txhdr_pos_r & TXHDR_HODLER_MSK)) {
            if (tx_retry_needed(txhdr)) {
                puts(" push back\r\n");
                txhdr_hodler[txhdr_pos_w & TXHDR_HODLER_MSK] = txhdr;
                txhdr_pos_w++;
            } else {
                pbuf_free(p);
            }
        } else {
#if 1
            puts(" NOT push back when no mem\r\n");
#endif
            pbuf_free(p);
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
            update_tx_pbuf_free_cnt_to_scratch_reg();
#endif
        }
    } else {
        pbuf_free(p);
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
        update_tx_pbuf_free_cnt_to_scratch_reg();
#endif
    }

    return 0;
}

void bl_tx_notify()
{
    //TODO static alloc taskHandle_output, no if else anymore
    if (taskHandle_output) {
        xTaskNotifyGive(taskHandle_output);
    }

    return;
}

#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta, bool from_local)
#else
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta)
#endif
{
    struct bl_txhdr *txhdr;
    struct pbuf *q;
    uint32_t *link_header;
    uint32_t *eth_header;
    struct ethhdr *eth;
    struct hostdesc *host;
    int loop = 0;
    u8 tid;
    uint16_t packet_len;

    if (NULL == bl_hw || 0 == (NETIF_FLAG_LINK_UP & netif->flags)) {//TODO avoid call output when Wi-Fi is not ready
        printf("[TX] wifi is down, return now\r\n");
        return ERR_CONN;
    }

    if (0 == taskHandle_output) {
        taskHandle_output = xTaskGetCurrentTaskHandle();
    }

    bl_hw_static = bl_hw;
    eth_header = (uint32_t*)p->payload;
    packet_len = p->tot_len;

    /*Make room in the header for tx*/
    if (pbuf_header(p, PBUF_LINK_ENCAPSULATION_HLEN)) {
        printf("[TX] Reserve room failed for header\r\n");
        return ERR_IF;
    }
    /*Use aligned link_header*/
    link_header = (uint32_t*)(((uint32_t)p->payload) + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload));
    if (16 + sizeof(struct bl_txhdr) + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload) > PBUF_LINK_ENCAPSULATION_HLEN) {
        printf("link_header size is %ld vs header %u\r\n",
                sizeof(struct bl_txhdr) + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload),
                PBUF_LINK_ENCAPSULATION_HLEN
        );
    }

    tid = 0;//XXX set TID to 0 for quick test purpose

    eth = (struct ethhdr *)(eth_header);
    txhdr = (struct bl_txhdr *)(link_header);
    memset(txhdr, 0, sizeof(struct bl_txhdr));
    host = &(txhdr->host);

    txhdr->p         = (uint32_t*)p;//XXX pattention to this filed
    // Fill-in the descriptor
    memcpy(&host->eth_dest_addr, eth->h_dest, ETH_ALEN);
    memcpy(&host->eth_src_addr, eth->h_source, ETH_ALEN);
    host->pbuf_addr = (uint32_t)p;
    host->ethertype = eth->h_proto;
    host->tid = tid;
    host->vif_idx = (is_sta ? bl_hw->vif_index_sta : bl_hw->vif_index_ap);
    host->flags = 0;
    host->packet_len = packet_len - sizeof(*eth);
    if (is_sta) {
        host->staid = bl_hw->sta_idx;
    } else {
        host->staid = bl_utils_idx_lookup(bl_hw, eth->h_dest);
    }

    loop = 0;
    for (q = p; q != NULL; q = q->next) {
        if (0 == loop) {
            /*The first pbuf*/
            host->pbuf_chained_ptr[loop] = (uint32_t)(((uint8_t*)q->payload) + sizeof(*eth)) + PBUF_LINK_ENCAPSULATION_HLEN;//eth header is skipped in the header
            host->pbuf_chained_len[loop] = q->len - sizeof(*eth) - PBUF_LINK_ENCAPSULATION_HLEN;//eth header is skipped in the header
        } else {
            /*Chained pbuf after*/
            if (loop >= 4) {
                /*exceed the limit for pbuf chained*/
                printf("[TX] [PBUF] Please fix for bigger chained pbuf, total_len %d\r\n",
                        p->tot_len
                );
            }
            host->pbuf_chained_ptr[loop] = (uint32_t)(q->payload);
            host->pbuf_chained_len[loop] = q->len;
#if 0
            printf("[%d] ptr %p, size %lu\r\n",
                    0,
                    (void*)host->pbuf_chained_ptr[0],
                    host->pbuf_chained_len[0]
            );
            printf("[%d] ptr %p, size %lu\r\n",
                    loop,
                    (void*)host->pbuf_chained_ptr[loop],
                    host->pbuf_chained_len[loop]
            );
#endif
        }
        loop++;
    }
    if (loop > 2) {
        printf("[TX] [LOOP] Chain Used %d\r\n", loop);
    }

    txhdr->status.value = 0;
    /* Fill-in TX descriptor */
    host->packet_addr = (uint32_t)(0x11111111);//FIXME we use this magic for unvaild packet_addr
    host->status_addr = (uint32_t)(&(txhdr->status));

    /*Ref this pbuf to avoid pbuf release*/
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
    if (from_local) {
        pbuf_ref(p);
    }
#else
    pbuf_ref(p);
#endif

    taskENTER_CRITICAL();
    utils_list_push_back(&tx_list_bl, &(txhdr->item));
    taskEXIT_CRITICAL();

    bl_irq_handler();

    return ERR_OK;
}
