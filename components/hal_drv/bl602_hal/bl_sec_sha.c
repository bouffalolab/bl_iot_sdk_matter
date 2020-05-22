#include <stdio.h>

#include <bl602_sec_eng.h>

#include "bl_irq.h"
#include "bl_sec.h"

typedef struct sha256_link_item {
    SEC_Eng_SHA256_Link_Ctx ctx;
    SEC_Eng_SHA_Link_Config_Type linkCfg;
    uint32_t tmp[16];
    uint32_t pad[16];
} sha256_link_item_t;

static const uint8_t shaSrcBuf1[64] =
{
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
};

static void SHA_Compare_Data(const uint8_t *expected, const uint8_t *input, uint32_t len)
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

void sha256_test_case0(void)
{
    SEC_ENG_SHA_ID_Type shaId = SEC_ENG_SHA_ID0;
    sha256_link_item_t sha256_link = {
        .linkCfg.shaMode = SEC_ENG_SHA256,
        .linkCfg.shaHashSel = 0,
        .linkCfg.shaIntClr = 0,
        .linkCfg.shaIntSet = 1,
        .linkCfg.shaMsgLen = 1,
        .linkCfg.shaSrcAddr = 0x50020000,
    };
    static const uint8_t sha256_test_result[] = 
    {
        0x31, 0x38, 0xbb, 0x9b, 0xc7, 0x8d, 0xf2, 0x7c, 0x47, 0x3e, 0xcf, 0xd1, 0x41, 0x0f, 0x7b, 0xd4,
        0x5e, 0xba, 0xc1, 0xf5, 0x9c, 0xf3, 0xff, 0x9c, 0xfe, 0x4d, 0xb7, 0x7a, 0xab, 0x7a, 0xed, 0xd3, 
    };


    bl_irq_register(SEC_SHA_IRQn, bl_sec_sha_IRQHandler);
    bl_irq_enable(SEC_SHA_IRQn);

    Sec_Eng_SHA_Enable_Link(shaId);
    Sec_Eng_SHA256_Link_Init(&sha256_link.ctx, shaId,
            (uint32_t)&sha256_link.linkCfg,
            sha256_link.tmp,
            sha256_link.pad
    );
    Sec_Eng_SHA256_Link_Update(&sha256_link.ctx, shaId,
            shaSrcBuf1,
            64
    );
    //FIXME Request to change driver API
    Sec_Eng_SHA256_Link_Finish(&sha256_link.ctx, shaId, (uint8_t*)sha256_link.linkCfg.result);
    Sec_Eng_SHA_Disable_Link(shaId);

    SHA_Compare_Data((const uint8_t*)sha256_link.linkCfg.result, sha256_test_result, sizeof(sha256_test_result));
}

int bl_sec_sha_test(void)
{
    puts("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
    puts("^^^^^^^^^^^^^^^^^^^^^^^SHA256 TEST CASE^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
    sha256_test_case0();
    puts("------------------------------------------------------------------------------------\r\n");

    return 0;
}
static void _clear_sha_int()
{
    uint32_t SHAx = SEC_ENG_BASE;
    uint32_t val;

    val = BL_RD_REG(SHAx, SEC_ENG_SE_SHA_0_CTRL);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_SHA_0_INT_CLR_1T);
    BL_WR_REG(SHAx, SEC_ENG_SE_SHA_0_CTRL, val);
}

void bl_sec_sha_IRQHandler(void)
{
    puts("--->>> SHA IRQ\r\n");
    _clear_sha_int();
}
