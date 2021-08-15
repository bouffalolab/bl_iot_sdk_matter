#ifndef __RWNX_TX_H__
#define __RWNX_TX_H__
#include <lwip/opt.h>
#include <lwip/mem.h>
#include <utils_list.h>

#include "lmac_types.h"
#include "ipc_shared.h"
#include "hal_desc.h"
#include "bl_utils.h"

/**
 ****************************************************************************************
 *
 * @file bl_tx.h
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#define TX_SW_RETRY_POLICY_ENABLE 1    /* 0 for infinite retry, 1 enable time & count based policy */
#define TX_SW_RETRY_TIMEOUT_MS    3000
#define TX_SW_RETRY_ROUND_LIMIT   256  /* 0=>0, 1=>(1+7), 2=>2*(1+7), ..., (n>=256)=>unlimited(only timeout matters) re-xmits */

/**
 * struct bl_txhdr - Stucture to control transimission of packet
 * (Added in skb headroom)
 *
 * @sw_hdr: Information from driver
 * @hw_hdr: Information for/from hardware
 */
struct bl_txhdr {
    struct utils_list_hdr item;
    union bl_hw_txstatus status;
    uint32_t *p;
    struct hostdesc host;
#if TX_SW_RETRY_POLICY_ENABLE
    struct {
        uint8_t retrying;
        uint8_t retry_times;
        uint32_t first_retry_tick_ms;
    } sw_retry_stat;
#endif
};

#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta, bool from_local);
#else
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta);
#endif
int bl_txdatacfm(void *pthis, void *host_id);
void bl_tx_notify();
void bl_tx_try_flush();
void bl_irq_handler();
#endif
