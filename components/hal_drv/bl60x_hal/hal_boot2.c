#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bl_boot2.h"
#include "hal_boot2.h"

#define PARTITION_BOOT2_RAM_ADDR_ACTIVE (0x0042BC00)
#define PARTITION_HEADER_BOOT2_RAM_ADDR (0x0042BC04)
#define PARTITION_MAGIC                 (0x54504642)
#define PARTITION_FW_PART_NAME          "FW_CPU0"
#define PARTITION_FW_PART_HEADER_SIZE   (0x1000)
//TODO use header file from project
#define FW_XIP_ADDRESS                  (0x01000000)

static PtTable_Stuff_Config partition_table;
static uint8_t partition_active_idx;

void hal_boot2_set_ptable_opt(pPtTable_Flash_Erase erase, pPtTable_Flash_Write write)
{
    PtTable_Set_Flash_Operation(erase, write);
}

int hal_boot2_update_ptable(PtTable_Entry_Config *ptEntry)
{
    int ret;

    ptEntry->activeIndex = !ptEntry->activeIndex;
    (ptEntry->age)++;
    ret = PtTable_Update_Entry(NULL,!partition_active_idx, &partition_table, ptEntry);
    return ret;
}

static void _dump_partition(PtTable_Stuff_Config *part)
{
    int i;

    printf("======= PtTable_Config @%p=======\r\n", part);
    printf("magicCode 0x%08X;", (unsigned int)(part->ptTable.magicCode));
    printf(" version 0x%04X;", part->ptTable.version);
    printf(" entryCnt %u;", part->ptTable.entryCnt);
    printf(" age %lu;", part->ptTable.age);
    printf(" crc32 0x%08X\r\n", (unsigned int)part->ptTable.crc32);

    printf("idx  type device activeIndex     name   Address[0]  Address[1]  Length[0]   Length[1]   age\r\n");
    for (i = 0; i < part->ptTable.entryCnt; i++) {
        printf("[%02d] ", i);
        printf(" %02u", part->ptEntries[i].type);
        printf("     %u", part->ptEntries[i].device);
        printf("         %u", part->ptEntries[i].activeIndex);
        printf("      %8s", part->ptEntries[i].name);
        printf("  %p", (void*)(part->ptEntries[i].Address[0]));
        printf("  %p", (void*)(part->ptEntries[i].Address[1]));
        printf("  %p", (void*)(part->ptEntries[i].maxLen[0]));
        printf("  %p", (void*)(part->ptEntries[i].maxLen[1]));
        printf("  %lu\r\n", (part->ptEntries[i].age));
    }
}

int hal_boot2_partition_bus_addr(const char *name, uint32_t *addr0, uint32_t *addr1, int *active)
{
    int i;
    uint32_t addr0_t, addr1_t;

    if (PARTITION_MAGIC != partition_table.ptTable.magicCode) {
        return -EIO;
    }

    /*Get Target partition*/
    for (i = 0; i < partition_table.ptTable.entryCnt; i++) {
        if (0 == strcmp((char *)&(partition_table.ptEntries[i].name[0]), name)) {
            break;
        }
    }
    if (partition_table.ptTable.entryCnt == i) {
        return -ENOENT;
    }
    addr0_t = partition_table.ptEntries[i].Address[0];
    addr1_t = partition_table.ptEntries[i].Address[1];
    *active = partition_table.ptEntries[i].activeIndex;

    /*cal partition address*/
    for (i = 0; i < partition_table.ptTable.entryCnt; i++) {
        if (0 == strcmp((char *)&(partition_table.ptEntries[i].name[0]), PARTITION_FW_PART_NAME)) {
            break;
        }
    }
    if (partition_table.ptTable.entryCnt == i) {
        return -ECANCELED;
    }
    /*Make sure target partition is after FW partition*/
    if ( (addr0_t && (addr0_t < partition_table.ptEntries[i].Address[0])) ||
         (addr0_t && (addr0_t < partition_table.ptEntries[i].Address[1])) ||
         (addr1_t && (addr1_t < partition_table.ptEntries[i].Address[0])) ||
         (addr1_t && (addr1_t < partition_table.ptEntries[i].Address[1]))) {
        return -EINVAL;
    }
    if ((0 != partition_table.ptEntries[i].activeIndex) &&
        (1 != partition_table.ptEntries[i].activeIndex)) {
        return -EFAULT;
    }
    *addr0 = addr0_t - partition_table.ptEntries[i].Address[partition_table.ptEntries[i].activeIndex] - PARTITION_FW_PART_HEADER_SIZE + FW_XIP_ADDRESS;
    *addr1 = addr1_t - partition_table.ptEntries[i].Address[partition_table.ptEntries[i].activeIndex] - PARTITION_FW_PART_HEADER_SIZE + FW_XIP_ADDRESS;

    return 0;
}

int hal_boot2_partition_bus_addr_active(const char *name, uint32_t *addr)
{
    uint32_t addr0, addr1;
    int active, ret;

    if ((ret = hal_boot2_partition_bus_addr(name, &addr0, &addr1, &active))) {
        return ret;
    }
    *addr = active ? addr1 : addr0;

    return 0;
}

int hal_boot2_partition_addr(const char *name, uint32_t *addr0, uint32_t *addr1, int *active)
{
    int i;

    if (PARTITION_MAGIC != partition_table.ptTable.magicCode) {
        return -EIO;
    }

    /*Get Target partition*/
    for (i = 0; i < partition_table.ptTable.entryCnt; i++) {
        if (0 == strcmp((char *)&(partition_table.ptEntries[i].name[0]), name)) {
            break;
        }
    }
    if (partition_table.ptTable.entryCnt == i) {
        return -ENOENT;
    }
    *addr0 = partition_table.ptEntries[i].Address[0];
    *addr1 = partition_table.ptEntries[i].Address[1];
    *active = partition_table.ptEntries[i].activeIndex;

    return 0;
}

int hal_boot2_partition_addr_active(const char *name, uint32_t *addr)
{
    uint32_t addr0, addr1;
    int active, ret;

    if ((ret = hal_boot2_partition_addr(name, &addr0, &addr1, &active))) {
        return ret;
    }
    *addr = active ? addr1 : addr0;

    return 0;
}

uint8_t hal_boot2_get_active_partition(void)
{
    return partition_active_idx;
}

int hal_boot2_get_active_entries(PtTable_Entry_Type type, PtTable_Entry_Config *ptEntry)
{
    if (PtTable_Get_Active_Entries(&partition_table, type, ptEntry)) {
        return -1;
    }
    return 0;
}

int hal_boot2_dump(void)
{
    _dump_partition(&partition_table);
    return 0;
}

int hal_boot2_init(void)
{
    PtTable_Stuff_Config *part_ram = (PtTable_Stuff_Config*)PARTITION_HEADER_BOOT2_RAM_ADDR;
    partition_active_idx = *(uint8_t*)PARTITION_BOOT2_RAM_ADDR_ACTIVE;

    _dump_partition(part_ram);

    printf("[HAL] [BOOT2] Active Partition[%u] consumed %d Bytes\r\n",
            partition_active_idx,
            sizeof(PtTable_Stuff_Config)
    );
    partition_table = *part_ram;
    _dump_partition(&partition_table);

    return 0;
}
