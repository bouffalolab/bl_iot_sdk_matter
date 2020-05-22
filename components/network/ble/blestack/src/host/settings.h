/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "hci_host.h"

struct bt_settings_handler {
	const char *name;
	int (*set)(int argc, char **argv, char *val);
	int (*commit)(void);
	int (*export)(int (*func)(const char *name, char *val));
};

#define BT_SETTINGS_DEFINE(_name, _set, _commit, _export)               \
	const struct bt_settings_handler _name __aligned(4)             \
			__in_section(_bt_settings, static, _name) = {   \
				.name = STRINGIFY(_name),               \
				.set = _set,                            \
				.commit = _commit,                      \
				.export = _export,                      \
			}

/* Max settings key length (with all components) */
#define BT_SETTINGS_KEY_MAX 36

/* Base64-encoded string buffer size of in_size bytes */
#define BT_SETTINGS_SIZE(in_size) ((((((in_size) - 1) / 3) * 4) + 4) + 1)

/* Helpers for keys containing a bdaddr */
void bt_settings_encode_key(char *path, size_t path_size, const char *subsys,
			    bt_addr_le_t *addr, const char *key);
int bt_settings_decode_key(char *key, bt_addr_le_t *addr);

void bt_settings_save_id(void);

int settings_save_one(const char *name, char *value);

int bt_settings_init(void);

#if defined(BFLB_BLE)
#define NV_LOCAL_NAME      "LOCAL_NAME"
#define NV_LOCAL_ID_ADDR   "LOCAL_ID_ADDR"
#define NV_LOCAL_IRK       "LOCAL_IRK"
#define NV_GATT_CCC        "GATT_CCC"
#define NV_KEY_POOL        "KEY_POOL"
#define NV_IMG_info        "IMG_INFO"

int bt_settings_get_bin(const char *key, u8_t *value, size_t length);
int bt_settings_set_bin(const char *key, const u8_t *value, size_t length);
void bt_settings_save_name(void);
void bt_local_info_load(void);
#endif
