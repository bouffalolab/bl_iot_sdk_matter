#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <bl_nv_data.h>
#ifndef LINUX
#include <FreeRTOS.h>
#include <hal_boot2.h>
#endif

#define BL_NV_MSG_LEN           (128)
#define BL_NV_SECTOR_NUM        (4 + 1)
#define BL_NV_SECTOR_SIZE       (4096)
#define BL_NV_SECTOR_MSG_COUNT  (BL_NV_SECTOR_SIZE / BL_NV_MSG_LEN - 1)
#define BL_NV_TOTAL_SECTOR_SIZE (BL_NV_SECTOR_NUM * BL_NV_SECTOR_SIZE)


#define INACTIVE         (0x00000000)
#define ACTIVE           (0x0000FFFF)
#define NEW              (0xFFFFFFFF)
#define MSG_NEW          (0xFFFFFFFF)

//for flash
typedef struct sector_head_t {
    uint32_t active_flag;       //inactive:0x00000000; active:0x0000FFFF; new:0xFFFFFFFF 
} bl_nv_data_sector_head_t;

//for env
typedef struct nv_data_op_t {
    uint8_t cur_sec_idx; //0 - 4;
    uint8_t cur_msg_idx; //0 - 30;
    uint16_t total_msg;  //0 - 99;
    uint16_t cur_pop_pos; //0 - 99;
}bl_nv_data_op_t;


static bl_nv_data_op_t nv_data_op_env;
static uint8_t nv_data_init = 0;

extern int XIP_SFlash_Erase_With_Lock(uint32_t addr, int len);
extern int XIP_SFlash_Write_With_Lock(uint32_t addr, uint8_t *src, int len);
extern int XIP_SFlash_Read_With_Lock(uint32_t addr, uint8_t *dst, int len);

#ifdef LINUX
extern int XIP_memcpy(void* dst, void* src_addr, int len);
extern uint8_t* XIP_read_msg(uint32_t src_addr);
extern uint32_t XIP_read_msg_info(uint32_t src_addr);
static uint32_t nv_data_base_xip_addr_get()
{
    return 0;
}

static uint32_t nv_data_base_addr_get()
{
    return 0;
}

//porting todo
static uint8_t* nv_sector_time_get()
{
    static uint32_t cur_time = 1;
    return (uint8_t*)&cur_time;
}
#else
static int XIP_memcpy(void* dst, void* src_addr, int len)
{
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, dst, len);
    return 0;
}

static uint8_t* XIP_read_msg(uint32_t src_addr)
{
    static uint8_t read_buf[BL_NV_MSG_LEN] = {0};
    memset(read_buf, 0, sizeof(read_buf));
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, read_buf, sizeof(read_buf));
    return read_buf;
}

static uint32_t XIP_read_msg_info(uint32_t src_addr)
{
    static uint32_t read_buf;
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, (uint8_t*)&read_buf, sizeof(read_buf));
    return read_buf;
}

//porting todo
static uint8_t* nv_sector_time_get()
{
    static uint32_t cur_time = 1;
    return (uint8_t*)&cur_time;
}

static uint32_t nv_data_base_xip_addr_get()
{
    uint32_t addr = 0;
    hal_boot2_partition_addr_active("DATA", &addr);;
    return addr;
    //return (0x01164000 - 0x61000);
}

static uint32_t nv_data_base_addr_get()
{
    uint32_t addr = 0;
    hal_boot2_partition_addr_active("DATA", &addr);;
    return addr;
    //return (0x00164000);
}
#endif


//static functions
static int nv_data_is_inited()
{
    return nv_data_init;
}

static int nv_data_init_done()
{   
    nv_data_init = 1;
    return 0;
}

static uint32_t nv_sector_base_addr_get(uint8_t sec_idx)
{
    return nv_data_base_addr_get() + sec_idx * BL_NV_SECTOR_SIZE;
}

static uint32_t nv_sector_base_xip_addr_get(uint8_t sec_idx)
{
    //TODO FIXME this is NOT efficiency, should cached the result during init
    return nv_data_base_xip_addr_get() + sec_idx * BL_NV_SECTOR_SIZE;
}

static void nv_sector_head_get(uint8_t sec_idx, bl_nv_data_sector_head_t *s_head)
{
    uint32_t sector_head_addr = nv_sector_base_xip_addr_get(sec_idx);
    XIP_memcpy((void*)s_head, (void*)sector_head_addr, sizeof(bl_nv_data_sector_head_t));
}

static int nv_sector_erase_sync(uint8_t sec_idx)
{
    printf("nv_sector_erase_sync sec_idx: %d\r\n", sec_idx);
    return XIP_SFlash_Erase_With_Lock(nv_sector_base_addr_get(sec_idx), BL_NV_SECTOR_SIZE);
}

//set sector to active
static int nv_sector_active_set_sync(uint8_t sec_idx)
{
    bl_nv_data_sector_head_t shead;
    
        //TODO: assert
    if (sec_idx >= BL_NV_SECTOR_NUM)
        return -1;
    
    nv_sector_head_get(sec_idx, &shead);
    
    //sector has a active; it must be 0xFF(never used)
    if (shead.active_flag != NEW)
        return -1;
    
    printf("nv_sector_active_set_sync sec_idx: %d\r\n", sec_idx);
    //active sector
    shead.active_flag = ACTIVE; 
    //restore to flash
    return XIP_SFlash_Write_With_Lock(nv_sector_base_addr_get(sec_idx), 
                        (uint8_t*)&shead, sizeof(bl_nv_data_sector_head_t));
}

//set sector to inactive
static int nv_sector_inactive_set_sync(uint8_t sec_idx)
{
    bl_nv_data_sector_head_t shead;
    
        //TODO: assert
    if (sec_idx >= BL_NV_SECTOR_NUM)
        return -1;
    
    nv_sector_head_get(sec_idx, &shead);
    
    //sector has a active; it must be 0xFF(never used)
    if (shead.active_flag != ACTIVE)
        return -1;
    
    printf("nv_sector_inactive_set_sync sec_idx: %d\r\n", sec_idx);
    //active sector
    shead.active_flag = INACTIVE; 
    //restore to flash
    return XIP_SFlash_Write_With_Lock(nv_sector_base_addr_get(sec_idx), 
                           (uint8_t*)&shead, sizeof(bl_nv_data_sector_head_t));                      
}

//set a inactive sector to new one
static int nv_sector_new_sector_sync(uint8_t sec_idx)
{
    bl_nv_data_sector_head_t shead;
    nv_sector_head_get(sec_idx, &shead);
    printf("nv_sector_new_sector_sync sec_idx:%d, active_flag: 0x%08X\r\n",
                                   sec_idx, (unsigned int)shead.active_flag);
    if (shead.active_flag == NEW) {
        nv_sector_active_set_sync(sec_idx);
    } else if (shead.active_flag == INACTIVE){
        nv_sector_erase_sync(sec_idx);
        nv_sector_active_set_sync(sec_idx);
        nv_data_op_env.total_msg -= BL_NV_SECTOR_MSG_COUNT;
    }
    nv_data_op_env.cur_sec_idx = sec_idx;
    nv_data_op_env.cur_msg_idx = 0;
    return 0;
}

//get msg info (time)
static uint32_t nv_sector_msg_info_get(uint8_t sec_idx, uint8_t idx)
{
    //must sector info addr is 4 align
    uint32_t sector_info_addr = nv_sector_base_xip_addr_get(sec_idx);
    return XIP_read_msg_info((sector_info_addr + ((idx + 1) << 2 )));
}

//set msg info content
static int nv_sector_new_msg_info_set(uint8_t sec_idx, uint8_t msg_idx)
{
    uint32_t sector_info_addr = nv_sector_base_addr_get(sec_idx);
    uint32_t msg_info_addr = (sector_info_addr + ((msg_idx + 1) << 2));
    return XIP_SFlash_Write_With_Lock(msg_info_addr, (uint8_t*)nv_sector_time_get(), 4);
    
}

//get msg content
static uint8_t* nv_sector_msg_get(uint8_t sec_idx, uint8_t idx)
{
    //must sector info addr is 4 align
    uint32_t sector_info_addr = nv_sector_base_xip_addr_get(sec_idx);
    printf("nv_sector_msg_get msg_addr:0x%08X\r\n", 
            (unsigned int)(sector_info_addr + ((idx + 1) * BL_NV_MSG_LEN)));
    return XIP_read_msg(sector_info_addr + ((idx + 1) * BL_NV_MSG_LEN));
}


//set msg content
static int nv_sector_new_msg_set(uint8_t sec_idx, uint8_t msg_idx,
                                                char* msg, uint16_t len)
{
    //must sector info addr is 4 align
    uint32_t sector_info_addr = nv_sector_base_addr_get(sec_idx);
    uint32_t msg_addr = (sector_info_addr + ((msg_idx + 1) * BL_NV_MSG_LEN));
    
    if (nv_sector_new_msg_info_set(sec_idx, msg_idx) != 0) {
        return -1;
    }
    
    if (XIP_SFlash_Write_With_Lock(msg_addr, (uint8_t*)msg, len) != 0) {
       return -1;
    }

    nv_data_op_env.cur_msg_idx++;
    nv_data_op_env.total_msg++;
    return 0;
}


//get active sector;
static int nv_sector_active_get(bl_nv_data_op_t* nv_data)
{
    int sec_idx = 0;
    int msg_idx = 0;
    bl_nv_data_sector_head_t shead[BL_NV_SECTOR_NUM];
    
    for (sec_idx = 0; sec_idx < BL_NV_SECTOR_NUM; sec_idx++) {
        nv_sector_head_get(sec_idx, &shead[sec_idx]);
        if (shead[sec_idx].active_flag == INACTIVE) {
            nv_data->total_msg += BL_NV_SECTOR_MSG_COUNT;
            printf("1 nv_sector_active_get INACTIVE(%d) total_msg:%d\r\n",
                    sec_idx, nv_data->total_msg);
        }
    }
    
    for (sec_idx = 0; sec_idx < BL_NV_SECTOR_NUM; sec_idx++) {
        if (shead[sec_idx].active_flag == ACTIVE) {
            nv_data->cur_sec_idx = sec_idx;
            for (msg_idx = 0; msg_idx < BL_NV_SECTOR_MSG_COUNT; msg_idx++) {
                if (nv_sector_msg_info_get(sec_idx, msg_idx) == MSG_NEW) {
                    nv_data->cur_msg_idx = msg_idx;
                    nv_data->total_msg += msg_idx;
                    printf("2 nv_sector_active_get ACTIVE:MSG_NEW(%d:%d) total_msg:%d)\r\n",
                        sec_idx, msg_idx, nv_data->total_msg);
                    return 0;
                }
            }
            
            // alloc a new sector, should not happen
            if (msg_idx == BL_NV_SECTOR_MSG_COUNT) {
                printf("3 nv_sector_active_get alloc a new sector, should not happen!!!\r\n");
                nv_sector_inactive_set_sync(sec_idx);
                nv_sector_new_sector_sync((sec_idx + 1) % BL_NV_SECTOR_NUM);
            }
            return 0;
        }
    }
    
    for (sec_idx = 0; sec_idx < BL_NV_SECTOR_NUM; sec_idx++) {
        if (shead[sec_idx].active_flag == NEW) {
            printf("nv_sector_active_get NEW(%d)\r\n", sec_idx);
            nv_sector_new_sector_sync(sec_idx);
            return 0;
        }
    }
    
    //error not find a sector
    return -1;
}

static char* nv_sector_get_msg_by_idx(uint16_t idx)
{
    uint16_t total_msg = bl_nv_data_msg_total_size();
    uint8_t delta_sec = 0;
    int8_t real_sec = 0;
    uint8_t real_idx = 0;
    
    //idx : [0-99]
    delta_sec = ((total_msg - 1)/BL_NV_SECTOR_MSG_COUNT) 
                   - ((total_msg - idx - 1) / BL_NV_SECTOR_MSG_COUNT);
    real_sec = nv_data_op_env.cur_sec_idx - delta_sec;
    if (real_sec < 0)
        real_sec += BL_NV_SECTOR_NUM;
    
    real_idx = (total_msg - idx - 1) % BL_NV_SECTOR_MSG_COUNT;

    printf("nv_sector_get_msg_by_idx %d (%d:%d:%d)=>(%d:%d)\r\n",
                                            delta_sec,
                                            idx,
                                            nv_data_op_env.cur_sec_idx,
                                            nv_data_op_env.cur_msg_idx, 
                                            real_sec, real_idx);
    return (char*)nv_sector_msg_get((uint8_t)real_sec, real_idx);
}

//external functions
int bl_nv_data_init()
{

    if(nv_data_is_inited()) {
        return 0;
    }

    memset(&nv_data_op_env, 0, sizeof(nv_data_op_env));
    
    if (nv_sector_active_get(&nv_data_op_env) == 0)
        return nv_data_init_done();
    else
        return -1;
}

int bl_nv_data_msg_pop_reset()
{
    nv_data_op_env.cur_pop_pos = 0;
    return 0;
}

int bl_nv_data_msg_get_by_idx(uint16_t idx, char* msg_out, int len)
{
    if (!nv_data_is_inited())
        return -1;
    
    if (idx >= nv_data_op_env.total_msg) {
        printf("bl_nv_data_msg_get_by_idx idx(%d) > nv_data_op_env.total_msg(%d) error!!!\r\n",
                                       idx, nv_data_op_env.total_msg);
        return -1;
    }
    
    memcpy(msg_out, nv_sector_get_msg_by_idx(idx), len);
    
    return 0;
}

int bl_nv_data_msg_push(char* msg_in, uint16_t msg_len)
{
    if (!nv_data_is_inited())
        return -1;
    
    if (nv_sector_new_msg_set(nv_data_op_env.cur_sec_idx, 
                    nv_data_op_env.cur_msg_idx, msg_in, msg_len) != 0) {
        printf("nv_sector_new_msg_set return error!!!\r\n");
        return -1;
    }
    
    printf("bl_nv_data_msg_push (%d:%d:%d)\r\n", nv_data_op_env.cur_sec_idx, 
                            nv_data_op_env.cur_msg_idx, nv_data_op_env.total_msg);
    
    if (nv_data_op_env.cur_msg_idx == BL_NV_SECTOR_MSG_COUNT) {
        nv_sector_inactive_set_sync(nv_data_op_env.cur_sec_idx);
        nv_sector_new_sector_sync((nv_data_op_env.cur_sec_idx + 1) % BL_NV_SECTOR_NUM);
    }

    return 0;
}

int bl_nv_data_msg_pop(char* msg_out, int len)
{
    if (bl_nv_data_msg_get_by_idx(nv_data_op_env.cur_pop_pos, msg_out, len) != 0)
        return -1;
    
    nv_data_op_env.cur_pop_pos++;
    return 0;
}

int bl_nv_data_msg_total_size()
{
    return nv_data_op_env.total_msg;
}

int bl_nv_data_reset()
{
    uint8_t sec_idx = 0;
    for (sec_idx = 0; sec_idx < BL_NV_SECTOR_NUM; sec_idx++) {
        nv_sector_erase_sync(sec_idx);
    }
    
    memset(&nv_data_op_env, 0, sizeof(nv_data_op_env));
    return 0;
    
}
