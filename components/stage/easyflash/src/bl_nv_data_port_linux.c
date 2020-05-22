#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <bl_nv.h>

#define BL_NV_MSG_LEN           (128)
#define BL_NV_SECTOR_NUM        (4 + 1)
#define BL_NV_SECTOR_SIZE       (4096)

int XIP_SFlash_Erase_With_Lock(uint32_t addr, int len)
{
    uint8_t flash_buffer[BL_NV_SECTOR_SIZE];
    if (len % BL_NV_SECTOR_SIZE != 0 && addr % BL_NV_SECTOR_SIZE != 0) {
        printf("erase error addr:0x%08X len:0x%08X(%d)\n", addr, len, len);
        return 0;
    }
    int fd = open("flash.txt", O_RDWR, 0777);
    memset(flash_buffer, 0xFF, sizeof(flash_buffer));
    lseek(fd, addr ,SEEK_SET);
    ssize_t res = write(fd, flash_buffer, len);
    if (res < 0)
        printf("erase write error %d\n", res);
    close(fd);
    return 0;
}

int XIP_SFlash_Write_With_Lock(uint32_t addr, uint8_t *src, int len)
{
    int fd = open("flash.txt",O_RDWR, 0777);
    lseek(fd, addr ,SEEK_SET);
    ssize_t res = write(fd, src, len);
    if (res < 0)
        printf("write error %d\n", res);
    close(fd);
}

int XIP_SFlash_Read_With_Lock(uint32_t addr, uint8_t *dst, int len)
{
    int fd = open("flash.txt",O_RDWR, 0777);
    lseek(fd, addr ,SEEK_SET);
    read(fd, dst, len);
    close(fd);
}

int XIP_memcpy(void* dst, void* src_addr, int len)
{
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, dst, len);
}

uint8_t* XIP_read_msg(uint32_t src_addr)
{
    static uint8_t read_buf[BL_NV_MSG_LEN] = {0};
    memset(read_buf, 0, sizeof(read_buf));
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, read_buf, sizeof(read_buf));
    return read_buf;
}

uint32_t XIP_read_msg_info(uint32_t src_addr)
{
    static uint32_t read_buf;
    XIP_SFlash_Read_With_Lock((uint32_t)src_addr, (uint8_t*)&read_buf, sizeof(read_buf));
    return read_buf;
}

int bl_nv_port_linux_init()
{
    uint8_t flash_buffer[5 * BL_NV_SECTOR_SIZE]; 
    
    int fd = open("flash.txt", O_CREAT | O_RDWR, 0777);
    if ( fd == -1 ) {
        return -1;
    }

    memset(flash_buffer, 0xFF, sizeof(flash_buffer));
    ssize_t res = write(fd, flash_buffer, sizeof(flash_buffer));
    if (res < 0) {
        return -1;
    }

    close(fd);
    return 0;
}

int main(int argc, char* argv[])
{
    uint8_t write_content[128] = {1,2,3,4,5};
    uint8_t read_content[128];
    //bl_nv_port_linux_init();
#if 0
    XIP_SFlash_Write_With_Lock(0, write_content, sizeof(write_content));
    getchar();
    XIP_SFlash_Read_With_Lock(0, read_content, sizeof(write_content));
    printf("%x,%x,%x,%x\n", read_content[0],read_content[1],read_content[2],read_content[3]);
    getchar();
    XIP_SFlash_Erase_With_Lock(0, 4096);
#endif

    uint16_t time = 0;
    bl_nv_data_init();
    for (time = 0; time < 32; time++) {
        write_content[4]++;
        bl_nv_data_msg_push(write_content, sizeof(write_content));
    }
#if 0
    for (time = 0; time < 32; time++) {
        bl_nv_data_msg_pop(read_content, sizeof(read_content));
        printf("0x%02x\n", read_content[4]);
    }
#endif
    
    return 0;
}





