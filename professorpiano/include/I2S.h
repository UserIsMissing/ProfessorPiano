/**
 * @file    I2S.h
 * 
 * 
 * 
 *
 * @author  Ryan Taylor
 *
 * @date    28 Feb 2025
 * 
 **/

#ifndef I2S_H
#define I2S_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// External declaration for I2S handle.
extern I2S_HandleTypeDef hi2s1;

#define SAMPLE_RATE       48000
#define NUM_CHANNELS      2       // stereo
#define BITS_PER_SAMPLE   16
#define AUDIO_BUFFER_SIZE 1024    // total 16-bit samples, i.e. 512 stereo frames
#define NUM_VOICES 4
#define WAVE_TABLE_SIZE 256

extern int16_t sineTable[WAVE_TABLE_SIZE];
extern int16_t i2sTxBuffer[AUDIO_BUFFER_SIZE];  // Holds interleaved stereo samples
// Voice structure
typedef struct {
    bool active;
    float phase;
    float phaseIncrement;
    float amplitude;
} Voice_t;
extern Voice_t voices[NUM_VOICES];

// Function prototypes.
void I2S_Init(void);
void InitSineTable(void);
void startVoice(int voiceIndex, float freq, float amplitude);
void stopVoice(int voiceIndex);



void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s);
void fillAudioBuffer(int16_t *pBuffer, int numSamples);
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s);
/** Error_Handler()
 * 
 * Enter a state of blinking the status LED on-board the Nucleo, indefinitely.
 */
void Error_Handler_3(void);

#endif // I2S_H
