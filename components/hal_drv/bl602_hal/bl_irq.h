#ifndef __BL_IRQ_H__
#define __BL_IRQ_H__
void bl_irq_enable(unsigned int source);
void bl_irq_disable(unsigned int source);
typedef enum {
    BL_IRQ_EXCEPTION_TYPE_LOAD_MISALIGN,
    BL_IRQ_EXCEPTION_TYPE_STORE_MISALIGN,
    BL_IRQ_EXCEPTION_TYPE_ACCESS_ILLEGAL,
    BL_IRQ_EXCEPTION_TYPE_ILLEGAL_INSTRUCTION,
} BL_IRQ_EXCEPTION_TYPE_T;
void bl_irq_exception_trigger(BL_IRQ_EXCEPTION_TYPE_T type, void *ptr);

void bl_irq_init(void);
/*The following section define the IRQ handler for other files*/
void bl_sec_aes_IRQHandler(void);
void bl_sec_sha_IRQHandler(void);
void bl_sec_pka_IRQHandler(void);
void bl_dma_IRQHandler(void);
void intc_irq(void);//MAC IRQ
void rwble_isr(void);
void bl_irq_handler(void);//IPC host IRQ

//XXX we should use IRQ number for header file
#define WIFI_IRQn (54 + 16)
#define WIFI_IPC_PUBLIC_IRQn (63 + 16)
#endif
