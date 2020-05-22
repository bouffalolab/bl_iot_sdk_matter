#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <bl_nv_key.h>

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


int bl_nv_port_linux_init()
{
    uint8_t flash_buffer[BL_NV_SECTOR_SIZE]; 
    
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
    uint8_t write_content[1023] = {1,2,3,4,5};
    uint8_t* read = NULL;
    uint32_t len_out = 0;
    uint16_t i = 0;
    if (strcmp(argv[1], "new") == 0) {
        bl_nv_port_linux_init();
    }
    bl_nv_key_init();
    bl_nv_key_set_value("baidu_ca1", write_content, sizeof(write_content));
    bl_nv_key_set_value("baidu_ca2", write_content, sizeof(write_content));
    bl_nv_key_set_value("baidu_ca3", write_content, sizeof(write_content));
    bl_nv_key_set_value("baidu_ca3", write_content, sizeof(write_content));
    bl_nv_key_set_value("baidu_ca4", write_content, sizeof(write_content));
    
    read = bl_nv_key_get_value("baidu_ca1", &len_out);
    for (i = 0; i < len_out; i++) {
        printf("%x,", read[i]);
    }
    printf("\r\n");

    printf("main over\r\n");
    return 0;
}





