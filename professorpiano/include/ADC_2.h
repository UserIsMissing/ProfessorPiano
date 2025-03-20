/**
 * @file    Adc.h
 *
 * Configure and read from ADC channels using the UCSC Nucleo I/O shield.
 *
 * @author  Adam Korycki
 * @author  HARE Lab
 * @author  jLab
 *
 * @date    16 Sep 2023
 *
 * @TODO    Switch to an interrupt-driven ADC framework that uses DMA to read/
 *          store data from each of the 7 channels when triggered. This special
 *          approach is required since the ADC SFRs are not labeled by channel,
 *          i.e. otherwise we cannot tell which ADC value was read by which
 *          ADC channel (values are mixed up).
 */

 //Changed ADC file to include ADC_CHANNEL_6 which is set at the switch, can cheese the system to get another ADC if we put the switch floating instead of on or off


#ifndef ADC_H_2
#define	ADC_H_2

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"


/*  MODULE-LEVEL VARIABLES, MACROS  */
#define ADC_0               ADC_CHANNEL_0  //PA0
#define ADC_1               ADC_CHANNEL_1  //PA1
#define ADC_2               ADC_CHANNEL_10 //PC0
#define ADC_3               ADC_CHANNEL_11 //PC1
#define ADC_4               ADC_CHANNEL_12 //PC2
#define ADC_5               ADC_CHANNEL_13 //PC3
#define ADC_6               ADC_PA6   // Pin 21

#define ADC_PA2             ADC_CHANNEL_2  //PA2 -> USART2_TTL_TX
#define ADC_PA3             ADC_CHANNEL_3  //PA3 -> USART_TTL_RX
#define POT                 ADC_CHANNEL_4  //PA4 -> Onboard potentiometer
#define ADC_PA5             ADC_CHANNEL_5  //PA5 -> NUCLEO_LED
#define ADC_PA6             ADC_CHANNEL_6  //PA6 -> SW1
#define ADC_PA7             ADC_CHANNEL_7  //PA7 -> SW2
#define ADC_PB0             ADC_CHANNEL_8  //PB0 -> GP_LED4
#define ADC_PB1             ADC_CHANNEL_9  //PB1 -> GP_LED5
#define ADC_PC4             ADC_CHANNEL_14 //PC4 -> BTN1
#define ADC_PC5             ADC_CHANNEL_15 //PC5 -> BTN2


#define ADC_NUM_CHANNELS    7
#define ADC_MIN             0
#define ADC_MAX             4095

#ifndef FALSE
#define FALSE ((int8_t) 0)
#endif  /*  FALSE   */
#ifndef TRUE
#define TRUE ((int8_t) 1)
#endif  /*  TRUE    */
#ifndef ERROR
#define ERROR ((int8_t) -1)
#endif  /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t) 1)
#endif  /*  SUCCESS */

ADC_HandleTypeDef hadc1;


/*  PROTOTYPES  */
/** ADC_Start()
 *
 * Start ADC conversions for polling.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_Start_2(void);

/** ADC_End()
 *
 * Disables the A/D subsystem and releases the pins used.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_End_2(void);

/** ADC_Read(channel)
 *
 * Selects ADC channel and returns 12-bit reading.
 *
 * @param   channel (uint32_t)  Select ADC channel
 *                                  (ADC_0, ADC_1, ..., ADC_5, POT)
 * @return          (uint16_t)  12-bit ADC reading.
 */
uint16_t ADC_Read_2(uint32_t channel);

/** ADC_Init()
 *
 * Initializes the ADC subsystem with an interrupt; selects ADC Pin4 by default.
 * 
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_Init_2(void);


#endif  /*  ADC_H_2   */
