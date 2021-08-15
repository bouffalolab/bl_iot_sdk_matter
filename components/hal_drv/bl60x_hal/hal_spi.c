
#include <string.h>
#include <stdio.h>
#include <device/vfs_spi.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/soc/spi.h>
#include <aos/kernel.h>

#include <bl60x_gpio.h>
#include <bl60x_glb.h>
#include <bl60x_ssp.h>
#include <bl60x_dma.h>
#include <bl_dma.h>
#include <bl_spi.h>
#include <bl_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <event_groups.h>

#include <libfdt.h>
#include <utils_log.h>

#define SPI_NUM_MAX         1/* only support spi0 */
#define SPI_DMA_TX_IRQn     DMA0_CH2_IRQn
#define SPI_DMA_RX_IRQn     DMA0_CH3_IRQn

#define EVT_GROUP_SPI_DMA_TX    (1<<0)
#define EVT_GROUP_SPI_DMA_RX    (1<<1)
#define EVT_GROUP_SPI_DMA_TR    (EVT_GROUP_SPI_DMA_TX | EVT_GROUP_SPI_DMA_RX)

typedef struct _spi_hw {
    uint8_t used;
    SSP_ID_Type ssp_id;
    uint8_t mode;
    uint32_t freq;
    uint8_t tx_dma_ch;
    uint8_t rx_dma_ch;
    uint8_t pin_clk;
    uint8_t pin_cs;
    uint8_t pin_mosi;
    uint8_t pin_miso;
    EventGroupHandle_t spi_dma_event_group;
} spi_hw_t;

typedef struct spi_priv_data {
    spi_hw_t hwspi[SPI_NUM_MAX];
} spi_priv_data_t;

spi_priv_data_t *g_hal_buf = NULL;

/*
    uint8_t pin_clk;
    uint8_t pin_cs;
    uint8_t pin_mosi;
    uint8_t pin_miso;
*/
static void hal_gpio_init(spi_hw_t *arg)
{
    GLB_GPIO_Cfg_Type GPIO_InitStructure;
    spi_hw_t *hw_arg = arg;

    if (!arg) {
        log_error("arg err.\r\n");
        return;
    }

    GPIO_InitStructure.drive = 0;
    GPIO_InitStructure.smtCtrl = 1;
    GPIO_InitStructure.pullType = GPIO_PULL_UP;

    /* Set GPIO as SPI SCLK */
    GPIO_InitStructure.gpioFun = GPIO16_FUN_SSP_SCLK;
    GPIO_InitStructure.gpioMode = GPIO_MODE_AF;
    GPIO_InitStructure.gpioPin = hw_arg->pin_clk;
    GLB_GPIO_Init(&GPIO_InitStructure);

    /* Set GPIO as SPI CS */
    #if 0
    bl_gpio_enable_output(hw_arg->pin_cs, 1, 0);
    bl_gpio_output_set(hw_arg->pin_cs, 1);
    #else
    GPIO_InitStructure.gpioFun = GPIO17_FUN_SSP_SFRM;
    GPIO_InitStructure.gpioMode = GPIO_MODE_AF;
    GPIO_InitStructure.gpioPin = hw_arg->pin_cs;
    GLB_GPIO_Init(&GPIO_InitStructure);
    #endif

    /* Set GPIO as SPI MOSI */
    GPIO_InitStructure.gpioFun = GPIO18_FUN_SSP_TXD;
    GPIO_InitStructure.gpioMode = GPIO_MODE_AF;
    GPIO_InitStructure.gpioPin = hw_arg->pin_mosi;
    GLB_GPIO_Init(&GPIO_InitStructure);

    /* Set GPIO as SPI MISO */
    GPIO_InitStructure.gpioFun = GPIO19_FUN_SSP_RXD;
    GPIO_InitStructure.gpioMode = GPIO_MODE_AF;
    GPIO_InitStructure.gpioPin = hw_arg->pin_miso;
    GLB_GPIO_Init(&GPIO_InitStructure);
}

/*
    ssp_id
    mode
    tx_dma_ch
    rx_dma_ch
*/
static void hal_spi_dma_init(spi_hw_t *arg)
{
    uint8_t clk_div;
    SSP_ID_Type sspId;
    SSP_CFG_Type sspCfgStruct;
    SSP_FIFO_Type sspFifoCfg;
    SPI_Param_Type spiParaStruct;
    DMA_Channel_Cfg_Type dmaTxChStruct;
    DMA_Channel_Cfg_Type dmaRxChStruct;
    spi_hw_t *hw_arg = arg;

    if (!arg) {
        log_error("arg err.\r\n");
        return;
    }

    /* clock */
    /*
        0  --->  40 Mhz
        1  --->  20 Mhz
        5  --->  6.66 Mhz
        10 --->  3.33 Mhz
    */
    clk_div = (uint8_t)((40000000/hw_arg->freq) - 1);
    log_info("clk_div = %d\r\n", clk_div);
    GLB_Set_SSP_CLK(1, clk_div);

    /* spi */
    sspId                            = hw_arg->ssp_id;

    SSP_Disable(sspId);

    sspCfgStruct.mode                = SSP_NORMAL;              /*!< SSP mode type */
    sspCfgStruct.frameFormat         = SSP_FRAME_SPI;           /*!< SSP frame format type */
    sspCfgStruct.masterOrSlave       = SSP_MASTER;              /*!< SSP function mode: Master or Slave */
    sspCfgStruct.trMode              = SSP_TR_MODE;             /*!< SSP tx/rx mode: SSP_TR_MODE or SSP_RX_MODE */
    sspCfgStruct.dataSize            = SSP_DATASIZE_8;          /*!< SSP frame data size */
    sspCfgStruct.sfrmPol             = SSP_SAMEFRM_PSP;         /*!< SSP serial frame polarity type */
    sspCfgStruct.slaveClkRunning     = SSP_SLAVECLK_TRANSFER;   //SSP_SLAVECLK_TRANSFER, SSP_SLAVECLK_CONTINUOUS/*!< SSP slave clock running type */
    sspCfgStruct.txd3StateType       = SSP_TXD3STATE_ELSB;      //SSP_TXD3STATE_ELSB, SSP_TXD3STATE_12SLSB/*!< SSP txd turns to three state type */
    sspCfgStruct.txd3StateEnable     = ENABLE;                  //ENABLE, DISABLE/*!< Enable or Disable SSP turns txd three state mode */
    sspCfgStruct.timeOutVal          = 0x3;                     /*!< Timeout value */
    SSP_Init(sspId, &sspCfgStruct);

    sspFifoCfg.fifoPackMode          = DISABLE;    /*!< Enable or Disale SSP Fifo packed mode */
    sspFifoCfg.rxFifoFullLevel       = 0x00;       //Trigger level for rx is (0x00 + 1) Bytes/*!< SSP receive fifo full threshold */
    sspFifoCfg.txFifoEmptyLevel      = 0x0F;       //Trigger level for tx is (0x0F + 1) Bytes/*!< SSP transmit fifo empty threshold */
    sspFifoCfg.rxDmaService          = ENABLE;     /*!< Enable or Diasble SSP receive fifo dma service request */
    sspFifoCfg.txDmaService          = ENABLE;     /*!< Enable or Diasble SSP transmit fifo dma service request */
    SSP_FifoConfig(sspId, &sspFifoCfg);

    if (hw_arg->mode == 0) {
        spiParaStruct.spiClkPhase        = SPI_SCPHA_0;
        spiParaStruct.spiClkPolarity     = SPI_SCPOL_LOW;
    } else if (hw_arg->mode == 1) {
        spiParaStruct.spiClkPhase        = SPI_SCPHA_1;
        spiParaStruct.spiClkPolarity     = SPI_SCPOL_LOW;
    } else if (hw_arg->mode == 2) {
        spiParaStruct.spiClkPhase        = SPI_SCPHA_0;
        spiParaStruct.spiClkPolarity     = SPI_SCPOL_HIGH;
    } else if (hw_arg->mode == 3) {
        spiParaStruct.spiClkPhase        = SPI_SCPHA_1;
        spiParaStruct.spiClkPolarity     = SPI_SCPOL_HIGH;
    } else {
        log_error("unsupport mode.\r\n");
    }
    SPI_Config(sspId, &spiParaStruct);

    /* dma */
    DMA_Enable();

    /* dma_tx */
    dmaTxChStruct.srcDmaAddr       = 0;                     /*!< 源地址 Source address of DMA transfer */
    dmaTxChStruct.destDmaAddr      = 0x4000f010;            /*!< 目的地址 Destination address of DMA transfer */
    dmaTxChStruct.transfLength     = 0;                     /*!< 传输长度 Transfer length, 0~4095, this is burst count */
    dmaTxChStruct.dir              = DMA_TRNS_M2P;          /*!< Memory_to_peripheral Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
    dmaTxChStruct.ch               = hw_arg->tx_dma_ch;         /*!< 传输通道 Channel select 0-4 */
    dmaTxChStruct.srcTransfWidth   = DMA_TRNS_WIDTH_8BITS;  /*!< 传输宽度 Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
    dmaTxChStruct.dstTransfWidth   = DMA_TRNS_WIDTH_8BITS;  /*!< 传输宽度 Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
    dmaTxChStruct.srcBurstSzie     = DMA_BURST_SIZE_4;      /*!< 源DMA突发传输的大小 Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    dmaTxChStruct.dstBurstSzie     = DMA_BURST_SIZE_4;      /*!< 目的DMA突发传输的大小 Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    dmaTxChStruct.srcAddrInc       = DMA_MINC_ENABLE;       /*!< 源地址自增 Source address increment. 0: No change, 1: Increment */
    dmaTxChStruct.destAddrInc      = DMA_PINC_DISABLE;      /*!< 目的地址自增 Destination address increment. 0: No change, 1: Increment */
    dmaTxChStruct.srcPeriph        = DMA_REQ_NONE;          /*!< 源DMA请求的外设 Source peripheral select */
    dmaTxChStruct.dstPeriph        = DMA_REQ_SSP0_TX;       /*!< 目的DMA请求的外设 Destination peripheral select */
    DMA_Channel_Init(&dmaTxChStruct);

    /* dma_rx */
    dmaRxChStruct.srcDmaAddr       = 0x4000f010;            /*!< 源地址 Source address of DMA transfer */
    dmaRxChStruct.destDmaAddr      = 0;                     /*!< 目的地址 Destination address of DMA transfer */
    dmaRxChStruct.transfLength     = 0;                     /*!< 传输长度 Transfer length, 0~4095, this is burst count */
    dmaRxChStruct.dir              = DMA_TRNS_P2M;          /*!< Peripheral_to_memory Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
    dmaRxChStruct.ch               = hw_arg->rx_dma_ch;     /*!< 传输通道 Channel select 0-4 */
    dmaRxChStruct.srcTransfWidth   = DMA_TRNS_WIDTH_8BITS;  /*!< 传输宽度 Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
    dmaRxChStruct.dstTransfWidth   = DMA_TRNS_WIDTH_8BITS;  /*!< 传输宽度 Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
    dmaRxChStruct.srcBurstSzie     = DMA_BURST_SIZE_4;      /*!< 源DMA突发传输的大小 Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    dmaRxChStruct.dstBurstSzie     = DMA_BURST_SIZE_4;      /*!< 目的DMA突发传输的大小 Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    dmaRxChStruct.srcAddrInc       = DMA_PINC_DISABLE;      /*!< 源地址自增 Source address increment. 0: No change, 1: Increment */
    dmaRxChStruct.destAddrInc      = DMA_MINC_ENABLE;       /*!< 目的地址自增 Destination address increment. 0: No change, 1: Increment */
    dmaRxChStruct.srcPeriph        = DMA_REQ_SSP0_RX;       /*!< 源DMA请求的外设 Source peripheral select */
    dmaRxChStruct.dstPeriph        = DMA_REQ_NONE;          /*!< 目的DMA请求的外设 Destination peripheral select */
    DMA_Channel_Init(&dmaRxChStruct);

    DMA_IntMask(dmaTxChStruct.ch, DMA_INT_ALL, MASK);             //禁止发送的所有的中断
    DMA_IntMask(dmaTxChStruct.ch, DMA_INT_TCOMPLETED, UNMASK);    //允许发送完成中断
    DMA_IntMask(dmaTxChStruct.ch, DMA_INT_ERR, UNMASK);           //允许发送错误中断
    NVIC_SetPriority(SPI_DMA_TX_IRQn, 3);                           //设置中断优先级
    NVIC_EnableIRQ(SPI_DMA_TX_IRQn);

    DMA_IntMask(dmaRxChStruct.ch, DMA_INT_ALL, MASK);             //禁止所有中断
    DMA_IntMask(dmaRxChStruct.ch, DMA_INT_TCOMPLETED, UNMASK);    //允许接收完成中断
    DMA_IntMask(dmaRxChStruct.ch, DMA_INT_ERR, UNMASK);           //允许接收错误中断
    NVIC_SetPriority(SPI_DMA_RX_IRQn, 3);                           //设置中断优先级
    NVIC_EnableIRQ(SPI_DMA_RX_IRQn);

    /*Enable rx DMA for protocal header parse, protocal handler entry is put into DMA int*/
    DMA_Channel_Enable(dmaTxChStruct.ch);
    DMA_Channel_Enable(dmaTxChStruct.ch);

    SSP_Enable(sspId);
}

static void hal_spi_dma_trans(spi_hw_t *arg, uint8_t *TxData, uint8_t *RxData, uint32_t Len)
{
    EventBits_t uxBits;

    if (!arg) {
        log_error("arg err.\r\n");
        return;
    }
    // log_info("hal_spi_dma_trans trx_ch = %d %d\r\n", arg->tx_dma_ch, arg->rx_dma_ch);
    // log_info("eventloop wait = %08lx\r\n", (uint32_t)arg->spi_dma_event_group);
    // bl_gpio_output_set(arg->pin_cs, 0);
    // aos_msleep(1);//need delete

    xEventGroupClearBits(arg->spi_dma_event_group, EVT_GROUP_SPI_DMA_TR);

    DMA_Channel_Disable(arg->tx_dma_ch);
    DMA_Channel_Disable(arg->rx_dma_ch);
    bl_dma_int_clear(arg->tx_dma_ch);
    bl_dma_int_clear(arg->rx_dma_ch);
    bl_dma_update_memsrc(arg->tx_dma_ch, (uint32_t)TxData, Len);
    bl_dma_update_memdst(arg->rx_dma_ch, (uint32_t)RxData, Len);
    DMA_Channel_Enable(arg->tx_dma_ch);
    DMA_Channel_Enable(arg->rx_dma_ch);

    uxBits = xEventGroupWaitBits(arg->spi_dma_event_group,
                                     EVT_GROUP_SPI_DMA_TR,
                                     pdTRUE,
                                     pdTRUE,
                                     portMAX_DELAY);
    // bl_gpio_output_set(arg->pin_cs, 1);
    // log_info("wait tr end.\r\n");

    // bl_gpio_output_set(arg->pin_cs, 1);

    if ((uxBits & EVT_GROUP_SPI_DMA_TR) == EVT_GROUP_SPI_DMA_TR) {
        log_info("recv all event group.\r\n");
    }
}

int32_t hal_spi_init(spi_dev_t *spi)
{
    int i;
    spi_priv_data_t *data;

    if (!spi) {
        log_error("arg err.\r\n");
    }

    data = (spi_priv_data_t *)spi->priv;
    if (data == NULL) {
        return -1;
    }

    for (i = 0; i < SPI_NUM_MAX; i++) {
        hal_gpio_init(&data->hwspi[i]);
        hal_spi_dma_init(&data->hwspi[i]);
    }

    log_info("hal_spi_init.\r\n");
    return 0;
}

int32_t hal_spi_finalize(spi_dev_t *spi)
{
    log_info("not support. hal_spi_finalize.\r\n");
    return 0;
}

int32_t hal_spi_send(spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    log_info("not support. hal_spi_send.\r\n");
    return 0;
}

int32_t hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout)
{
    log_info("not support. hal_spi_recv.\r\n");
    return 0;
}

int32_t hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data, uint16_t size, uint32_t timeout)
{
    log_info("not support. hal_spi_send_recv.\r\n");
    return 0;
}

int hal_spi_set_rwmode(spi_dev_t *spi_dev, int mode)
{
    spi_priv_data_t *data;

    log_info("set rwmode = %d\r\n", mode);
    if ((mode < 0) || (mode > 3)) {
        log_error("mode is err.\r\n");
        return -1;
    }

    data = (spi_priv_data_t *)spi_dev->priv;
    data->hwspi[spi_dev->port].mode = mode;
    spi_dev->config.mode = mode;

    hal_spi_init(spi_dev);
    return 0;
}

int hal_spi_set_rwspeed(spi_dev_t *spi_dev, uint32_t speed)
{
    spi_priv_data_t *data;
    int i;
    uint8_t real_flag = 0;
    uint32_t real_speed = 0;

    log_info("set rwspeed = %ld\r\n", speed);
    if (spi_dev->config.freq == speed) {
        log_info("speed not change.\r\n");
        return 0;
    }

    for (i = 0; i < 256; i++) {
        if (speed == (40000000/(i+1))) {
            real_speed = speed;
            real_flag = 1;
        } else if (speed < (40000000/(i+1))) {
            continue;
        } else {
            break;
        }
    }

    if (real_flag != 1) {
        if (i == 0) {
            log_error("The max speed is 40000000 Hz, please set it smaller.");
            return -1;
        } else if (i == 256) {
            log_error("The min speed is 156250 Hz, please set it bigger.");
            return -1;
        } else {
            if ( ((40000000/(i+1)) - speed) > (speed - (40000000/i)) ) {
                real_speed = (40000000/(i+1));
                log_info("not support speed: %ld, change real_speed = %ld\r\n", speed, real_speed);
            } else {
                real_speed = (40000000/i);
                log_info("not support speed: %ld, change real_speed = %ld\r\n", speed, real_speed);
            }
        }
    }

    data = (spi_priv_data_t *)spi_dev->priv;
    data->hwspi[spi_dev->port].freq = real_speed;
    spi_dev->config.freq = real_speed;

    hal_spi_init(spi_dev);
    return 0;
}

int hal_spi_transfer(spi_dev_t *spi_dev, void *xfer, uint8_t size)
{
    uint16_t i;
    spi_ioc_transfer_t * s_xfer;
    spi_priv_data_t *priv_data;

    if ((!spi_dev) || (!xfer)) {
        log_error("arg err.\r\n");
        return -1;
    }

    priv_data = (spi_priv_data_t *)spi_dev->priv;
    if (priv_data == NULL) {
        log_error("priv_data NULL.\r\n");
        return -1;
    }

    s_xfer = (spi_ioc_transfer_t *)xfer;

    log_info("hal_spi_transfer = %d\r\n", size);

    bl_gpio_output_set(priv_data->hwspi[spi_dev->port].pin_cs, 0);
    for (i = 0; i < size; i++) {
        log_info("transfer xfer[%d].len = %ld\r\n", i, s_xfer[i].len);
        hal_spi_dma_trans(&priv_data->hwspi[spi_dev->port],
            (uint8_t *)s_xfer[i].tx_buf, (uint8_t *)s_xfer[i].rx_buf, s_xfer[i].len);
    }
    bl_gpio_output_set(priv_data->hwspi[spi_dev->port].pin_cs, 1);

    return 0;
}

int vfs_spi_init_fullname(const char *fullname, uint8_t port,
                            uint8_t mode, uint32_t freq, uint8_t tx_dma_ch, uint8_t rx_dma_ch,
                            uint8_t pin_clk, uint8_t pin_cs, uint8_t pin_mosi, uint8_t pin_miso)
{
    int ret, len;
    spi_dev_t *spi;

    len = strlen(fullname);
    if (len + 1 > 32) {
        log_error("arg err.\r\n");
        return -EINVAL;
    }

    //TODO use one bigger mem for these two small struct
    if (NULL == g_hal_buf) {
        g_hal_buf = (spi_priv_data_t*)aos_malloc(sizeof(spi_priv_data_t));
        if (NULL == g_hal_buf) {
            log_error("mem err.\r\n");
            return -ENOMEM;
        }
        memset(g_hal_buf, 0, sizeof(spi_priv_data_t));
    }

    g_hal_buf->hwspi[port].spi_dma_event_group = xEventGroupCreate();
    log_info("port%d eventloop init = %08lx\r\n", port,
        (uint32_t)g_hal_buf->hwspi[port].spi_dma_event_group);
    if (NULL == g_hal_buf->hwspi[port].spi_dma_event_group) {
        aos_free(g_hal_buf);
        return -ENOMEM;
    }

    spi = (spi_dev_t*)aos_malloc(sizeof(spi_dev_t));
    if (NULL == spi) {
        log_error("mem err.\r\n");
        vEventGroupDelete(g_hal_buf->hwspi[port].spi_dma_event_group);
        aos_free(g_hal_buf);
        return -ENOMEM;
    }

    memset(spi, 0, sizeof(spi_dev_t));
    spi->port = port;
    spi->config.mode = mode;
    spi->config.freq = freq;
    g_hal_buf->hwspi[port].ssp_id = port;
    g_hal_buf->hwspi[port].mode = mode;
    g_hal_buf->hwspi[port].freq = freq;
    g_hal_buf->hwspi[port].tx_dma_ch = tx_dma_ch;
    g_hal_buf->hwspi[port].rx_dma_ch = rx_dma_ch;
    g_hal_buf->hwspi[port].pin_clk = pin_clk;
    g_hal_buf->hwspi[port].pin_cs = pin_cs;
    g_hal_buf->hwspi[port].pin_mosi = pin_mosi;
    g_hal_buf->hwspi[port].pin_miso = pin_miso;
    spi->priv = g_hal_buf;

    log_info("[HAL] [SPI] Register Under %s for :\r\nport=%d, mode=%d, freq=%ld, tx_dma_ch=%d, rx_dma_ch=%d, pin_clk=%d, pin_cs=%d, pin_mosi=%d, pin_miso=%d\r\n",
        fullname, port, mode, freq, tx_dma_ch, rx_dma_ch, pin_clk, pin_cs, pin_mosi, pin_miso);

    ret = aos_register_driver(fullname, &spi_ops, spi);
    if (ret != VFS_SUCCESS) {
        aos_free(spi);
        vEventGroupDelete(g_hal_buf->hwspi[port].spi_dma_event_group);
        aos_free(g_hal_buf);
        return ret;
    }

    return VFS_SUCCESS;
}

#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))

int spi_arg_set_fdt2(const void * fdt, uint32_t dtb_spi_offset)
{
    #define SPI_MODULE_MAX 1
    uint8_t port;
    uint8_t mode;
    uint32_t freq;
    uint8_t tx_dma_ch;
    uint8_t rx_dma_ch;
    uint8_t pin_clk;
    uint8_t pin_cs;
    uint8_t pin_mosi;
    uint8_t pin_miso;
    char *path = NULL;

    int offset1 = 0;
    int offset2 = 0;
    const uint32_t *addr_prop = 0;
    int lentmp = 0;
    const char *result = 0;
    int countindex = 0;

    int i;

    const char *spi_node[SPI_MODULE_MAX] = {
        "spi@4000F000"
    };

    /* spi */
    for (i = 0; i < SPI_MODULE_MAX; i++) {
        /* get spi0 ? spi1 ? spi2 offset1 */
        offset1 = fdt_subnode_offset(fdt, dtb_spi_offset, spi_node[i]);
        if (0 >= offset1) {
            continue;
        }

        result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
        if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
            log_info("spi[%d] status != okay\r\n", i);
            continue;
        }

        /* set path */
        countindex = fdt_stringlist_count(fdt, offset1, "path");
        if (countindex != 1) {
            log_info("spi[%d] path_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "path", 0, &lentmp);
        if ((lentmp < 0) || (lentmp > 32)) {
            log_info("spi[%d] path lentmp = %d\r\n", i, lentmp);
        }
        path = (char *)result;

        /* sure port == i */
        addr_prop = fdt_getprop(fdt, offset1, "port", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] port NULL.\r\n", i);
            continue;
        }
        port = BL_FDT32_TO_U8(addr_prop, 0);
        if (port != i) {
            log_error("fdt err. port[%d] != i[%d].\r\n", port, i);
            continue;
        }

        /* get mode */
        addr_prop = fdt_getprop(fdt, offset1, "mode", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] mode NULL.\r\n", i);
            continue;
        }
        mode = BL_FDT32_TO_U8(addr_prop, 0);

        /* get freq */
        addr_prop = fdt_getprop(fdt, offset1, "freq", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] freq NULL.\r\n", i);
            continue;
        }
        freq = BL_FDT32_TO_U32(addr_prop, 0);

        /* set pin */
        offset2 = fdt_subnode_offset(fdt, offset1, "pin");
        if (0 >= offset1) {
            continue;
        }

        /* get pin_clk */
        addr_prop = fdt_getprop(fdt, offset2, "clk", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] clk NULL.\r\n", i);
            continue;
        }
        pin_clk = BL_FDT32_TO_U8(addr_prop, 0);

        /* get pin_cs */
        addr_prop = fdt_getprop(fdt, offset2, "cs", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] cs NULL.\r\n", i);
            continue;
        }
        pin_cs = BL_FDT32_TO_U8(addr_prop, 0);

        /* get pin_mosi */
        addr_prop = fdt_getprop(fdt, offset2, "mosi", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] mosi NULL.\r\n", i);
            continue;
        }
        pin_mosi = BL_FDT32_TO_U8(addr_prop, 0);

        /* get pin_miso */
        addr_prop = fdt_getprop(fdt, offset2, "miso", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] miso NULL.\r\n", i);
            continue;
        }
        pin_miso = BL_FDT32_TO_U8(addr_prop, 0);

        /* set dma_cfg */
        offset2 = fdt_subnode_offset(fdt, offset1, "dma_cfg");
        if (0 >= offset1) {
            continue;
        }

        /* get tx_dma_ch */
        addr_prop = fdt_getprop(fdt, offset2, "tx_dma_ch", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] tx_dma_ch NULL.\r\n", i);
            continue;
        }
        tx_dma_ch = BL_FDT32_TO_U8(addr_prop, 0);

        /* get rx_dma_ch */
        addr_prop = fdt_getprop(fdt, offset2, "rx_dma_ch", &lentmp);
        if (addr_prop == NULL) {
            log_info("spi[%d] rx_dma_ch NULL.\r\n", i);
            continue;
        }
        rx_dma_ch = BL_FDT32_TO_U8(addr_prop, 0);

        vfs_spi_init_fullname((const char *)path, port, mode, freq, tx_dma_ch, rx_dma_ch,           pin_clk, pin_cs, pin_mosi, pin_miso);
        log_info("init ok and read %08lx\r\n", (uint32_t)g_hal_buf->hwspi[0].spi_dma_event_group);
    }
    return 0;
}

int vfs_spi_fdt_init(uint32_t fdt, uint32_t dtb_spi_offset)
{
    spi_arg_set_fdt2((const void *)fdt, dtb_spi_offset);
    log_info("vfs_spi_fdt_init ok.\r\n");
    return 0;
}

void bl_spi0_dma_int_handler_tx(void)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (NULL != g_hal_buf) {
        // log_info("tx_isr eventloop set %d %08lx\r\n", g_hal_buf->hwspi[0].tx_dma_ch,
        //     (uint32_t)g_hal_buf->hwspi[0].spi_dma_event_group);
        bl_dma_int_clear(g_hal_buf->hwspi[0].tx_dma_ch);

        if (g_hal_buf->hwspi[0].spi_dma_event_group != NULL) {
            xResult = xEventGroupSetBitsFromISR(g_hal_buf->hwspi[0].spi_dma_event_group,
                                                EVT_GROUP_SPI_DMA_TX,
                                                &xHigherPriorityTaskWoken);
        }

        if(xResult != pdFAIL) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        log_error("bl_spi0_dma_int_handler_tx no clear isr.\r\n");
    }

    // log_info("spi0_dma_tc_cb\r\n");
}

void bl_spi0_dma_int_handler_rx(void)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (NULL != g_hal_buf) {
        // log_info("rx_isr eventloop set %d %08lx.\r\n", g_hal_buf->hwspi[0].rx_dma_ch,
        //     (uint32_t)g_hal_buf->hwspi[0].spi_dma_event_group);
        bl_dma_int_clear(g_hal_buf->hwspi[0].rx_dma_ch);

        if (g_hal_buf->hwspi[0].spi_dma_event_group != NULL) {
            xResult = xEventGroupSetBitsFromISR(g_hal_buf->hwspi[0].spi_dma_event_group,
                                                EVT_GROUP_SPI_DMA_RX,
                                                &xHigherPriorityTaskWoken);
        }

        if(xResult != pdFAIL) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        log_error("bl_spi0_dma_int_handler_rx no clear isr.\r\n");
    }

    // log_info("spi0_dma_rc_cb\r\n");
}
