#ifndef __BL_SEC_H__
#define __BL_SEC_H__
#include <stddef.h>
#include <stdint.h>
int bl_sec_init(void);
int bl_sec_test(void);
int bl_sec_aes_enc(uint8_t *key, int keysize, uint8_t *input, uint8_t *output);
uint32_t bl_sec_get_random_word(void);
#endif
