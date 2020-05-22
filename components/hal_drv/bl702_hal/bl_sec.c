#include <stdio.h>

#include <sec_eng_reg.h>
#include <bl702_sec_eng.h>
#include <aos/kernel.h>

#include "bl_sec.h"
#include "bl_irq.h"

#define REG_VALUE_TRNG_INIT (0x40004200)
#define REG_VALUE_TRNG_VAL  (0x40004208)

#define xstr(a) str_macro(a)
#define str_macro(a) #a
#define TRNG_LOOP_COUNTER   (17)

#define TRNG_SIZE_IN_WORD (8)
#define TRNG_SIZE_IN_BYTES (32)
static uint32_t trng_buffer[TRNG_SIZE_IN_WORD];
static unsigned int trng_idx = 0;

static inline void _trng_trigger()
{
    uint32_t TRNGx = SEC_ENG_BASE + SEC_ENG_TRNG_OFFSET;
    uint32_t val;

    val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0);    
    if (BL_IS_REG_BIT_SET(val, SEC_ENG_SE_TRNG_BUSY)) {
        return;
    }
    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_1, trng_buffer[0]);
    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_2, trng_buffer[1]);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_SET_1T);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_CLR_1T);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_EN);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_TRIG_1T);

    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0, val);
}

static inline void wait_trng4feed()
{
    uint32_t TRNGx = SEC_ENG_BASE + SEC_ENG_TRNG_OFFSET;
    uint32_t val;

    val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0);

    while (BL_IS_REG_BIT_SET(val, SEC_ENG_SE_TRNG_BUSY)) {
        /*wait until trng is NOT busy*/
        val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0);
    }

    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_CLR_1T);
    val = BL_CLR_REG_BIT(val, SEC_ENG_SE_TRNG_TRIG_1T);
    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0, val);

    printf("Feed random number is %08lx\r\n", trng_buffer[0]);
    trng_buffer[0] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_0);
    trng_buffer[1] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_1);
    trng_buffer[2] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_2);
    trng_buffer[3] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_3);
    trng_buffer[4] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_4);
    trng_buffer[5] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_5);
    trng_buffer[6] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_6);
    trng_buffer[7] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_7);
}

uint32_t bl_sec_get_random_word(void)
{
    trng_idx = (trng_idx & 0x7);
    if (0 == trng_idx) {
        _trng_trigger();
    }
    return trng_buffer[trng_idx++];
}

int bl_rand()
{
    unsigned int val;
    int counter = 0;

    do {
        val = bl_sec_get_random_word();
        if ((counter++) > TRNG_LOOP_COUNTER) {
            puts("[BL] [SEC] Failed after loop " xstr(TRNG_LOOP_COUNTER) "\r\n");
            break;
        }
    } while (0 == val);
    val >>= 1;//leave signe bit alone
    return val;
}

void sec_trng_IRQHandler(void)
{
    uint32_t TRNGx = SEC_ENG_BASE + SEC_ENG_TRNG_OFFSET;
    uint32_t val;

    if (aos_now_ms() < 1000 * 2) {
        /*debug when boot*/
        puts("[BL] [SEC] TRNG Handler\r\n");
    }
    val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_CLR_1T);
    val = BL_CLR_REG_BIT(val, SEC_ENG_SE_TRNG_TRIG_1T);
    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL_0, val);

    printf("random number is %08lx\r\n", trng_buffer[0]);
    trng_buffer[0] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_0);
    trng_buffer[1] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_1);
    trng_buffer[2] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_2);
    trng_buffer[3] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_3);
    trng_buffer[4] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_4);
    trng_buffer[5] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_5);
    trng_buffer[6] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_6);
    trng_buffer[7] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_7);
}

int bl_sec_init(void)
{
    _trng_trigger();
    wait_trng4feed();
    /*Trigger again*/
    _trng_trigger();
    wait_trng4feed();
    bl_irq_register(SEC_TRNG_IRQn, sec_trng_IRQHandler);
    bl_irq_enable(SEC_TRNG_IRQn);

    return 0;
}

int bl_exp_mod(uint32_t *src, uint32_t *result, int len, uint32_t *exp, int exp_len, uint32_t *mod, int mod_len)
{
    return 0;
}

int bl_sec_test(void)
{
    puts("------------------TRNG TEST---------------------------------\r\n");
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%08x]**************\r\n", bl_rand());
    puts("------------------------------------------------------------\r\n");

    return 0;
}

void _dump_rsa_data(const uint8_t *data, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        switch (i & 0xF) {
            case 0x0:
            {
                printf("[%04X]:", i);
                printf(" %02X", data[i]);
            }
            break;
            case 0xF:
            {
                printf(" %02X", data[i]);
                puts("\r\n");
            }
            break;
            default:
            {
                printf(" %02X", data[i]);
            }
        }
    }
}

static void RSA_Compare_Data(const uint8_t *expected, const uint8_t *input, uint32_t len)
{
    int i = 0, is_failed = 0;

    for (i = 0; i < len; i++) {
        if (input[i] != expected[i]) {
            is_failed = 1;
            printf("%s[%02d], %02x %02x\r\n",
                input[i] ==expected[i] ? "S" : "F",
                i,
                input[i],
                expected[i]
            );
        }
    }
    if (is_failed) {
        printf("====== Failed %lu Bytes======\r\n", len);
    } else {
        printf("====== Success %lu Bytes=====\r\n", len);
    }
}

static void _pka_test_case2(void)
{
    static const uint8_t n[256] = {
        0xd8, 0xa6, 0x4f, 0xea, 0x28, 0xf9, 0xdf, 0x07, 0x04, 0x55, 0xfa, 0xfb, 0x50, 0x5d, 0xbe, 0xb6,
        0x9f, 0x7b, 0x53, 0x96, 0xef, 0x05, 0x5e, 0x0a, 0xf5, 0x2d, 0xe3, 0x67, 0x78, 0x07, 0x6b, 0xf6, 
        0xb2, 0x17, 0xac, 0x2e, 0x51, 0x42, 0x84, 0xbb, 0xfe, 0x3e, 0x5f, 0x0c, 0x85, 0xc4, 0x9d, 0xd4, 
        0x8b, 0xd5, 0xfa, 0x17, 0x2d, 0xb1, 0x26, 0x81, 0xe7, 0x79, 0x07, 0x45, 0x82, 0x42, 0x22, 0x3d, 
        0x0d, 0x97, 0xcf, 0xde, 0xea, 0xb8, 0xba, 0x16, 0x05, 0x8a, 0x5b, 0x0f, 0xec, 0x07, 0x30, 0xa4, 
        0xc6, 0xbf, 0xff, 0x20, 0x52, 0x1b, 0x94, 0xad, 0xfa, 0xb7, 0x6e, 0x83, 0x14, 0x48, 0x58, 0x14, 
        0x99, 0xe7, 0xa3, 0x9e, 0xc1, 0x08, 0xbd, 0xfe, 0x20, 0x11, 0x56, 0xdb, 0x96, 0x0a, 0xbb, 0x0b, 
        0xbc, 0xd4, 0x37, 0x55, 0xf9, 0x9c, 0x6d, 0x5b, 0x87, 0x4e, 0x50, 0x9f, 0x24, 0x0e, 0x3a, 0x1a, 
        0x0c, 0x54, 0x67, 0xbd, 0x0f, 0x34, 0x03, 0x5e, 0x45, 0x5b, 0x93, 0x42, 0xbe, 0x71, 0xe6, 0xa7, 
        0xf9, 0x49, 0x1a, 0xb3, 0xb2, 0xfb, 0x0e, 0xee, 0x3d, 0xcf, 0x0c, 0x5a, 0xf8, 0xb5, 0x80, 0x42, 
        0x7c, 0x0c, 0x75, 0xc5, 0xe1, 0x17, 0x29, 0x39, 0x55, 0x2b, 0xb1, 0xf5, 0x72, 0x06, 0x9e, 0x54, 
        0x0b, 0x0e, 0xf2, 0x95, 0xc8, 0x5b, 0x69, 0xaf, 0x5b, 0x81, 0x97, 0xae, 0xb1, 0x6e, 0xc4, 0x6d, 
        0x95, 0xd8, 0x22, 0x1e, 0x39, 0xf0, 0x76, 0x54, 0x19, 0x96, 0x03, 0x4c, 0x25, 0x85, 0x2f, 0xe1, 
        0x84, 0xd7, 0xc1, 0x62, 0xe1, 0x9e, 0x9f, 0x1f, 0xd4, 0xb8, 0xf0, 0xc2, 0x68, 0x76, 0x7c, 0xcf, 
        0x43, 0x3e, 0x60, 0x93, 0xd0, 0x89, 0x65, 0xae, 0x72, 0xcd, 0xd6, 0x00, 0x0d, 0x91, 0x42, 0x90,
        0x98, 0x02, 0xa9, 0xf6, 0x82, 0x1b, 0xb5, 0x22, 0xfd, 0xb6, 0xc2, 0x5c, 0xad, 0x86, 0x81, 0x1d,
    };
    static const uint8_t m[256] = {
        0x30, 0x31, 0x36, 0x64, 0x61, 0x34, 0x31, 0x66, 0x34, 0x62, 0x66, 0x35, 0x38, 0x61, 0x36, 0x32,
        0x35, 0x61, 0x61, 0x35, 0x63, 0x33, 0x30, 0x37, 0x62, 0x63, 0x64, 0x31, 0x61, 0x37, 0x35, 0x30,
        0x33, 0x64, 0x62, 0x30, 0x36, 0x63, 0x39, 0x37, 0x62, 0x30, 0x39, 0x31, 0x39, 0x33, 0x38, 0x61,
        0x32, 0x31, 0x62, 0x35, 0x66, 0x36, 0x38, 0x65, 0x33, 0x37, 0x37, 0x61, 0x62, 0x38, 0x39, 0x39,
        0x62, 0x65, 0x66, 0x37, 0x63, 0x61, 0x31, 0x36, 0x35, 0x30, 0x65, 0x38, 0x66, 0x30, 0x38, 0x64,
        0x37, 0x32, 0x38, 0x37, 0x64, 0x64, 0x30, 0x66, 0x36, 0x64, 0x32, 0x61, 0x64, 0x36, 0x34, 0x31,
        0x32, 0x38, 0x38, 0x33, 0x38, 0x63, 0x35, 0x39, 0x35, 0x61, 0x32, 0x64, 0x31, 0x30, 0x65, 0x34,
        0x36, 0x37, 0x61, 0x62, 0x35, 0x34, 0x35, 0x33, 0x63, 0x34, 0x65, 0x63, 0x37, 0x37, 0x30, 0x35,
        0x33, 0x38, 0x61, 0x63, 0x39, 0x66, 0x38, 0x30, 0x36, 0x66, 0x30, 0x38, 0x66, 0x66, 0x33, 0x30,
        0x38, 0x65, 0x36, 0x65, 0x64, 0x62, 0x35, 0x35, 0x34, 0x31, 0x66, 0x39, 0x66, 0x30, 0x34, 0x36,
        0x63, 0x36, 0x37, 0x32, 0x62, 0x31, 0x32, 0x30, 0x37, 0x37, 0x35, 0x35, 0x62, 0x30, 0x35, 0x66,
        0x35, 0x36, 0x64, 0x33, 0x61, 0x36, 0x36, 0x31, 0x37, 0x64, 0x63, 0x37, 0x35, 0x34, 0x64, 0x35,
        0x65, 0x32, 0x30, 0x34, 0x63, 0x31, 0x36, 0x31, 0x36, 0x61, 0x31, 0x33, 0x65, 0x33, 0x62, 0x31,
        0x34, 0x65, 0x38, 0x65, 0x32, 0x39, 0x63, 0x39, 0x35, 0x33, 0x33, 0x38, 0x36, 0x65, 0x65, 0x64,
        0x62, 0x63, 0x30, 0x39, 0x34, 0x30, 0x37, 0x62, 0x39, 0x34, 0x33, 0x34, 0x38, 0x37, 0x37, 0x36,
        0x36, 0x37, 0x63, 0x62, 0x33, 0x30, 0x39, 0x63, 0x36, 0x33, 0x30, 0x34, 0x32, 0x32, 0x36, 0x32, 
    };
    static const uint8_t e[4] = {
        0x00, 0x01, 0x00, 0x01,
    };
    static const uint8_t nprime[256] = {
        0x38, 0x62, 0xc1, 0xf5, 0x55, 0x2d, 0x3d, 0x60, 0x5e, 0x42, 0xe1, 0x65, 0xde, 0xed, 0x35, 0xd5,
        0xc5, 0x85, 0xe4, 0x4e, 0xeb, 0x74, 0xa5, 0x22, 0xb3, 0xed, 0x5f, 0x5b, 0xb1, 0xb9, 0xe9, 0x0a,
        0x7d, 0xa5, 0x74, 0x58, 0xf8, 0xa1, 0xab, 0x17, 0x74, 0xd0, 0x07, 0xa3, 0x7f, 0xd2, 0x9b, 0x50,
        0x2a, 0xed, 0x5e, 0xdc, 0x5a, 0x69, 0xfe, 0x0e, 0xb1, 0xd8, 0x53, 0x35, 0x9b, 0xef, 0x1d, 0x76,
        0x52, 0x9e, 0x87, 0x3c, 0xb0, 0x82, 0x4e, 0x03, 0xdf, 0x75, 0xed, 0x09, 0x9f, 0x3d, 0x37, 0xf6,
        0xe8, 0x0d, 0xc9, 0x2e, 0x81, 0xf2, 0x9d, 0x2e, 0xaa, 0xe6, 0x53, 0x79, 0x6b, 0x99, 0xef, 0x46,
        0x36, 0xd9, 0x2e, 0x9d, 0x15, 0xd1, 0x7f, 0x23, 0x14, 0xb9, 0xeb, 0x33, 0xa7, 0xd4, 0x8e, 0x86,
        0x60, 0xc9, 0xd9, 0x7c, 0xca, 0x54, 0x59, 0x57, 0x94, 0x1e, 0x52, 0x4d, 0xc8, 0x3f, 0x9b, 0x24,
        0x28, 0x25, 0xcb, 0x57, 0xca, 0x8f, 0x16, 0x5a, 0x37, 0xc2, 0xc6, 0xae, 0xc5, 0xe7, 0xc4, 0x2e,
        0xf3, 0x24, 0x1c, 0xb7, 0xe9, 0xf5, 0x92, 0x4e, 0xd4, 0x51, 0x50, 0xff, 0xde, 0x44, 0x3c, 0xae,
        0x72, 0xbd, 0x16, 0x39, 0x63, 0x8a, 0x22, 0x9c, 0x95, 0xda, 0x21, 0xf0, 0x4c, 0x12, 0x36, 0x2d,
        0x00, 0xad, 0xb3, 0x89, 0xb5, 0x09, 0x9e, 0x3d, 0x24, 0x81, 0xfc, 0xef, 0x99, 0x95, 0x22, 0x9d,
        0xb3, 0x94, 0x39, 0x32, 0xdd, 0xc4, 0x2b, 0x2f, 0xb0, 0x13, 0xfe, 0xb5, 0x5e, 0xc7, 0x64, 0x93,
        0x7a, 0xb5, 0x81, 0x93, 0x1f, 0x9f, 0x96, 0x1e, 0x7a, 0x5c, 0x8d, 0xde, 0x8f, 0xae, 0xd9, 0xc8,
        0xdd, 0x35, 0x1e, 0x17, 0x47, 0xb6, 0xab, 0xed, 0xb6, 0x82, 0x22, 0x4c, 0x62, 0xbd, 0x12, 0x4e,
        0x44, 0x5c, 0x48, 0x2b, 0x75, 0x63, 0x1c, 0xde, 0xfa, 0x15, 0x0d, 0xb1, 0x50, 0x31, 0xb6, 0xcb, 
    };
    static const uint8_t inv_r[256] = {
        0x2f, 0xb7, 0xf5, 0x4a, 0xd2, 0x19, 0xde, 0x24, 0x7c, 0xdb, 0xcd, 0x52, 0x6e, 0xbc, 0x2c, 0x5c,
        0x76, 0x9a, 0x36, 0xc3, 0x87, 0x33, 0xf7, 0xe9, 0x3d, 0x5b, 0x3d, 0xcd, 0x33, 0x7a, 0x3b, 0x4e,
        0x55, 0xf5, 0xd9, 0x42, 0x76, 0x63, 0x28, 0x7a, 0xa8, 0x7c, 0xf7, 0xd1, 0xf6, 0x0d, 0x26, 0xba,
        0xbe, 0x9f, 0x35, 0xf4, 0x86, 0xc5, 0x93, 0x4c, 0xe8, 0x76, 0xda, 0x88, 0xb8, 0xbe, 0xad, 0x25,
        0x6b, 0xe7, 0x44, 0x3b, 0x1c, 0x2c, 0x99, 0x15, 0xee, 0x33, 0x46, 0xc6, 0xe0, 0xb0, 0x39, 0x6d,
        0x20, 0xb2, 0x68, 0xc7, 0x75, 0x41, 0x2c, 0xff, 0xcb, 0x93, 0x1d, 0x40, 0xd2, 0x0e, 0x64, 0xea,
        0x2e, 0x0a, 0x55, 0x9f, 0x04, 0x9d, 0xfd, 0x5e, 0x24, 0xa9, 0x28, 0x5c, 0x2d, 0x1b, 0x29, 0x87,
        0x61, 0x6b, 0x50, 0x6a, 0x31, 0x31, 0x43, 0x12, 0x13, 0xe3, 0x1f, 0x47, 0x8a, 0x11, 0xd2, 0x5b,
        0x26, 0x5e, 0x79, 0x04, 0x0b, 0xa8, 0xb0, 0x36, 0x22, 0xda, 0x3c, 0x5e, 0xb9, 0x09, 0x48, 0xb0,
        0x32, 0x38, 0x25, 0xec, 0xfd, 0x5e, 0xef, 0xff, 0x80, 0x33, 0x9f, 0x94, 0x8c, 0x6e, 0x2a, 0xfb,
        0xbf, 0x65, 0x18, 0x98, 0x7e, 0xff, 0x41, 0xde, 0x00, 0x2f, 0xd2, 0x7d, 0xbf, 0x4c, 0x54, 0x4e,
        0x1c, 0x46, 0xd6, 0xab, 0xf6, 0x07, 0x34, 0x63, 0xe3, 0x0b, 0x81, 0xa0, 0x94, 0x7d, 0xaf, 0x7e,
        0x37, 0xd6, 0xc5, 0xa6, 0x4a, 0x90, 0x6c, 0x44, 0x6a, 0xd9, 0x0f, 0x20, 0xb2, 0xef, 0x22, 0xa0,
        0xdf, 0x38, 0x2d, 0x0b, 0xb3, 0x03, 0xb2, 0xc8, 0xe6, 0x8d, 0x74, 0xbf, 0x45, 0x91, 0xe0, 0x22,
        0x16, 0xbf, 0xc4, 0xda, 0x54, 0x26, 0xaa, 0x65, 0x85, 0x88, 0xc3, 0xfb, 0x9f, 0xfc, 0x14, 0xc4,
        0xff, 0x8b, 0x88, 0x47, 0x5f, 0xb1, 0x55, 0xdf, 0x47, 0x5c, 0xc0, 0x27, 0x39, 0x7b, 0xe8, 0xad, 
    };
    uint32_t result[64];
    static const uint8_t encrypted[256] = {
        0x9e, 0xf6, 0x6f, 0x46, 0xf5, 0x51, 0x1a, 0xbc, 0xc2, 0x9c, 0x49, 0x02, 0x21, 0x6c, 0x20, 0xae,
        0x49, 0x91, 0xcd, 0xba, 0xb9, 0x4f, 0xaf, 0xfd, 0x8d, 0x9a, 0x27, 0xbc, 0x0b, 0x69, 0x57, 0xc4,
        0xba, 0x18, 0xe1, 0x56, 0x45, 0x55, 0xbb, 0x3f, 0x7b, 0xca, 0x45, 0xb3, 0x9a, 0x0e, 0xd7, 0x64,
        0x6e, 0x71, 0xce, 0xd3, 0x08, 0xc9, 0x4b, 0x97, 0xab, 0x24, 0xe4, 0x6c, 0xe3, 0xc7, 0x52, 0x97,
        0x3c, 0x45, 0x17, 0x3b, 0x17, 0x0a, 0x90, 0x50, 0xed, 0x73, 0x4b, 0x49, 0x07, 0xee, 0x13, 0xaf,
        0x47, 0x1e, 0xd0, 0x24, 0xb1, 0xd2, 0xc8, 0x09, 0x75, 0xf3, 0x14, 0x9c, 0x71, 0x99, 0xe3, 0x94,
        0x5b, 0xf6, 0xef, 0x2e, 0x79, 0xf5, 0x1d, 0xdc, 0xa7, 0xc5, 0xed, 0x0a, 0x3f, 0x1d, 0x43, 0xd0,
        0x19, 0x14, 0x3a, 0xb7, 0x35, 0xc2, 0x3f, 0xa1, 0x9c, 0x00, 0xde, 0xf6, 0x96, 0x55, 0xf8, 0x0c,
        0x79, 0x08, 0x68, 0xf3, 0x84, 0x7c, 0x2e, 0x0c, 0x51, 0xb6, 0x5e, 0x9e, 0xcd, 0x50, 0xcc, 0x5f,
        0x71, 0x99, 0xc1, 0x0d, 0xf0, 0x3c, 0xd0, 0x80, 0x02, 0xf0, 0x8f, 0x12, 0x3e, 0x49, 0xa4, 0x9b,
        0x1f, 0x14, 0x05, 0xf2, 0x7b, 0x41, 0xc1, 0x3e, 0x8a, 0xb2, 0xab, 0x70, 0x28, 0x2f, 0x20, 0x94,
        0x17, 0x65, 0xf3, 0x89, 0x28, 0x6d, 0xcd, 0x0c, 0xea, 0x03, 0x4a, 0x10, 0x9d, 0xf9, 0x2e, 0xf4,
        0x64, 0x79, 0x7a, 0xec, 0x46, 0xb4, 0xdf, 0xce, 0x6a, 0x8e, 0xd8, 0x35, 0x62, 0xb3, 0x04, 0xea,
        0xf9, 0xc4, 0xde, 0xba, 0x2a, 0x5e, 0xbf, 0x59, 0xfa, 0xef, 0x2a, 0x42, 0x18, 0xc9, 0xf5, 0x7a,
        0x73, 0xb8, 0x67, 0x78, 0x97, 0x6d, 0x75, 0x4b, 0xdd, 0xfb, 0x9b, 0xe6, 0x4c, 0x04, 0x9c, 0x61,
        0x5f, 0x9a, 0x12, 0xbf, 0x2e, 0x75, 0x63, 0xdd, 0x50, 0xba, 0x2c, 0xef, 0xb0, 0x9a, 0x65, 0x24,
    };

    Sec_Eng_PKA_Reset();
    Sec_Eng_PKA_BigEndian_Enable();

    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  0, (uint32_t*)n, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  1, (uint32_t*)nprime, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  2, (uint32_t*)m, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  3, (uint32_t*)e, 1, 0);

    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 4, 0, 0);
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 5, 0, 1);
    Sec_Eng_PKA_LMUL2N(
            SEC_ENG_PKA_REG_SIZE_512, 2,
            SEC_ENG_PKA_REG_SIZE_256, 2,
            2048,
            0
    );
    Sec_Eng_PKA_MREM(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_512, 2,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            0
    );
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_512, 2, 0, 1);

    Sec_Eng_PKA_MEXP(
            SEC_ENG_PKA_REG_SIZE_256, 4,
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_256, 3,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            1
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_256, 4,
            1
    );
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  1, (uint32_t*)inv_r, 64, 0);
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 4, 0, 0);
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 5, 0, 1);
    Sec_Eng_PKA_LMUL(
            SEC_ENG_PKA_REG_SIZE_512, 2,
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_256, 1,
            0
    );
    Sec_Eng_PKA_MREM(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_512, 2,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            0
    );
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_512, 2, 0, 1);
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, sizeof(result));
    RSA_Compare_Data(encrypted, (uint8_t*)result, sizeof(result));
}

static void __attribute__((unused)) dump_xgcd_step(uint32_t result[64])
{
    puts(" ---- PKA 8:0\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 0,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:1\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 1,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:2\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:3\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 3,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:4\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 4,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:5\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 5,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:6\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 6,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);

    puts(" ---- PKA 8:7\r\n");
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 7,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, 256);
}


static void _pka_test_case_xgcd(void)
{
    int count = 0;
    static const uint8_t n[256] = {
        0xd8, 0xa6, 0x4f, 0xea, 0x28, 0xf9, 0xdf, 0x07, 0x04, 0x55, 0xfa, 0xfb, 0x50, 0x5d, 0xbe, 0xb6,
        0x9f, 0x7b, 0x53, 0x96, 0xef, 0x05, 0x5e, 0x0a, 0xf5, 0x2d, 0xe3, 0x67, 0x78, 0x07, 0x6b, 0xf6, 
        0xb2, 0x17, 0xac, 0x2e, 0x51, 0x42, 0x84, 0xbb, 0xfe, 0x3e, 0x5f, 0x0c, 0x85, 0xc4, 0x9d, 0xd4, 
        0x8b, 0xd5, 0xfa, 0x17, 0x2d, 0xb1, 0x26, 0x81, 0xe7, 0x79, 0x07, 0x45, 0x82, 0x42, 0x22, 0x3d, 
        0x0d, 0x97, 0xcf, 0xde, 0xea, 0xb8, 0xba, 0x16, 0x05, 0x8a, 0x5b, 0x0f, 0xec, 0x07, 0x30, 0xa4, 
        0xc6, 0xbf, 0xff, 0x20, 0x52, 0x1b, 0x94, 0xad, 0xfa, 0xb7, 0x6e, 0x83, 0x14, 0x48, 0x58, 0x14, 
        0x99, 0xe7, 0xa3, 0x9e, 0xc1, 0x08, 0xbd, 0xfe, 0x20, 0x11, 0x56, 0xdb, 0x96, 0x0a, 0xbb, 0x0b, 
        0xbc, 0xd4, 0x37, 0x55, 0xf9, 0x9c, 0x6d, 0x5b, 0x87, 0x4e, 0x50, 0x9f, 0x24, 0x0e, 0x3a, 0x1a, 
        0x0c, 0x54, 0x67, 0xbd, 0x0f, 0x34, 0x03, 0x5e, 0x45, 0x5b, 0x93, 0x42, 0xbe, 0x71, 0xe6, 0xa7, 
        0xf9, 0x49, 0x1a, 0xb3, 0xb2, 0xfb, 0x0e, 0xee, 0x3d, 0xcf, 0x0c, 0x5a, 0xf8, 0xb5, 0x80, 0x42, 
        0x7c, 0x0c, 0x75, 0xc5, 0xe1, 0x17, 0x29, 0x39, 0x55, 0x2b, 0xb1, 0xf5, 0x72, 0x06, 0x9e, 0x54, 
        0x0b, 0x0e, 0xf2, 0x95, 0xc8, 0x5b, 0x69, 0xaf, 0x5b, 0x81, 0x97, 0xae, 0xb1, 0x6e, 0xc4, 0x6d, 
        0x95, 0xd8, 0x22, 0x1e, 0x39, 0xf0, 0x76, 0x54, 0x19, 0x96, 0x03, 0x4c, 0x25, 0x85, 0x2f, 0xe1, 
        0x84, 0xd7, 0xc1, 0x62, 0xe1, 0x9e, 0x9f, 0x1f, 0xd4, 0xb8, 0xf0, 0xc2, 0x68, 0x76, 0x7c, 0xcf, 
        0x43, 0x3e, 0x60, 0x93, 0xd0, 0x89, 0x65, 0xae, 0x72, 0xcd, 0xd6, 0x00, 0x0d, 0x91, 0x42, 0x90,
        0x98, 0x02, 0xa9, 0xf6, 0x82, 0x1b, 0xb5, 0x22, 0xfd, 0xb6, 0xc2, 0x5c, 0xad, 0x86, 0x81, 0x1d,
    };
#if 0
    static const uint8_t nprime[256] = {
        0x38, 0x62, 0xc1, 0xf5, 0x55, 0x2d, 0x3d, 0x60, 0x5e, 0x42, 0xe1, 0x65, 0xde, 0xed, 0x35, 0xd5,
        0xc5, 0x85, 0xe4, 0x4e, 0xeb, 0x74, 0xa5, 0x22, 0xb3, 0xed, 0x5f, 0x5b, 0xb1, 0xb9, 0xe9, 0x0a,
        0x7d, 0xa5, 0x74, 0x58, 0xf8, 0xa1, 0xab, 0x17, 0x74, 0xd0, 0x07, 0xa3, 0x7f, 0xd2, 0x9b, 0x50,
        0x2a, 0xed, 0x5e, 0xdc, 0x5a, 0x69, 0xfe, 0x0e, 0xb1, 0xd8, 0x53, 0x35, 0x9b, 0xef, 0x1d, 0x76,
        0x52, 0x9e, 0x87, 0x3c, 0xb0, 0x82, 0x4e, 0x03, 0xdf, 0x75, 0xed, 0x09, 0x9f, 0x3d, 0x37, 0xf6,
        0xe8, 0x0d, 0xc9, 0x2e, 0x81, 0xf2, 0x9d, 0x2e, 0xaa, 0xe6, 0x53, 0x79, 0x6b, 0x99, 0xef, 0x46,
        0x36, 0xd9, 0x2e, 0x9d, 0x15, 0xd1, 0x7f, 0x23, 0x14, 0xb9, 0xeb, 0x33, 0xa7, 0xd4, 0x8e, 0x86,
        0x60, 0xc9, 0xd9, 0x7c, 0xca, 0x54, 0x59, 0x57, 0x94, 0x1e, 0x52, 0x4d, 0xc8, 0x3f, 0x9b, 0x24,
        0x28, 0x25, 0xcb, 0x57, 0xca, 0x8f, 0x16, 0x5a, 0x37, 0xc2, 0xc6, 0xae, 0xc5, 0xe7, 0xc4, 0x2e,
        0xf3, 0x24, 0x1c, 0xb7, 0xe9, 0xf5, 0x92, 0x4e, 0xd4, 0x51, 0x50, 0xff, 0xde, 0x44, 0x3c, 0xae,
        0x72, 0xbd, 0x16, 0x39, 0x63, 0x8a, 0x22, 0x9c, 0x95, 0xda, 0x21, 0xf0, 0x4c, 0x12, 0x36, 0x2d,
        0x00, 0xad, 0xb3, 0x89, 0xb5, 0x09, 0x9e, 0x3d, 0x24, 0x81, 0xfc, 0xef, 0x99, 0x95, 0x22, 0x9d,
        0xb3, 0x94, 0x39, 0x32, 0xdd, 0xc4, 0x2b, 0x2f, 0xb0, 0x13, 0xfe, 0xb5, 0x5e, 0xc7, 0x64, 0x93,
        0x7a, 0xb5, 0x81, 0x93, 0x1f, 0x9f, 0x96, 0x1e, 0x7a, 0x5c, 0x8d, 0xde, 0x8f, 0xae, 0xd9, 0xc8,
        0xdd, 0x35, 0x1e, 0x17, 0x47, 0xb6, 0xab, 0xed, 0xb6, 0x82, 0x22, 0x4c, 0x62, 0xbd, 0x12, 0x4e,
        0x44, 0x5c, 0x48, 0x2b, 0x75, 0x63, 0x1c, 0xde, 0xfa, 0x15, 0x0d, 0xb1, 0x50, 0x31, 0xb6, 0xcb, 
    };
    static const uint8_t inv_r[256] = {
        0x2f, 0xb7, 0xf5, 0x4a, 0xd2, 0x19, 0xde, 0x24, 0x7c, 0xdb, 0xcd, 0x52, 0x6e, 0xbc, 0x2c, 0x5c,
        0x76, 0x9a, 0x36, 0xc3, 0x87, 0x33, 0xf7, 0xe9, 0x3d, 0x5b, 0x3d, 0xcd, 0x33, 0x7a, 0x3b, 0x4e,
        0x55, 0xf5, 0xd9, 0x42, 0x76, 0x63, 0x28, 0x7a, 0xa8, 0x7c, 0xf7, 0xd1, 0xf6, 0x0d, 0x26, 0xba,
        0xbe, 0x9f, 0x35, 0xf4, 0x86, 0xc5, 0x93, 0x4c, 0xe8, 0x76, 0xda, 0x88, 0xb8, 0xbe, 0xad, 0x25,
        0x6b, 0xe7, 0x44, 0x3b, 0x1c, 0x2c, 0x99, 0x15, 0xee, 0x33, 0x46, 0xc6, 0xe0, 0xb0, 0x39, 0x6d,
        0x20, 0xb2, 0x68, 0xc7, 0x75, 0x41, 0x2c, 0xff, 0xcb, 0x93, 0x1d, 0x40, 0xd2, 0x0e, 0x64, 0xea,
        0x2e, 0x0a, 0x55, 0x9f, 0x04, 0x9d, 0xfd, 0x5e, 0x24, 0xa9, 0x28, 0x5c, 0x2d, 0x1b, 0x29, 0x87,
        0x61, 0x6b, 0x50, 0x6a, 0x31, 0x31, 0x43, 0x12, 0x13, 0xe3, 0x1f, 0x47, 0x8a, 0x11, 0xd2, 0x5b,
        0x26, 0x5e, 0x79, 0x04, 0x0b, 0xa8, 0xb0, 0x36, 0x22, 0xda, 0x3c, 0x5e, 0xb9, 0x09, 0x48, 0xb0,
        0x32, 0x38, 0x25, 0xec, 0xfd, 0x5e, 0xef, 0xff, 0x80, 0x33, 0x9f, 0x94, 0x8c, 0x6e, 0x2a, 0xfb,
        0xbf, 0x65, 0x18, 0x98, 0x7e, 0xff, 0x41, 0xde, 0x00, 0x2f, 0xd2, 0x7d, 0xbf, 0x4c, 0x54, 0x4e,
        0x1c, 0x46, 0xd6, 0xab, 0xf6, 0x07, 0x34, 0x63, 0xe3, 0x0b, 0x81, 0xa0, 0x94, 0x7d, 0xaf, 0x7e,
        0x37, 0xd6, 0xc5, 0xa6, 0x4a, 0x90, 0x6c, 0x44, 0x6a, 0xd9, 0x0f, 0x20, 0xb2, 0xef, 0x22, 0xa0,
        0xdf, 0x38, 0x2d, 0x0b, 0xb3, 0x03, 0xb2, 0xc8, 0xe6, 0x8d, 0x74, 0xbf, 0x45, 0x91, 0xe0, 0x22,
        0x16, 0xbf, 0xc4, 0xda, 0x54, 0x26, 0xaa, 0x65, 0x85, 0x88, 0xc3, 0xfb, 0x9f, 0xfc, 0x14, 0xc4,
        0xff, 0x8b, 0x88, 0x47, 0x5f, 0xb1, 0x55, 0xdf, 0x47, 0x5c, 0xc0, 0x27, 0x39, 0x7b, 0xe8, 0xad, 
    };
#endif
    static const uint8_t n_exp[256] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
    };
    static const uint8_t all_zero[256] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    };
    uint32_t result[64];
    uint8_t pka_a_eq_0 = 0;


    (void) count;
    Sec_Eng_PKA_Reset();
    Sec_Eng_PKA_BigEndian_Enable();

    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  1, (uint32_t*)n, 64, 0);
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 2, 0, 0);
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_256, 3, 0, 1);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  0, (uint32_t*)n_exp, 64, 0);


    Sec_Eng_PKA_LMUL2N(
            SEC_ENG_PKA_REG_SIZE_512, 1,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            2048,
            0
    );
    Sec_Eng_PKA_LDIV(
            SEC_ENG_PKA_REG_SIZE_256, 0,
            SEC_ENG_PKA_REG_SIZE_512, 1,
            SEC_ENG_PKA_REG_SIZE_256, 1,
            0
    );
    Sec_Eng_PKA_MREM(
            SEC_ENG_PKA_REG_SIZE_256, 4,
            SEC_ENG_PKA_REG_SIZE_512, 1,
            SEC_ENG_PKA_REG_SIZE_256, 1,
            0
    );
    Sec_Eng_PKA_CREG(SEC_ENG_PKA_REG_SIZE_512, 1, 0, 1);
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 2,
            SEC_ENG_PKA_REG_SIZE_256, 1,
            0
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 1,
            SEC_ENG_PKA_REG_SIZE_256, 4,
            1
    );
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  4, (uint32_t*)all_zero, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  5, (uint32_t*)n_exp, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  6, (uint32_t*)n_exp, 64, 0);
    Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  7, (uint32_t*)all_zero, 64, 0);

    Sec_Eng_PKA_LMUL(
            SEC_ENG_PKA_REG_SIZE_256, 8,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            SEC_ENG_PKA_REG_SIZE_256, 5,
            0
    );
    Sec_Eng_PKA_LSUB(
            SEC_ENG_PKA_REG_SIZE_256, 8,
            SEC_ENG_PKA_REG_SIZE_256, 4,
            SEC_ENG_PKA_REG_SIZE_256, 8,
            0
    );
    Sec_Eng_PKA_LMUL(
            SEC_ENG_PKA_REG_SIZE_256, 9,
            SEC_ENG_PKA_REG_SIZE_256, 0,
            SEC_ENG_PKA_REG_SIZE_256, 7,
            0
    );
    Sec_Eng_PKA_LSUB(
            SEC_ENG_PKA_REG_SIZE_256, 9,
            SEC_ENG_PKA_REG_SIZE_256, 6,
            SEC_ENG_PKA_REG_SIZE_256, 9,
            0
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 4,
            SEC_ENG_PKA_REG_SIZE_256, 5,
            0
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 5,
            SEC_ENG_PKA_REG_SIZE_256, 8,
            0
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 6,
            SEC_ENG_PKA_REG_SIZE_256, 7,
            0
    );
    Sec_Eng_PKA_Move_Data(
            SEC_ENG_PKA_REG_SIZE_256, 7,
            SEC_ENG_PKA_REG_SIZE_256, 9,
            1
    );

#if 0
    printf("Dumping Step count %d\r\n", count++);
    dump_xgcd_step(result);
#endif
    while (!pka_a_eq_0) {
        Sec_Eng_PKA_LDIV(
                SEC_ENG_PKA_REG_SIZE_256, 0,
                SEC_ENG_PKA_REG_SIZE_256, 2,
                SEC_ENG_PKA_REG_SIZE_256, 1,
                0
        );
        Sec_Eng_PKA_MREM(
                SEC_ENG_PKA_REG_SIZE_256, 3,
                SEC_ENG_PKA_REG_SIZE_256, 2,
                SEC_ENG_PKA_REG_SIZE_256, 1,
                0
        );
        Sec_Eng_PKA_LMUL(
                SEC_ENG_PKA_REG_SIZE_256, 8,
                SEC_ENG_PKA_REG_SIZE_256, 0,
                SEC_ENG_PKA_REG_SIZE_256, 5,
                0
        );
        Sec_Eng_PKA_LSUB(
                SEC_ENG_PKA_REG_SIZE_256, 8,
                SEC_ENG_PKA_REG_SIZE_256, 4,
                SEC_ENG_PKA_REG_SIZE_256, 8,
                0
        );
        Sec_Eng_PKA_LMUL(
                SEC_ENG_PKA_REG_SIZE_256, 9,
                SEC_ENG_PKA_REG_SIZE_256, 0,
                SEC_ENG_PKA_REG_SIZE_256, 7,
                0
        );
        Sec_Eng_PKA_LSUB(
                SEC_ENG_PKA_REG_SIZE_256, 9,
                SEC_ENG_PKA_REG_SIZE_256, 6,
                SEC_ENG_PKA_REG_SIZE_256, 9,
                0
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 2,
                SEC_ENG_PKA_REG_SIZE_256, 1,
                0
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 4,
                SEC_ENG_PKA_REG_SIZE_256, 5,
                0
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 5,
                SEC_ENG_PKA_REG_SIZE_256, 8,
                0
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 6,
                SEC_ENG_PKA_REG_SIZE_256, 7,
                0
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 7,
                SEC_ENG_PKA_REG_SIZE_256, 9,
                1
        );
        Sec_Eng_PKA_Move_Data(
                SEC_ENG_PKA_REG_SIZE_256, 1,
                SEC_ENG_PKA_REG_SIZE_256, 3,
                1
        );
        Sec_Eng_PKA_Write_Data(SEC_ENG_PKA_REG_SIZE_256,  10, (uint32_t*)n_exp, 64, 0);
        Sec_Eng_PKA_LCMP(
                &pka_a_eq_0,
                SEC_ENG_PKA_REG_SIZE_256, 1,
                SEC_ENG_PKA_REG_SIZE_256, 10
        );
#if 0
        printf("Dumping Step count %d\r\n", count++);
        dump_xgcd_step(result);
#endif
    }
    Sec_Eng_PKA_Read_Data(
            SEC_ENG_PKA_REG_SIZE_256, 6,
            result,
            64
    );
    _dump_rsa_data((uint8_t*)result, sizeof(result));
    //RSA_Compare_Data(encrypted, (uint8_t*)result, sizeof(result));
}

int bl_pka_test(void)
{
    bl_irq_register(SEC_PKA_IRQn, bl_sec_pka_IRQHandler);
    bl_irq_enable(SEC_PKA_IRQn);

    _pka_test_case2();
    _pka_test_case_xgcd();
    _pka_test_case2();

    return 0;
}

void bl_sec_pka_IRQHandler(void)
{
    puts("--->>> PKA IRQ\r\n");
    SEC_Eng_IntMask(SEC_ENG_INT_PKA, MASK);
}
