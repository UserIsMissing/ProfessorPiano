/**
 * @file    I2C.c
 * @author  Adam Korycki
 *
 * @date    3 Oct 2023
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include "stm32f4xx_hal.h"
 #include "stm32f4xx_hal_i2c.h"
 #include "I2C_2.h"
 
 
 /*  MODULE-LEVEL DEFINITIONS, MACROS    */
 // Boolean defines for TRUE, FALSE, SUCCESS and ERROR.
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
 
 I2C_HandleTypeDef hi2c2;
 
 static uint8_t initStatus = FALSE;
 
 
 /*  FUNCTIONS   */
 /** I2C_Init()
  *
  * Initializes the I2C System at standard speed (100Kbps).
  *
  * @return SUCCESS or ERROR
  */
 int8_t I2C_Init_2(void)
 {
     if (initStatus == FALSE)
     {
         hi2c2.Instance = I2C2;
         hi2c2.Init.ClockSpeed = 100000;
         hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
         hi2c2.Init.OwnAddress1 = 0;
         hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
         hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
         hi2c2.Init.OwnAddress2 = 0;
         hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
         hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
         if (HAL_I2C_Init(&hi2c2) != HAL_OK)
         {
             return ERROR;
         }
         initStatus = TRUE;
     }
     return SUCCESS;
 }
 
 void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
 {
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(hi2c->Instance==I2C2)
   {
   /* USER CODE BEGIN I2C2_MspInit 0 */
 
   /* USER CODE END I2C2_MspInit 0 */
 
     __HAL_RCC_GPIOB_CLK_ENABLE();
     /**I2C2 GPIO Configuration
     PB10     ------> I2C2_SCL
     PB9     ------> I2C2_SDA
     */
     GPIO_InitStruct.Pin = GPIO_PIN_10;
     GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
     GPIO_InitStruct.Pin = GPIO_PIN_9;
     GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
     GPIO_InitStruct.Pull = GPIO_PULLUP;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
     /* Peripheral clock enable */
     __HAL_RCC_I2C2_CLK_ENABLE();
   /* USER CODE BEGIN I2C2_MspInit 1 */
       HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
       HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
       HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
       HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
   /* USER CODE END I2C2_MspInit 1 */
   }
 }

 /** I2C_ReadRegister(I2CAddress, deviceRegisterAddress)
  *
  * Reads one device register on chosen I2C device.
  *
  * @param   I2CAddresss             (unsigned char) 7-bit address of I2C device
  *                                                  wished to interact with.
  * @param   deviceRegisterAddress   (unsigned char) 8-bit address of register on
  *                                                  device.
  * @return                          (unsigned char) Value at register or 0.
  */
 unsigned char I2C_ReadRegister_2(
     unsigned char I2CAddress,
     unsigned char deviceRegisterAddress
 )
 {
     HAL_StatusTypeDef ret;
     I2CAddress = I2CAddress << 1; // use 8-bit address
     uint8_t* data = &deviceRegisterAddress;
 
     // Start condition; wait for it to end, this is internal and cannot stall.
     ret = HAL_I2C_Master_Transmit(&hi2c2, I2CAddress, data, 1, HAL_MAX_DELAY);
     if (ret != HAL_OK)
     {
         printf("I2C Tx Error on read start condition\r\n");
         return 0;
     }
 
     // Get byte; wait for it to end, this is internal and cannot stall.
     ret = HAL_I2C_Master_Receive(&hi2c2, I2CAddress, data, 1, HAL_MAX_DELAY);
     if (ret != HAL_OK)
     {
         printf("I2C Rx Error on read byte\r\n");
         return 0;
     }
 
     return *data;
 }
 
 /** I2C_WriteReg(I2CAddress, deviceRegisterAddress, data)
  *
  * Writes one device register on chosen I2C device.
  *
  * @param   I2CAddress              (unsigned char) 7-bit address of I2C device
  *                                                  wished to interact with.
  * @param   deviceRegisterAddress   (unsigned char) 8-bit address of register on
  *                                                  device.
  * @param   data                    (uint8_t)       Data wished to be written to
  *                                                  register.
  * @return                          (unsigned char) [SUCCESS, ERROR]
  */
 unsigned char I2C_WriteReg_2(
     unsigned char I2CAddress,
     unsigned char deviceRegisterAddress,
     uint8_t data
 )
 {
     HAL_StatusTypeDef ret;
     I2CAddress = I2CAddress << 1; // Use 8-bit address.

    //  printf("I2C Write: Addr=0x%X, Reg=0x%X, Data=0x%X\n", I2CAddress >> 1, deviceRegisterAddress, data);

     ret = HAL_I2C_Mem_Write(
         &hi2c2,
         I2CAddress,
         deviceRegisterAddress,
         I2C_MEMADD_SIZE_8BIT,
         &data,
         1,
         HAL_MAX_DELAY
     );
     if (ret != HAL_OK)
     {
        // printf("I2C Tx Error on write data\r\n");
         return ERROR;
     }
 
     return SUCCESS;   
 }
 
 /** I2C_ReadInt(I2CAddress, deviceRegisterAddress, isBigEndian)
  *
  * Reads two sequential registers to build a 16-bit value. isBigEndian dictates
  * whether the first bits are either the high or low bits of the value.
  *
  * @param   I2CAddress              (char)  7-bit address of I2C device wished
  *                                          to interact with.
  * @param   deviceRegisterAddress   (char)  8-bit lower address of register on
  *                                          device.
  * @param   isBigEndian             (char)  Boolean determining if device is big
  *                                          or little endian.
  * @return                          (int)   [SUCCESS, ERROR]
  */
 int I2C_ReadInt_2(
     char I2CAddress,
     char deviceRegisterAddress,
     char isBigEndian
 )
 {
     short data = 0; // 16-bit return value.
     
     // Get first byte.
     unsigned char byte = I2C_ReadRegister_2(I2CAddress, deviceRegisterAddress);
     if (isBigEndian)
     {
         data = byte << 8;
     } else {
         data = byte;
     }
 
     // Get second byte.
     byte = I2C_ReadRegister_2(I2CAddress, deviceRegisterAddress + 1);
     if (isBigEndian)
     {
         data |= byte;
     } else {
         data |= byte << 8;
     }
     return data;
 }


/**
 * @brief Scan the I2C bus for devices.
 * @param hi2c  Pointer to an initialized I2C_HandleTypeDef (e.g., &hI2C2 or &hi2c2).
 */
void I2C_ScanBus_BNO055_A(void)
{
    printf("Starting BNO055_A scan...\r\n");
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_IsDeviceReady(&hi2c2, 40 << 1, 1, 100);
    if (ret == HAL_OK)
    {
        printf("BNO055_A device found at 7-bit address 0x%02X\r\n", 40 << 1);
    }
    
    printf("BNO055_A scan complete.\r\n");
}

void I2C_ScanBus_BNO055_B(void)
{
    printf("Starting BNO055_B scan...\r\n");
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_IsDeviceReady(&hi2c2, 41 << 1, 1, 100);
    if (ret == HAL_OK)
    {
        printf("BNO055_B device found at 7-bit address 0x%02X\r\n", 41 << 1);
    }
    printf("BNO055_B scan complete.\r\n");
}

void I2C_ScanBus_LCD_ADDRESS(void)
{
    printf("Starting LCD scan...\r\n");
    HAL_StatusTypeDef ret;
    

    ret = HAL_I2C_IsDeviceReady(&hi2c2, (0x7c>>1) << 1, 1, 100);
    if (ret == HAL_OK)
    {
        printf("LCD device found at 7-bit address 0x%02X\r\n", (0x7c>>1) << 1);
    }
    printf("LCD scan complete.\r\n");
}

void I2C_ScanBus_RGB_ADDRESS(void)
{
    printf("Starting RGB scan...\r\n");
    HAL_StatusTypeDef ret;
    
    ret = HAL_I2C_IsDeviceReady(&hi2c2, (0x5A>>1) << 1, 1, 100);
    if (ret == HAL_OK)
    {
        printf("RGB device found at 7-bit address 0x%02X\r\n", (0x5A>>1) << 1);
    }
    printf("RGB scan complete.\r\n");
}

void I2C_ScanBus(void)
{
    printf("Starting I2C scan...\r\n");
    HAL_StatusTypeDef ret;
    
    for (uint8_t addr = 0; addr < 128; addr++)
    {
        ret = HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 1, 100);
        if (ret == HAL_OK)
        {
            // We got an ACK at "addr"
            printf("I2C device found at 7-bit address 0x%02X\r\n", addr << 1);
        }
    }
    printf("I2C scan complete.\r\n");
}