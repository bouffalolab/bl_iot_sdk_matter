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
#include <loopset_adc.h>
#include <blog.h>
#include <event_type_code.h>
#include <bl_adc.h>
#include <bl_irq.h>

static int check_gpio_num(int gpio_num)
{
    int gpio_arr[] = {4, 5, 6, 9, 10, 11, 12, 13, 14, 15};
    int i;

    for (i = 0; i < sizeof(gpio_arr) / sizeof(gpio_arr[0]); i++) {
        if (gpio_num == gpio_arr[i]) {
            return 0;
        }
    }

    return -1;

}

int hal_adc_init(int gpio_num, int oneshot, int sampling_ms)
{
    if ((check_gpio_num(gpio_num) == -1) || (oneshot != 0 && oneshot != 1) || (sampling_ms < 0)) {
        blog_error("illegal para adc channel only support  4, 5, 6, 9, 10, 11, 12, 13 ,14 ,15\r\n");
        return -1;
    } 
    
    bl_adc_init(gpio_num, oneshot, sampling_ms);
    loopapp_adc_create(oneshot, sampling_ms);
    bl_adc_int_enable();

    return 0;
}

int hal_adc_start(void)
{
    bl_adc_start();
    loopapp_adc_process(0);

    return 0;
}

int hal_adc_stop(void)
{
    bl_adc_stop();
    loopapp_adc_process(1);

    return 0;
}

