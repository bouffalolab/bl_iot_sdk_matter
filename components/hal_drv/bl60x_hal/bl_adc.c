#include <bl60x_adc.h>
#include <bl60x_gpio.h>
#include <bl60x_glb.h>
#include <bl60x_dma.h>
#include <bl60x_hbn.h>
#include "bl_adc.h"
//#include <bflb_platform.h>


/** @defgroup  ADC_SCAN_Private_Macros
 *  @{
 */

//All the pin has the same funcion number for ADC
#define ADC_GPIO_FUN        GPIO10_FUN_AUX_ADC_ACOMP_3
//All the pin has the same funcion number for HBN
#define HBN_GPIO_FUN        GPIO10_FUN_REG_GPIO_10
#define ADC_POS_CHANNEL     ADC_Chan3
#define ADC_NEG_CHANNEL     ADC_Chan3_NEG
#define ADC_CALIBRATION_INDEX    0xBFAB0000
#define ADC_CALIBRATION_FILTR    61440

uint8_t GpioInterruptFlag = 0;
static uint8_t count_int, count_adc;

/*we still use this ugly code but NOT array like code, because we still want toolchain GC works*/
static cb_adc_notify_t cb_notify_gpio10;
static void *cb_adc_notify_arg_gpio10;

/*@} end of group ADC_SCAN_Private_Macros */

/******************************************************************************

 * @brief  ADC  GPIO initialization
 *
 * @param  pin ADC port
 *
 * @return 0
 *
*******************************************************************************/
static void ADC_GPIO_Init(uint8_t pin)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.pullType=GPIO_PULL_NONE;
    cfg.drive=1;
    cfg.smtCtrl=1;
    cfg.gpioMode=GPIO_MODE_INPUT;
    cfg.gpioPin=pin;
    cfg.gpioFun=ADC_GPIO_FUN;

    GLB_GPIO_Init(&cfg);
}

/******************************************************************************

 * @brief  HBN  GPIO initialization
 *
 * @param  pin ADC port
 *
 * @return 0
 *
*******************************************************************************/
static void HBN_GPIO_Init(uint8_t pin)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.pullType=GPIO_PULL_NONE;
    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioMode=GPIO_MODE_INPUT;
    cfg.gpioPin=pin;
    cfg.gpioFun=HBN_GPIO_FUN;

    GLB_GPIO_Init(&cfg);
}

void HBN_IRQHandler(void)
{
    GpioInterruptFlag = 1;
    HBN_Clear_IRQ(HBN_INT_GPIO10);
}

/******************************************************************************
 * @brief  Enable gpio interrupt
 *
 * @param  pin ADC port
 *
 * @return 0
 *
*******************************************************************************/
static int Gpio_Interrupt_Enable(uint8_t pin, int rising_trigger)
{
    HBN_INT_Type int_type;

    switch (pin) {
        case 7:
        {
            int_type = HBN_INT_GPIO7;
        }
        break;
        case 8:
        {
            int_type = HBN_INT_GPIO8;
        }
        break;
        case 9:
        {
            int_type = HBN_INT_GPIO9;
        }
        break;
        case 10:
        {
            int_type = HBN_INT_GPIO10;
        }
        break;
        case 11:
        {
            int_type = HBN_INT_GPIO11;
        }
        break;
        case 29:
        {
            int_type = HBN_INT_GPIO29;
        }
        break;
        case 30:
        {
            int_type = HBN_INT_GPIO30;
        }
        break;
        case 31:
        {
            int_type = HBN_INT_GPIO31;
        }
        break;
        case 32:
        {
            int_type = HBN_INT_GPIO32;
        }
        break;
        default:
        {
            /*we may need to warn error here*/
            return -1;
        }
    }
    HBN_Clear_IRQ(int_type);
    HBN_GPIO_Init(pin);
    HBN_GPIO_INT_Enable(int_type, rising_trigger ? HBN_GPIO_INT_TRIGGER_RISING : HBN_GPIO_INT_TRIGGER_FALLING);
    return 0;
}

/****************************************************************************//**
 * @brief  ADC print one channel result
 *
 * @param  result: ADC result
 * @param  singleEnd: Wether it's single end result
 *
 * @return None
 *
*******************************************************************************/
static float ADC_Print_One_Chan_Result(uint32_t result)
{
    float vol = 0;

    /* ADC_INPUT_SINGLE_END mode 14 bits has no signed bit */
    result = (result&0xffff)>>2;
    vol = result*3.2/16384.0;
    return vol;
}

static inline ADC_Chan_Type _get_adc_channel(int pin)
{
    switch (pin) {
        case 7:
        /*GPIO7_FUN_AUX_ADC_0 */
        {
            return ADC_Chan0;
        }
        break;
        case 8:
        /*GPIO8_FUN_AUX_ADC_1 */
        {
            return ADC_Chan1;
        }
        break;
        case 9:
        /*GPIO9_FUN_AUX_ADC_2 */
        {
            return ADC_Chan2;
        }
        break;
        case 10:
        /*GPIO10_FUN_AUX_ADC_3 */
        {
            return ADC_Chan3;
        }
        break;
        case 11:
        /*GPIO11_FUN_AUX_ADC_4 */
        {
            return ADC_Chan4;
        }
        break;
        case 12:
        /*GPIO12_FUN_AUX_ADC_5 */
        {
            return ADC_Chan5;
        }
        break;
        case 13:
        /*GPIO13_FUN_AUX_ADC_6 */
        {
            return ADC_Chan6;
        }
        break;
        case 14:
        /*GPIO14_FUN_AUX_ADC_7 */
        {
            return ADC_Chan7;
        }
        break;
        case 29:
        /*GPIO29_FUN_AUX_ADC_8 */
        {
            return ADC_Chan8;
        }
        break;
        case 30:
        /*GPIO30_FUN_AUX_ADC_9 */
        {
            return ADC_Chan9;
        }
        break;
        case 31:
        /*GPIO31_FUN_AUX_ADC_10 */
        {
            return ADC_Chan10;
        }
        break;
        case 32:
        /*GPIO32_FUN_AUX_ADC_11 */
        {
            return ADC_Chan11;
        }
        break;
        default:
        {
            /*empty*/
            //TODO assert here
        }
    }
    /*XXX better idea ?*/
    return ADC_Chan0;
}
/****************************************************************************//**
 * @brief  ADC get one channel result
 *
 * @param  pin: ADC pin number
 *
 * @return ADC result
 *
*******************************************************************************/
static uint32_t ADC_Get_One_Chan_Result(int pin)
{
    uint32_t tmp;
    ADC_Chan_Type posChan, negChan;

    while (RESET == ADC_FIFO_Empty()){
        ADC_Read_FIFO();
    }
    posChan = _get_adc_channel(pin);
    negChan = posChan - ADC_Chan0 + ADC_Chan0_NEG;
    ADC_Start(posChan, negChan, 0);
        
    /* ADC Busy is pulse, so we use FIFO signal instead */
    while (SET==ADC_FIFO_Empty());
    /* Stop ADC immediately after getting result because ADC clock is slow
       we need give ADC enough time to make the next time's start work */
    ADC_Stop();
    tmp = ADC_Read_FIFO();

    return tmp;
}

/******************************************************************************

 * @brief  ADC  initialization
 *
 * @param  pin ADC port
 *
 * @return 0
 *
*******************************************************************************/
int bl_adc_gpio_init(uint8_t pin)
{
    ADC_Cfg_Type adcCfg = {
        .clk=ADC_CLK_2M,
        .dataWidth=ADC_DATA_WIDTH_14_WITH_16_AVERAGE,
        .swReset=ENABLE,
        .sigType=ADC_INPUT_SINGLE_END,
        .vref=ADC_VREF_3P2V,
        .gain1=ADC_PGA_GAIN_1,
        .gain2=ADC_PGA_GAIN_1,
    };
    ADC_FIFO_Cfg_Type fifoCfg = {
        .fifoThreshold=ADC_FIFO_THRESHOLD_1,
        .dmaEn=ENABLE,
    };


    /*XXX we don't include the condtion that bl_adc_gpio_init is called in IRQ disabled context*/
    __disable_irq();
    count_adc++;
    if (1 == count_adc) {
        ADC_Init(&adcCfg);
        ADC_FIFO_Cfg(&fifoCfg);
    }
    __enable_irq();

    ADC_GPIO_Init(pin);
    return 0;
}

/****************************************************************************//**
 * @brief  ADC get one channel result
 *
 * @param  pin: ADC port
 * 
 * @param  value: ADC value
 * 
 * @return 0
*******************************************************************************/

int bl_adc_Value_get(uint8_t pin, uint32_t *value)
{
    *value = 1000 * (ADC_Print_One_Chan_Result(ADC_Get_One_Chan_Result(pin)));
    return 0;
}
/******************************************************************************

 * @brief  ADC  Uinitialization
 *
 * @param  pin ADC port
 *
 * @return 0
 *
*******************************************************************************/
int bl_adc_finalize(uint8_t port)
{
    GLB_GPIO_Cfg_Type cfg;
    cfg.drive = 0;
    cfg.smtCtrl = 1;
    cfg.gpioPin = port;
    cfg.gpioFun = GPIO0_FUN_REG_GPIO_0;//all the function number of GPIO is the same, we use def from GPIO0 here
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg); 

    /*XXX we don't include the condtion that bl_adc_finalize is called in IRQ disabled context*/
    __disable_irq();
    count_adc--;
    //TODO do un-init when count_adc is 0
    __enable_irq();

    return 0;
}

void HBN_OUT0_IRQHandler(void)
{
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO7)){
        HBN_Clear_IRQ(HBN_INT_GPIO7);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO8)){
        HBN_Clear_IRQ(HBN_INT_GPIO8);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO9)){
        HBN_Clear_IRQ(HBN_INT_GPIO9);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO10)){
        HBN_Clear_IRQ(HBN_INT_GPIO10);
        if (cb_notify_gpio10) {
            cb_notify_gpio10(cb_adc_notify_arg_gpio10);
        }
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO11)){
        HBN_Clear_IRQ(HBN_INT_GPIO11);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO29)){
        HBN_Clear_IRQ(HBN_INT_GPIO29);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO30)){
        HBN_Clear_IRQ(HBN_INT_GPIO30);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO31)){
        HBN_Clear_IRQ(HBN_INT_GPIO31);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_GPIO32)){
        HBN_Clear_IRQ(HBN_INT_GPIO32);
    }
    if(SET==HBN_Get_INT_State(HBN_INT_RTC)){
        HBN_Clear_IRQ(HBN_INT_RTC);
void HBN_Clear_RTC_INT();
        HBN_Clear_RTC_INT();
    }    
}

int bl_adc_int_notify_register(uint8_t id, cb_adc_notify_t cb, void *arg)
{
    switch (id) {
        case 10:
        {
            cb_notify_gpio10 = cb;
            cb_adc_notify_arg_gpio10 = arg;
        }
        break;
        case 7:
        case 8:
        case 9:
        case 29:
        case 30:
        case 31:
        case 32:
        default:
        {
            return -1;
        }
    }

    return 0;
}

int bl_adc_int_notify_unregister(uint8_t id, cb_adc_notify_t cb, void *arg)
{
    switch (id) {
        case 10:
        {
            cb_notify_gpio10 = NULL;
            cb_adc_notify_arg_gpio10 = NULL;
        }
        break;
        case 7:
        case 8:
        case 9:
        case 29:
        case 30:
        case 31:
        case 32:
        default:
        {
            return -1;
        }
    }

    return 0;
}

int bl_adc_int_config_disable(uint8_t id)
{
    /*XXX we don't include the condtion that function is called in IRQ disabled context*/
    __disable_irq();
    count_int--;
    if (0 == count_int) {
        NVIC_DisableIRQ(HBN_OUT0_IRQn);
    }
    __enable_irq();

    return 0;
}

int bl_adc_int_config_trigger_high(uint8_t id)
{
    __disable_irq();
    count_int++;
    /*check if it's the first call for HBN0*/
    if (1 == count_int) {
        NVIC_SetPriority((IRQn_Type)HBN_OUT0_IRQn, 7);
        NVIC_EnableIRQ(HBN_OUT0_IRQn);
    }
    __enable_irq();
    return Gpio_Interrupt_Enable(id, 1);
}

int bl_adc_int_config_trigger_low(uint8_t id)
{
    __disable_irq();
    count_int++;
    /*check if it's the first call for HBN0*/
    if (1 == count_int) {
        NVIC_SetPriority((IRQn_Type)HBN_OUT0_IRQn, 7);
        NVIC_EnableIRQ(HBN_OUT0_IRQn);
    }
    __enable_irq();
    return Gpio_Interrupt_Enable(id, 0);
}

int bl_adc_int_config_trigger_higher(uint8_t id, int level)
{
    return -1;
}

int bl_adc_int_config_trigger_lower(uint8_t id, int level)
{
    return -1;
}
