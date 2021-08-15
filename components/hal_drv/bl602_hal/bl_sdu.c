//#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
#include <stdint.h>
#include <bl602_glb.h>
#include <bl602.h>
#include <bl602_sdu.h>
#include <bl_irq.h>
#include <platform_device.h>
#include <bl_sdu.h>
#include <blog.h>
#include <bl_timer.h>
#include <utils_log.h>

#if 1
#undef log_debug
#undef log_warn
#undef log_error
#define log_debug(...) do {}while(0)
#define log_warn(...) do {}while(0)
#define log_error(...) do {}while(0)
#endif

#define SDU_INT_HOST    0

/*SDIO CLK*/
#define BL_BOOT_SDIO_CLK0_GPIO            GLB_GPIO_PIN_0
#define BL_BOOT_SDIO_CLK0_INPUT_FUN       GPIO0_FUN_SWGPIO_0
#define BL_BOOT_SDIO_CLK0_NORMAL_FUN      GPIO0_FUN_SDIO_CLK
/*SDIO CMD*/
#define BL_BOOT_SDIO_CMD0_GPIO            GLB_GPIO_PIN_1
#define BL_BOOT_SDIO_CMD0_INPUT_FUN       GPIO1_FUN_SWGPIO_1
#define BL_BOOT_SDIO_CMD0_NORMAL_FUN      GPIO1_FUN_SDIO_CMD
/*SDIO DATA0*/
#define BL_BOOT_SDIO_DATA00_GPIO          GLB_GPIO_PIN_2
#define BL_BOOT_SDIO_DATA00_INPUT_FUN     GPIO1_FUN_SWGPIO_1
#define BL_BOOT_SDIO_DATA00_NORMAL_FUN    GPIO2_FUN_SDIO_DAT0
#define BL_BOOT_SDIO_DATA10_GPIO          GLB_GPIO_PIN_3
#define BL_BOOT_SDIO_DATA10_INPUT_FUN     GPIO3_FUN_SWGPIO_3
#define BL_BOOT_SDIO_DATA10_NORMAL_FUN    GPIO3_FUN_SDIO_DAT1
#define BL_BOOT_SDIO_DATA20_GPIO          GLB_GPIO_PIN_4
#define BL_BOOT_SDIO_DATA20_INPUT_FUN     GPIO4_FUN_SWGPIO_4
#define BL_BOOT_SDIO_DATA20_NORMAL_FUN    GPIO4_FUN_SDIO_DAT2
#define BL_BOOT_SDIO_DATA30_GPIO          GLB_GPIO_PIN_5
#define BL_BOOT_SDIO_DATA30_INPUT_FUN     GPIO5_FUN_SWGPIO_5
#define BL_BOOT_SDIO_DATA30_NORMAL_FUN    GPIO5_FUN_SDIO_DAT3

#define WL_REGS8(x)     	(*(volatile unsigned char *)(x))
#define SDIO_RX_BUF_SIZE	2048
#define SDIO_TX_BUF_SIZE	2048
#define SDIO_MAX_PORT_NUM   1
#define SDU_INT_HOST        0
#define SDU_SEND_TIME_OUT   100

uint32_t bootrom_readbuf[2][(1024 * 4 + 3) / 4];
uint32_t bootrom_cmd_ack_buf[16];
volatile uint32_t rx_buf_index=0;
bl_sdio_read_cb_t gl_read_callback = NULL;
#if 0
uint8_t __attribute__((aligned(4))) gbuf[2050];
int g_data_flag = 1;
#endif

/* Check whether rx buf has been attached to one port by corresponding bit */
static volatile uint16_t sdu_rx_buf_attached[NUM_FUNC];

void sdu_interrupt_entry(void);


static void sdu_attach_rx_bufs(uint8_t fn_num,uint8_t port_num)
{
    /* use only one port */
    bootrom_readbuf[rx_buf_index%2][0]=0xffffffff;
    SdioFuncReg[fn_num]->WrIdx = port_num;
    SdioFuncReg[fn_num]->SqWriteBase = (uint32_t)&bootrom_readbuf[rx_buf_index%2];
    SdioFuncReg[fn_num]->WrBitMap = (1<<port_num);
    sdu_rx_buf_attached[fn_num] |= (1<<port_num);

#if SDU_INT_HOST
    /* Generate host sdio interrupt */
    sdio_GEN_CARD2HOST_INT(fn_num,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
#endif
}

uint8_t sdu_read_s_reg(uint32_t offset)
{
    return WL_REGS8(BL_FUNC_SCRATCH_BASE + offset);
}

void sdu_write_s_reg(uint32_t offset, const uint8_t val)
{
    WL_REGS8(BL_FUNC_SCRATCH_BASE + offset) = val;
}

int32_t sdu_host_check(void)
{
    uint16_t blockSize;
    //uint8_t powerUp = WL_REGS8(BL_FUNC_SCRATCH_BASE);

    //WL_REGS8(BL_SDIO_HS_SUPPORT)=(WL_REGS8(BL_SDIO_HS_SUPPORT)&0xfe);
    /* Wait for host driver to set register 0x60 of func1 as 1 */
    //if(!powerUp){//read register, if val is 1, power up, shake hands success.
    //	return -1;
    //}
    printf("SDIO Host write\r\n");
    WL_REGS8(BL_FUNC_SCRATCH_BASE)=0;    //clear power value;
    blockSize = WL_REGS8(SDIO_FN1_BLK_SIZE_0); //read reg, get blockSize0
    blockSize |= ((WL_REGS8(SDIO_FN1_BLK_SIZE_1) &
                   SDIO_FN1_BLK_SIZE_1_MASK ) << 8);

    SdioFuncReg[FUNC_WIFI]->RdBitMap = 0x0;
    SdioFuncReg[FUNC_WIFI]->WrBitMap = 0x0;//reset rd and wr bitmap
    /* toggle SDIO_CCR_CIC_DnLdOvr on WL_SDIO_CCR_CARD_INT_CAUSE */
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = SDIO_CCR_CIC_DnLdOvr;//?
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = 0;///clear int

    //if (!flag_mport[FUNC_WIFI]){
        // It's not needed to enable the registers if only using single port
        //Should the host instruct what to set?
        SdioFuncReg[FUNC_WIFI]->Config2 |= CONFIG2_MSK ;
        /* Enable Bit 4 in config register, to receive interrupt as
           soon as the aggregation/deaggregation is complete for a single
           packet instead of waiting for the complete aggregation/deaggration
           to complete.
        */
        SdioFuncReg[FUNC_WIFI]->Config |= 0x00000010;
    //}

    /* unmask the interrupts */
    //SdioFuncReg[FUNC_WIFI]->CardIntMask = SDIO_CCR_CIM_MASK;
    SdioFuncReg[FUNC_WIFI]->CardIntMask = 0x0067;
   /* select interrupt reset mode */
    SdioFuncReg[FUNC_WIFI]->CardIntMode = 0;
    /* disable sdu interrupt*/

    sdu_attach_rx_bufs(FUNC_WIFI,0);
#if 0
    if (SdioFuncReg[FUNC_WIFI]->WrBitMap & SDIO_DATA_PORTS_MASK)
    {
        sdio_GEN_CARD2HOST_INT(FUNC_WIFI,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
    }
    else
    {
        sdio_GEN_CARD2HOST_INT(FUNC_WIFI,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_IORdy));
    }
#endif

    return 0;
}

void sdu_deinit(void)
{
    uint8_t value;

    value = SdioFuncReg[FUNC_WIFI]->CardIntStatus;
    /* Clear Interrupt Bit */
    SdioFuncReg[FUNC_WIFI]->CardIntStatus = ~value|SDIO_CCR_CIC_PwrUp;

    NVIC_ClearPendingIRQ(SDIO_IRQn);
    //GLB_AHB_Slave1_Reset(BL_AHB_SDU);
}

typedef void (*sdio_tx_cfm_cb_t)(int idx, void *arg);

struct sdio_tx_desc {
    bool used;
    bool is_cmd; // true: cmd, false: frame
    sdio_tx_cfm_cb_t cb;
    void *cb_arg;
    int cb_arg_idx;
    struct sdio_tx_desc *twin;
    uint8_t ref;
};

static struct sdio_tx_desc sdio_tx_descs[16] = { 0 };

#pragma pack(push, 1)
struct sdio_top_msg {
    uint8_t type_lsb;
    uint8_t type_msb;
    uint8_t len_lsb;            // length of pld, excluding padding
    uint8_t len_msb;
    uint8_t pld_off;
    uint8_t is_amsdu;
    uint8_t has_twin;
    uint8_t subtype_lsb;
    uint8_t subtype_msb;
    uint8_t first_part_len_lsb; // length of pld, excluding padding
    uint8_t first_part_len_msb;
    uint8_t _pad0[1];
    uint8_t pld_w_pad[];
};
#pragma pack(pop)

static uint8_t curr_upld_port = 0;
static uint16_t rd_bitmap_old = 0;

int sdu_send_data(void *data, uint32_t len, bool is_cmd, sdio_tx_cfm_cb_t cb, void *cb_arg)
{
    const uint8_t fn = 0;
    uint64_t count_time;
    struct sdio_tx_desc *desc;

    (void)len;

    log_info("sdu_send -----> buf [%d]\r\n", len);
    //log_buf(data, len);
    count_time = bl_timer_now_us64();
    while ((SdioFuncReg[fn]->RdBitMap & (1<< curr_upld_port))){
        if((bl_timer_now_us64() - count_time) / 1000 > SDU_SEND_TIME_OUT){
            printf("sdu send timeout\r\n");
            return -1;
        }
    }

    desc = &sdio_tx_descs[curr_upld_port];
    if (desc->used) {
        printf("sdio tx desc used!\r\n");
        return -1;
    }

    __disable_irq();
    desc->used = true;
    desc->is_cmd = is_cmd;
    desc->cb = cb;
    desc->cb_arg = cb_arg;
    desc->cb_arg_idx = curr_upld_port;
    desc->ref = 1;
    desc->twin = NULL;
    SdioFuncReg[fn]->RdIdx = curr_upld_port;
    SdioFuncReg[fn]->RdLen[curr_upld_port] = SDIO_TX_BUF_SIZE;
    SdioFuncReg[fn]->SqReadBase = (uint32_t)data;
    SdioFuncReg[fn]->RdBitMap = 1 << curr_upld_port;
    rd_bitmap_old = rd_bitmap_old | (1 << curr_upld_port);
    /* printf("sdu_send_data %lu port %u, old %x\r\n", len, curr_upld_port, rd_bitmap_old); */
    sdio_GEN_CARD2HOST_INT(0, SDIO_CCR_CS_UpLdRdy);
    curr_upld_port = (curr_upld_port + 1) % 16;
    __enable_irq();

    return 0;
}

int sdu_send_twin_data(void *data1, void *data2, uint32_t len, bool is_cmd, sdio_tx_cfm_cb_t cb, void *cb_arg)
{
    const uint8_t fn = 0;
    uint64_t count_time;
    struct sdio_tx_desc *desc, *desc_next;
    uint8_t next_upld_port;

    (void)len;

    next_upld_port = (curr_upld_port + 1) % 16;

    // wait for 2 rd port ready
    count_time = bl_timer_now_us64();
    while ((SdioFuncReg[fn]->RdBitMap & (1 << curr_upld_port))){
        if((bl_timer_now_us64() - count_time) / 1000 > SDU_SEND_TIME_OUT){
            printf("sdu wait rdbitmap timeout\r\n");
            return -1;
        }
    }
    count_time = bl_timer_now_us64();
    while ((SdioFuncReg[fn]->RdBitMap & (1 << next_upld_port))){
        if((bl_timer_now_us64() - count_time) / 1000 > SDU_SEND_TIME_OUT){
            printf("sdu wait rdbitmap timeout\r\n");
            return -1;
        }
    }

    desc = &sdio_tx_descs[next_upld_port];
    if (desc->used) {
        printf("sdio tx desc used!\r\n");
        return -1;
    }

    desc_next = &sdio_tx_descs[curr_upld_port];
    if (desc_next->used) {
        printf("sdio tx desc used!\r\n");
        return -1;
    }

    __disable_irq();
    // next upload port first
    desc_next->used = true;
    desc_next->is_cmd = is_cmd;
    desc_next->cb = cb;
    desc_next->cb_arg = cb_arg;
    desc_next->cb_arg_idx = next_upld_port;
    desc_next->ref = 1;
    desc_next->twin = desc;
    SdioFuncReg[fn]->RdIdx = next_upld_port;
    SdioFuncReg[fn]->RdLen[next_upld_port] = SDIO_TX_BUF_SIZE;
    SdioFuncReg[fn]->SqReadBase = (uint32_t)data2;
    SdioFuncReg[fn]->RdBitMap = 1 << next_upld_port;
    rd_bitmap_old = rd_bitmap_old | (1 << next_upld_port);
    /* log_debug("sdu_send_data %lu port %u, old %x\r\n", len, curr_upld_port, rd_bitmap_old); */

    // then current upload port
    desc->used = true;
    desc->is_cmd = is_cmd;
    desc->cb = cb;
    desc->cb_arg = cb_arg;
    desc->cb_arg_idx = curr_upld_port;
    desc->ref = 1;
    desc->twin = desc_next;
    SdioFuncReg[fn]->RdIdx = curr_upld_port;
    SdioFuncReg[fn]->RdLen[curr_upld_port] = SDIO_TX_BUF_SIZE;
    SdioFuncReg[fn]->SqReadBase = (uint32_t)data1;
    SdioFuncReg[fn]->RdBitMap = 1 << curr_upld_port;
    rd_bitmap_old = rd_bitmap_old | (1 << curr_upld_port);
    /* log_debug("sdu_send_data %lu port %u, old %x\r\n", len, curr_upld_port, rd_bitmap_old); */
    /* printf("sdu_send_twin port %u %u, old %x\r\n", curr_upld_port, next_upld_port, rd_bitmap_old); */

    sdio_GEN_CARD2HOST_INT(0, SDIO_CCR_CS_UpLdRdy);
    curr_upld_port = (curr_upld_port + 2) % 16;
    __enable_irq();

    return 0;
}

uint32_t *sdu_receive_data(uint32_t recv_len, uint8_t int_status)
{
    /* The port number for host to download packets*/
    static uint8_t curr_dnld_port = 0;
    uint8_t fn;
    /* uint8_t value; */
    uint8_t CRCError;
    uint32_t *recv_buf=NULL;

    for (fn = 0; fn < NUM_FUNC; fn++)
    {
        switch(fn)
        {
            case FUNC_WIFI:
            {
                /* value = SdioFuncReg[fn]->CardIntStatus; */
                /* Read the CRC error for the CMD 53 write*/
                CRCError = SdioFuncReg[fn]->HostTransferStatus;

                if (int_status & SDIO_CCR_CIC_DnLdOvr)
                {
                    /* Clear Interrupt Bit */
                    //SdioFuncReg[fn]->CardIntStatus = ~value|SDIO_CCR_CIC_PwrUp;
                    /* SdioFuncReg[fn]->CardIntStatus = value & 0xfe; */
                    if(CRCError & SDIO_CCR_HOST_INT_DnLdCRC_err)
                    {
                        break;
                    }
                    if (!(SdioFuncReg[fn]->WrBitMap & (1<< curr_dnld_port)) &&
                        (sdu_rx_buf_attached[fn] & (1<< curr_dnld_port)))
                    {
                        /*application data format:cmdID+rsvd+dataLen0+dataLen1+...*/
                        recv_buf=(uint32_t *)&bootrom_readbuf[rx_buf_index%2];
                        /*clear current download port attach flag*/
                        sdu_rx_buf_attached[fn] &= ~(1<< curr_dnld_port);///clear attached flag
                        rx_buf_index++;
                        sdu_attach_rx_bufs(fn,curr_dnld_port);
                        /*move on to next port*/
                        curr_dnld_port++;
                        if(curr_dnld_port == SDIO_MAX_PORT_NUM){
                            curr_dnld_port = 0;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return recv_buf;
}

void gen_card2host_int(void)
{
    //sdio_GEN_CARD2HOST_INT(0,(SDIO_CCR_CS_ReadCISRdy | SDIO_CCR_CS_DnLdRdy  | SDIO_CCR_CS_IORdy));
    //GLB_GPIO_Write(GLB_GPIO_PIN_3, 1);
    sdio_GEN_CARD2HOST_INT(0, SDIO_CCR_CS_UpLdRdy);
    //sdio_GEN_CARD2HOST_INT(1, SDIO_CCR_CS_UpLdRdy);
    //GLB_GPIO_Write(GLB_GPIO_PIN_3, 0);


    return;
}

int bl_sdu_init(void)
{
    GLB_GPIO_Cfg_Type cfg;
    uint8_t gpiopins[6];
    uint8_t gpiofuns[6];
    uint8_t i=0;

    cfg.gpioMode=GPIO_MODE_AF;
    cfg.pullType=GPIO_PULL_NONE;
    cfg.drive=1;
    cfg.smtCtrl=1;

    gpiopins[0]=BL_BOOT_SDIO_CLK0_GPIO;
    gpiopins[1]=BL_BOOT_SDIO_CMD0_GPIO;
    gpiopins[2]=BL_BOOT_SDIO_DATA00_GPIO;
    gpiopins[3]=BL_BOOT_SDIO_DATA10_GPIO;
    gpiopins[4]=BL_BOOT_SDIO_DATA20_GPIO;
    gpiopins[5]=BL_BOOT_SDIO_DATA30_GPIO;
    gpiofuns[0]=BL_BOOT_SDIO_CLK0_NORMAL_FUN;
    gpiofuns[1]=BL_BOOT_SDIO_CMD0_NORMAL_FUN;
    gpiofuns[2]=BL_BOOT_SDIO_DATA00_NORMAL_FUN;
    gpiofuns[3]=BL_BOOT_SDIO_DATA10_NORMAL_FUN;
    gpiofuns[4]=BL_BOOT_SDIO_DATA20_NORMAL_FUN;
    gpiofuns[5]=BL_BOOT_SDIO_DATA30_NORMAL_FUN;

    for(i=0;i<sizeof(gpiopins);i++){
        cfg.gpioPin=gpiopins[i];
        cfg.gpioFun=gpiofuns[i];
        if(i==0){
            /*sdio clk is input*/
            cfg.gpioMode=GPIO_MODE_INPUT;
        }else{
            /*data are bidir*/
            cfg.gpioMode=GPIO_MODE_AF;
        }
        GLB_GPIO_Init(&cfg);
    }

    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_SDU);
    bl_irq_register_with_ctx(SDIO_IRQn, sdu_interrupt_entry, NULL);
    bl_irq_enable(SDIO_IRQn);

    return 0;
}

int32_t bl_sdio_handshake(void)
{
    int32_t ret=0;

    ret = sdu_host_check();
    if(ret >= 0){
        return 0;
    }

	return -1;
}

int bl_sdio_read_cb_register(void *env, bl_sdio_read_cb_t cb, const void *cb_arg)
{
    if (cb == NULL) {
        blog_error("cb can not be NULL \r\n");

        return -1;
    }

    gl_read_callback = cb;

    return 0;
}

#define LSB(x) ((x) & 0xff)
#define SLSB(x) (((x) >> 8) & 0xff)
#define OFFSET_OF(mem, st) ((uintptr_t)&(((st *)0)->mem))

static void sd_cmd_cfm_cb(int idx, void *arg)
{
    vPortFree(arg);
}

int bl_sdio_write_tlv(void *env, const uint16_t type, const void *data_ptr, const uint16_t data_len)
{
    struct sdio_top_msg *msg;
    void *pld_start;

    msg = pvPortMalloc(SDIO_TX_BUF_SIZE);
    if (!msg) {
        return -1;
    }
    msg->type_lsb = LSB(type);
    msg->type_msb = SLSB(type);
    msg->len_lsb = LSB(data_len);
    msg->len_msb = SLSB(data_len);
    msg->pld_off = OFFSET_OF(pld_w_pad, struct sdio_top_msg);
    msg->is_amsdu = false;
    msg->has_twin = 0;

    pld_start = (void *)msg + msg->pld_off;
    memcpy(pld_start, data_ptr, data_len);
    return sdu_send_data(msg, data_len, false, sd_cmd_cfm_cb, msg);
}

#define ALIGN_BEFORE(ptr, align) ((uintptr_t)(ptr) - ((uintptr_t)(ptr) & ((align) - 1)))

int bl_sdio_write_pbuf_tlv(void *env, const uint16_t type, const uint16_t subtype, struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg)
{
    struct sdio_top_msg *msg;
    void *data_ptr = p->payload;
    uint16_t data_len = p->tot_len;
    struct pbuf *next;

    msg = (struct sdio_top_msg *)ALIGN_BEFORE(data_ptr - sizeof(*msg), 4);

    msg->type_lsb = LSB(type);
    msg->type_msb = SLSB(type);
    msg->len_lsb = LSB(data_len);
    msg->len_msb = SLSB(data_len);
    msg->pld_off = data_ptr - (void *)msg;
    msg->is_amsdu = is_amsdu;
    msg->has_twin = 0;
    msg->subtype_lsb = LSB(subtype);
    msg->subtype_msb = SLSB(subtype);
    msg->first_part_len_lsb = LSB(p->len);
    msg->first_part_len_msb = SLSB(p->len);

    next = p->next;
    if (next) {
        // this is an A-AMSDU with two pbds
        if (!is_amsdu) {
            printf("p->next NOT NULL but is not amsdu, this should not happen\r\n");
            return -1;
        }
        msg->has_twin = 1;

#if 1
        {
            struct pbuf *tmp = next;
            if (tmp->next) {
                while(1) {
                    extern void aos_msleep(int ms);
                    aos_msleep(100);
                    log_error("logic err\r\n");
                }
            }
        }
        log_info("sdu_send -----> buf1 [%d] and buf2 [%d]\r\n", p->len, next->len);
#endif
        return sdu_send_twin_data(msg, next->payload, data_len, false, cb, cb_arg);
    } else {
        return sdu_send_data(msg, data_len, false, cb, cb_arg);
    }
}

#define BIT_ISSET(val, bitn) ((val) & (1 << (bitn)))

void sdu_interrupt_entry(void)
{
    uint16_t len;
    uint8_t *pbuf;
    uint8_t value;
    uint16_t rd_bitmap;
    uint16_t rd_bitmap_diff;
    struct sdio_tx_desc *desc;

    value = SdioFuncReg[0]->CardIntStatus;
    SdioFuncReg[0]->CardIntStatus = ~value | SDIO_CCR_CIC_PwrUp;

    /* printf("sdirq: ints %x\r\n", value); */
    if (value & SDIO_CCR_CIC_DnLdOvr) {
        pbuf = (uint8_t *)sdu_receive_data(2048, value);
        if (pbuf) {
            len = pbuf[0] | (pbuf[1] << 8);
            if (gl_read_callback != NULL) {
                gl_read_callback(NULL, pbuf + 2, len);
            }
        }
    }

    if (value & SDIO_CCR_CIC_UpLdOvr) {
        rd_bitmap = SdioFuncReg[0]->RdBitMap;
        rd_bitmap_diff = rd_bitmap ^ rd_bitmap_old;
        /* log_warn("sdirq: upldovr event, rd_bitmap %x, rd_bitmap_old %x, rdr_bitmap_diff %x\r\n", */
        /*         rd_bitmap, rd_bitmap_old, rd_bitmap_diff); */
        if (rd_bitmap_diff) {
            for (int i = 0; i < 16; ++i) {
                if (BIT_ISSET(rd_bitmap_diff, i)) {
                    if (BIT_ISSET(rd_bitmap_old, i)) {
                        desc = &sdio_tx_descs[i];
                        if (desc->used) {
                            if (desc->cb) {
                                if (desc->twin == NULL) {
                                    if (--desc->ref == 0) {
                                        /* printf("f %d\r\n", i); */
                                        desc->cb(i, desc->cb_arg);
                                        desc->used = false;
                                    }
                                } else {
                                    struct sdio_tx_desc *twin = desc->twin;
                                    if (--desc->ref == 0) {
                                        if (twin->ref == 0) {
                                            /* printf("ft %d %d\r\n", twin->cb_arg_idx, desc->cb_arg_idx); */
                                            desc->cb(i, desc->cb_arg);
                                            desc->used = false;
                                            twin->used = false;
                                        } else {
                                            /* printf("tw %d ref %d, d %d\r\n", twin->cb_arg_idx, twin->ref, desc->cb_arg_idx); */
                                        }
                                    } else {
                                        /* printf("ref left %d\r\n", desc->ref); */
                                    }
                                }
                            }
                        } else {
                            log_error("port %d seems cleared, but used is false\r\n", i);
                        }
                    } else {
                        log_error("Impossible condition 1\r\n");
                    }
                }
            }
        } else {
            log_error("rd_bitmap_diff 0, this should not happen\r\n");
        }
        rd_bitmap_old = rd_bitmap;
    }

    return;
}

//#endif
