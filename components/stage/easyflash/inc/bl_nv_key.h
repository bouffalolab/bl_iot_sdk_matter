#ifndef __BL_NV_KEY_H__
#define __BL_NV_KEY_H__

//#define LINUX 1
void bl_nv_key_init();
void bl_nv_key_reset();
int bl_nv_key_set_value(const char *key, uint8_t *value, uint32_t size);
uint8_t* bl_nv_key_get_value(const char *key, uint32_t* len_out);
#endif /*__BL_NV_KEY_H__*/