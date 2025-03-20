/**
 * @file    BOARD.c
 * 
 * Generic driver set up and clean up for functional testing
 * for the UCSC Nucleo I/O Shield connected to a Nucleo STM32F411RE
 * development kit (with an 8 MHz HSE).
 */

 #include <Board_2.h>
 #include <leds.h>
 #include <stdio.h>
 
 /*  PROTOTYPES  */
 static int8_t  Nucleo_ConfigSysClk_2(void);
 static int8_t  Nucleo_ConfigClks_2(void);
 static int8_t  Nucleo_ConfigPins_2(void);
 static int8_t  Nucleo_Init_2(void);
 static int8_t  Serial_ConfigPins_2(void);
 //  -- Removed the custom Serial_BRRCalculator, let HAL do it --
 static int8_t  Serial_Begin_2(void);
 static int8_t  Serial_Init_2(void);
 void           Error_Handler_2(void);
 
 /*  MODULE-LEVEL DEFINITIONS, MACROS  */
 static uint8_t initStatus = FALSE;
 
 // Define UART2 handler and setup printf() functionality.
 UART_HandleTypeDef huart2;
 
 #ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar_2(int ch)
  #define GETCHAR_PROTOTYPE int __io_getchar_2(void)
 #else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
 #endif
 
 PUTCHAR_PROTOTYPE
 {
     HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
     return ch;
 }
 
 GETCHAR_PROTOTYPE
 {
     uint8_t ch = 0;
     
     // Clear the Overrun flag just before receiving the first character.
     __HAL_UART_CLEAR_OREFLAG(&huart2);
     
     // Wait for reception of a character on the USART RX line and echo this
     // character on console.
     HAL_UART_Receive(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
     HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
     return ch;
 }
 
 // Defaults.
 #define SERIAL_BAUDRATE ((uint32_t)115200)
 
 /******************************************************************************
  *                              CLOCK CONFIGURATION
  *****************************************************************************/
 /**
  * System clock configuration for 8MHz HSE => 48MHz SYSCLK
  *
  * - HSE = 8MHz
  * - PLLM = 8, PLLN = 192, PLLP = 4 => 48MHz
  * - APB1 = 24MHz, APB2 = 48MHz
  * - Flash latency = 1 (check datasheet)
  *
  * @return (int8_t) [SUCCESS, ERROR]
  */
 static int8_t Nucleo_ConfigSysClk_2(void)
 {
     RCC_OscInitTypeDef       RCC_OscInitStruct    = {0};
     RCC_ClkInitTypeDef       RCC_ClkInitStruct    = {0};
     RCC_PeriphCLKInitTypeDef PeriphClkInitStruct  = {0};
 
     __HAL_RCC_PWR_CLK_ENABLE();
     __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
 
     RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
     RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
     RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
     RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
     // For 8MHz -> 48MHz
     RCC_OscInitStruct.PLL.PLLM            = 8;
     RCC_OscInitStruct.PLL.PLLN            = 197;
     RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4; // = /4
     RCC_OscInitStruct.PLL.PLLQ            = 4;
 
     if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
         Error_Handler_2();
     }
 
     RCC_ClkInitStruct.ClockType =
           RCC_CLOCKTYPE_SYSCLK
         | RCC_CLOCKTYPE_HCLK
         | RCC_CLOCKTYPE_PCLK1
         | RCC_CLOCKTYPE_PCLK2;
     RCC_ClkInitStruct.SYSCLKSource     = RCC_SYSCLKSOURCE_PLLCLK;
     RCC_ClkInitStruct.AHBCLKDivider    = RCC_SYSCLK_DIV1;  // 48MHz
     RCC_ClkInitStruct.APB1CLKDivider   = RCC_HCLK_DIV2;    // 24MHz
     RCC_ClkInitStruct.APB2CLKDivider   = RCC_HCLK_DIV1;    // 48MHz
 
     if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
         Error_Handler_2();
     }
 
     PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
     PeriphClkInitStruct.PLLI2S.PLLI2SM       = 8;
     PeriphClkInitStruct.PLLI2S.PLLI2SN       = 197;  //197 for 48.1mhz, 181 for 44.1 Mhz
     PeriphClkInitStruct.PLLI2S.PLLI2SR       = 4;
     if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
         Error_Handler_2();
     }
 
 
     return SUCCESS;
 }
 
 /**
  * Set up all system clock and GPIO clocks.
  * 
  * @return (int8_t) [SUCCESS, ERROR]
  */
 static int8_t Nucleo_ConfigClks_2(void)
 {
     Nucleo_ConfigSysClk_2();
 
     // Enable GPIO bank clocks (redundant calls are harmless).
     __HAL_RCC_GPIOC_CLK_ENABLE();
     __HAL_RCC_GPIOA_CLK_ENABLE();
     __HAL_RCC_GPIOB_CLK_ENABLE();
     __HAL_RCC_GPIOD_CLK_ENABLE();
 
     return SUCCESS;
 }
 
 /******************************************************************************
  *                               SERIAL SETUP
  *****************************************************************************/
 /**
  * Configure pins for use with serial communications on UART2 (PA2, PA3).
  */
 static int8_t Serial_ConfigPins_2(void)
 {
     GPIO_InitTypeDef GPIO_InitStruct = {0};
 
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
     GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
     GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
     return SUCCESS;
 }
 
 /**
  * Open the serial connection (UART2) at 115200 baud.
  */
 static int8_t Serial_Begin_2(void)
 {
     huart2.Instance          = USART2;
     huart2.Init.BaudRate     = SERIAL_BAUDRATE; // 115200
     huart2.Init.WordLength   = UART_WORDLENGTH_8B;
     huart2.Init.StopBits     = UART_STOPBITS_1;
     huart2.Init.Parity       = UART_PARITY_NONE;
     huart2.Init.Mode         = UART_MODE_TX_RX;
     huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
     huart2.Init.OverSampling = UART_OVERSAMPLING_16;
 
     if (HAL_UART_Init(&huart2) != HAL_OK) {
         Error_Handler_2();
     }
     return SUCCESS;
 }
 
 /**
  * Complete the serial initialization steps.
  */
 static int8_t Serial_Init_2(void)
 {
     Serial_ConfigPins_2();
     Serial_Begin_2();
     return SUCCESS;
 }
 
 /******************************************************************************
  *                        ONBOARD NUCLEO FUNCTIONALITY
  *****************************************************************************/
 /**
  * Configure pins for the Nucleo onboard LED (PA5) and Button (PC13).
  *
  * @return (int8_t) [SUCCESS, ERROR]
  */
 static int8_t Nucleo_ConfigPins_2(void)
 {
     GPIO_InitTypeDef GPIO_InitStruct = {0};
 
    //  // LED on PA5
    //  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //  GPIO_InitStruct.Pull  = GPIO_NOPULL;
    //  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    //  GPIO_InitStruct.Pin   = GPIO_PIN_5;
    //  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
     // USER Button on PC13 (Big Blue), falling edge
     GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
     GPIO_InitStruct.Pin  = GPIO_PIN_13;
     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 
     return SUCCESS;
 }
 
 /**
  * Initialize the Nucleo board: clocks, pins, LED off by default, etc.
  *
  * @return (int8_t) [SUCCESS, ERROR]
  */
 static int8_t Nucleo_Init_2(void)
 {
     Nucleo_ConfigClks_2();
     Nucleo_ConfigPins_2();
 
     // Turn LED off initially
    //  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
 
     return SUCCESS;
 }
 
 /******************************************************************************
  *                         BOARD LIFECYCLE FUNCTIONS
  *****************************************************************************/
 /**
  * BOARD_Init
  * 1. Initialize the HAL framework.
  * 2. Configure system/clock, Nucleo pins, UART2, etc.
  * 3. Initialize external LEDs (if your "leds.h" requires it).
  *
  * @return (int8_t) [SUCCESS, ERROR]
  */
 int8_t BOARD_Init_2(void)
 {
     if (initStatus == FALSE)
     {
         HAL_Init();
 
         if (Nucleo_Init_2() == ERROR)
         {
             return ERROR;
         }
         if (Serial_Init_2() == ERROR)
         {
             return ERROR;
         }
         LEDS_Init(); // If you have an external LED driver library.
 
         initStatus = TRUE;
     }
     // For debugging, print the current clock frequencies.
    //  printf("Sys=%lu HCLK=%lu PCLK1=%lu PCLK2=%lu\r\n",
    //      HAL_RCC_GetSysClockFreq(),
    //      HAL_RCC_GetHCLKFreq(),
    //      HAL_RCC_GetPCLK1Freq(),
    //      HAL_RCC_GetPCLK2Freq()
    //  );
     return SUCCESS;
 }
 
 /**
  * BOARD_End
  * Shuts down all peripherals; sets the initStatus to FALSE.
  * (TODO: turn pins into inputs if desired).
  *
  * @return initStatus (int8_t) [TRUE, FALSE]
  */
 int8_t BOARD_End_2(void)
 {
     initStatus = FALSE;
     // Kill all interrupts for a "clean" shutdown (optional).
     __disable_irq();
 
     return initStatus;
 }
 
 /**
  * Error_Handler
  * Blink the onboard LED forever to indicate something went wrong.
  */
 void Error_Handler_2(void)
 {
     __disable_irq();
     static int halfBlinkPeriod = 500000;
     static int timer = 0;
 
     while (1)
     {
        //  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
         timer = 0;
         while (timer < halfBlinkPeriod) { timer++; }
 
        //  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
         timer = 0;
         while (timer < halfBlinkPeriod) { timer++; }
     }
 }
 