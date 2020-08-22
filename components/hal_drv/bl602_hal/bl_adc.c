/*
 * Copyright (c) 2020 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <bl602_aon.h>
#include <bl602_hbn.h>
#include <bl602_common.h>
#include <bl602_glb.h>
#include <bl602_hbn.h>
#include <bl602_dma.h>
#include <bl602_adc.h>
#include <bl602.h>
#include <bl_irq.h>
#include <loopset_adc.h>
#include <blog.h>
#include <event_type_code.h>

#include <blog.h>

//#define TEMP_OFFSET_X   22791
#define TEMP_OFFSET_X 2318

ADC_CFG_Type adcCfg = {
    .v18Sel=ADC_V18_SEL_1P82V,                /*!< ADC 1.8V select */
    .v11Sel=ADC_V11_SEL_1P1V,                 /*!< ADC 1.1V select */
    .clkDiv=ADC_CLK_DIV_16,                   /*!< Clock divider */
    .gain1=ADC_PGA_GAIN_NONE,                 /*!< PGA gain 1 */
    .gain2=ADC_PGA_GAIN_NONE,                 /*!< PGA gain 2 */
    .chopMode=ADC_CHOP_MOD_AZ_PGA_ON,           /*!< ADC chop mode select */
    .biasSel=ADC_BIAS_SEL_MAIN_BANDGAP,       /*!< ADC current form main bandgap or aon bandgap */
    .vcm=ADC_PGA_VCM_1V,                      /*!< ADC VCM value */
    .vref=ADC_VREF_2V,                      /*!< ADC voltage reference */
    .inputMode=ADC_INPUT_SINGLE_END,          /*!< ADC input signal type */
    .resWidth=ADC_DATA_WIDTH_16_WITH_256_AVERAGE,              /*!< ADC resolution and oversample rate */
    .offsetCalibEn=0,                         /*!< Offset calibration enable */
    .offsetCalibVal=0,                        /*!< Offset calibration value */
};


ADC_FIFO_Cfg_Type adcFifoCfg = {
    .fifoThreshold = ADC_FIFO_THRESHOLD_1,
    .dmaEn = DISABLE,
};

void ADC_Clock_Init(uint8_t div)
{
    GLB_Set_ADC_CLK(ENABLE,GLB_ADC_CLK_96M,div);
    blog_info("GLB_Set_ADC_CLK_Div(%d) == clock 96M/(%d+1)\r\n",div,div);
}

void TSEN_Calibration(void)
{
    ADC_SET_TSVBE_LOW();
    ADC_Start();
    ARCH_Delay_MS(100);
    while(ADC_Get_FIFO_Count() == 0);
    ADC_Read_FIFO();

    ADC_SET_TSVBE_HIGH();
    ADC_Start();
    ARCH_Delay_MS(100);
    while(ADC_Get_FIFO_Count() == 0);
    ADC_Read_FIFO();

    ADC_SET_TSVBE_LOW();
}

static void ADC_tsen_case(void)
{
	ADC_Result_Type result;
	uint32_t regVal=0;
    uint8_t i=0;
    uint32_t v0=0,v1=0;
    float v_error=0;

    ADC_Reset();

    ADC_Disable();
    ADC_Enable();

    ADC_Init(&adcCfg);
    ADC_Channel_Config(ADC_CHAN_TSEN_P,ADC_CHAN_GND,0);
    ADC_Tsen_Init(ADC_TSEN_MOD_INTERNAL_DIODE);

    ADC_FIFO_Cfg(&adcFifoCfg);
    TSEN_Calibration();

	for(i=0;i<40;i++){
		ADC_Start();

		while(ADC_Get_FIFO_Count() == 0);

		do{
			regVal = ADC_Read_FIFO();
			ADC_Parse_Result(&regVal,1,&result);

            if(i%2 ==0){
                v0 = result.value;
            }else{
                v1 = result.value;
            }  
		}while(ADC_Get_FIFO_Count() != 0);

        if(i%2 !=0){
              v_error = (float)v0 - (float)v1;
              v_error = v_error - TEMP_OFFSET_X;
              v_error = v_error /7.753;
            blog_info(" v0=%ld  v1 = %ld \n",v0,v1);
            //MSG(" ((v0-v1)-X)/7.753= %d \n",(uint32_t)(v_error * 1000));
            blog_info(" chip Tempture = %ld degree centigrade\n",(uint32_t)(v_error));
        }
        if( i%2 ==0 )
            ADC_SET_TSVBE_HIGH();
        else
            ADC_SET_TSVBE_LOW();

		ARCH_Delay_MS(500);
	}
}


int test_adc_init(void)
{
    ADC_Clock_Init(2);

    ADC_Reset();

    ADC_Disable();
    ADC_Enable();

    ADC_Init(&adcCfg);
    ADC_Channel_Config(ADC_CHAN_TSEN_P, ADC_CHAN_GND, 0);
    ADC_Tsen_Init(ADC_TSEN_MOD_INTERNAL_DIODE);

    ADC_FIFO_Cfg(&adcFifoCfg);
    TSEN_Calibration();

    return 0;
}

int test_adc_get(int16_t *tmp)
{
    ADC_Result_Type result;
    uint32_t regVal=0;
    uint8_t i=0;
    uint32_t v0=0,v1=0;
    float v_error=0;

    for (i = 0; i < 2; i++) {
        ADC_Start();

        while(ADC_Get_FIFO_Count() == 0) {
            extern void aos_msleep(int ms);
            aos_msleep(1);
        }

        do{
            regVal = ADC_Read_FIFO();
            ADC_Parse_Result(&regVal,1,&result);

            if(i%2 == 0) {
                v0 = result.value;
            } else {
                v1 = result.value;
            }
        } while(ADC_Get_FIFO_Count() != 0);

        if (i%2 != 0) {
            v_error = (float)v0 - (float)v1;
            //blog_info("v_error = %d\r\n", v_error);
            v_error = v_error - TEMP_OFFSET_X;
            v_error = v_error / 7.753;
            //blog_info("v0 = %ld, v1 = %ld\r\n", v0, v1);
            //blog_info(" chip Tempture = %ld degree centigrade\r\n", (uint32_t)(v_error));
            *tmp = (int16_t)v_error;
        }
        if (i%2 == 0) {
            ADC_SET_TSVBE_HIGH();
        } else {
            ADC_SET_TSVBE_LOW();
        }
    }
    return 0;
}

int test_adc_test(void)
{
    ADC_tsen_case();
    return 0;
}

//adc driver
static uint32_t adc_get_data(void)
{
	ADC_Result_Type result;
    uint32_t regval=0;

    ADC_IntMask(ADC_INT_ADC_READY,MASK);

    do{
        regval = ADC_Read_FIFO();
        if(regval){
            ADC_Parse_Result(&regval,1,&result);
        }
    } while(ADC_Get_FIFO_Count()!=0);

    return result.volt * 1000;
}

static void adc_clock_init(uint8_t div)
{
    GLB_Set_ADC_CLK(ENABLE,GLB_ADC_CLK_96M,div);
    
    return;
}

int start_adc_data_collect(void)
{
    ADC_Start();

    return 0;
}

static void adc_interrupt_entry(void)
{   
    uint32_t data;

    if(ADC_GetIntStatus(ADC_INT_ADC_READY) == SET) {
        ADC_IntMask(ADC_INT_ALL, MASK);
        ADC_IntClr(ADC_INT_ADC_READY);
        data = adc_get_data();
        loopapp_adc_trigger(data, CODE_ADC_DATA);
    }

    if(ADC_GetIntStatus(ADC_INT_POS_SATURATION) == SET) {
        ADC_IntMask(ADC_INT_ALL, MASK);
        ADC_IntClr(ADC_INT_POS_SATURATION); 
        loopapp_adc_trigger(0, CODE_ADC_POS_SATURATION); 
    }

    if(ADC_GetIntStatus(ADC_INT_NEG_SATURATION) == SET) {
        ADC_IntMask(ADC_INT_ALL, MASK);
        ADC_IntClr(ADC_INT_NEG_SATURATION);
        loopapp_adc_trigger(0, CODE_ADC_NEG_SATURATION);
    }
    
    if(ADC_GetIntStatus(ADC_INT_FIFO_UNDERRUN) == SET) {
        ADC_IntMask(ADC_INT_ALL, MASK);
        ADC_IntClr(ADC_INT_FIFO_UNDERRUN);
        loopapp_adc_trigger(0, CODE_ADC_FIFO_UNDERRUN);
    }
    
    if(ADC_GetIntStatus(ADC_INT_FIFO_OVERRUN) == SET) { 
        ADC_IntMask(ADC_INT_ALL, MASK);
        ADC_IntClr(ADC_INT_FIFO_OVERRUN); 
        data = adc_get_data();
        loopapp_adc_trigger(data, CODE_ADC_FIFO_OVERRUN);
    }

    ADC_IntMask(ADC_INT_ALL, UNMASK);

    return;
}

static int adc_gpio_init(int gpio_num)
{
    GLB_GPIO_Cfg_Type gpiocfg;
    int channel;
    
    printf("gpio num = %d \r\n", gpio_num);
    gpiocfg.gpioPin = gpio_num;
    if (gpio_num == GLB_GPIO_PIN_4) {
        gpiocfg.gpioFun = GPIO4_FUN_GPIP_CH1;
        channel = 1;
    } else if (gpio_num == GLB_GPIO_PIN_5) {    
        gpiocfg.gpioFun = GPIO5_FUN_GPIP_CH4;
        channel = 4;
    } else if (gpio_num == GLB_GPIO_PIN_6) {
        gpiocfg.gpioFun = GPIO6_FUN_GPIP_CH5;
        channel = 5;
    } else if (gpio_num == GLB_GPIO_PIN_9) {
        gpiocfg.gpioFun = GPIO9_FUN_GPIP_CH6_GPIP_CH7;
        channel = 7;
    } else if (gpio_num == GLB_GPIO_PIN_10) {
        gpiocfg.gpioFun = GPIO10_FUN_MICBIAS_GPIP_CH8_GPIP_CH9;
        channel = 9;
    } else if (gpio_num == GLB_GPIO_PIN_11) {
        gpiocfg.gpioFun = GPIO11_FUN_IRLED_OUT_GPIP_CH10;
        channel = 10;
    } else if (gpio_num == GLB_GPIO_PIN_12) {
        gpiocfg.gpioFun = GPIO12_FUN_GPIP_CH0_GPADC_VREF_EXT;
        channel = 0;
    } else if (gpio_num == GLB_GPIO_PIN_13) {
        gpiocfg.gpioFun = GPIO13_FUN_GPIP_CH3;
        channel = 3;
    } else if (gpio_num == GLB_GPIO_PIN_14) {
        gpiocfg.gpioFun = GPIO14_FUN_GPIP_CH2;
        channel = 2;
    } else if (gpio_num == GLB_GPIO_PIN_15) {
        gpiocfg.gpioFun = GPIO15_FUN_PSW_IRRCV_OUT_GPIP_CH11;
        channel = 11;
    } else {
        blog_error("not correct gpio. adc channel only support gpio: 3, 4, 5, 9, 10, 11, 12, 13 ,14 ,15");
        return -1;
    }

    gpiocfg.gpioMode = GPIO_MODE_AF;
    gpiocfg.pullType = GPIO_PULL_NONE;
    gpiocfg.drive = 0;
    gpiocfg.smtCtrl = 0; 
    GLB_GPIO_Init(&gpiocfg);
    
    return channel;
}

static void adc_config(void)
{
    ADC_CFG_Type adccfg;

    adccfg.v18Sel = ADC_V18_SEL_1P82V;                /*!< ADC 1.8V select */
    adccfg.v11Sel = ADC_V11_SEL_1P1V;                 /*!< ADC 1.1V select */
    adccfg.clkDiv = ADC_CLK_DIV_16;                   /*!< Clock divider */
    adccfg.gain1 = ADC_PGA_GAIN_NONE;                 /*!< PGA gain 1 */
    adccfg.gain2 = ADC_PGA_GAIN_NONE;                 /*!< PGA gain 2 */
    adccfg.chopMode = ADC_CHOP_MOD_ALL_OFF;           /*!< ADC chop mode select */
    adccfg.biasSel = ADC_BIAS_SEL_MAIN_BANDGAP;       /*!< ADC current form main bandgap or aon bandgap */
    adccfg.vcm = ADC_PGA_VCM_1V;                      /*!< ADC VCM value */
    adccfg.vref = ADC_VREF_3P3V;                      /*!< ADC voltage reference */
    adccfg.inputMode = ADC_INPUT_SINGLE_END;          /*!< ADC input signal type */
    adccfg.resWidth = ADC_DATA_WIDTH_12;              /*!< ADC resolution and oversample rate */
    adccfg.offsetCalibEn = 0;                         /*!< Offset calibration enable */
    adccfg.offsetCalibVal = 0;                        /*!< Offset calibration value */        
 
    ADC_Init(&adccfg);
}

static void adc_fifocfg(void)
{
    ADC_FIFO_Cfg_Type adc_fifocfg;

    adc_fifocfg.fifoThreshold = ADC_FIFO_THRESHOLD_1;
    adc_fifocfg.dmaEn = DISABLE;
    
    ADC_FIFO_Cfg(&adc_fifocfg);

    return;
}

int bl_adc_init(int gpio_num, int oneshot, int sampling_ms)
{
    int channel;

    channel = adc_gpio_init(gpio_num);
    adc_clock_init(2);
    ADC_Reset();
    ADC_IntMask(ADC_INT_ALL,MASK);
    ADC_Disable();
    ADC_Enable();
    adc_config();
    ADC_Channel_Config(channel, ADC_CHAN_GND, 0);
    adc_fifocfg();

    ADC_IntMask(ADC_INT_ALL, MASK);

    bl_irq_register_with_ctx(GPADC_DMA_IRQn, adc_interrupt_entry, NULL);
    ADC_IntClr(ADC_INT_ALL);
    bl_irq_enable(GPADC_DMA_IRQn);

    return 0;
}

int bl_adc_start(void)
{
    ADC_IntMask(ADC_INT_POS_SATURATION, UNMASK);
    ADC_IntMask(ADC_INT_NEG_SATURATION, UNMASK);
    ADC_IntMask(ADC_INT_FIFO_UNDERRUN, UNMASK);
    ADC_IntMask(ADC_INT_FIFO_OVERRUN, UNMASK);
    ADC_IntMask(ADC_INT_ADC_READY, UNMASK);

    ADC_Start();
    return 0;
}

int bl_adc_stop(void)
{
    ADC_Stop();
    ADC_IntMask(ADC_INT_ALL, MASK);

    return 0;
}
void bl_adc_int_enable(void) 
{
    bl_irq_enable(GPADC_DMA_IRQn);
}
