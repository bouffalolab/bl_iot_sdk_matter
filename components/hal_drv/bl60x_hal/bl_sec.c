/**
 ****************************************************************************************
 *
 * @file bl_sec.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <bl60x_sec_eng.h>
#include <aos/kernel.h>

#include "bl_sec.h"

#define xstr(a) str_macro(a)
#define str_macro(a) #a
#define TRNG_LOOP_COUNTER   (17)

//#define ENABLE_AES_SEC     1


/*
 * Note:
 * for BL60x TRNG to generate number, 86T bus clock is needed, 391T clock is needed
 * for the first time.
 */

#ifdef ENABLE_AES_SEC
uint8_t aesTmpBuf1[16];
uint8_t aesTmpBuf2[16];

static uint32_t Aes_Compare_Data(uint8_t *expected,uint8_t *input,uint32_t len)
{
    int i=0;
    for(i=0; i<len; i++){
        if(input[i]!=expected[i]){
            printf("Compare fail at %d,input %02x, but expect %02x\r\n",i,input[i],expected[i]);
        }
    }

    return 0;
}

static uint8_t aesTestECB128Key[3][32] =
{
    { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
      0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
};

static uint8_t aesTestECB128Iv[3][16] =
{
    { 0 },
};

static uint8_t aesIVZero[16] = {
    0
}
;

static uint8_t aesTestECB128Pt[3][48] =
{
    { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
      0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 },
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
};

static uint8_t aesTestECB128Ct[3][48] =
{
    { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xDc, 0x09, 0xfb,
      0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32 },
    { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
      0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a },
};

static uint8_t aesTestECB128TextLen[3] =
{
   16,16,0
};

int Sec_Eng_AES_Deal_One_Case(SEC_ENG_AES_ID_Type aesId,uint8_t *aesKey,uint8_t *aesIv,uint8_t *aesPt,
                        uint8_t *aesCt,uint32_t textLen,SEC_ENG_AES_Type aesType,SEC_ENG_AES_Key_Type keyType)
{
    uint32_t time_irq_start, time_irq_end;

    if(textLen==0){
        return 0;
    }
    
    printf("Encryption\r\n");
    
    time_irq_start = *(volatile uint32_t*)0x60B080A4;
    Sec_Eng_AES_Init(aesId,aesType,keyType,SEC_ENG_AES_ENCRYPTION);
    Sec_Eng_AES_Set_Key_IV_BE(aesId,SEC_ENG_AES_KEY_SW,aesKey,aesIv);
    /* Move to OCRAM buffer */
    memcpy(aesTmpBuf1,aesPt,textLen);
    Sec_Eng_AES_Crypt(aesId,aesTmpBuf1, textLen,aesTmpBuf2);
    Sec_Eng_AES_Finish(aesId);
    time_irq_end = *(volatile uint32_t*)0x60B080A4;
    printf("[ECB] Enc time %u\r\n", time_irq_end - time_irq_start);
    Aes_Compare_Data(aesCt,aesTmpBuf2,textLen);

    printf("Decryption\n");
    time_irq_start = *(volatile uint32_t*)0x60B080A4;
    Sec_Eng_AES_Init(aesId,aesType,keyType,SEC_ENG_AES_DECRYPTION);
    Sec_Eng_AES_Set_Key_IV_BE(aesId,SEC_ENG_AES_KEY_SW,aesKey,aesIv);
    /* Move to OCRAM buffer */
    memcpy(aesTmpBuf1,aesCt,textLen);
    Sec_Eng_AES_Crypt(aesId,aesTmpBuf1, textLen,aesTmpBuf2);
    Sec_Eng_AES_Finish(aesId);
    time_irq_end = *(volatile uint32_t*)0x60B080A4;
    printf("[ECB] Dec time %u\r\n", time_irq_end - time_irq_start);
    Aes_Compare_Data(aesPt,aesTmpBuf2,textLen);
    
    return SUCCESS;
}

static int Sec_Eng_AES_Case(SEC_ENG_AES_ID_Type aesId)
{
    uint32_t i;
    
    printf("Sec Eng AES %d\r\n",aesId);
    
    printf("ECB-128\r\n");
    for(i=0; i<3; i++){
        Sec_Eng_AES_Deal_One_Case(aesId,aesTestECB128Key[i],aesTestECB128Iv[i],
                                    aesTestECB128Pt[i],aesTestECB128Ct[i],
                                    aesTestECB128TextLen[i],SEC_ENG_AES_ECB,
                                    SEC_ENG_AES_KEY_128BITS);
    }
    return SUCCESS;
}

int bl_sec_aes_enc(uint8_t *key, int keysize, uint8_t *input, uint8_t *output)
{
    uint32_t buffer[4];

    //TODO lock protect
    //buffer must in OCRAM, which DMA able
    memcpy(buffer, input, sizeof(buffer));

    Sec_Eng_AES_Init(
            SEC_ENG_AES_ID0,
            SEC_ENG_AES_ECB, 
            (128 == keysize) ? SEC_ENG_AES_KEY_128BITS : 
                ((192 == keysize) ? SEC_ENG_AES_KEY_192BITS : SEC_ENG_AES_KEY_256BITS),
            SEC_ENG_AES_ENCRYPTION
    );
    Sec_Eng_AES_Set_Key_IV_BE(SEC_ENG_AES_ID0, SEC_ENG_AES_KEY_SW, key, aesIVZero);
    Sec_Eng_AES_Crypt(SEC_ENG_AES_ID0, (uint8_t*)buffer, 16, (uint8_t*)buffer);
    Sec_Eng_AES_Finish(SEC_ENG_AES_ID0);
    memcpy(output, buffer, sizeof(buffer));

    return 0;
}
#endif

#define TRNG_SIZE_IN_WORD (8)
#define TRNG_SIZE_IN_BYTES (32)
static uint32_t trng_buffer[TRNG_SIZE_IN_WORD];
static unsigned int trng_idx = 0;

void sec_trng_IRQHandler(void)
{
    uint32_t TRNGx = SEC_ENG_BASE + SEC_ENG_TRNG_OFFSET;
    uint32_t val;

    if (aos_now_ms() < 1000 * 2) {
        /*debug when boot*/
        printf("[BL] [SEC] TRNG Handler\r\n");
    }
    val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_CLR);
    val = BL_CLR_REG_BIT(val, SEC_ENG_SE_TRNG_TRIG);
    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL, val);

    trng_buffer[0] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_0);
    trng_buffer[1] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_1);
    trng_buffer[2] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_2);
    trng_buffer[3] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_3);
    trng_buffer[4] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_4);
    trng_buffer[5] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_5);
    trng_buffer[6] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_6);
    trng_buffer[7] = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_DOUT_7);
}

static inline void _trng_trigger()
{
    uint32_t TRNGx = SEC_ENG_BASE + SEC_ENG_TRNG_OFFSET;
    uint32_t val;

    val = BL_RD_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL);    
    if (BL_IS_REG_BIT_SET(val, SEC_ENG_SE_TRNG_BUSY)) {
        return;
    }
    val = BL_SET_REG_BITS_VAL(val, SEC_ENG_SE_TRNG_RESEED_N, trng_buffer[0]);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_INT_SET);
    val = BL_CLR_REG_BIT(val, SEC_ENG_SE_TRNG_INT_CLR);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_EN);
    val = BL_SET_REG_BIT(val, SEC_ENG_SE_TRNG_TRIG);

    BL_WR_REG(TRNGx, SEC_ENG_SE_TRNG_CTRL, val);
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

int bl_sec_init(void)
{
    _trng_trigger();
    NVIC_SetPriority((IRQn_Type)SEC_TRNG_IRQn, 0);
    NVIC_EnableIRQ((IRQn_Type)SEC_TRNG_IRQn);
    return 0;
}

int bl_sec_test(void)
{
#ifdef ENABLE_AES_SEC
    printf("**********AES SPEED TEST*********\r\n");
    Sec_Eng_NP_Release_AES_Access();
    Sec_Eng_AP_Release_AES_Access();
    Sec_Eng_NP_Request_AES_Access();
    Sec_Eng_AES_Enable_BE(SEC_ENG_AES_ID0);
    Sec_Eng_AES_Case(SEC_ENG_AES_ID0);
    printf("##########AES SPEED TEST#########\r\n");
#endif
    printf("---------------------------------\r\n");
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("**********TRNG TEST rand[%x]**************\r\n", bl_rand());
    printf("##########TRNG TEST rand[%x]##############\r\n", bl_rand());

    return 0;
}
