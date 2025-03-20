#ifndef GPIO_2_H
#define GPIO_2_H

//Created a GPIO library to toggle LEDS, this utalizes the PWM pins because we do not use them.


#include "stm32f4xx_hal.h"

#define GPIO_0 PIN_A8  //PWM0
#define GPIO_1 PIN_A9  //PWM1
#define GPIO_2 PIN_A10 //PWM2
#define GPIO_3 PIN_A11 //PWM3
#define GPIO_4 PIN_B6  //PWM4
#define GPIO_5 PIN_B8  //PWM5
#define GPIO_6 PIN_A7  //GP_IN4

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET

/**
 * @brief Enumerate each pin you want to manage in this driver.
 */
typedef enum {
    GPIO_0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_2_NUM_PINS // keep this last as a “count” of pins
} Gpio2Pin_t;

/**
 * @brief Initializes all pins we want to use as GPIO.
 */
void GPIO_Init_2(void);

/**
 * @brief Write a High or Low state to the specified GPIO pin.
 *
 * @param pin   The pin from Gpio2Pin_t
 * @param state Either GPIO_PIN_SET or GPIO_PIN_RESET
 */
void GPIO_WritePin_2(Gpio2Pin_t pin, GPIO_PinState state);

/**
 * @brief Toggle the specified GPIO pin.
 *
 * @param pin   The pin from Gpio2Pin_t
 */
void GPIO_TogglePin_2(Gpio2Pin_t pin);

/**
 * @brief Read the input state of the specified GPIO pin.
 *
 * @param pin   The pin from Gpio2Pin_t
 * @return      GPIO_PinState (GPIO_PIN_SET or GPIO_PIN_RESET)
 */
GPIO_PinState GPIO_ReadPin_2(Gpio2Pin_t pin);

#endif // GPIO_2_H
