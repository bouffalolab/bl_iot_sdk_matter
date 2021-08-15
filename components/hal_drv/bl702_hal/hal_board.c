#include <stdint.h>
#include <stdio.h>
#include <bl_efuse.h>
#include <bl_wireless.h>
#include <hal_boot2.h>
#include <hal_sys.h>

#include <libfdt.h>

#include <blog.h>
#define USER_UNUSED(a) ((void)(a))

#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))

static uint32_t factory_addr = 0;

static int update_mac_config_get_mac_from_dtb(const void *fdt, int offset1, uint8_t mac_addr[8])
{
    int lentmp;
    const uint8_t *addr_prop = 0;

    /* set mac_addr */
    addr_prop = fdt_getprop(fdt, offset1, "mac_addr", &lentmp);
    if (8 == lentmp) {

        memcpy(mac_addr, addr_prop, 8);
        //blog_info("mac_addr :\r\n");
        //blog_buf(mac_addr, 8);
    } else {
        blog_error("mac_addr NULL.\r\n");
        return -1;
    }

    return 0;
}

static int update_mac_config_get_mac_from_efuse(uint8_t mac_addr[8])
{
    uint8_t result_or, result_and;

    bl_efuse_read_mac(mac_addr);
    result_or = mac_addr[0] | mac_addr[1] | mac_addr[2] | mac_addr[3] | mac_addr[4] | mac_addr[5] | mac_addr[6] | mac_addr[7];
    result_and = mac_addr[0] & mac_addr[1] & mac_addr[2] & mac_addr[3] & mac_addr[4] & mac_addr[5] & mac_addr[6] & mac_addr[7];

    if (0 == result_or || 1 == result_and) {
        /*all zero or one found in efuse*/
        return -1;
    }
    return 0;
}

static int update_mac_config_get_mac_from_factory(uint8_t mac_addr[8])
{
    uint8_t result_or, result_and;

    if (bl_efuse_read_mac_factory(mac_addr)) {
        return -1;
    }
    result_or = mac_addr[0] | mac_addr[1] | mac_addr[2] | mac_addr[3] | mac_addr[4] | mac_addr[5] | mac_addr[6] | mac_addr[7];
    result_and = mac_addr[0] & mac_addr[1] & mac_addr[2] & mac_addr[3] & mac_addr[4] & mac_addr[5] & mac_addr[6] & mac_addr[7];
    if (0 == result_or || 1 == result_and) {
        /*all zero or one found in efuse*/
        return -1;
    }
    return 0;
}

/*
 * Update MAC address according to order string
 * BFM:
 *  'B' for EFUSE built-in MAC address
 *  'F' for Flash built-in MAC address
 *  'M' for manufacutre configured EFUSE built-in MAC address
 * */
#define MAC_ORDER_ADDR_LEN_MAX      (3)
static void update_mac_config_with_order(const void *fdt, int offset1, const char *order)
{
    int i, set, len;
    uint8_t mac_addr[8];
    static const uint8_t mac_default[] = {0x18, 0xB9, 0x05, 0x88, 0x88, 0x88, 0x88, 0x88};

    set = 0;
    len = strlen(order);
    for (i = 0; i < MAC_ORDER_ADDR_LEN_MAX && i < len; i++) {
        switch (order[i]) {
            case 'B':
            {
                if (0 == update_mac_config_get_mac_from_efuse(mac_addr)) {
                    set = 1;
                    blog_debug("get MAC from B ready\r\n");
                    goto break_scan;
                } else {
                    blog_debug("get MAC from B failed\r\n");
                }
            }
            break;
            case 'F':
            {
                if (0 == update_mac_config_get_mac_from_dtb(fdt, offset1, mac_addr)) {
                    set = 1;
                    blog_debug("get MAC from F ready\r\n");
                    goto break_scan;
                } else {
                    blog_debug("get MAC from F failed\r\n");
                }
            }
            break;
            case 'M':
            {
                if (0 == update_mac_config_get_mac_from_factory(mac_addr)) {
                    set = 1;
                    blog_debug("get MAC from M ready\r\n");
                    goto break_scan;
                } else {
                    blog_debug("get MAC from M failed\r\n");
                }
            }
            break;
            default:
            {
                BL_ASSERT(0);
            }
        }
    }
break_scan:
    if (0 == set) {
        blog_info("Using Default MAC address\r\n");
        memcpy(mac_addr, mac_default, 8);
    }
    //FIXME maybe we should set a different MAC address
    blog_info("Set MAC addrress %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5],
            mac_addr[6],
            mac_addr[7]
    );
    bl_wireless_mac_addr_set(mac_addr);
}

static void update_mac_config(const void *fdt, int offset1)
{
    int countindex = 0, lentmp = 0;
    const char *result = 0;
    char mac_mode[4];

    countindex = fdt_stringlist_count(fdt, offset1, "mode");
    if (1 == countindex) {
        result = fdt_stringlist_get(fdt, offset1, "mode", 0, &lentmp);
        blog_print("MAC address mode length %d\r\n", lentmp);
        if (lentmp <= MAC_ORDER_ADDR_LEN_MAX) {
            memcpy(mac_mode, result, lentmp);
            mac_mode[3] = '\0';
            blog_print("MAC address mode is %s\r\n", mac_mode);
            update_mac_config_with_order(fdt, offset1, mac_mode);
        }
    }
}

static int hal_board_load_fdt_info(const void *dtb)
{
    const void *fdt = (const void *)dtb;/* const */

    int wireless_offset = 0;    /* subnode wireless */
    int offset1 = 0;        /* subnode offset1 */

    wireless_offset = fdt_subnode_offset(fdt, 0, "wireless");
    if (!(wireless_offset > 0)) {
       blog_error("wireless NULL.\r\n");
    }

    offset1 = fdt_subnode_offset(fdt, wireless_offset, "mac");
    if (offset1 > 0) {
        update_mac_config(fdt, offset1);
    }

    return 0;
}


uint32_t hal_board_get_factory_addr(void)
{
    return factory_addr;
}

int hal_board_cfg(uint8_t board_code)
{
    int ret;
    uint32_t size;

    USER_UNUSED(ret);
    ret = hal_boot2_partition_addr_active("factory", &factory_addr, &size);
    blog_info("[MAIN] [BOARD] [FLASH] addr from partition is %08x, ret is %d\r\n", (unsigned int)factory_addr, ret);
    if (0 == factory_addr) {
        blog_error("[MAIN] [BOARD] [FLASH] Dead loop. Reason: NO valid Param Parition found\r\n");
        while (1) {
        }
    }

    ret = hal_boot2_partition_bus_addr_active("factory", &factory_addr, &size);
    blog_info("[MAIN] [BOARD] [XIP] addr from partition is %08x, ret is %d\r\n", (unsigned int)factory_addr, ret);
    if (0 == factory_addr) {
        blog_error("[MAIN] [BOARD] [XIP] Dead loop. Reason: NO valid Param Parition found\r\n");
        while (1) {
        }
    }

    hal_board_load_fdt_info((const void *)factory_addr);

    return 0;
}
