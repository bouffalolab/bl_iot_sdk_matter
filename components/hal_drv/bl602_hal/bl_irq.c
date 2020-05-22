#include <stdint.h>
#include <stdio.h>

#include <bl602.h>

#include <clic.h>
#include "bl_irq.h"

/*Fixmed Currently we pu all the UART0_IRQHandler here*/
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void sec_trng_IRQHandler(void);

void bl_irq_enable(unsigned int source)
{
    *(volatile uint8_t*)(CLIC_HART0_ADDR + CLIC_INTIE + source) = 1;
}

void bl_irq_disable(unsigned int source)
{
    *(volatile uint8_t*)(CLIC_HART0_ADDR + CLIC_INTIE + source) = 0;
}

void bl_irq_pending_set(unsigned int source)
{
    *(volatile uint8_t*)(CLIC_HART0_ADDR + CLIC_INTIP + source) = 1;
}

void bl_irq_pending_clear(unsigned int source)
{
    *(volatile uint8_t*)(CLIC_HART0_ADDR + CLIC_INTIP + source) = 0;
}

void bl_irq_exception_trigger(BL_IRQ_EXCEPTION_TYPE_T type, void *ptr)
{
    uint32_t val = 0x12345678;;

    switch (type) {
        case BL_IRQ_EXCEPTION_TYPE_LOAD_MISALIGN:
        {
            val = *(uint32_t*)ptr;
        }
        break;
        case BL_IRQ_EXCEPTION_TYPE_STORE_MISALIGN:
        {
            *(uint32_t*)ptr = val;
        }
        break;
        case BL_IRQ_EXCEPTION_TYPE_ACCESS_ILLEGAL:
        {
            *(uint32_t*)ptr = val;
        }
        break;
        case BL_IRQ_EXCEPTION_TYPE_ILLEGAL_INSTRUCTION:
        {
            uint32_t fun_val = 0;
            typedef void (*ins_ptr_t)(void);
            ins_ptr_t func = (ins_ptr_t)&fun_val;

            func();
        }
        break;
        default:
        {
            /*nothing here*/
        }
    }
    printf("Trigger exception val is %08lx\r\n", val);
}

void bl_irq_default(void)
{
    while (1) {
        /*dead loop*/
    }
}

/*XXX
 *
 * Use default IRQ handler by default
 * 
 * */
void mac_irq(void)          __attribute__((weak, alias ("bl_irq_default")));
void rwble_isr(void)        __attribute__((weak, alias ("bl_irq_default")));
void bl_irq_handler(void)   __attribute__((weak, alias ("bl_irq_default")));

void interrupt_entry(uint32_t mcause) 
{
    //XXX magic number used here
    static void (*handler_list[16 + 64])(void) = {
        [UART0_IRQn]            = UART0_IRQHandler,
        [UART1_IRQn]            = UART1_IRQHandler,
        [SEC_PKA_IRQn]          = bl_sec_pka_IRQHandler,
        [SEC_TRNG_IRQn]         = sec_trng_IRQHandler,
        [SEC_AES_IRQn]          = bl_sec_aes_IRQHandler,
        [SEC_SHA_IRQn]          = bl_sec_sha_IRQHandler,
        [DMA_ALL_IRQn]          = bl_dma_IRQHandler,
        [WIFI_IRQn]             = mac_irq,
        [WIFI_IPC_PUBLIC_IRQn]  = bl_irq_handler,
        [BLE_IRQn]              = rwble_isr, //FIXME rename rwble_isr
    };
    void (*handler)(void) = NULL;

    mcause &= 0x7FFFFFF;
    if (mcause < sizeof(handler_list)/sizeof(handler_list[0])) {
        handler = handler_list[mcause];
    }
    if (handler) {
        handler();
    } else {
        printf("Cannot handle mcause 0x%lx:%lu, adjust to externel(0x%lx:%lu)\r\n",
                mcause,
                mcause,
                mcause - 16,
                mcause - 16
        );
        while (1) {
            /*dead loop now*/
        }
    }
}

static void __dump_exception_code_str(uint32_t code)
{
    printf("Exception code: %lu\r\n", code);
    switch (code) {
        case 0x00:
        /*Instruction address misaligned*/
        {
            puts("  msg: Instruction address misaligned\r\n");
        }
        break;
        case 0x01:
        /*Instruction access fault*/
        {
            puts("  msg: Instruction access fault\r\n");
        }
        break;
        case 0x02:
        /*Illegal instruction*/
        {
            puts("  msg: Illegal instruction\r\n");
        }
        break;
        case 0x03:
        /*Breakpoint*/
        {
            puts("  msg: Breakpoint\r\n");
        }
        break;
        case 0x04:
        /*Load address misaligned*/
        {
            puts("  msg: Load address misaligned\r\n");
        }
        break;
        case 0x05:
        /*Load access fault*/
        {
            puts("  msg: Load access fault\r\n");
        }
        break;
        case 0x06:
        /*Store/AMO access misaligned*/
        {
            puts("  msg: Store/AMO access misaligned\r\n");
        }
        break;
        case 0x07:
        /*Store/AMO access fault*/
        {
            puts("  msg: Store/AMO access fault\r\n");
        }
        break;
        case 0x08:
        /*Environment call from U-mode*/
        {
            puts("  msg: Environment call from U-mode\r\n");
        }
        break;
        case 0x09:
        /*Environment call from S-mode*/
        {
            puts("  msg: Environment call from S-mode\r\n");
        }
        break;
        case 0x0a:
        case 0x0e:
        /*Reserved*/
        {
            puts("  msg: Reserved\r\n");
        }
        break;
        case 0x0b:
        /*Environment call from M-mode*/
        {
            puts("  msg: Environment call from M-mode\r\n");
        }
        break;
        case 0x0c:
        /*Instruction page fault*/
        {
            puts("  msg: Instruction page fault\r\n");
        }
        break;
        case 0x0d:
        /*Load page fault*/
        {
            puts("  msg: Load page fault\r\n");
        }
        break;
        case 0x0f:
        /*Store/AMO page fault*/
        {
            puts("  msg: Store/AMO page fault\r\n");
        }
        break;
        default:{
            puts("  msg: Reserved default exception\r\n");
        }
    }
}

void exception_entry(uint32_t mcause, uint32_t mepc, uint32_t mtval)
{
    puts("Exception Entry--->>>\r\n");
    printf("mcause %08lx, mepc %08lx, mtval %08lx\r\n",
        mcause,
        mepc,
        mtval
    );
    __dump_exception_code_str(mcause & 0xFFFF);
    while (1) {
        /*Deap loop now*/
    }
}

void bl_irq_init(void)
{
    uint32_t ptr;

    puts("[IRQ] Clearing and Disable all the pending IRQ...\r\n");

    /*clear mask*/
    for (ptr = 0x02800400; ptr < 0x02800400 + 128; ptr++) {
        *(uint8_t*)ptr = 0;
    }
    /*clear pending*/
    for (ptr = 0x02800000; ptr < 0x02800000 + 128; ptr++) {
        *(uint8_t*)ptr = 0;
    }
}
