/**
 * @file    BNO055.h
 *
 * Software module to communicate with the IMU over I2C.
 * Provides access to each raw sensor axis along with raw temperature.
 *
 * @author  Aaron Hunter
 * @author  Adam Korycki
 * 
 * @date    11 Apr 2022
 */

//changed BNO055 file to make use of the BNO055's dual address system by making the address an input for the functions

 #ifndef BNO055_H_2
 #define BNO055_H_2
 
 #include <stdint.h>
 
 
 /*  MODULE-LEVEL DEFINITIONS, MACROS    */
 /** BNO055 Address A: when ADR (COM3) pin is tied to ground (default). **/
 #define BNO055_ADDRESS_A (0x28)
 /** BNO055 Address B: when ADR (COM3) pin is tied to +3.3V. **/
 #define BNO055_ADDRESS_B (0x29)
 /** BNO055 ID **/
 #define BNO055_ID (0xA0)
 // Page numbers.
 #define BNO055_PAGE0 0
 #define BNO055_PAGE1 1
 /** Sensor configuration values: 
  * ACC_PWR_Mode <2:0> ACC_BW <2:0> ACC_Range <1:0>
  * 64Hz [GYR_Config_0]: xx110xxxb | 250 dps [GYR_Config_0]: xxxxx011b
  **/
 #define ACC_CONFIG_PARAMS (0x18) // +/-2g, 62.5 Hz BW
 #define GYRO_CONFIG_PARAMS_0 (0x33)
 #define UNITS_PARAM (0x01)
 
 
 /*  PROTOTYPES  */
 /** BNO055_Init()
  *
  * Initializes the BNO055 for usage.
  * Sensors will be at:
  *  + Accel: 2g
  *  + Gyro: 250dps
  *
  * @return  (int8_t)    [SUCCESS, ERROR]
  */
 int8_t BNO055_Init_2(uint8_t address);
 
 /** BNO055_ReadAccelX()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadAccelX_2(uint8_t address);
 
 /** BNO055_ReadAccelY()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadAccelY_2(uint8_t address);
 
 /**
  * BNO055_ReadAccelZ()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadAccelZ_2(uint8_t address);
 
 /** BNO055_ReadGyroX()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadGyroX_2(uint8_t address);
 
 /** BNO055_ReadGyroY()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadGyroY_2(uint8_t address);
 
 /** BNO055_ReadGyroZ()
  *
  * Reads sensor axis as given by name.
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadGyroZ_2(uint8_t address);
 
 /** BNO055_ReadMagX()
  *
  * Reads sensor axis as given by name.
  *
  * @return  (int)   Returns raw sensor reading as an int.
  */
 int BNO055_ReadMagX_2(uint8_t address);
 
 /** BNO055_ReadMagY()
  *
  * Reads sensor axis as given by name.
  *
  * @return  (int)   Returns raw sensor reading as an int.
  */
 int BNO055_ReadMagY_2(uint8_t address);
 
 /** BNO055_ReadMagZ()
  *
  * Reads sensor axis as given by name
  *
  * @return	(int)	Returns raw sensor reading as an int.
  */
 int BNO055_ReadMagZ_2(uint8_t address);
 
 /** BNO055_ReadTemp()
  *
  * @brief Reads sensor axis as given by name.
  *
  * @return  (int)   Returns raw sensor reading as an int.
  */
 int BNO055_ReadTemp_2(uint8_t address);
 
 
 #endif  /*  BNO055_H_2    */
 