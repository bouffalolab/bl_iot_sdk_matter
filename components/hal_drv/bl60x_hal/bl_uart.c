#include <bl60x_common.h>
#include <bl60x_glb.h>
#include <bl60x_gpio.h>
#include <bl60x_uart.h>

#include "bl_uart.h"

#include <utils_log.h>

#define UART_NUMBER_SUPPORTED   3
#define UART_INT_SRC_MODEM      0
#define UART_INT_SRC_TDR        1
#define UART_INT_SRC_RDA        2
#define UART_INT_SRC_RLSE       3

/*
 * We use HALF Empty for TX-Fifo trigger level
 * Also, the TX FIFO has depth 64 bytes
 * */
#define FIFO_TX_SIZE_BURST  (32)

static const uint32_t uartAddr[3] = {
    UART0_BASE,
    UART1_BASE,
    UART2_BASE
};

//TODO use pinmux function map provided by uplayer
static const GLB_GPIO_Type uart_pin[UART_NUMBER_SUPPORTED][2] =
{
    {GLB_GPIO_PIN_16,   GLB_GPIO_PIN_17},
    {GLB_GPIO_PIN_8,    GLB_GPIO_PIN_8},//FIXME only use UART1 for PIN8 of TX
    {GLB_GPIO_PIN_7,    GLB_GPIO_PIN_14},
};
static const int uart_fun[UART_NUMBER_SUPPORTED][2] =
{
    {GPIO16_FUN_UART_SIG_4,     GPIO17_FUN_UART_SIG_5},
    {GPIO8_FUN_UART_SIG_8,      GPIO8_FUN_UART_SIG_8},
    {GPIO7_FUN_UART_SIG_7,      GPIO14_FUN_UART_SIG_2},
};
static const GLB_UART_SIG_Type uart_sig[UART_NUMBER_SUPPORTED][2] =
{
    {GLB_UART_SIG_4,    GLB_UART_SIG_5},
    {GLB_UART_SIG_8,    GLB_UART_SIG_8},
    {GLB_UART_SIG_7,    GLB_UART_SIG_2},
};

static const GLB_UART_SIG_FUN_Type uart_sig_fun[UART_NUMBER_SUPPORTED][2] =
{
    {GLB_UART_SIG_FUN_UART0_RXD,    GLB_UART_SIG_FUN_UART0_TXD},
    {GLB_UART_SIG_FUN_UART1_RXD,    GLB_UART_SIG_FUN_UART1_TXD},
    {GLB_UART_SIG_FUN_UART2_RXD,    GLB_UART_SIG_FUN_UART2_TXD}
};

static cb_uart_notify_t cbs_notify[UART_NUMBER_SUPPORTED];
static void *cbs_uart_notify_arg[UART_NUMBER_SUPPORTED][8];//0 for rx callback arg, 1-3 for rx ring buffer; 4 for tx callback arg, 5-7 for tx ring buffer

int bl_uart_gpio_init(uint8_t id, uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, int baudrate)
{
    GLB_GPIO_Cfg_Type cfg;
    UART_CFG_Type subcaseUartCfg;
    UART_FifoCfg_Type fifoCfg;

    if (!(id < UART_NUMBER_SUPPORTED)) {
        return -1;
    }

    cfg.drive=0;
    cfg.smtCtrl=1;

    if (rx) {
        /*set GPIO as UART RX */
        cfg.gpioPin = uart_pin[id][0];
        cfg.gpioFun = uart_fun[id][0];
        cfg.gpioMode = GPIO_MODE_AF;
        cfg.pullType = GPIO_PULL_NONE;
        GLB_GPIO_Init(&cfg);
        /* select uart gpio function */
        GLB_Uart_Fun_Sel(uart_sig[id][0], uart_sig_fun[id][0]);
    }

    if (tx) {
        /*set GPIO as UART TX */
        cfg.gpioPin = uart_pin[id][1];
        cfg.gpioFun = uart_fun[id][1];
        cfg.gpioMode = GPIO_MODE_AF;
        cfg.pullType = GPIO_PULL_NONE;
        GLB_GPIO_Init(&cfg);
        /* select uart gpio function */
        GLB_Uart_Fun_Sel(uart_sig[id][1], uart_sig_fun[id][1]);
    }

    // uart_env_prepare(baudrate);
    subcaseUartCfg.uartClk = 160 * 1000 * 1000;
    subcaseUartCfg.baudRate = baudrate;
    subcaseUartCfg.dataBits = UART_DATABITS_8;
    subcaseUartCfg.stickyParity = DISABLE;
    subcaseUartCfg.parity = UART_PARITY_NONE;
    subcaseUartCfg.highSpeedUart = DISABLE;
    subcaseUartCfg.nrzCodeing = DISABLE;
    fifoCfg.fifoEnable = ENABLE;
    fifoCfg.autoFlowControl = DISABLE;
    fifoCfg.rxFifoReset = ENABLE;
    fifoCfg.txFifoReset = ENABLE;
    fifoCfg.periBusType = UART_PBUS_BITS_8;
    fifoCfg.fifoDmaEnable = DISABLE;
    fifoCfg.rxFifoLevel = UART_RXFIFO_BYTES_32;
    fifoCfg.txFifoLevel = UART_TXFIFO_HALF_EMPTY;//Warning, don't change this settings.

    /* disable all interrupt */
    UART_IntMask((UART_ID_Type)id, UART_INT_ALL, MASK);
    /* disable uart before config */
    UART_Disable((UART_ID_Type)id);
    /* uart init with default configuration */
    UART_Init((UART_ID_Type)id, &subcaseUartCfg);
    /* uart fifo init with default configuration */
    UART_FifoConfig((UART_ID_Type)id, &fifoCfg);
    /* enable uart */
    UART_Enable((UART_ID_Type)id);

    GLB_Set_UART_CLK(1 ,GLB_UART_CLK_FCLK, 0);

    return 0;
}

/*
    arg
        id: 0 1 2
        tx: pin_number 0-40  0xff-invalid
        rx: pin_number 0-40  0xff-invalid
        rts: pin_number 0-40  0xff-invalid
        cts: pin_number 0-40  0xff-invalid
        baudrate:
*/
int bl_uart_init(uint8_t id, uint8_t tx_pin, uint8_t rx_pin, uint8_t cts_pin, uint8_t rts_pin, uint32_t baudrate)
{
    GLB_GPIO_Cfg_Type cfg;
    UART_CFG_Type subcaseUartCfg;
    UART_FifoCfg_Type fifoCfg;

    if (!(id < UART_NUMBER_SUPPORTED)) {
        return -1;
    }

    cfg.drive=0;
    cfg.smtCtrl=1;

    if (rx_pin != 0xFF) {
        /*set GPIO as UART TX */
        cfg.gpioPin = rx_pin;
        cfg.gpioFun = 7;
        cfg.gpioMode = GPIO_MODE_AF;
        cfg.pullType = GPIO_PULL_NONE;
        GLB_GPIO_Init(&cfg);
        /* select uart gpio function */
        GLB_Uart_Fun_Sel(rx_pin%12, id*4 + 3);
    }

    if (tx_pin != 0xFF) {
        /*set GPIO as UART RX */
        cfg.gpioPin = tx_pin;
        cfg.gpioFun = 7;
        cfg.gpioMode = GPIO_MODE_AF;
        cfg.pullType = GPIO_PULL_NONE;
        GLB_GPIO_Init(&cfg);
        /* select uart gpio function */
        GLB_Uart_Fun_Sel(tx_pin%12, id*4 + 2);
    }

    if ((cts_pin != 0xFF) || (rts_pin != 0xFF)) {
        log_error("cts not support!.\r\n");
    }

    // uart_env_prepare(baudrate);

    subcaseUartCfg.uartClk = 160 * 1000 * 1000;
    subcaseUartCfg.baudRate = baudrate;
    subcaseUartCfg.dataBits = UART_DATABITS_8;
    subcaseUartCfg.stickyParity = DISABLE;
    subcaseUartCfg.parity = UART_PARITY_NONE;
    subcaseUartCfg.highSpeedUart = DISABLE;
    subcaseUartCfg.nrzCodeing = DISABLE;
    fifoCfg.fifoEnable = ENABLE;
    fifoCfg.autoFlowControl = DISABLE;
    fifoCfg.rxFifoReset = ENABLE;
    fifoCfg.txFifoReset = ENABLE;
    fifoCfg.periBusType = UART_PBUS_BITS_8;
    fifoCfg.fifoDmaEnable = DISABLE;
    fifoCfg.rxFifoLevel = UART_RXFIFO_BYTES_32;
    fifoCfg.txFifoLevel = UART_TXFIFO_HALF_EMPTY;//Warning, don't change this settings.

    /* disable all interrupt */
    UART_IntMask((UART_ID_Type)id, UART_INT_ALL, MASK);
    /* disable uart before config */
    UART_Disable((UART_ID_Type)id);
    /* uart init with default configuration */
    UART_Init((UART_ID_Type)id, &subcaseUartCfg);
    /* uart fifo init with default configuration */
    UART_FifoConfig((UART_ID_Type)id, &fifoCfg);
    /* enable uart */
    UART_Enable((UART_ID_Type)id);

    GLB_Set_UART_CLK(1 ,GLB_UART_CLK_FCLK, 0);

    return 0;
}

int bl_uart_early_init(uint8_t id, uint8_t tx_pin, uint32_t baudrate)
{
    GLB_GPIO_Cfg_Type cfg;
    UART_CFG_Type subcaseUartCfg;
    UART_FifoCfg_Type fifoCfg;

    /* init cfg */
    cfg.drive=0;
    cfg.smtCtrl=1;
    /*set GPIO as UART TX */
    cfg.gpioPin = tx_pin;//GLB_GPIO_PIN_17;
    cfg.gpioFun = 7;//GPIO17_FUN_UART_SIG_5;
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
    /* select uart gpio function */
    //GLB_Uart_Fun_Sel(GLB_UART_SIG_5, GLB_UART_SIG_FUN_UART2_TXD);
    GLB_Uart_Fun_Sel(tx_pin%12, id * 4 + 2);

    //FIXME attention with CLOCK issue
    subcaseUartCfg.uartClk = 160 * 1000 * 1000;

    subcaseUartCfg.baudRate = baudrate;
    subcaseUartCfg.dataBits = UART_DATABITS_8;
    subcaseUartCfg.stickyParity = DISABLE;
    subcaseUartCfg.parity = UART_PARITY_NONE;
    subcaseUartCfg.highSpeedUart = DISABLE;
    subcaseUartCfg.nrzCodeing = DISABLE;

    fifoCfg.fifoEnable = ENABLE;
    fifoCfg.autoFlowControl = DISABLE;
    fifoCfg.rxFifoReset = ENABLE;
    fifoCfg.txFifoReset = ENABLE;
    fifoCfg.periBusType = UART_PBUS_BITS_8;
    fifoCfg.fifoDmaEnable = DISABLE;
    fifoCfg.rxFifoLevel = UART_RXFIFO_BYTES_32;
    fifoCfg.txFifoLevel = UART_TXFIFO_HALF_EMPTY;//Warning, don't change this settings.

    /* disable all interrupt */
    UART_IntMask((UART_ID_Type)id, UART_INT_ALL, MASK);
    /* disable uart before config */
    UART_Disable((UART_ID_Type)id);
    /* uart init with default configuration */
    UART_Init((UART_ID_Type)id, &subcaseUartCfg);
    /* uart fifo init with default configuration */
    UART_FifoConfig((UART_ID_Type)id, &fifoCfg);
    /* enable uart */
    UART_Enable((UART_ID_Type)id);

    GLB_Set_UART_CLK(1 ,GLB_UART_CLK_FCLK, 0);

    return 0;
}

int bl_uart_debug_early_init(uint32_t baudrate)
{
    GLB_GPIO_Cfg_Type cfg;
    UART_CFG_Type subcaseUartCfg;
    UART_FifoCfg_Type fifoCfg;
    uint8_t id = 2;

    /* init cfg */
    cfg.drive=0;
    cfg.smtCtrl=1;
    /*set GPIO as UART TX */
    cfg.gpioPin = GLB_GPIO_PIN_14;
    cfg.gpioFun = GPIO14_FUN_UART_SIG_2;
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
    /* select uart gpio function */
    GLB_Uart_Fun_Sel(GLB_UART_SIG_2, GLB_UART_SIG_FUN_UART2_TXD);

    //FIXME attention with CLOCK issue
    subcaseUartCfg.uartClk = 160 * 1000 * 1000;

    subcaseUartCfg.baudRate = baudrate;
    subcaseUartCfg.dataBits = UART_DATABITS_8;
    subcaseUartCfg.stickyParity = DISABLE;
    subcaseUartCfg.parity = UART_PARITY_NONE;
    subcaseUartCfg.highSpeedUart = DISABLE;
    subcaseUartCfg.nrzCodeing = DISABLE;

    fifoCfg.fifoEnable = ENABLE;
    fifoCfg.autoFlowControl = DISABLE;
    fifoCfg.rxFifoReset = ENABLE;
    fifoCfg.txFifoReset = ENABLE;
    fifoCfg.periBusType = UART_PBUS_BITS_8;
    fifoCfg.fifoDmaEnable = DISABLE;
    fifoCfg.rxFifoLevel = UART_RXFIFO_BYTES_32;
    fifoCfg.txFifoLevel = UART_TXFIFO_HALF_EMPTY;//Warning, don't change this settings.

    /* disable all interrupt */
    UART_IntMask((UART_ID_Type)id, UART_INT_ALL, MASK);
    /* disable uart before config */
    UART_Disable((UART_ID_Type)id);
    /* uart init with default configuration */
    UART_Init((UART_ID_Type)id, &subcaseUartCfg);
    /* uart fifo init with default configuration */
    UART_FifoConfig((UART_ID_Type)id, &fifoCfg);
    /* enable uart */
    UART_Enable((UART_ID_Type)id);

    GLB_Set_UART_CLK(1 ,GLB_UART_CLK_FCLK, 0);

    return 0;
}

int bl_uart_int_rx_enable(uint8_t id)
{
    UART_IntMask((UART_ID_Type)id, UART_INT_RDA, UNMASK);
    UART_IntMask((UART_ID_Type)id, UART_INT_RTO, UNMASK);
    return 0;
}

int bl_uart_int_rx_disable(uint8_t id)
{
    UART_IntMask((UART_ID_Type)id, UART_INT_RDA, MASK);
    UART_IntMask((UART_ID_Type)id, UART_INT_RTO, MASK);
    return 0;
}

int bl_uart_int_tx_enable(uint8_t id)
{
    UART_IntMask((UART_ID_Type)id, UART_INT_TDR, UNMASK);
    return 0;
}

int bl_uart_int_tx_disable(uint8_t id)
{
    UART_IntMask((UART_ID_Type)id, UART_INT_TDR, MASK);
    return 0;
}

int bl_uart_flush(uint8_t id)
{
    while (SET != UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_TEMT)){
        /*empty*/
    }
    return 0;
}

void bl_uart_setbaud(uint8_t id, uint32_t baudrate)
{
    UART_CFG_Type subcaseUartCfg;

    subcaseUartCfg.uartClk = 160 * 1000 * 1000;
    subcaseUartCfg.baudRate = baudrate;
    subcaseUartCfg.dataBits = UART_DATABITS_8;
    subcaseUartCfg.stickyParity = DISABLE;
    subcaseUartCfg.parity = UART_PARITY_NONE;
    subcaseUartCfg.highSpeedUart = DISABLE;
    subcaseUartCfg.nrzCodeing = DISABLE;

    /* disable uart before config */
    UART_Disable((UART_ID_Type)id);
    /* uart init with default configuration */
    UART_Init((UART_ID_Type)id, &subcaseUartCfg);
    /* enable uart */
    UART_Enable((UART_ID_Type)id);
}

int bl_uart_data_send(uint8_t id, uint8_t data)
{ 
    while (SET != UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_TDRQ)) {
      /*empty*/
    }
    UART_SendData((UART_ID_Type)id, data);

    return 0;
}

int bl_uart_string_send(uint8_t id, char *data)
{
    int i = 0;
    char chr;

    while ((chr = data[i++])) {
        while (SET != UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_TDRQ)) {
            /*empty*/
        }
        UART_SendData((UART_ID_Type)id, chr);
    }

    return 0;
}

int bl_uart_datas_send(uint8_t id, uint8_t *data, int len)
{
    int i = 0;

    while (i < len) {
        while (SET != UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_TDRQ)) {
            /*empty*/
        }
        UART_SendData((UART_ID_Type)id, data[i]);
        i++;
    }

    return 0;
}

int bl_uart_data_recv(uint8_t id)
{
    if (SET == UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_DR)) {
        return UART_ReceiveData((UART_ID_Type)id);
    }
    return -1;
}

int bl_uart_int_status_clear(uint8_t id)
{
    uint32_t intId;
    uart_reg_t * UARTx;

    switch (id) {
        case 0:
        {
            UARTx = (uart_reg_t *)UART0_BASE;
        }
        break;
        case 1:
        {
            UARTx = (uart_reg_t *)UART1_BASE;
        }
        break;
        case 2:
        {
            UARTx = (uart_reg_t *)UART2_BASE;
        }
        break;
        default:
        {
            while (1) {
                /*hang here*/
            }
        }
    }
    intId = UARTx->IIR_FCR.WORD;
    (void)intId;
#if 0
    if(((intId >>1)&0x3) == 2) {
        rt_hw_serial_isr(&(uart0.uart), RT_SERIAL_EVENT_RX_IND);
    }
#endif

    return 0;
}

int bl_uart_int_enable(uint8_t id, uint8_t *rx_buffer, uint8_t *rx_idx_write, uint8_t *rx_idx_read, uint8_t *tx_buffer, uint8_t *tx_idx_write, uint8_t *tx_idx_read)
{
    /*setup ringbuffer*/
    //TODO use struct for ring buffer
    cbs_uart_notify_arg[id][1] = rx_buffer;
    cbs_uart_notify_arg[id][2] = rx_idx_write;
    cbs_uart_notify_arg[id][3] = rx_idx_read;
    cbs_uart_notify_arg[id][5] = tx_buffer;
    cbs_uart_notify_arg[id][6] = tx_idx_write;
    cbs_uart_notify_arg[id][7] = tx_idx_read;

    /*enable INT for receving data to ringbuffer*/
    switch (id) {
        case 0:
        {
            bl_uart_int_rx_enable(0);
            bl_uart_int_tx_enable(0);
            NVIC_SetPriority((IRQn_Type)UART0_IRQn, 7);
            NVIC_EnableIRQ((IRQn_Type)UART0_IRQn);
        }
        break;
        case 1:
        {
            bl_uart_int_rx_enable(1);
            bl_uart_int_tx_enable(1);
            NVIC_SetPriority((IRQn_Type)UART1_IRQn, 7);
            NVIC_EnableIRQ((IRQn_Type)UART1_IRQn);
        }
        break;
        case 2:
        {
            bl_uart_int_rx_enable(2);
            bl_uart_int_tx_enable(2);
            NVIC_SetPriority((IRQn_Type)UART2_IRQn, 7);
            NVIC_EnableIRQ((IRQn_Type)UART2_IRQn);
        }
        break;
        default:
        {
            return -1;
        }
    }

    return 0;
}

int bl_uart_int_cb_notify_register(uint8_t id, cb_uart_notify_t cb, void *arg)
{
    if (!(id < UART_NUMBER_SUPPORTED)) {
        /*UART ID overflow*/
        return -1;
    }

    cbs_uart_notify_arg[id][0] = arg;
    cbs_notify[id] = cb;

    return 0;
}

int bl_uart_int_cb_notify_unregister(uint8_t id, cb_uart_notify_t cb, void *arg)
{
    if (!(id < UART_NUMBER_SUPPORTED)) {
        /*UART ID overflow*/
        return -1;
    }
    cbs_uart_notify_arg[id][0] = NULL;
    cbs_notify[id] =NULL;

    return 0;
}

static inline void uart_generic_notify_handler(uint8_t id)
{
    uint32_t ch, intId, UARTx, intSrc;
    cb_uart_notify_t cb;
    void *arg;
    uint8_t *buffer, *idx_write, *idx_read;
    uint8_t idx_w, idx_r;
    int i;

    UARTx = uartAddr[id];
    intId = BL_RD_REG(UARTx, UART_IIR);
    bl_uart_int_status_clear(id);

        intSrc = BL_GET_REG_BITS_VAL(intId,UART_IID10);

        if (BL_IS_REG_BIT_SET(intId, UART_TOD) || intSrc == UART_INT_SRC_RDA) {
            /*Receive Data ready*/
            cb = cbs_notify[id];
            arg = cbs_uart_notify_arg[id][0];
            buffer = cbs_uart_notify_arg[id][1];
            idx_write = cbs_uart_notify_arg[id][2];
            idx_read = cbs_uart_notify_arg[id][3];

            idx_w = *idx_write;
            idx_r = *idx_read;

            while (SET == UART_GetLineStatus((UART_ID_Type)id, UART_LINESTATUS_DR)) {
                /*we loop untill buffer is empty*/
                //TODO untill buffer is NOT full?
                ch = UART_ReceiveData((UART_ID_Type)id);
                if (((idx_w + 1) & BL_UART_BUFFER_SIZE_MASK) != idx_r) {
                    /*buffer is not full. so we read to ring buffer and callback*/
                    buffer[idx_w] = ch;
                    idx_w = ((idx_w + 1) & BL_UART_BUFFER_SIZE_MASK);
                } else {
                    /*FIXME data is droped here*/
                }
            }
            *idx_write = idx_w;
            if (cb) {
                /*notify up layer*/
                cb(arg);
            }
        }

        intId = BL_RD_REG(UARTx, UART_LSR);
        /* Transmit data request interrupt */
        if (BL_IS_REG_BIT_SET(intId, UART_TDRQ)) {
            buffer = cbs_uart_notify_arg[id][5];
            idx_write = cbs_uart_notify_arg[id][6];
            idx_read = cbs_uart_notify_arg[id][7];

            idx_w = *idx_write;
            idx_r = *idx_read;
            /*Burst write with HALF FIFO*/
            i = 0;
            while ((i++) < FIFO_TX_SIZE_BURST) {
                if (idx_r != idx_w) {
                    UART_SendData((UART_ID_Type)id, buffer[idx_r]);
                } else {
                    /*No more data for write*/
                    break;
                }
                idx_r = ((idx_r + 1) & BL_UART_BUFFER_SIZE_MASK);
            }
            if ((*idx_read) == idx_r) {
                /*No data for tx during this loop, so disable it*/
                bl_uart_int_tx_disable(id);
            } else {
                /*Update ringbuffer status*/
                *idx_read = idx_r;
            }
        }
    return;
}

void UART0_IRQHandler(void)
{
    uart_generic_notify_handler(0);
}

void UART1_IRQHandler(void)
{
    uart_generic_notify_handler(1);
}

void UART2_IRQHandler(void)
{
    uart_generic_notify_handler(2);
}
