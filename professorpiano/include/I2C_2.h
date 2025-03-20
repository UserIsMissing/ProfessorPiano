/**
 * @file    I2C.c
 * @author  Adam Korycki
 *
 * @date    3 Oct 2023
 */

 //Changed the I2C file to add a I2C scan bus function that acts as a buffer before start of any I2C communication to ensure all address's are set and at least 1 ack signal is intact. This function has helped me with the unable to read Tx issue and an issue encountered with clocking for running the I2S and I2C protocol simulatenously.

 #ifndef I2C_H_2
 #define	I2C_H_2
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include "stm32f4xx_hal.h"
 #include "stm32f4xx_hal_i2c.h"

 I2C_HandleTypeDef hi2c2;

 /** I2C_Init()
  *
  * Initializes the I2C System at standard speed (100Kbps).
  *
  * @return SUCCESS or ERROR
  */
 int8_t I2C_Init_2(void);
 
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
 unsigned char I2C_ReadRegister_2(unsigned char I2CAddress,unsigned char deviceRegisterAddress);
 
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
 unsigned char I2C_WriteReg_2(unsigned char I2CAddress, unsigned char deviceRegisterAddress, uint8_t data);
 
 /** I2C_ReadInt(I2CAddress, deviceRegisterAddress, isBigEndian)
  *
  * Reads two sequential registers to build a 16-bit value. isBigEndian dictates
  * whether the first bits are either the high or low bits of the value.
  *
  * @param   I2CAddresss             (char)  7-bit address of I2C device wished
  *                                          to interact with.
  * @param   deviceRegisterAddress   (char)  8-bit lower address of register on
  *                                          device.
  * @param   isBigEndian             (char)  Boolean determining if device is big
  *                                          or little endian.
  * @return                          (int)   [SUCCESS, ERROR]
  */
 int I2C_ReadInt_2(char I2CAddress, char deviceRegisterAddress, char isBigEndian);
 
 
void I2C_ScanBus_BNO055_A(void);
void I2C_ScanBus_BNO055_B(void);
void I2C_ScanBus_LCD_ADDRESS(void);
void I2C_ScanBus_RGB_ADDRESS(void);
void I2C_ScanBus(void);

#endif
