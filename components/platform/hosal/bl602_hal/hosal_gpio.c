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
#include "bl_gpio.h"
#include "bl602_glb.h"
#include "bl602_gpio.h"
#include "hosal_gpio.h"
#include "bl602_common.h"
#include "blog.h"
#include "bl602.h"
#include "bl_irq.h"

typedef void (*gpio_handler_t)(void *);
static void gpio_unregister(uint8_t gpioPin);
static int gpio_output_get_value(uint8_t pin);
static gpio_handler_t g_gpio_handler_list[GLB_GPIO_PIN_MAX];
static void *g_gpio_handler_arg[GLB_GPIO_PIN_MAX];

static int gpio_output_get_value(uint8_t pin)
{
    uint32_t val;
    uint32_t pos;

    val = *((uint32_t *)(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET));
    pos = pin % 32;

    if(val & (1 << pos)) {
        return 1;
    } else{
        return 0;
    }
}

static void gpio_unregister(uint8_t gpioPin)
{
    if (gpioPin >= GLB_GPIO_PIN_MAX) {
        return;
    }
    g_gpio_handler_list[gpioPin] = NULL;
    g_gpio_handler_arg[gpioPin] = NULL;

    bl_gpio_intmask(gpioPin, 1);
}

int32_t hosal_gpio_init(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    switch (gpio->config) {
    case INPUT_PULL_DOWN:
        bl_gpio_enable_input(gpio->port, 0, 1);
        break;
    case INPUT_PULL_UP:
        bl_gpio_enable_input(gpio->port, 1, 0);
        break;
    case INPUT_HIGH_IMPEDANCE:
        bl_gpio_enable_input(gpio->port, 0, 0);
        break;
    case OUTPUT_PUSH_PULL:
        bl_gpio_enable_output(gpio->port, 1, 0);
        break;
    case OUTPUT_OPEN_DRAIN_NO_PULL:
        bl_gpio_enable_output(gpio->port, 0, 0);
        break;
    case OUTPUT_OPEN_DRAIN_PULL_UP:
        bl_gpio_enable_output(gpio->port, 0, 0);
        break;
    case IRQ_MODE:
        break;
    default:
        return -1;
    }
    return 0;
}

int32_t hosal_gpio_output_high(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    bl_gpio_output_set(gpio->port, 1);
    return 0;
}

int32_t hosal_gpio_output_low(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    bl_gpio_output_set(gpio->port, 0);
    return 0;
}

int32_t hosal_gpio_output_toggle(hosal_gpio_dev_t *gpio)
{
    uint8_t val;
    
    if (gpio == NULL) {
        return -1;
    }

    val = gpio_output_get_value(gpio->port);
    val = (val) ? 0 : 1;
    bl_gpio_output_set(gpio->port, val);
    return 0;
}

int32_t hosal_gpio_input_get(hosal_gpio_dev_t *gpio, uint32_t *value)
{
    uint8_t hal_val = 0;

    if (NULL == gpio || NULL == value) {
        return -1;
    }

    bl_gpio_input_get(gpio->port, &hal_val);
    *value = hal_val;

    return 0;
}

int32_t hosal_gpio_enable_irq(hosal_gpio_dev_t *gpio, hosal_gpio_irq_trigger_t trigger,
                            gpio_irq_handler_t handler, void *arg)
{
    gpio_ctx_t ctx;

    if (gpio == NULL) {
        return -1;
    }

    ctx.arg = arg;
    ctx.gpioPin = gpio->port;
    ctx.gpio_handler = handler;
    ctx.next = NULL;
    ctx.intCtrlMod = GLB_GPIO_INT_CONTROL_SYNC;
    if (trigger == IRQ_TRIGGER_FALLING_EDGE) {
        ctx.intTrgMod = GLB_GPIO_INT_TRIG_NEG_PULSE;
    } else if (trigger == IRQ_TRIGGER_RISING_EDGE) {
        ctx.intTrgMod = GLB_GPIO_INT_TRIG_POS_PULSE;
    } else {
        return -1;
    }

    bl_gpio_intmask(ctx.gpioPin, 1);
    bl_set_gpio_intmod(ctx.gpioPin, ctx.intCtrlMod, ctx.intTrgMod);
    bl_irq_register_with_ctx(GPIO_INT0_IRQn, ctx.gpio_handler, gpio);
    bl_gpio_intmask(ctx.gpioPin, 0);
    bl_irq_enable(GPIO_INT0_IRQn);

    return 0;
}

int32_t hosal_gpio_disable_irq(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }
    gpio_unregister(gpio->port);
    
    return 0;
}

int32_t hosal_gpio_clear_irq(hosal_gpio_dev_t *gpio)
{
    if (gpio  == NULL) {
        return -1;
    }

    bl_gpio_int_clear(gpio->port, 1);
    bl_gpio_int_clear(gpio->port, 0);
    
    return 0;
}

int32_t hosal_gpio_finalize(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }
    gpio_unregister(gpio->port);

    return 0;    
}



