#include <Octave.h>
#include <stdio.h> // For printf
#include "stm32f4xx_hal.h"
#include <BNO055_2.h>

// Initialize current octave
int currentOctave = STARTING_OCTAVE;
float angleXA = 0;
float prevXA = 0;
float gyroX_samples[SAMPLE_SIZE] = {0};
float gyroX = 0;
// Store previous readings
int sample_index = 0;
int lastOctaveChangeTime = 0; // Store last change timestamp

// Trapezoidal filter to smooth gyro data
float trapezoidal_average(float *samples, float new_value)
{
    samples[sample_index] = new_value;
    float avg = (samples[(sample_index - 1 + SAMPLE_SIZE) % SAMPLE_SIZE] + new_value) * 0.5;
    sample_index = (sample_index + 1) % SAMPLE_SIZE;
    return avg;
}

// Octave change function

int updateOctave(int address)
{
    float rawGyroXA = (BNO055_ReadGyroX_2(address) - 18.19) / 2.98;
    float accelX = BNO055_ReadAccelX_2(address);
    float filteredXA = trapezoidal_average(gyroX_samples, rawGyroXA) - DRIFT_CORRECTIONX * HAL_GetTick();
    gyroX += ((filteredXA + prevXA) * 0.5 * DT);
    prevXA = filteredXA;

    uint32_t currentTime = HAL_GetTick();
    bool moveRight = (gyroX > OCTAVE_THRESHOLD_GYRO) || (accelX > OCTAVE_THRESHOLD_ACCEL);
    bool moveLeft = (gyroX < -OCTAVE_THRESHOLD_GYRO) || (accelX < -OCTAVE_THRESHOLD_ACCEL);

    if (currentTime - lastOctaveChangeTime > OCTAVE_CHANGE_COOLDOWN)
    {
        if (moveRight && !moveLeft && currentOctave > OCTAVE_MIN)
        {
            currentOctave--;
#ifdef PRINT_OCTAVE_CHANGE
            printf("Octave Down! Current Octave: %d\n", currentOctave);
#endif
            lastOctaveChangeTime = currentTime;
            return currentOctave;
        }
        else if (moveLeft && !moveRight && currentOctave < OCTAVE_MAX)
        {
            currentOctave++;

#ifdef PRINT_OCTAVE_CHANGE
            printf("Octave Up! Current Octave: %d\n", currentOctave);
#endif

            lastOctaveChangeTime = currentTime;
            return currentOctave;
        }
        // else
        // {
        //     currentOctave = STARTING_OCTAVE;
        //     return currentOctave;
        // }
    }
    return currentOctave;
}
