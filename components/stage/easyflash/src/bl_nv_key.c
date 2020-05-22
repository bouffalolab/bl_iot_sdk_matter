#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <bl_nv_key.h>
#ifndef LINUX
#include <FreeRTOS.h>
#include <hal_boot2.h>
#endif

#define BL_NV_KEY_REGION_MAGIC_CODE "BLKY"

//can't modify BL_NV_KEY_MAX_KEY_LEN, must be 32
#define BL_NV_KEY_MAX_KEY_INFO_LEN  (32)
#define BL_NV_KEY_MAX_KEY_LEN       (32 - 8)
#define BL_NV_KEY_ITEM_NUM      ((1 << 4) - 1) // the first key is magic code

#define BL_NV_KEY_KEY_REGION_LEN (BL_NV_KEY_MAX_KEY_INFO_LEN * (BL_NV_KEY_ITEM_NUM + 1))

#define BL_NV_KEY_SECTOR_SIZE   (4096)
#define BL_NV_KEY_MAX_VAL_LEN   (4096 - BL_NV_KEY_KEY_REGION_LEN)

extern int XIP_SFlash_Erase_With_Lock(uint32_t addr, int len);
extern int XIP_SFlash_Write_With_Lock(uint32_t addr, uint8_t *src, int len);
extern int XIP_SFlash_Read_With_Lock(uint32_t addr, uint8_t *dst, int len);


//flash layout
typedef struct _bl_nv_key_region {
    char key[BL_NV_KEY_MAX_KEY_LEN];
    uint32_t value_offset;
    uint32_t value_len;
} bl_nv_key_region_t;

static uint8_t  initd = 0;
static uint32_t cur_value_offset = BL_NV_KEY_KEY_REGION_LEN;

#ifdef LINUX
extern int XIP_memcpy(void* dst, void* src_addr, int len);
static uint32_t nv_key_base_addr_get()
{
    return 0;
}

static uint32_t nv_key_base_xip_addr_get()
{
    return 0;
}

static uint32_t XIP_value_len_get(uint32_t xip_addr, uint32_t offset)
{
    uint32_t len = 0;
    XIP_SFlash_Read_With_Lock((xip_addr + offset), (uint8_t*)&len, sizeof(len));
    return len;
}

static uint8_t* XIP_value_get(uint32_t xip_addr, uint32_t offset)
{
    static uint8_t value[1024] = {0};
    XIP_SFlash_Read_With_Lock((xip_addr + offset), (uint8_t*)value, sizeof(value));
    return value;
}

static int XIP_strcmp(const char* s1, const char* s2)
{
    char value[32] = {0};
    XIP_SFlash_Read_With_Lock((uint32_t)s2, (uint8_t*)value, sizeof(value));
    return strcmp(s1, value);
}

#else
static int XIP_memcpy(void* dst, void* src_addr, int len)
{
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, dst, len);
    return 0;
}

static int XIP_strcmp(const char* s1, const char* s2)
{
    return strcmp(s1, s2);
}

static uint8_t* XIP_value_get(uint32_t xip_addr, uint32_t offset)
{
    return (uint8_t*)(xip_addr + offset);
}

static uint32_t nv_key_base_addr_get()
{
    uint32_t addr = 0;
    hal_boot2_partition_addr_active("KEY", &addr);
    return addr;
}

static uint32_t nv_key_base_xip_addr_get()
{
    uint32_t addr = 0;
    hal_boot2_partition_bus_addr_active("KEY", &addr);
    return addr;
}
#endif


static int nv_key_cmp_magic_code()
{
    uint32_t key_region_addr = nv_key_base_xip_addr_get();
    if (XIP_strcmp(BL_NV_KEY_REGION_MAGIC_CODE, (const char*)key_region_addr) != 0)
        return -1;
    
    return 0;
}

static int nv_key_hardware_magic_write()
{
    char magic_code[5] = {0};
    uint32_t is_magic_code = nv_key_cmp_magic_code();
    printf("[INFO] nv_key_hardware_magic_write %d\r\n", (unsigned int)is_magic_code);
    if ( is_magic_code != 0 ) {
        strcpy(magic_code, BL_NV_KEY_REGION_MAGIC_CODE);
        XIP_SFlash_Write_With_Lock(nv_key_base_addr_get(), (uint8_t*)magic_code, sizeof(magic_code));
    }
    return 0;
}

static uint32_t nv_key_key_addr_get(uint8_t key_idx)
{
    return nv_key_base_addr_get() + ((key_idx + 1) * BL_NV_KEY_MAX_KEY_INFO_LEN);
}

static uint32_t nv_key_value_offset_get(uint8_t key_idx)
{
    uint32_t value_offset = 0;
    XIP_memcpy(&value_offset, (void*)(nv_key_key_addr_get(key_idx) + BL_NV_KEY_MAX_KEY_LEN ), sizeof(value_offset));
    return value_offset;
}


static uint32_t nv_key_value_len_get(uint8_t key_idx)
{
    uint32_t value_len = 0;
    XIP_memcpy(&value_len, (void*)(nv_key_key_addr_get(key_idx) + BL_NV_KEY_MAX_KEY_LEN + 4), sizeof(value_len));
    return value_len;
}

static int nv_key_value_info_get(uint8_t key_idx, uint32_t* offset, uint32_t* value_len)
{
    uint32_t value_info[2] = {0};
    XIP_memcpy(&value_info, (void*)( nv_key_key_addr_get(key_idx) + BL_NV_KEY_MAX_KEY_LEN ), sizeof(value_info));
    *offset = value_info[0];
    *value_len = value_info[1];
    return 0;
}

static int nv_key_key_is_free(uint8_t key_idx)
{
    uint8_t key_1 = 0;
    XIP_memcpy(&key_1, (void*)nv_key_key_addr_get(key_idx), 1);
    if (key_1 == 0xFF) {
        return 1;
    }
    return 0;
}

static int nv_key_key_has(const char* key_in, uint8_t key_idx)
{
    char key[BL_NV_KEY_MAX_KEY_LEN] = {0};
    XIP_memcpy(&key, (void*)nv_key_key_addr_get(key_idx), sizeof(key));
    if (strcmp(key_in, key) != 0) {
        return 0;
    }
    return 1;
}

static int nv_key_find_new_pos(const char* key, uint16_t value_len)
{
    uint8_t key_idx = 0;
    uint32_t offset = 0;
    uint32_t len = 0;
    uint32_t next_value_offset = BL_NV_KEY_KEY_REGION_LEN;
        
    if ( strlen(key) >= BL_NV_KEY_MAX_KEY_LEN )
        return -3; // key len > 28
    
    for (key_idx = 0; key_idx < BL_NV_KEY_ITEM_NUM; key_idx++) {
        if (nv_key_key_is_free(key_idx)) {
            if (next_value_offset + value_len > BL_NV_KEY_SECTOR_SIZE ) {
                printf("[ERR] nv_key_find_new_pos all keys len is larger than 4KB\r\n");
                return -4; //value len > Key Region Max Len
            }
            cur_value_offset = next_value_offset;
            printf("[INFO] nv_key_find_new_pos [cur_value_offset] %d\r\n", (unsigned int)cur_value_offset);
            return key_idx; //find a pos
        }
        
        if (nv_key_key_has(key, key_idx)) {
            printf("[ERR] nv_key_find_new_pos [nv_key_key_has] duplication (%s, %d)\r\n",key, (unsigned int)key_idx);
            return -2; //key has inside
        }
        
        nv_key_value_info_get(key_idx, &offset, &len);
        next_value_offset = offset + len;
    }
    printf("[ERR] nv_key_find_new_pos key full\r\n");
    return -1; //full
}

static int nv_key_key_write(const char* key_in, uint32_t len, int pos)
{
    char key[BL_NV_KEY_MAX_KEY_LEN] = {0};
    uint32_t value_info[2] = {0};
    strcpy(key, key_in);
    
    value_info[0] = cur_value_offset;
    value_info[1] = len;
    
    //write key
    XIP_SFlash_Write_With_Lock(nv_key_key_addr_get(pos), (uint8_t*)key, strlen(key) + 1);
    //write offset
    XIP_SFlash_Write_With_Lock(nv_key_key_addr_get(pos) + BL_NV_KEY_MAX_KEY_LEN 
        , (uint8_t*)&value_info, sizeof(value_info));
    return 0;
}

static int nv_key_value_write(uint8_t* value_in_ram, uint32_t len, int pos)
{
    //write value self
    return XIP_SFlash_Write_With_Lock(nv_key_base_addr_get() + nv_key_value_offset_get(pos), value_in_ram, len);
}

static int nv_key_write(const char* key, uint8_t* value, uint32_t len, int pos)
{
    nv_key_key_write(key, len, pos);
    nv_key_value_write(value, len, pos);
    return 0;
}

static int nv_key_set(const char* key, uint8_t* value, uint32_t len)
{
    int new_key_pos = -1;
    
    new_key_pos = nv_key_find_new_pos(key, len);
    if (new_key_pos < 0) {
        printf("[ERR] nv_key_set [nv_key_find_new_pos] %d\r\n", new_key_pos);
        return new_key_pos;
    }
    nv_key_write(key, value, len, new_key_pos);
    
    return 0;
    
}

static void nv_key_reinit()
{
    nv_key_hardware_magic_write();
    cur_value_offset = BL_NV_KEY_KEY_REGION_LEN;
}

//external functions
void bl_nv_key_init()
{
    if (initd != 0)
        return;
    
    nv_key_hardware_magic_write();
        
    initd = 1;
}

void bl_nv_key_reset()
{
    XIP_SFlash_Erase_With_Lock(nv_key_base_addr_get(), BL_NV_KEY_SECTOR_SIZE);
    nv_key_reinit();
}


int bl_nv_key_set_value(const char *key, uint8_t *value, uint32_t size)
{
    
    if (initd == 0)
        return -1;
    
    if (key == NULL 
        || strlen(key) >= BL_NV_KEY_MAX_KEY_LEN 
        || value == NULL) {
        return -1;
    }
    
    return nv_key_set(key, value, size);
}


uint8_t* bl_nv_key_get_value(const char *key, uint32_t* len_out)
{
    uint32_t xip_addr = 0;
    uint32_t offset = 0;
    
    if (key == NULL || initd == 0) {
        return NULL;
    }
    
    uint8_t key_idx = 0; 
    for (key_idx = 0; key_idx < BL_NV_KEY_ITEM_NUM; key_idx++) {
        if (nv_key_key_has(key, key_idx)) {
            xip_addr = nv_key_base_xip_addr_get();
            offset = nv_key_value_offset_get(key_idx);
            *len_out = nv_key_value_len_get(key_idx);
            return XIP_value_get(xip_addr, offset);
        }
    }
    
    return NULL;

}


