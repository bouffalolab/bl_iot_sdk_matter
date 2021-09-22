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

#ifndef __HOSAL_GPIO_H_
#define __HOSAL_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @addtogroup hosal_gpio GPIO
 *  HOSAL GPIO API
 *
 *  @{
 */

/**
 * @brief gpio config struct
 */
typedef enum {
    ANALOG_MODE,               /**< @brief Used as a function pin, input and output analog */
    IRQ_MODE,                  /**< @brief Used to trigger interrupt */
    INPUT_PULL_UP,             /**< @brief Input with an internal pull-up resistor - use with devices
                                  that actively drive the signal low - e.g. button connected to ground */
    INPUT_PULL_DOWN,           /**< @brief Input with an internal pull-down resistor - use with devices
                                  that actively drive the signal high - e.g. button connected to a power rail */
    INPUT_HIGH_IMPEDANCE,      /**< @brief Input - must always be driven, either actively or by an external pullup resistor */
    OUTPUT_PUSH_PULL,          /**< @brief Output actively driven high and actively driven low -
                                  must not be connected to other active outputs - e.g. LED output */
    OUTPUT_OPEN_DRAIN_NO_PULL, /**< @brief Output actively driven low but is high-impedance when set high -
                                  can be connected to other open-drain/open-collector outputs.
                                  Needs an external pull-up resistor */
    OUTPUT_OPEN_DRAIN_PULL_UP, /**< @brief Output actively driven low and is pulled high
                                  with an internal resistor when set high -
                                  can be connected to other open-drain/open-collector outputs. */
    OUTPUT_OPEN_DRAIN_AF,      /**< @brief Alternate Function Open Drain Mode. */
    OUTPUT_PUSH_PULL_AF,       /**< @brief Alternate Function Push Pull Mode. */
} hosal_gpio_config_t;

/**
 * @brief GPIO interrupt trigger
 */
typedef enum {
    IRQ_TRIGGER_RISING_EDGE  = 0x1,    /**< @brief Interrupt triggered at input signal's rising edge  */
    IRQ_TRIGGER_FALLING_EDGE = 0x2,    /**< @brief Interrupt triggered at input signal's falling edge */
    IRQ_TRIGGER_BOTH_EDGES   = 0x03,   /**< @brief Interrupt triggered at input signal's falling edge || rising edge */
} hosal_gpio_irq_trigger_t;

/**
 * @brief GPIO mode
 */
typedef enum {
    GPIO_INPUT     = 0x0000U, /**< @brief Input Floating Mode */
    GPIO_OUTPUT_PP = 0x0001U, /**< @brief Output Push Pull Mode */
    GPIO_OUTPUT_OD = 0x0011U, /**< @brief Output Open Drain Mode */
} hosal_gpio_mode_t;

/**
 * @brief GPIO pinstate
 */
typedef enum {
    GPIO_PinState_Reset = 0,                    /**< @brief Pin state 0 */
    GPIO_PinState_Set   = !GPIO_PinState_Reset, /**< @brief Pin state 1 */
} hosal_gpio_pinstate_t;

/**
 * @brief GPIO dev struct
 */
typedef struct {
    uint8_t        port;         /**< @brief gpio port */
    hosal_gpio_config_t  config; /**< @brief gpio config */
    void          *priv;         /**< @brief priv data */
} hosal_gpio_dev_t;

/**
 * @brief GPIO interrupt callback handler
 *
 *@param[in] parg  ：Set the custom parameters specified
 */
typedef void (*gpio_irq_handler_t)(void *arg);

/**
 * @brief Initialises a GPIO pin
 *
 * @note  Prepares a GPIO pin for use.
 *
 * @param[in]  gpio           the gpio pin which should be initialised
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_init(hosal_gpio_dev_t *gpio);

/**
 * @brief Sets an output GPIO board_config.high
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set high
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_output_high(hosal_gpio_dev_t *gpio);

/**
 * @brief Sets an output GPIO pin low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set low
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_output_low(hosal_gpio_dev_t *gpio);

/**
 * @brief Trigger an output GPIO pin's output. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be toggled
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_output_toggle(hosal_gpio_dev_t *gpio);

/**
 * @brief Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param[in]  gpio   the gpio pin which should be read
 * @param[out] value  gpio value
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_input_get(hosal_gpio_dev_t *gpio, uint32_t *value);

/**
 * @brief Enables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which is set to
 * output mode is undefined.
 *
 * @param[in]  gpio     the gpio pin which will provide the interrupt trigger
 * @param[in]  trigger  the type of trigger (rising/falling edge or both)
 * @param[in]  handler  a function pointer to the interrupt handler
 * @param[in]  arg      an argument that will be passed to the interrupt handler
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_enable_irq(hosal_gpio_dev_t *gpio, hosal_gpio_irq_trigger_t trigger,
                            gpio_irq_handler_t handler, void *arg);

/**
 * @brief Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been setted up using
 * @ref hal_gpio_input_irq_enable is undefined.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_disable_irq(hosal_gpio_dev_t *gpio);

/**
 * @brief Clear an interrupt status for an input GPIO pin.
 * Using this function on a gpio pin which has generated a interrupt.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_clear_irq(hosal_gpio_dev_t *gpio);

/**
 * @brief Set a GPIO pin in default state.
 *
 * @param[in]  gpio  the gpio pin which should be deinitialised
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int32_t hosal_gpio_finalize(hosal_gpio_dev_t *gpio);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */

