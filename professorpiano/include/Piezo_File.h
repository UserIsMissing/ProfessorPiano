/*
 * *  File: Piezo_File.c
 @a   Author: Cole Schreiner
 @f   Date Created: 3/6/2025
 @d   Description:
 *      State Machine for the Piezo sensor.
 *      It will switch between states depending on if the correct note is being played for given a song.
 *      Correct note is determined by the state machine expecting a specific finger to play white or black keys.
 *      For the right hand, it starts in Middle c (unless BNO055 is used to change the octave).
 *      Fingers are as follows:
 *           Thumb = C
 *           Index = D
 *           Middle = E
 *           Ring = F
 *           Pinky = G
 *      Sharp/Flat notes for right hand:
 *           Thumb = C#
 *           Index = D#
 *           Middle = F#
 *           Ring = G#
 *           Pinky = A#
 */

/*
Alternatively, I might just use a function for the song instead of a state machine.
*/

// INCLUDES ***************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <ADC_2.h>
#include <string.h>
#include <timers.h>
#include <leds.h>
#include <math.h>
// #include <NoteFrequencies.h>

// DEFINE TESTS ***********************************************************************************
// #define EXCLUDE_MAIN
#define PIEZO

// #define PIEZO_CALIBRATE
// #define PIEZO_TELEPLOT
#define PIEZO_FREEPLAY
// #define SONG_TEST_SONG

#define HARDCODED
#define SOUND

// #define TWINKLETWINKLE_SONG

// DEFINES ****************************************************************************************
#define PRINT_FLAG 1
#define SLOW_PRINTING 100

// #ifdef HARDCODED
// #define Calibrate_B_Thumb_BlackKey 60
// #define Calibrate_B_Thumb_WhiteKey 110

// #define Calibrate_Thumb_BlackKey 60
// #define Calibrate_Thumb_WhiteKey 130

// #define Calibrate_Index_BlackKey 80
// #define Calibrate_Index_WhiteKey 160

// #define Calibrate_Middle_BlackKey 70
// #define Calibrate_Middle_WhiteKey 300

// #define Calibrate_Ring_BlackKey 50
// #define Calibrate_Ring_WhiteKey 170

// #define Calibrate_Pinky_BlackKey 55
// #define Calibrate_Pinky_WhiteKey 110

// #define Calibrate_A_Pinky_BlackKey 55
// #define Calibrate_A_Pinky_WhiteKey 100
// #endif // HARDCODED

#define NOISE_THRESHOLD 45

// PINOUTS ****************************************************************************************
#define B_THUMB_PIN ADC_6   // Pin 21 - Piezo Sensor (ADC_6)   PA6
#define THUMB_PIN   ADC_0   // Pin 36 - Piezo Sensor (ADC_0)   PA0
#define INDEX_PIN   ADC_1   // Pin 37 - Piezo Sensor (ADC_1)   PA1
#define MIDDLE_PIN  ADC_2   // Pin 38 - Piezo Sensor (ADC_2)   PC0
#define RING_PIN    ADC_3   // Pin 39 - Piezo Sensor (ADC_3)   PC1
#define PINKY_PIN   ADC_4   // Pin 40 - Piezo Sensor (ADC_4)   PC2
#define A_PINKY_PIN ADC_5   // Pin 41 - Piezo Sensor (ADC_5)   PC3
// Ground                           Pin 42 - Ground                 GND

// LED Pins
#define LED_B_THUMB_PIN GPIO_0
#define LED_THUMB_PIN   GPIO_1
#define LED_INDEX_PIN   GPIO_2
#define LED_MIDDLE_PIN  GPIO_3
#define LED_RING_PIN    GPIO_4
#define LED_PINKY_PIN   GPIO_5
#define LED_A_PINKY_PIN GPIO_6

#define LEDON   HIGH
#define LEDOFF  LOW
// #define LED_ALL_PINS    GPIO_
// #define LED_OFF_PINS    GPIO_
// GLOBAL VARIABLES *******************************************************************************

// TYPEDEFS ***************************************************************************************
typedef enum
{
    Unknown = 0,
    B_Thumb = 1,
    Thumb = 2,
    Index = 3,
    Middle = 4,
    Ring = 5,
    Pinky = 6,
    A_Pinky = 7
} Finger_t;


typedef enum
{
    INVALID_KEY = -1,
    BLACK_KEY = 1,
    WHITE_KEY = 0
} KeyType_t;

typedef enum
{
    Note_B = 0,
    Note_C = 1,
    Note_Cs = 2,
    Note_D = 3,
    Note_Ds = 4,
    Note_E = 5,
    Note_F = 6,
    Note_Fs = 7,
    Note_G = 8,
    Note_Gs = 9,
    Note_A = 10,
    Note_As = 11
} Note_t;

// LOOKUP TABLES **********************************************************************************
extern const Note_t NOTE_MAP[7][2];
extern const char *TWINKLETWINKLE[];
extern const char *SONG_TEST[];
extern const char *SONG_TEST2[];
extern const char *GetNoteString(Note_t note);
extern const uint8_t FINGER_PIN[8];
extern const uint8_t LED_PIN[8];

// Define the number of octaves and notes
#define NUM_OCTAVES 8  // From Octave 0 to Octave 7
#define NUM_NOTES 12   // 12 notes per octave (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
extern const float NOTE_FREQUENCIES[NUM_OCTAVES][NUM_NOTES];

// GLOBAL VARIABLES *******************************************************************************
#define MAX_VOICES 3
extern int isVoiceActive[MAX_VOICES];
extern float voiceFrequency[MAX_VOICES];
extern uint32_t STARTSOUNDTIME[MAX_VOICES]; // Time when the sound started
extern uint32_t SOUND_DURATION;             // Duration of the sound

// FUNCTION PROTOTYPES ****************************************************************************
/**
 * @author Cole Schreiner
 * @brief Tracks expired sounds and stops them if they have exceeded their duration.
 */
void StopTheVoicesInYourHead();

/**
 * @brief Measures key press thresholds for a given finger.
 * @param finger The finger being used.
 * @param blackKeyThreshold Pointer to store the black key threshold.
 * @param whiteKeyThreshold Pointer to store the white key threshold.
 */
void MeasureKeyPressThresholds(Finger_t finger, uint16_t *blackKeyThreshold, uint16_t *whiteKeyThreshold);

/**
 * @brief Turns off all LEDs.
 */
void OFF_LEDS(void);

/**
 * @brief Sets the LEDs to indicate the played note.
 */
void SET_LEDS(const char *note);

/**
 * @brief Initializes the Piezo sensor, setting baselines and thresholds for each finger.
 */
void Piezo_Init(void);

/**
 * @brief Detects peaks in Piezo readings and determines key presses.
 * @param Piezo_Read The current Piezo sensor reading.
 * @param finger The finger being used for pressing.
 * @param currentTime The current system time.
 * @return The detected peak value or 0 if no valid peak is found.
 */
uint16_t PiezoMovingPeakDetector(uint16_t Piezo_Read, Finger_t finger, uint32_t currentTime);

/**
 * @brief Determines whether a pressed key is a white or black key.
 * @param PiezoPeak The detected peak value.
 * @param finger The finger being used for pressing.
 * @return 1 if a white key is pressed, 0 for a black key, and -1 if no key is detected.
 */
int WhiteOrBlackKey(int PiezoPeak, Finger_t finger);

/**
 * @brief Guides the user in playing Twinkle Twinkle Little Star by detecting correct notes.
 * @return 1 if the song is completed, 0 if still in progress.
 */
int TwinkleTwinkle(void);


// ************************************************************************************************
// Frequency lookup table for each note and octave
// static const float NOTE_FREQUENCIES[NUM_OCTAVES][NUM_NOTES] = {
//     // Octave 0
//     {16.35f, 17.32f, 18.35f, 19.45f, 20.60f, 21.83f, 23.12f, 24.50f, 25.96f, 27.50f, 29.14f, 30.87f},
//     // Octave 1
//     {32.70f, 34.65f, 36.71f, 38.89f, 41.20f, 43.65f, 46.25f, 49.00f, 51.91f, 55.00f, 58.27f, 61.74f},
//     // Octave 2
//     {65.41f, 69.30f, 73.42f, 77.78f, 82.41f, 87.31f, 92.50f, 98.00f, 103.83f, 110.00f, 116.54f, 123.47f},
//     // Octave 3
//     {130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 185.00f, 196.00f, 207.65f, 220.00f, 233.08f, 246.94f},
//     // Octave 4
//     {261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f},
//     // Octave 5
//     {523.25f, 554.37f, 587.33f, 622.25f, 659.25f, 698.46f, 739.99f, 783.99f, 830.61f, 880.00f, 932.33f, 987.77f},
//     // Octave 6
//     {1046.50f, 1108.73f, 1174.66f, 1244.51f, 1318.51f, 1396.91f, 1479.98f, 1567.98f, 1661.22f, 1760.00f, 1864.66f, 1975.53f},
//     // Octave 7
//     {2093.00f, 2217.46f, 2349.32f, 2489.02f, 2637.02f, 2793.83f, 2959.96f, 3135.96f, 3322.44f, 3520.00f, 3729.31f, 3951.07f}
// };

// // Function to get the frequency of a note at a given octave
// static float GetNoteFrequency(Note_t note, uint8_t octave) {
//     if (octave >= NUM_OCTAVES || note < Note_C || note > Note_B) {
//         return 0.0f; // Invalid octave or note
//     }
//     return NOTE_FREQUENCIES[octave][note - 1]; // Note_C is index 0 in the table
// };