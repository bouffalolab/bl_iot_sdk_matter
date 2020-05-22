/** @file
 *  @brief HoG Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#define HID_PAGE_KBD   0x07
#define HID_PAGE_CONS  0x0C

enum hid_usage{
     KEY_RIGHT = 0x004f,
     KEY_LEFT,
     KEY_DOWN,
     KEY_UP,
     KEY_SELECT = 0x0058,
     KEY_BACK = 0x00f1,
     KEY_FF = 0x00b3 ,
     KEY_PREV,
     KEY_PLAY_PSE = 0x00cd,
     KEY_MENU = 0x0040,
     KEY_HOME = 0x0223,
     KEY_VOICE = 0x0221,
     KEY_TV_PWR = 0x0066,
     KEY_TV_INPUT = 0x0089,
     KEY_TV_MUTE = 0x00e2,
     KEY_TV_VOL_INS = 0X00e9,
     KEY_TV_VOL_DES
};


void hog_init(void);
int bt_hog_notify(struct bt_conn *conn, u16_t hid_usage,u8_t press);
extern volatile u8_t Voicekey_is_press;

#ifdef __cplusplus
}
#endif
