#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "platform_gpio.h"
#include "platform_device.h"

#ifndef USE_UART_OUTPUT
#define USE_UART_OUTPUT     1
#endif

static uint8_t uart_dbg_disable=0;
#ifdef BOOTROM
#define UART_DBG_ID     UART1_ID
//static IRQn_Type    uart_dbg_irqn=UART1_IRQn;
#else
#define UART_DBG_ID     UART0_ID
//static IRQn_Type    uart_dbg_irqn=UART0_IRQn;
#endif



static UART_CFG_Type uart_dbg_cfg={  
                                32*1000*1000,         /*UART clock*/
                                2000000,              /* baudrate  */
                                UART_DATABITS_8,      /* data bits  */
                                UART_STOPBITS_1,      /* stop bits */
                                UART_PARITY_NONE,     /* parity  */
                                DISABLE,              /* Disable auto flow control */
                                DISABLE,              /* rx input de-glitch function */
                                DISABLE,              /* Disable RTS output SW control mode */
                                UART_LSB_FIRST,       /* UART each data byte is send out LSB-first */
                              };
static UART_FifoCfg_Type fifoCfg = {
                                16,                   /* TX FIFO threshold */
                                16,                   /* RX FIFO threshold */
                                DISABLE,              /* Disable tx dma req/ack interface */
                                DISABLE               /* Disable rx dma req/ack interface */
};

static const uint8_t hexTable[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

#define BFLB_SYSTICK_INT_RATE   20
static volatile uint32_t systick_int_cnt=0;

void bflb_platform_uart_dbg_init(uint32_t bdrate)
{
    /* init debug uart gpio first */
    bflb_platform_init_uart_debug_gpio();
    
    /* Init UART clock*/
    GLB_Set_UART_CLK(ENABLE ,HBN_UART_CLK_FCLK,0);
    uart_dbg_cfg.uartClk=SystemCoreClockGet();
    
    if(bdrate!=0){
        uart_dbg_cfg.baudRate=bdrate;
    }

    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_UART0+UART_DBG_ID);

    /* disable all interrupt */
    UART_IntMask(UART_DBG_ID, UART_INT_ALL, MASK);

    /* disable uart before config */
    UART_Disable(UART_DBG_ID,UART_TXRX);

    /* uart init with default configuration */
    UART_Init(UART_DBG_ID, &uart_dbg_cfg);

    /* UART fifo configuration */
    UART_FifoConfig(UART_DBG_ID,&fifoCfg);


    /* Enable tx free run mode */
    UART_TxFreeRun(UART_DBG_ID,ENABLE);

    /* Set rx time-out value */
    UART_SetRxTimeoutValue(UART_DBG_ID,16);

    /* enable uart */
    UART_Enable(UART_DBG_ID,UART_TXRX);
}

void bflb_platform_usart_dbg_send(uint8_t *data,uint32_t len)
{
    UART_SendData(UART_DBG_ID,data,len);
}

void bflb_platform_uart_dbg_deinit()
{
    UART_Disable(UART_DBG_ID,UART_TXRX);
    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_UART0+UART_DBG_ID);
    bflb_platform_deinit_uart_debug_gpio();
}

void bflb_platform_printf(char *fmt,...)
{
    static char print_buf[128];
    va_list ap;
    if(!uart_dbg_disable){
        va_start(ap, fmt);
        vsnprintf(print_buf, sizeof(print_buf)-1, fmt, ap);
        va_end(ap);
        bflb_platform_usart_dbg_send((uint8_t*)print_buf,strlen(print_buf));
    }
}

void bflb_platform_dump(const void *data,uint32_t len)
{
    uint32_t i=0;
    uint8_t *p=(uint8_t *)data;

    for(i=0;i<len;i++){
        if(i%16==0){
            bflb_platform_printf("\r\n");
        }
        bflb_platform_printf("%02x ",p[i]);
    }
    bflb_platform_printf("\r\n");
}

void bflb_platform_prints(char *data)
{
    bflb_platform_usart_dbg_send((uint8_t*)data,strlen(data));
}

void bflb_platform_printx(uint32_t val)
{
    uint8_t print_buf[8];
    int32_t i;
    for(i=7;i>=0;i--){
        print_buf[i]=hexTable[(val&0xf)];
        val>>=4;
    }
    bflb_platform_usart_dbg_send((uint8_t*)print_buf,8);
}

void bflb_platform_printc(char c)
{
    UART_SendData(UART_DBG_ID,(uint8_t *)&c,1);
}

void bflb_platform_clear_time()
{
    *(volatile uint64_t*) (CLIC_CTRL_ADDR + CLIC_MTIME) = 0;
}

uint64_t  bflb_platform_get_time_us()
{

    uint32_t tmpValLow,tmpValHigh,tmpValLow1,tmpValHigh1;
    uint32_t cnt=0;
    uint32_t tmp;

    do{
        tmpValLow=*(volatile uint32_t*) (CLIC_CTRL_ADDR + CLIC_MTIME);
        tmpValHigh=*(volatile uint32_t*) (CLIC_CTRL_ADDR + CLIC_MTIME+4);
        tmpValLow1=*(volatile uint32_t*) (CLIC_CTRL_ADDR + CLIC_MTIME);
        tmpValHigh1=*(volatile uint32_t*) (CLIC_CTRL_ADDR + CLIC_MTIME+4);
        cnt++;
        if(cnt>4){
            break;
        }
    }while(tmpValLow>tmpValLow1||tmpValHigh>tmpValHigh1);
#if 0
    tmp=(SystemCoreClockGet()>>3)/1000000;
#else
    tmp=2000000/1000000;
#endif
    if(tmpValHigh1==0){
    	return(uint64_t)(tmpValLow1/tmp);
    }else{
    	return (((uint64_t)tmpValHigh1<<32)+tmpValLow1)/tmp;
    }
}

void bflb_platform_start_time()
{
    *(volatile uint64_t*) (CLIC_CTRL_ADDR + CLIC_MTIME) = 0;
}

void bflb_platform_stop_time()
{
    
}

void bflb_platform_init_time()
{   
    NVIC_DisableIRQ(MTIME_IRQn);
    /* Set MTimer the same frequency as SystemCoreClock */
    GLB_Set_MTimer_CLK(1,GLB_MTIMER_CLK_CPU_CLK,7);
    bflb_platform_clear_time();
}

void bflb_platform_deinit_time()
{
    NVIC_DisableIRQ(MTIME_IRQn);
    bflb_platform_stop_time();
}

void bflb_platform_delay_ms(uint32_t time)
{
    uint64_t cnt=0;

    bflb_platform_stop_time();
    bflb_platform_start_time();
    while(bflb_platform_get_time_us()<time*1000){
        cnt++;
        if(cnt>time*1000*160){
            break;
        }
    }
}

void bflb_platform_delay_us(uint32_t time)
{
    uint64_t cnt=0;

    bflb_platform_start_time();
    while(bflb_platform_get_time_us()<time){
        cnt++;
        if(cnt>time*160){
            break;
        }
    }
}

void bflb_platform_init(uint32_t baudrate)
{
    bflb_platform_init_time();
    
    Sec_Eng_Trng_Enable();

    if(!uart_dbg_disable){
        bflb_platform_uart_dbg_init(baudrate);
        bflb_platform_prints("system clock=");
        bflb_platform_printx(SystemCoreClockGet()/1000000);
    }
}


void bflb_platform_deinit()
{
    if(!uart_dbg_disable){
        bflb_platform_uart_dbg_deinit();
    }
    bflb_platform_deinit_time();
}

void bflb_platform_print_set(uint8_t logDisable)
{
    uart_dbg_disable=logDisable;
}

int bflb_platform_get_random(uint8_t *data,uint32_t len)
{
    uint8_t tmpBuf[32];
    uint32_t readLen=0;
    uint32_t i=0,cnt=0;

    while(readLen<len){
        if(Sec_Eng_Trng_Read(tmpBuf)!=SUCCESS){
            return -1;
        }
        cnt=len-readLen;
        if(cnt>sizeof(tmpBuf)){
            cnt=sizeof(tmpBuf);
        }
        for(i=0;i<cnt;i++){
            data[readLen+i]=tmpBuf[i];
        }
        readLen+=cnt;
    }

    return 0;
}
