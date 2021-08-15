#ifndef __BL_SDU_H__
#define __BL_SDU_H__

#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
#include <stdbool.h>
#include <stdint.h>
#include <lwip/pbuf.h>

int bl_sdu_init(void);
int32_t bl_sdio_handshake(void);
typedef void (*bl_sdio_read_cb_t)(void *cb_arg, const void *data_ptr, const uint16_t data_len);
int bl_sdio_read_cb_register(void *env, bl_sdio_read_cb_t cb, const void *cb_arg);

/* data_len is len(V), excluding len(TL) */
int bl_sdio_write_tlv(void *env, const uint16_t type, const void *data_ptr, const uint16_t data_len);
/* data_len is len(V), excluding len(TL) */
int bl_sdio_write_pbuf_tlv(void *env, const uint16_t type, const uint16_t subtype, struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg);
#endif

#endif
