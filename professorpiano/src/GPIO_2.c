#include "GPIO_2.h"

/**
 * @brief Internal structure to map each Gpio2Pin_t enum to (port, pin).
 */
static const struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} gpioPinTable[GPIO_2_NUM_PINS] = {
    [PIN_A8]  = {GPIOA, GPIO_PIN_8},
    [PIN_A9]  = {GPIOA, GPIO_PIN_9},
    [PIN_A10] = {GPIOA, GPIO_PIN_10},
    [PIN_A11] = {GPIOA, GPIO_PIN_11},
    [PIN_B6]  = {GPIOB, GPIO_PIN_6},
    [PIN_B8]  = {GPIOB, GPIO_PIN_8},
    [PIN_A7]  = {GPIOA, GPIO_PIN_7},
};

/**
 * @brief Initializes all the pins (PA8, PA9, PA10, PA11, PB6, PB8) as general 
 *        purpose outputs, push-pull, no pull-ups. 
 *
 *        If you need input pins or pull-ups, adapt the config below.
 */
void GPIO_Init_2(void)
{
    // 1. Enable clocks for GPIOA and GPIOB if not already enabled.
    //    The calls below are safe even if the clocks are already on.
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. Configure pins in a loop
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;    // push-pull output
    GPIO_InitStruct.Pull  = GPIO_NOPULL;            // no pull-up/down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    // speed as needed

    for (int i = 0; i < GPIO_2_NUM_PINS; i++) {
        GPIO_InitStruct.Pin = gpioPinTable[i].pin;
        HAL_GPIO_Init(gpioPinTable[i].port, &GPIO_InitStruct);
        // Optional: set each pin LOW initially
        HAL_GPIO_WritePin(gpioPinTable[i].port, gpioPinTable[i].pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Write a logic state (SET or RESET) to the specified pin.
 */
void GPIO_WritePin_2(Gpio2Pin_t pin, GPIO_PinState state)
{
    HAL_GPIO_WritePin(gpioPinTable[pin].port, gpioPinTable[pin].pin, state);
}

/**
 * @brief Toggle the specified pin (if it’s an output).
 */
void GPIO_TogglePin_2(Gpio2Pin_t pin)
{
    HAL_GPIO_TogglePin(gpioPinTable[pin].port, gpioPinTable[pin].pin);
}

/**
 * @brief Read the input or output state of the specified pin.
 *
 * @return GPIO_PinState (GPIO_PIN_SET or GPIO_PIN_RESET)
 */
GPIO_PinState GPIO_ReadPin_2(Gpio2Pin_t pin)
{
    return HAL_GPIO_ReadPin(gpioPinTable[pin].port, gpioPinTable[pin].pin);
}
