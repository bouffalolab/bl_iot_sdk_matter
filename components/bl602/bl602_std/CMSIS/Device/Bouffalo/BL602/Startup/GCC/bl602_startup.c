/**************************************************************************//**
 * @file     startup_ARMCM4.s
 * @brief    CMSIS Core Device Startup File for
 *           ARMCM4 Device Series
 * @version  V5.00
 * @date     26. April 2016
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "bl70x.h"
/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __etext0;
extern uint32_t __etext1;
extern uint32_t __etext2;
extern uint32_t __tcm_code_start__;
extern uint32_t __tcm_code_end__;
extern uint32_t __ocram_data_start__;
extern uint32_t __ocram_data_end__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );


/*----------------------------------------------------------------------------
  Local define
 *----------------------------------------------------------------------------*/
/*TODO: DV will clear all RAM */
//#define __STARTUP_CLEAR_BSS
#define __START     main

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
#ifndef __START
extern void  _start(void) __attribute__((noreturn));    /* PreeMain (C library entry point) */
#else
extern int  __START(void) __attribute__((noreturn));    /* main entry point */
#endif

#ifndef __NO_SYSTEM_INIT
extern void SystemInit (void);            /* CMSIS System Initialization      */
#endif

#ifdef BOOTROM
#define MSG(...)
#endif

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void);                          /* Default empty handler */
void Reset_Handler(void);                            /* Reset Handler */


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
#ifndef __STACK_SIZE
  #define	__STACK_SIZE  0x00000400
#endif
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

#ifndef __HEAP_SIZE
  #define	__HEAP_SIZE   0x00000400
#endif
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler              (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler                (void) __attribute__ ((weak, alias("Default_Handler")));

void NMI_Handler(void)
{
    while(1){};
}

void MemManage_Handler(void)
{
    while(1){};
}
void BusFault_Handler(void)
{
    while(1){};
}
void UsageFault_Handler(void)
{
    while(1){};
}
void SVC_Handler(void)
{
    while(1){};
}
void DebugMon_Handler(void)
{
    while(1){};
}
void PendSV_Handler(void)
{
    while(1){};
}

#if defined(CPU_AP_CM4) || defined(CPU_AP_E21)
void BMX_ERR_IRQHandler                (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< BMX Error Interrupt                                               */
void BMX_TO_IRQHandler                 (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< BMX Timeout Interrupt                                             */
void GPIO_IRQ0_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQ1_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT0_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT1_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID1_CDET_INT_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID1_PKA_INT_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID1_TRNG_INT_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID1_AES_INT_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID1_SHA_INT_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH0_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 0 Interrupt                                          */
void DMA0_CH1_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 1 Interrupt                                          */
void DMA0_CH2_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 2 Interrupt                                          */
void DMA0_CH3_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 3 Interrupt                                          */
void DMA0_CH4_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 4 Interrupt                                          */
void DMA0_CH5_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 5 Interrupt                                          */
void DMA0_CH6_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 6 Interrupt                                          */
void DMA0_CH7_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 7 Interrupt                                          */
void SF_CTRL_ID1_NP_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SF_CTRL_ID1_AP_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void GPADC_DMA_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void EFUSE_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void UART_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER0_WDG_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER1_CH0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER1_CH1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void TIMER1_CH2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void TIMER1_WDG_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void SPI2_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void PDS_WAKEUP_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void HBN_OUT0_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler"))); 
void HBN_OUT1_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler"))); 
void BOR_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler"))); 
void MAC154_INT_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void MAC154_AES_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void BLE_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void USB_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void TOUCH_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
#endif
#ifdef CPU_NP_E21
void BMX_ERR_IRQHandler                (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< BMX Error Interrupt                                               */
void BMX_TO_IRQHandler                 (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< BMX Timeout Interrupt                                             */
void GPIO_IRQ0_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQ1_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT0_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT1_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_CDET_INT_IRQHandl     (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_PKA_INT_IRQHandle     (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_TRNG_INT_IRQHandle    (void) __attribute__ ((weak, alias("Default_Handler")));  
void SEC_ENG_ID0_AES_INT_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_SHA_INT_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH0_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 0 Interrupt                                          */
void DMA0_CH1_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 1 Interrupt                                          */
void DMA0_CH2_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 2 Interrupt                                          */
void DMA0_CH3_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 3 Interrupt                                          */
void DMA0_CH4_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 4 Interrupt                                          */
void DMA0_CH5_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 5 Interrupt                                          */
void DMA0_CH6_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 6 Interrupt                                          */
void DMA0_CH7_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));        /*!< DMA0 Channel 7 Interrupt                                          */
void SF_CTRL_ID0_NP_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SF_CTRL_ID0_AP_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void GPADC_DMA_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void EFUSE_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void UART_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER0_WDG_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER1_CH0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));  
void TIMER1_CH1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void TIMER1_CH2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void TIMER1_WDG_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void SPI2_IRQHandler                   (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_NP2AP3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void IPC_AP2NP3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void PDS_WAKEUP_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler"))); 
void HBN_OUT0_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler"))); 
void HBN_OUT1_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler"))); 
void BOR_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler"))); 
void MAC154_INT_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void MAC154_AES_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void BLE_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void USB_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void TOUCH_IRQHandler                  (void) __attribute__ ((weak, alias("Default_Handler")));
#endif
#ifdef CPU_NP_CM0
void BMX_ERR_OR_I2C0_IRQHandler                     (void) __attribute__ ((weak, alias("Default_Handler")));
void BMX_TO_OR_I2C1_IRQHandler                      (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQ0_OR_PWM_IRQHandler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQ1_OR_TIMER0_CH0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_CH1_IRQHandler                          (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT0_OR_TIMER0_CH2_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));
void RF_TOP_INT1_OR_TIMER0_WDG_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_CH0_IRQHandler                          (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_CH1_IRQHandler                          (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_CH2_IRQHandler                          (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_CDET_INT_OR_TIMER1_WDG_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_PKA_INT_OR_SPI2_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_TRNG_INT_OR_IPC_NP2AP0_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_AES_INT_OR_IPC_NP2AP1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_ENG_ID0_SHA_INT_OR_IPC_NP2AP2_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH0_INT_OR_IPC_NP2AP3_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH1_INT_OR_IPC_AP2NP0_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH2_INT_OR_IPC_AP2NP1_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH3_INT_OR_IPC_AP2NP2_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH4_INT_OR_IPC_AP2NP3_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH5_INT_OR_PDS_WAKEUP_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH6_INT_OR_HBN_OUT0_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA0_CH7_INT_OR_HBN_OUT1_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void SF_CTRL_ID0_NP_OR_BOR_IRQHandler               (void) __attribute__ ((weak, alias("Default_Handler")));
void SF_CTRL_ID0_AP_OR_MAC154_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void GPADC_DMA_MAC154_AES_IRQHandler                (void) __attribute__ ((weak, alias("Default_Handler")));
void EFUSE_OR_BLE_IRQHandler                        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI0_OR_USB_IRQHandler                         (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_OR_TOUCH_IRQHandler                       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART_IRQHandler                                (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler                               (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler                               (void) __attribute__ ((weak, alias("Default_Handler")));
#endif
/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
const pFunc __Vectors[] __attribute__ ((section(".vectors"))) = {
#if defined(CPU_AP_CM4) || defined(CPU_AP_E21)
  /* Cortex-M4 Exceptions Handler */
  (pFunc)((uint32_t)&__StackTop),           /*!< Initial Stack Pointer                                              */
  Reset_Handler,                            /*   Reset Handler                                                      */
  NMI_Handler,                              /*!< NMI Handler                                                        */
  HardFault_Handler,                        /*!< Hard Fault Handler                                                 */
  MemManage_Handler,                        /*!< MPU Fault Handler                                                  */
  BusFault_Handler,                         /*!< Bus Fault Handler                                                  */
  UsageFault_Handler,                       /*!< Usage Fault Handler                                                */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  SVC_Handler,                              /*!< SVCall Handler                                                     */
  DebugMon_Handler,                         /*!< Debug Monitor Handler                                              */
  0,                                        /*!< Reserved                                                           */
  PendSV_Handler,                           /*!< PendSV Handler                                                     */
  SysTick_Handler,                          /*!< SysTick Handler                                                    */
  /* External interrupts */
  BMX_ERR_IRQHandler,                           /*!< BMX Error Interrupt                                               */
  BMX_TO_IRQHandler,                            /*!< BMX Timeout Interrupt                                             */
  GPIO_IRQ0_IRQHandler,
  GPIO_IRQ1_IRQHandler,
  0,
  RF_TOP_INT0_IRQHandler,
  RF_TOP_INT1_IRQHandler,
  0,
  0,
  0,
  SEC_ENG_ID1_CDET_INT_IRQHandler,
  SEC_ENG_ID1_PKA_INT_IRQHandler,
  SEC_ENG_ID1_TRNG_INT_IRQHandler,
  SEC_ENG_ID1_AES_INT_IRQHandler,
  SEC_ENG_ID1_SHA_INT_IRQHandler,
  DMA0_CH0_IRQHandler,                          /*!< DMA0 Channel 0 Interrupt                                          */
  DMA0_CH1_IRQHandler,                          /*!< DMA0 Channel 1 Interrupt                                          */
  DMA0_CH2_IRQHandler,                          /*!< DMA0 Channel 2 Interrupt                                          */
  DMA0_CH3_IRQHandler,                          /*!< DMA0 Channel 3 Interrupt                                          */
  DMA0_CH4_IRQHandler,                          /*!< DMA0 Channel 4 Interrupt                                          */
  DMA0_CH5_IRQHandler,                          /*!< DMA0 Channel 5 Interrupt                                          */
  DMA0_CH6_IRQHandler,                          /*!< DMA0 Channel 6 Interrupt                                          */
  DMA0_CH7_IRQHandler,                          /*!< DMA0 Channel 7 Interrupt                                          */
  SF_CTRL_ID1_NP_IRQHandler,
  SF_CTRL_ID1_AP_IRQHandler,
  GPADC_DMA_IRQHandler,
  EFUSE_IRQHandler,
  SPI0_IRQHandler,
  SPI1_IRQHandler,
  UART_IRQHandler,
  UART1_IRQHandler,
  UART2_IRQHandler,
  I2C0_IRQHandler,                              /*!< I2C 0 Interrupt                                                   */
  I2C1_IRQHandler,                              /*!< I2C 1 Interrupt                                                   */
  PWM_IRQHandler,                               /*!< PWM Interrupt                                                     */
  TIMER0_CH0_IRQHandler,                        /*!< Timer 0 Channel 0 Interrupt                                       */
  TIMER0_CH1_IRQHandler,                        /*!< Timer 0 Channel 1 Interrupt                                       */
  TIMER0_CH2_IRQHandler,                        /*!< Timer 0 Channel 2 Interrupt                                       */
  TIMER0_WDG_IRQHandler,                        /*!< Timer 0 Watch Dog Interrupt                                       */
  TIMER1_CH0_IRQHandler,                        /*!< Timer 1 Channel 0 Interrupt                                       */
  TIMER1_CH1_IRQHandler,                        /*!< Timer 1 Channel 1 Interrupt                                       */
  TIMER1_CH2_IRQHandler,                        /*!< Timer 1 Channel 2 Interrupt                                       */
  TIMER1_WDG_IRQHandler,                        /*!< Timer 1 Watch Dog Interrupt                                       */
  SPI2_IRQHandler,
  IPC_NP2AP0_IRQHandler,                        /*!< IPC NP to AP Interrupt 0                                          */
  IPC_NP2AP1_IRQHandler,                        /*!< IPC NP to AP Interrupt 1                                          */
  IPC_NP2AP2_IRQHandler,                        /*!< IPC NP to AP Interrupt 2                                          */
  IPC_NP2AP3_IRQHandler,                        /*!< IPC NP to AP Interrupt 3                                          */
  IPC_AP2NP0_IRQHandler,                        /*!< IPC AP to NP Interrupt 0                                          */
  IPC_AP2NP1_IRQHandler,                        /*!< IPC AP to NP Interrupt 1                                          */
  IPC_AP2NP2_IRQHandler,                        /*!< IPC AP to NP Interrupt 2                                          */
  IPC_AP2NP3_IRQHandler,                        /*!< IPC AP to NP Interrupt 3                                          */
  PDS_WAKEUP_IRQHandler,                        /*!< PDS WAKEUP Interrupt                                              */
  HBN_OUT0_IRQHandler,                          /*!< Hibernate out 0 Interrupt                                         */
  HBN_OUT1_IRQHandler,                          /*!< Hibernate out 1 Interrupt                                         */
  BOR_IRQHandler,                               /*!< BOR Interrupt                                                     */
  MAC154_INT_IRQHandler,
  MAC154_AES_IRQHandler,
  BLE_IRQHandler,
  USB_IRQHandler,
  TOUCH_IRQHandler,
#endif
#ifdef CPU_NP_E21
  /* Cortex-M4 Exceptions Handler */
  (pFunc)((uint32_t)&__StackTop),           /*!< Initial Stack Pointer                                              */
  Reset_Handler,                            /*   Reset Handler                                                      */
  NMI_Handler,                              /*!< NMI Handler                                                        */
  HardFault_Handler,                        /*!< Hard Fault Handler                                                 */
  MemManage_Handler,                        /*!< MPU Fault Handler                                                  */
  BusFault_Handler,                         /*!< Bus Fault Handler                                                  */
  UsageFault_Handler,                       /*!< Usage Fault Handler                                                */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  SVC_Handler,                              /*!< SVCall Handler                                                     */
  DebugMon_Handler,                         /*!< Debug Monitor Handler                                              */
  0,                                        /*!< Reserved                                                           */
  PendSV_Handler,                           /*!< PendSV Handler                                                     */
  SysTick_Handler,                          /*!< SysTick Handler                                                    */
  /* External interrupts */
  BMX_ERR_IRQHandler,                           /*!< BMX Error Interrupt                                               */
  BMX_TO_IRQHandler,                            /*!< BMX Timeout Interrupt                                             */
  GPIO_IRQ0_IRQHandler,
  GPIO_IRQ1_IRQHandler,
  0,
  RF_TOP_INT0_IRQHandler,
  RF_TOP_INT1_IRQHandler,
  0,
  0,
  0,
  SEC_ENG_ID0_CDET_INT_IRQHandler,
  SEC_ENG_ID0_PKA_INT_IRQHandler,
  SEC_ENG_ID0_TRNG_INT_IRQHandler,
  SEC_ENG_ID0_AES_INT_IRQHandler,
  SEC_ENG_ID0_SHA_INT_IRQHandler,
  DMA0_CH0_IRQHandler,                          /*!< DMA0 Channel 0 Interrupt                                          */
  DMA0_CH1_IRQHandler,                          /*!< DMA0 Channel 1 Interrupt                                          */
  DMA0_CH2_IRQHandler,                          /*!< DMA0 Channel 2 Interrupt                                          */
  DMA0_CH3_IRQHandler,                          /*!< DMA0 Channel 3 Interrupt                                          */
  DMA0_CH4_IRQHandler,                          /*!< DMA0 Channel 4 Interrupt                                          */
  DMA0_CH5_IRQHandler,                          /*!< DMA0 Channel 5 Interrupt                                          */
  DMA0_CH6_IRQHandler,                          /*!< DMA0 Channel 6 Interrupt                                          */
  DMA0_CH7_IRQHandler,                          /*!< DMA0 Channel 7 Interrupt                                          */
  SF_CTRL_ID0_NP_IRQHandler,
  SF_CTRL_ID0_AP_IRQHandler,
  GPADC_DMA_IRQHandler,
  EFUSE_IRQHandler,
  SPI0_IRQHandler,
  SPI1_IRQHandler,
  UART_IRQHandler,
  UART1_IRQHandler,
  UART2_IRQHandler,
  I2C0_IRQHandler,                              /*!< I2C 0 Interrupt                                                   */
  I2C1_IRQHandler,                              /*!< I2C 1 Interrupt                                                   */
  PWM_IRQHandler,                               /*!< PWM Interrupt                                                     */
  TIMER0_CH0_IRQHandler,                        /*!< Timer 0 Channel 0 Interrupt                                       */
  TIMER0_CH1_IRQHandler,                        /*!< Timer 0 Channel 1 Interrupt                                       */
  TIMER0_CH2_IRQHandler,                        /*!< Timer 0 Channel 2 Interrupt                                       */
  TIMER0_WDG_IRQHandler,                        /*!< Timer 0 Watch Dog Interrupt                                       */
  TIMER1_CH0_IRQHandler,                        /*!< Timer 1 Channel 0 Interrupt                                       */
  TIMER1_CH1_IRQHandler,                        /*!< Timer 1 Channel 1 Interrupt                                       */
  TIMER1_CH2_IRQHandler,                        /*!< Timer 1 Channel 2 Interrupt                                       */
  TIMER1_WDG_IRQHandler,                        /*!< Timer 1 Watch Dog Interrupt                                       */
  SPI2_IRQHandler,
  IPC_NP2AP0_IRQHandler,                        /*!< IPC NP to AP Interrupt 0                                          */
  IPC_NP2AP1_IRQHandler,                        /*!< IPC NP to AP Interrupt 1                                          */
  IPC_NP2AP2_IRQHandler,                        /*!< IPC NP to AP Interrupt 2                                          */
  IPC_NP2AP3_IRQHandler,                        /*!< IPC NP to AP Interrupt 3                                          */
  IPC_AP2NP0_IRQHandler,                        /*!< IPC AP to NP Interrupt 0                                          */
  IPC_AP2NP1_IRQHandler,                        /*!< IPC AP to NP Interrupt 1                                          */
  IPC_AP2NP2_IRQHandler,                        /*!< IPC AP to NP Interrupt 2                                          */
  IPC_AP2NP3_IRQHandler,                        /*!< IPC AP to NP Interrupt 3                                          */
  PDS_WAKEUP_IRQHandler,                        /*!< PDS WAKEUP Interrupt                                              */
  HBN_OUT0_IRQHandler,                          /*!< Hibernate out 0 Interrupt                                         */
  HBN_OUT1_IRQHandler,                          /*!< Hibernate out 1 Interrupt                                         */
  BOR_IRQHandler,                               /*!< BOR Interrupt                                                     */
  MAC154_INT_IRQHandler,
  MAC154_AES_IRQHandler,
  BLE_IRQHandler,
  USB_IRQHandler,
  TOUCH_IRQHandler,
#endif
#ifdef CPU_NP_CM0
  /* Cortex-M0 Exceptions Handler */
  (pFunc)((uint32_t)&__StackTop),           /*!< Initial Stack Pointer                                              */
  Reset_Handler,                            /*   Reset Handler                                                      */
  NMI_Handler,                              /*!< NMI Handler                                                        */
  HardFault_Handler,                        /*!< Hard Fault Handler                                                 */
  0,
  0,
  0,
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  SVC_Handler,                              /*!< SVCall Handler                                                     */
  0,                                        /*!< Reserved                                                           */
  0,                                        /*!< Reserved                                                           */
  PendSV_Handler,                           /*!< PendSV Handler                                                     */
  SysTick_Handler,                          /*!< SysTick Handler                                                    */
  /* External interrupts */
  BMX_ERR_OR_I2C0_IRQHandler,
  BMX_TO_OR_I2C1_IRQHandler,
  GPIO_IRQ0_OR_PWM_IRQHandler,
  GPIO_IRQ1_OR_TIMER0_CH0_IRQHandler,
  TIMER0_CH1_IRQHandler,
  RF_TOP_INT0_OR_TIMER0_CH2_IRQHandler,
  RF_TOP_INT1_OR_TIMER0_WDG_IRQHandler,
  TIMER1_CH0_IRQHandler,
  TIMER1_CH1_IRQHandler,
  TIMER1_CH2_IRQHandler,
  SEC_ENG_ID0_CDET_INT_OR_TIMER1_WDG_IRQHandler,
  SEC_ENG_ID0_PKA_INT_OR_SPI2_IRQHandler,
  SEC_ENG_ID0_TRNG_INT_OR_IPC_NP2AP0_IRQHandler,
  SEC_ENG_ID0_AES_INT_OR_IPC_NP2AP1_IRQHandler,
  SEC_ENG_ID0_SHA_INT_OR_IPC_NP2AP2_IRQHandler,
  DMA0_CH0_INT_OR_IPC_NP2AP3_IRQHandler,
  DMA0_CH1_INT_OR_IPC_AP2NP0_IRQHandler,
  DMA0_CH2_INT_OR_IPC_AP2NP1_IRQHandler,
  DMA0_CH3_INT_OR_IPC_AP2NP2_IRQHandler,
  DMA0_CH4_INT_OR_IPC_AP2NP3_IRQHandler,
  DMA0_CH5_INT_OR_PDS_WAKEUP_IRQHandler,
  DMA0_CH6_INT_OR_HBN_OUT0_IRQHandler,
  DMA0_CH7_INT_OR_HBN_OUT1_IRQHandler,
  SF_CTRL_ID0_NP_OR_BOR_IRQHandler,
  SF_CTRL_ID0_AP_OR_MAC154_IRQHandler,
  GPADC_DMA_MAC154_AES_IRQHandler,
  EFUSE_OR_BLE_IRQHandler,
  SPI0_OR_USB_IRQHandler,
  SPI1_OR_TOUCH_IRQHandler,
  UART_IRQHandler,
  UART1_IRQHandler,
  UART2_IRQHandler,
#endif
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) {
  uint32_t *pSrc, *pDest;
  uint32_t *pTable __attribute__((unused));  

  /* set MSP anyway*/
  __set_MSP((uint32_t)&__StackTop);

/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
  pTable = &__copy_table_start__;

  for (; pTable < &__copy_table_end__; pTable = pTable + 3) {
		pSrc  = (uint32_t*)*(pTable + 0);
		pDest = (uint32_t*)*(pTable + 1);
		for (; pDest < (uint32_t*)(*(pTable + 1) + *(pTable + 2)) ; ) {
      *pDest++ = *pSrc++;
		}
	}
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
  pSrc  = &__etext0;
    pDest = &__tcm_code_start__;
    for ( ; pDest < &__tcm_code_end__ ; ) {
        *pDest++ = *pSrc++;
    }
    /* BF Add OCRAM data copy */
    pSrc  = &__etext1;
    pDest = &__ocram_data_start__;
    for ( ; pDest < &__ocram_data_end__ ; ) {
        *pDest++ = *pSrc++;
    }
    
    /* BF Add TCM data copy */
    pSrc  = &__etext2;
    pDest = &__data_start__;
    for ( ; pDest < &__data_end__ ; ) {
        *pDest++ = *pSrc++;
    }
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
  pTable = &__zero_table_start__;

  for (; pTable < &__zero_table_end__; pTable = pTable + 2) {
		pDest = (uint32_t*)*(pTable + 0);
		for (; pDest < (uint32_t*)(*(pTable + 0) + *(pTable + 1)) ; ) {
      *pDest++ = 0;
		}
	}
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
  pDest = &__bss_start__;

  for ( ; pDest < &__bss_end__ ; ) {
    *pDest++ = 0ul;
  }
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
#endif

#ifndef __START
#define __START _start
#endif
	__START();

}


/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void) {

	while(1);
}
