#ifndef __BL_NV_H__
#define __BL_NV_H__

//#define LINUX 1
int bl_nv_data_init();
int bl_nv_data_msg_pop_reset();
int bl_nv_data_msg_get_by_idx(uint16_t idx, char* msg_out, int len);
int bl_nv_data_msg_push(char* msg_in, uint16_t msg_len);
int bl_nv_data_msg_pop(char* msg_out, int len);
int bl_nv_data_msg_total_size();
int bl_nv_data_reset();

#endif /*__BL_NV_H__*/