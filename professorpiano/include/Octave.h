#ifndef OCTAVE_H
#define OCTAVE_H

#include <stdint.h>  // For uint32_t type
#include <stdbool.h> // For boolean logic

// Octave thresholds for movement detection
#define OCTAVE_THRESHOLD_GYRO 90   
#define OCTAVE_THRESHOLD_ACCEL 800 
#define DRIFT_CORRECTIONX 0.0001
// 
// #define PRINT_OCTAVE_CHANGE

#define SAMPLE_SIZE 10  
#define DT 0.00001      // Time step in seconds

// Octave settings
#define OCTAVE_CHANGE_COOLDOWN 300  // Cooldown in milliseconds
#define OCTAVE_HYSTERESIS 500        // Prevents small oscillations
#define OCTAVE_MIN 3
#define STARTING_OCTAVE 4
#define OCTAVE_MAX 7
extern int currentOctave;

int updateOctave(int address);
float trapezoidal_average(float *samples, float new_value);

#endif // OCTAVE_H
