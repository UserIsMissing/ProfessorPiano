#include "stm32f4xx_hal.h"
#include "Board_2.h"
#include "I2S.h"
#include "I2C_2.h"
#include "ADC_2.h"
#include "GPIO_2.h"
#include "BNO055_2.h"
#include <timers.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <wav_packet_reader.h>
#include <Piezo_File.h>
#include <math.h>
#include <DFRobot_LCD.h>
#include <Octave.h>

// PINOUTS ******************************************************************************
// #define INDEX_PIN ADC_1 // Pin 37 - Piezo Sensor (ADC_1)

// DEFINE VARIABLES *********************************************************************

// PIEZO ----------
// #define WhiteKey 100
// #define BlackKey 60
// DEFINE TESTS *************************************************************************
//  #define OCTAVE_TEST

// #define I2S_TEST

// // #define EXCLUDE_MAIN
 // #define PIEZO
// #define PIEZO_CALIBRATE
// // #define PIEZO_TELEPLOT
// // #define PIEZO_FREEPLAY

// #define HARDCODED

// #define SOUND
// #define SONG_TEST_SONG

// Global Variables *********************************************************************

// **************************************************************************************

#ifndef EXCLUDE_MAIN
int main(void)
{
    BOARD_Init_2();
    GPIO_Init_2();
    ADC_Init_2();
    TIMER_Init();
    I2S_Init();
    BNO055_Init_2(BNO055_ADDRESS_A);
    // BNO055_Init_2(BNO055_ADDRESS_B);
    InitSineTable();
    if (HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)i2sTxBuffer, AUDIO_BUFFER_SIZE) != HAL_OK)
    {
        Error_Handler_3();
    }
    DFRobot_RGBLCD_Init(&myLCD, 16, 2, LCD_ADDRESS, RGB_ADDRESS);
    DFRobot_RGBLCD_Clear(&myLCD);
    DFRobot_RGBLCD_SetCursor(&myLCD, 0, 0);
    DFRobot_RGBLCD_Print(&myLCD, "PIANO PROFESSOR!");
    DFRobot_RGBLCD_SetColor(&myLCD, BLUE);
    OFF_LEDS();

#ifdef I2S_TEST
    startVoice(0, 261.6f, 0.25f);
    startVoice(1, 329.6f, 0.25f);
    startVoice(2, 392.0f, 0.25f);
    startVoice(3, 523.3f, 0.25f);
    GPIO_WritePin_2(GPIO_0, HIGH);
    GPIO_WritePin_2(GPIO_1, HIGH);
    GPIO_WritePin_2(GPIO_2, HIGH);
    GPIO_WritePin_2(GPIO_3, HIGH);
    GPIO_WritePin_2(GPIO_4, HIGH);
    GPIO_WritePin_2(GPIO_5, HIGH);
    GPIO_WritePin_2(GPIO_6, HIGH);

    // Initialize the LCD with 16 columns, 2 rows, standard addresses:
    DFRobot_RGBLCD_Init(&myLCD, 16, 2, LCD_ADDRESS, RGB_ADDRESS);
    DFRobot_RGBLCD_Clear(&myLCD);
    DFRobot_RGBLCD_SetColor(&myLCD, BLUE);

    DFRobot_RGBLCD_SetCursor(&myLCD, 0, 0);
    DFRobot_RGBLCD_Print(&myLCD, "Professor Piano");
    DFRobot_RGBLCD_SetCursor(&myLCD, 0, 10);
    DFRobot_RGBLCD_Print(&myLCD, "ECE 167");


    while (1)
    {
        // printf("ADC: %d\n", ADC_Read_2(ADC_5));
        printf("A: X: %d, Y: %d, Z:%d\n", BNO055_ReadAccelX_2(BNO055_ADDRESS_A),BNO055_ReadAccelY_2(BNO055_ADDRESS_A), BNO055_ReadAccelZ_2(BNO055_ADDRESS_A));
        // printf("A: X: %d, Y: %d, Z: %d; B: X: %d, Y:%d, Z:%d; ADC: %d\n", BNO055_ReadAccelX_2(BNO055_ADDRESS_A),BNO055_ReadAccelY_2(BNO055_ADDRESS_A), BNO055_ReadAccelZ_2(BNO055_ADDRESS_A), BNO055_ReadAccelX_2(BNO055_ADDRESS_B), BNO055_ReadAccelY_2(BNO055_ADDRESS_B), BNO055_ReadAccelZ_2(BNO055_ADDRESS_B), ADC_Read_2(ADC_6));
        // DFRobot_RGBLCD_SetCursor(&myLCD, 0, 0);
        // int16_t BNO_A = BNO055_ReadAccelX_2(BNO055_ADDRESS_A);
        // char textBuf_A[20];
        // sprintf(textBuf_A, "BNO_A: %d", BNO_A);
        // DFRobot_RGBLCD_Print(&myLCD, textBuf_A);
       

        // DFRobot_RGBLCD_SetCursor(&myLCD, 0, 10);
        // int16_t BNO_B = BNO055_ReadAccelX_2(BNO055_ADDRESS_A);
        // char textBuf_B[20];
        // sprintf(textBuf_B, "BNO_B: %d", BNO_B);

        // DFRobot_RGBLCD_Print(&myLCD, textBuf_B);
        HAL_Delay(10);
    }
#endif //I2S_TEST

#ifdef OCTAVE_TEST
    while (TRUE)
    {
        updateOctave(BNO055_ADDRESS_A);
        printf("Octave: %d\n", currentOctave);
        // updateOctave(BNO055_ADDRESS_B);
    }

#endif // OCTAVE_TEST

#ifdef PIEZO
#ifdef PIEZO_CALIBRATE
    Piezo_Init();
#endif // PIEZO_CALIBRATE

    while (1)
    {

#ifdef PIEZO_FREEPLAY

        uint32_t currentTime = TIMERS_GetMilliSeconds();

        uint16_t Piezo_Read_Index[7] =
            {
                ADC_Read_2(B_THUMB_PIN),
                ADC_Read_2(THUMB_PIN),
                ADC_Read_2(INDEX_PIN),
                ADC_Read_2(MIDDLE_PIN),
                ADC_Read_2(RING_PIN),
                ADC_Read_2(PINKY_PIN),
                ADC_Read_2(A_PINKY_PIN)};

        updateOctave(BNO055_ADDRESS_A);
        // printf("Octave: %d\n", currentOctave);

        // Process all fingers
        for (int i = 0; i < 7; i++)
        {
            uint16_t peak = PiezoMovingPeakDetector(Piezo_Read_Index[i], (Finger_t)i, currentTime);
            if (peak > NOISE_THRESHOLD)
            // if (validPeakDetected == 1)
            {
                // Check if the played note matches the expected note
                KeyType_t keyType = WhiteOrBlackKey(peak, (Finger_t)i);
                if ((keyType == 0) || (keyType == 1))
                {
                    const char *played_note = GetNoteString(NOTE_MAP[i][keyType]);
                    SET_LEDS(played_note);
                    printf("Finger %d played note: %s\n", i, played_note);
                }
            }
        }
        /*
            if ((TIMERS_GetMilliSeconds() - STARTSOUNDTIME) > SOUND_DURATION)
            {
                stopVoice(0); // Stop the voice if the duration is exceeded
                // printf("STOPPING NOTE\n");
            }
        */
#endif // PIEZO_FREEPLAY

#ifdef PIEZO_TELEPLOT
        printf(">B_Thumb:%d\n", ADC_Read_2(B_THUMB_PIN));
        printf(">Thumb:%d\n", ADC_Read_2(THUMB_PIN));
        printf(">Index:%d\n", ADC_Read_2(INDEX_PIN));
        printf(">Middle:%d\n", ADC_Read_2(MIDDLE_PIN));
        printf(">Ring:%d\n", ADC_Read_2(RING_PIN));
        printf(">Pinky:%d\n", ADC_Read_2(PINKY_PIN));
        printf(">A_Pinky:%d\n", ADC_Read_2(A_PINKY_PIN));
#endif // PIEZO_TELEPLOT

#if defined(SONG_TEST_SONG) || defined(TWINKLETWINKLE_SONG)
        TwinkleTwinkle();
#endif // SONG_TEST_SONG || TWINKLETWINKLE_SONG

        StopTheVoicesInYourHead();

        HAL_Delay(20);
        // i++;
    }
#endif // PIEZO
}
#endif // EXCLUDE_MAIN