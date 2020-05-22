/*****************************************************************************************
*
* @file utils.c
*
* @brief entry
*
* Copyright (C) Bouffalo Lab 2019
*
* History: 2019-11 crealted by Lanlan Gong @ Shanghai
*
*****************************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void co_skip_delimits(const char **ptr, char delimits)
{
    while(**ptr == delimits)(*ptr)++;
}

void co_get_bytearray_from_string(char** params, uint8_t *result, int array_size)
{
    int i = 0;
    char rand[3];

    co_skip_delimits((const char **)params, ' ');
    for(i=0; i < array_size; i++){
        strncpy(rand, (const char*)*params, 2);
        rand[2]='\0';
        result[i] = strtol(rand, NULL, 16);
        *params = *params + 2;
    }
}

void co_get_uint16_from_string(char** params, uint16_t *result)
{
    uint8_t ret_array[2];
    co_get_bytearray_from_string(params, ret_array, 2);
    *result = (ret_array[0]<<8)|ret_array[1];
}

void co_get_uint32_from_string(char** params, uint32_t *result)
{
    uint8_t ret_array[4];
    co_get_bytearray_from_string(params, ret_array, 4);
    *result = (ret_array[0]<<24)|(ret_array[1]<<16)|(ret_array[2]<<8)|ret_array[3];
}

void co_reverse_bytearray(uint8_t *src, uint8_t *result, int array_size)
{
    for(int i=0; i < array_size;i++){
        result[array_size - i -1] = src[i];
    }
}

unsigned int find_msb_set(uint32_t data)
{
    uint32_t count = 0;
    uint32_t mask = 0x80000000;

    if (!data) {
        return 0;
    }
    while ((data & mask) == 0) {
        count += 1u;
        mask = mask >> 1u;
    }
    return (32 - count);
}

unsigned int find_lsb_set(uint32_t data)
{
    uint32_t count = 0;
    uint32_t mask = 0x00000001;

    if (!data) {
        return 0;
    }
    while ((data & mask) == 0) {
        count += 1u;
        mask = mask << 1u;
    }
    return (1 + count);
}
