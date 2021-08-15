#include "bl60x_hbn.h"
#include "bl60x_glb.h"
#include "bl60x_aon.h"


static void bl_hbn_turn_gpio_into_hz(uint64_t wakeup_pin_map)
{
    uint32_t i=0;
    
    for(i=0;i<GLB_GPIO_PIN_MAX;i++){
        /* If the GPIO is used in HBN, continue */
        if (wakeup_pin_map&((uint64_t)1<<i)){
            continue;
        }
        GLB_GPIO_Set_HZ((GLB_GPIO_Type)i);
    }
}

void bl_hbn_enter_with_gpio_wakeup(uint64_t wakeup_pin_map,uint64_t edge)
{
    GLB_GPIO_Cfg_Type gpio_cfg={
        .gpioPin=GLB_GPIO_PIN_7,
        .gpioFun=11,
        .gpioMode=GPIO_MODE_INPUT,
        .pullType=GPIO_PULL_NONE, 
        .drive=0,
        .smtCtrl=1, 
    };
    uint32_t ie_enable=0;
    
    HBN_32K_Sel(HBN_32K_RC);
    AON_Power_Off_Xtal_32K();
    
    HBN_Clear_IRQ(HBN_INT_GPIO7);
    HBN_Clear_IRQ(HBN_INT_GPIO8);
    HBN_Clear_IRQ(HBN_INT_GPIO9);
    HBN_Clear_IRQ(HBN_INT_GPIO10);
    HBN_Clear_IRQ(HBN_INT_GPIO29);
    HBN_Clear_IRQ(HBN_INT_GPIO30);
    HBN_Clear_IRQ(HBN_INT_GPIO31);
    HBN_Clear_IRQ(HBN_INT_GPIO32);

    if(wakeup_pin_map&(1<<7)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_7;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<7)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO7,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO7,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x01;
    }
    if(wakeup_pin_map&(1<<8)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_8;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<8)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO8,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO8,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x02;
    }
    if(wakeup_pin_map&(1<<9)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_9;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<9)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO9,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO9,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x04;
    }
    if(wakeup_pin_map&(1<<10)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_10;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<10)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO10,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO10,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x08;
    }
    if(wakeup_pin_map&(1<<29)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_29;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<29)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO29,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO29,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x10;
    }
    if(wakeup_pin_map&(1<<30)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_30;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<30)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO30,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO30,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x20;
    }
    if(wakeup_pin_map&(1<<31)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_31;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&(1<<31)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO31,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO31,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x40;
    }
    if(wakeup_pin_map&((uint64_t)1<<32)){
        gpio_cfg.gpioPin=GLB_GPIO_PIN_8;
        GLB_GPIO_Init(&gpio_cfg);
        if(edge&((uint64_t)1<<32)){
            HBN_GPIO_INT_Enable(HBN_INT_GPIO32,HBN_GPIO_INT_TRIGGER_RISING);
        }else{
            HBN_GPIO_INT_Enable(HBN_INT_GPIO32,HBN_GPIO_INT_TRIGGER_FALLING);
        }
        ie_enable|=0x80;
    }
    
    NVIC_EnableIRQ(HBN_OUT0_IRQn);
    NVIC_EnableIRQ(HBN_OUT1_IRQn);
    
    bl_hbn_turn_gpio_into_hz(wakeup_pin_map);
    
    HBN_Enable(0,ie_enable,HBN_LDO_LEVEL_1P10V);
    
    BL60X_Delay_MS(1000);
    /* Never get here*/
    while(1);
}

void bl_hbn_enter_with_rtc_wakeup(uint32_t sleep_time_ms)
{
    uint32_t valLow=0,valHigh=0;
    uint64_t val;
    
    HBN_32K_Sel(HBN_32K_RC);
    AON_Power_Off_Xtal_32K();
    NVIC_EnableIRQ(HBN_OUT0_IRQn);
    HBN_Get_RTC_Timer_Val(&valLow,&valHigh);
    val=valLow+((uint64_t)valHigh<<32);
    val+=(sleep_time_ms*32768/1000);
    
    /* Set RTC Timer */
    HBN_Set_RTC_Timer(HBN_RTC_INT_DELAY_0T,val&0xffffffff,val>>32,HBN_RTC_COMP_BIT0_39);
    HBN_Enable_RTC_Counter();
    
    bl_hbn_turn_gpio_into_hz(0x0);
    HBN_Enable(0,0,HBN_LDO_LEVEL_1P10V);
    BL60X_Delay_MS(1000);
    /* Never get here*/
    while(1);
}
