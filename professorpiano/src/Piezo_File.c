/**
 *   @file: Piezo_File.c
 *   @author: Cole Schreiner
 *   @date Created: 3/6/2025
 *   @details: State Machine for the Piezo sensor.
 *      @brief It will switch between states depending on if the correct note is being played for given a song.
 *      @brief Correct note is determined by the state machine expecting a specific finger to play white or black keys.
 *      @brief For the right hand, it starts in Middle c (unless BNO055 is used to change the octave).
 *      @brief Fingers are as follows:
 *      @brief      Thumb = C
 *      @brief      Index = D
 *      @brief      Middle = E
 *      @brief      Ring = F
 *      @brief      Pinky = G
 *      @brief Sharp/Flat notes for right hand:
 *      @brief      Thumb = C#
 *      @brief      Index = D#
 *      @brief      Middle = F#
 *      @brief      Ring = G#
 *      @brief      Pinky = A#
 */
/*
Alternatively, I might just use a function for the song instead of a state machine.
 */

// HJappy Birthday Notes
// GAGCBGGAGDCGGGECCBAFFECDC

// INCLUDES ***************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <Piezo_File.h>
#include "Board.h"
#include "I2S.h"
#include <GPIO_2.h>
#include <math.h>
#include <Octave.h>
#include <BNO055_2.h>
#include <DFRobot_LCD.h>

// DEFINE TESTS ***********************************************************************************

// DEFINES ****************************************************************************************
// #define Index_WhiteKey 100
// #define Index_BlackKey 60

#define PEAK_WINDOW 10
#define PRIORITY_TIME_WINDOW 150
// GLOBAL VARIABLES ***** **************************************************************************
// volatile uint16_t adcBuffer[PEAK_WINDOW] = {0}; // Circular buffer for peak detection
// volatile uint16_t adcBufferIndex = 0;           // Index for circular buffer
volatile uint16_t PiezoPeak = 0;                                                                                         // Current peak value for peak detection or moving average
static uint32_t lastPressTime[7] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}; // Last press time for each finger
static int validPeakDetected = 0;                                                                                        // Flag to indicate a valid peak

volatile float frequency = 0.0f; // Frequency of the note being played

// Global variables to store calibrated finger thresholds
#ifndef HARDCODED
static uint16_t Calibrate_B_Thumb_BlackKey = 0;
static uint16_t Calibrate_B_Thumb_WhiteKey = 0;
static uint16_t Calibrate_Thumb_BlackKey = 0;
static uint16_t Calibrate_Thumb_WhiteKey = 0;
static uint16_t Calibrate_Index_BlackKey = 0;
static uint16_t Calibrate_Index_WhiteKey = 0;
static uint16_t Calibrate_Middle_BlackKey = 0;
static uint16_t Calibrate_Middle_WhiteKey = 0;
static uint16_t Calibrate_Ring_BlackKey = 0;
static uint16_t Calibrate_Ring_WhiteKey = 0;
static uint16_t Calibrate_Pinky_BlackKey = 0;
static uint16_t Calibrate_Pinky_WhiteKey = 0;
static uint16_t Calibrate_A_Pinky_BlackKey = 0;
static uint16_t Calibrate_A_Pinky_WhiteKey = 0;
#endif // HARDCODED
#ifdef HARDCODED
static uint16_t Calibrate_B_Thumb_BlackKey = 50;
static uint16_t Calibrate_B_Thumb_WhiteKey = 110;
static uint16_t Calibrate_Thumb_BlackKey = 60;
static uint16_t Calibrate_Thumb_WhiteKey = 130;
static uint16_t Calibrate_Index_BlackKey = 50;
static uint16_t Calibrate_Index_WhiteKey = 60;
static uint16_t Calibrate_Middle_BlackKey = 70;
static uint16_t Calibrate_Middle_WhiteKey = 300;
static uint16_t Calibrate_Ring_BlackKey = 50;
static uint16_t Calibrate_Ring_WhiteKey = 170;
static uint16_t Calibrate_Pinky_BlackKey = 55;
static uint16_t Calibrate_Pinky_WhiteKey = 80;
static uint16_t Calibrate_A_Pinky_BlackKey = 55;
static uint16_t Calibrate_A_Pinky_WhiteKey = 100;
#endif // HARDCODED

int isVoiceActive[MAX_VOICES] = {0};       // Array to track active voices
float voiceFrequency[MAX_VOICES] = {0.0f}; // Frequencies of the active voices
uint32_t STARTSOUNDTIME[MAX_VOICES] = {0}; // Time when the sound started
uint32_t SOUND_DURATION = 500;             // Duration of the sound

// TYPEDEFS ***************************************************************************************

// LOOKUP TABLES **********************************************************************************
const Note_t NOTE_MAP[7][2] =
    {
        /* WHITE_KEY  BLACK_KEY      FINGER */
        {Note_B, Note_B},  /* B_Thumb */
        {Note_C, Note_Cs}, /* Thumb */
        {Note_D, Note_Ds}, /* Index */
        {Note_E, Note_E},  /* Middle */
        {Note_F, Note_Fs}, /* Ring */
        {Note_G, Note_Gs}, /* Pinky */
        {Note_A, Note_As}  /* A_Pinky */
};

const char *TWINKLETWINKLE[] =
    {
        "C",
        "G",
        "A",
        "G",
        "F",
        "E",
        "D",
        "C",
        "G",
        "F",
        "E",
        "D",
        "G",
        "F",
        "E",
        "D",
        "C",
        "G",
        "A",
        "G",
        "F",
        "E",
        "D",
        "C"};

const char *SONG_TEST[] =
    {
        "C",
        "D",
        "E",
        "F",
        "G",
        "C#",
        "D#",
        "F#",
        "G#"};

const char *SONG_TEST2[] =
    {
        "C",
        "C",
        "C",
        "D",
        "D",
        "D",
        "E",
        "E",
        "E",
        "F",
        "F",
        "F",
        "G",
        "G",
        "G"};

const char *GetNoteString(Note_t note)
{
    switch (note)
    {
    case Note_B:
        return "B";
    case Note_C:
        return "C";
    case Note_Cs:
        return "C#";
    case Note_D:
        return "D";
    case Note_Ds:
        return "D#";
    case Note_E:
        return "E";
    case Note_F:
        return "F";
    case Note_Fs:
        return "F#";
    case Note_G:
        return "G";
    case Note_Gs:
        return "G#";
    case Note_A:
        return "A";
    case Note_As:
        return "A#";
    default:
        return "Unknown";
    }
};

const uint8_t FINGER_PIN[8] = {
    [B_Thumb] = ADC_6, // Thumb = 1
    [Thumb] = ADC_0,   // Thumb = 2
    [Index] = ADC_1,   // Index = 3
    [Middle] = ADC_2,  // Middle = 4
    [Ring] = ADC_3,    // Ring = 5
    [Pinky] = ADC_4,   // Pinky = 6
    [A_Pinky] = ADC_5  // A_Pinky = 7
};

const uint8_t LED_PIN[8] = {
    [B_Thumb] = LED_B_THUMB_PIN, // B_Thumb = 1
    [Thumb] = LED_THUMB_PIN,     // Thumb = 2
    [Index] = LED_INDEX_PIN,     // Index = 3
    [Middle] = LED_MIDDLE_PIN,   // Middle = 4
    [Ring] = LED_RING_PIN,       // Ring = 5
    [Pinky] = LED_PINKY_PIN,     // Pinky = 6
    [A_Pinky] = LED_A_PINKY_PIN  // A_Pinky = 7
};

const float NOTE_FREQUENCIES[NUM_OCTAVES][NUM_NOTES] = {
    // Notes go: B, C,      C#,             D,      D#,         E,          F,          F#,         G,          G#, A, A#
    // Octave 0
    // {16.35f,    17.32f,     18.35f,     19.45f,     20.60f,     21.83f,     23.12f,     24.50f,     25.96f,     27.50f,     29.14f}
    // Octave 1
    {30.87f, 32.70f, 34.65f, 36.71f, 38.89f, 41.20f, 43.65f, 46.25f, 49.00f, 51.91f, 55.00f, 58.27f},
    // Octave 2
    {61.74f, 65.41f, 69.30f, 73.42f, 77.78f, 82.41f, 87.31f, 92.50f, 98.00f, 103.83f, 110.00f, 116.54f},
    // Octave 3
    {123.47f, 130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 184.99f, 196.00f, 207.65f, 220.00f, 233.08f},
    // Octave 4
    {246.94f, 261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f, 466.16f},
    // Octave 5
    {493.88f, 523.25f, 554.37f, 587.33f, 622.25f, 659.25f, 698.46f, 739.99f, 783.99f, 830.61f, 880.00f, 932.33f},
    // Octave 6
    {987.77f, 1046.50f, 1108.73f, 1174.66f, 1244.51f, 1318.51f, 1396.91f, 1479.98f, 1567.98f, 1661.22f, 1760.00f, 1864.66f},
    // Octave 7
    {1975.53f, 2093.00f, 2217.46f, 2349.32f, 2489.02f, 2637.02f, 2793.83f, 2959.96f, 3135.96f, 3322.44f, 3520.00f, 3729.31f},
    // Octave 8
    {3951.07f, 4186.01f, 4434.92f, 4698.63f, 4978.03f, 5274.04f, 5587.65f, 5919.91f, 6271.93f, 6644.88f, 7040.00f, 7458.62f}};

/*
const float NOTE_FREQUENCIES[NUM_OCTAVES][NUM_NOTES] = {
    // Notes go: C, C#, D, D#, E, F, F#, G, G#, A, A#, B
    // Octave 0
    // {16.35f,    17.32f,     18.35f,     19.45f,     20.60f,     21.83f,     23.12f,     24.50f,     25.96f,     27.50f,     29.14f,  30.87f}
    // Octave 1
    {32.70f,     34.65f,     36.71f,     38.89f,     41.20f,     43.65f,     46.25f,     49.00f,     51.91f,     55.00f,     58.27f,    61.74f},
    // Octave 2
    {65.41f,     69.30f,     73.42f,     77.78f,     82.41f,     87.31f,     92.50f,     98.00f,     103.83f,    110.00f,    116.54f,   123.47f},
    // Octave 3
    {130.81f,    138.59f,    146.83f,    155.56f,    164.81f,    174.61f,    184.99f,    196.00f,    207.65f,    220.00f,    233.08f,   246.94f},
    // Octave 4
    {261.63f,    277.18f,    293.66f,    311.13f,    329.63f,    349.23f,    369.99f,    392.00f,    415.30f,    440.00f,    466.16f,   493.88f},
    // Octave 5
    {523.25f,    554.37f,    587.33f,    622.25f,    659.25f,    698.46f,    739.99f,    783.99f,    830.61f,    880.00f,    932.33f,   987.77f},
    // Octave 6
    {1046.50f,   1108.73f,   1174.66f,   1244.51f,   1318.51f,   1396.91f,   1479.98f,   1567.98f,   1661.22f,   1760.00f,   1864.66f,  1975.53f},
    // Octave 7
    {2093.00f,   2217.46f,   2349.32f,   2489.02f,   2637.02f,   2793.83f,   2959.96f,   3135.96f,   3322.44f,   3520.00f,   3729.31f,  3951.07f},
    // Octave 8
    {4186.01f,   4434.92f,   4698.63f,   4978.03f,   5274.04f,   5587.65f,   5919.91f,   6271.93f,   6644.88f,   7040.00f,   7458.62f}};
 */
// ************************************************************************************************
// Function to get the frequency of a note at a given octave
/*
static float GetNoteFrequency(Note_t note, uint8_t octave, KeyType_t keyType)
{
    if (octave >= NUM_OCTAVES)
    {
        return 0.0f; // Invalid octave or note
    }
    return NOTE_FREQUENCIES[octave][note + keyType]; // Note_B is index 0 in the table
};
 */
static float GetNoteFrequency(Note_t note, uint8_t octave, KeyType_t keyType)
{
    if (octave >= NUM_OCTAVES)
    {
        return 0.0f; // Invalid octave
    }

    // Use NOTE_MAP to get the correct note
    // Note_t mappedNote = NOTE_MAP[note][keyType];

    // Map the note to its index in the NOTE_FREQUENCIES table
    int index = 0;
    switch (note)
    {
    case Note_B:
        index = 0;
        break;
    case Note_C:
        index = 1;
        break;
    case Note_Cs:
        index = 2;
        break;
    case Note_D:
        index = 3;
        break;
    case Note_Ds:
        index = 4;
        break;
    case Note_E:
        index = 5;
        break;
    case Note_F:
        index = 6;
        break;
    case Note_Fs:
        index = 7;
        break;
    case Note_G:
        index = 8;
        break;
    case Note_Gs:
        index = 9;
        break;
    case Note_A:
        index = 10;
        break;
    case Note_As:
        index = 11;
        break;
    default:
        return 0.0f; // Invalid note
    }

    return NOTE_FREQUENCIES[octave][index];
}

// Function to measure key press thresholds for a specific finger
void MeasureKeyPressThresholds(Finger_t finger, uint16_t *blackKeyThreshold, uint16_t *whiteKeyThreshold)
{
    HAL_Delay(200); // Small delay before starting the calibration
    printf("---------------------------------------------------------------\n");
    printf("Calibrating finger %d...\n", finger);

    uint32_t currentTime = HAL_GetTick(); // Get the current time for peak detection

    // Measure soft presses (BlackKey threshold)
    printf("Perform 3 soft presses for finger %d...\n", finger);
    uint16_t softPressSum = 0;
    for (int i = 0; i < 3; i++)
    {
        printf("Press finger %d softly...\n", finger);

        uint16_t peak = 0;
        while (peak == 0) // Wait for a valid peak
        {
            uint16_t adcValue = ADC_Read_2(FINGER_PIN[finger]);
            peak = PiezoMovingPeakDetector(adcValue, finger, currentTime);
            HAL_Delay(10); // Small delay to avoid busy-waiting
        }

        softPressSum += peak;
        printf("Soft press %d: %d\n", i + 1, peak);
        HAL_Delay(100); // Small delay between presses
    }
    *blackKeyThreshold = softPressSum / 3; // Calculate average
    printf("---------------------------------------------------------------\n");

    // Measure hard presses (WhiteKey threshold)
    printf("Perform 3 hard presses for finger %d...\n", finger);
    uint16_t hardPressSum = 0;
    for (int i = 0; i < 3; i++)
    {
        printf("Press finger %d firmly...\n", finger);

        uint16_t peak = 0;
        while (peak == 0) // Wait for a valid peak
        {
            uint16_t adcValue = ADC_Read_2(FINGER_PIN[finger]);
            peak = PiezoMovingPeakDetector(adcValue, finger, currentTime);
            HAL_Delay(10); // Small delay to avoid busy-waiting
        }

        hardPressSum += peak;
        printf("Hard press %d: %d\n", i + 1, peak);
        HAL_Delay(100); // Small delay between presses
    }
    *whiteKeyThreshold = hardPressSum / 3; // Calculate average

    printf("Finger %d calibration complete: BlackKey = %d, WhiteKey = %d\n", finger, *blackKeyThreshold, *whiteKeyThreshold);
}

// Main initialization function
#ifdef PIEZO_CALIBRATE
void Piezo_Init()
{
    HAL_Delay(1000);

    // Calibrate thresholds for each finger
    MeasureKeyPressThresholds(B_Thumb, &Calibrate_B_Thumb_BlackKey, &Calibrate_B_Thumb_WhiteKey);
    MeasureKeyPressThresholds(Thumb, &Calibrate_Thumb_BlackKey, &Calibrate_Thumb_WhiteKey);
    MeasureKeyPressThresholds(Index, &Calibrate_Index_BlackKey, &Calibrate_Index_WhiteKey);
    MeasureKeyPressThresholds(Middle, &Calibrate_Middle_BlackKey, &Calibrate_Middle_WhiteKey);
    MeasureKeyPressThresholds(Ring, &Calibrate_Ring_BlackKey, &Calibrate_Ring_WhiteKey);
    MeasureKeyPressThresholds(Pinky, &Calibrate_Pinky_BlackKey, &Calibrate_Pinky_WhiteKey);
    MeasureKeyPressThresholds(A_Pinky, &Calibrate_A_Pinky_BlackKey, &Calibrate_A_Pinky_WhiteKey);

    printf("Calibration complete!\n");
    printf("BlackKey Thresholds: B_Thumb = %d, Thumb = %d, Index = %d, Middle = %d, Ring = %d, Pinky = %d, A_Pinky = %d\n",
           Calibrate_B_Thumb_BlackKey, Calibrate_Index_BlackKey, Calibrate_Middle_BlackKey, Calibrate_Ring_BlackKey, Calibrate_Pinky_BlackKey, Calibrate_A_Pinky_BlackKey);
    printf("WhiteKey Thresholds: B_Thumb = %d, Thumb = %d, Index = %d, Middle = %d, Ring = %d, Pinky = %d, A_Pinky = %d\n",
           Calibrate_B_Thumb_WhiteKey, Calibrate_Thumb_WhiteKey, Calibrate_Index_WhiteKey, Calibrate_Middle_WhiteKey, Calibrate_Ring_WhiteKey, Calibrate_Pinky_WhiteKey, Calibrate_A_Pinky_WhiteKey);
    printf("---------------------------------------------------------------\n");
    printf("---------------------------------------------------------------\n");
}
#endif // PIEZO_CALIBRATE

uint16_t PiezoMovingPeakDetector(uint16_t Piezo_Read, Finger_t finger, uint32_t currentTime)
{
    // Use arrays to track state for each finger
    static uint16_t lastAdcValue[7] = {0};
    static uint16_t maxPeak[7] = {0};
    static uint8_t isRising[7] = {0};
    // static bool isSoundPlaying = false;

    // Noise filter
    if (Piezo_Read < NOISE_THRESHOLD)
    {
        return 0;
    }

    // Check if the signal is rising or falling
    if (Piezo_Read > lastAdcValue[finger])
    {
        // Signal is rising, update the peak
        maxPeak[finger] = Piezo_Read;
        isRising[finger] = 1;
    }
    // Signal is falling
    else if (isRising[finger] && Piezo_Read < lastAdcValue[finger])
    {
        // Signal started dropping, finalize the peak
        uint16_t finalPeak = maxPeak[finger];
        maxPeak[finger] = 0;
        isRising[finger] = 0;
        lastAdcValue[finger] = Piezo_Read;

        // Update last press time for this finger
        lastPressTime[finger] = currentTime;

        // Process the peak right away
        KeyType_t keyType = WhiteOrBlackKey(finalPeak, finger);
        if ((keyType == 0) || (keyType == 1))
        {
            const char *note = GetNoteString(NOTE_MAP[finger][keyType]);
            if (PRINT_FLAG)
            {
                printf("%s Key Pressed on %d finger (Peak: %d, Note: %s)\n",
                       keyType == WHITE_KEY ? "White" : "Black",
                       finger, finalPeak, note);

                char textBuf[20];
                char NOTE = *note;
                sprintf(textBuf, "Playing note %c", NOTE);
                DFRobot_RGBLCD_SetCursor(&myLCD, 0, 10);
                DFRobot_RGBLCD_Print(&myLCD, textBuf);
            }
            // #ifndef SOUND
            // Get the frequency of the played note
            Note_t noteEnum = NOTE_MAP[finger][keyType];
            // uint8_t currentOctave = 4; // Assuming octave 4 for simplicity
            // int octave = updateOctave(BNO055_ADDRESS_A);
            // printf("Octave: %d\n", currentOctave);
            frequency = GetNoteFrequency(noteEnum, currentOctave, keyType);

            /*
                STARTSOUNDTIME = TIMERS_GetMilliSeconds();
                if ((TIMERS_GetMilliSeconds() - STARTSOUNDTIME) < SOUND_DURATION)
                {
                    startVoice(0, frequency, 0.25f); // Start the voice with the calculated frequency
                    printf("frequency: %f\n", frequency);
                }
            */
            for (int i = 0; i < MAX_VOICES; i++)
            {
                if (!isVoiceActive[i])
                {
                    startVoice(i, frequency, 0.1f); // Start the voice with the calculated frequency
                    STARTSOUNDTIME[i] = TIMERS_GetMilliSeconds();
                    voiceFrequency[i] = frequency;
                    isVoiceActive[i] = 1; // Mark this voice as active
                    // printf("Voice %d started: frequency = %f\n", i, frequency);
                    break;
                }
            }
        }
        StopTheVoicesInYourHead();
        return finalPeak;
    }
    /*
        if ((TIMERS_GetMilliSeconds() - STARTSOUNDTIME) > SOUND_DURATION)
        {
            stopVoice(0); // Stop the voice if the duration is exceeded
            // printf("STOPPING NOTE\n");
        }
     */
    lastAdcValue[finger] = Piezo_Read;
    return 0;
}

// Periodically check if any voice has exceeded its duration and stop it
void StopTheVoicesInYourHead()
{
    uint32_t currentTime = TIMERS_GetMilliSeconds();

    for (int i = 0; i < MAX_VOICES; i++)
    {
        if (isVoiceActive[i] && (currentTime - STARTSOUNDTIME[i] > SOUND_DURATION))
        {
            stopVoice(i); // Stop the voice
            isVoiceActive[i] = 0;
            // printf("Voice %d stopped\n", i);
        }
    }
}

int WhiteOrBlackKey(int PiezoPeak, Finger_t finger)
{
    // Define thresholds for each finger
    uint16_t blackKeyThreshold = 0;
    uint16_t whiteKeyThreshold = 0;

    // Set thresholds based on the finger
    switch (finger)
    {
    case B_Thumb:
        blackKeyThreshold = Calibrate_B_Thumb_BlackKey;
        whiteKeyThreshold = Calibrate_B_Thumb_WhiteKey;
        break;
    case Thumb:
        blackKeyThreshold = Calibrate_Thumb_BlackKey;
        whiteKeyThreshold = Calibrate_Thumb_WhiteKey;
        break;
    case Index:
        blackKeyThreshold = Calibrate_Index_BlackKey;
        whiteKeyThreshold = Calibrate_Index_WhiteKey;
        break;
    case Middle:
        blackKeyThreshold = Calibrate_Middle_BlackKey;
        whiteKeyThreshold = Calibrate_Middle_WhiteKey;
        break;
    case Ring:
        blackKeyThreshold = Calibrate_Ring_BlackKey;
        whiteKeyThreshold = Calibrate_Ring_WhiteKey;
        break;
    case Pinky:
        blackKeyThreshold = Calibrate_Pinky_BlackKey;
        whiteKeyThreshold = Calibrate_Pinky_WhiteKey;
        break;
    case A_Pinky:
        blackKeyThreshold = Calibrate_A_Pinky_BlackKey;
        whiteKeyThreshold = Calibrate_A_Pinky_WhiteKey;
        break;
    default:
        // Invalid finger, return INVALID_KEY
        validPeakDetected = 0;
        return INVALID_KEY;
    }

    // Check if the peak corresponds to a white or black key
    if (PiezoPeak > whiteKeyThreshold)
    {
        validPeakDetected = 1;
        return WHITE_KEY;
    }
    else if ((PiezoPeak > blackKeyThreshold) && (PiezoPeak < whiteKeyThreshold))
    {
        validPeakDetected = 1;
        return BLACK_KEY;
    }
    else
    {
        // Piezo reading is too small or invalid
        validPeakDetected = 0;
        return INVALID_KEY;
    }
}

void OFF_LEDS()
{
    GPIO_WritePin_2(LED_B_THUMB_PIN, LOW);
    GPIO_WritePin_2(LED_THUMB_PIN, LOW);
    GPIO_WritePin_2(LED_INDEX_PIN, LOW);
    GPIO_WritePin_2(LED_MIDDLE_PIN, LOW);
    GPIO_WritePin_2(LED_RING_PIN, LOW);
    GPIO_WritePin_2(LED_PINKY_PIN, LOW);
    GPIO_WritePin_2(LED_A_PINKY_PIN, LOW);
}

void SET_LEDS(const char *note)
{
    // Determine which finger corresponds to the note
    Finger_t finger = Unknown;

    // Search the NOTE_MAP table for the note
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            const char *mappedNote = GetNoteString(NOTE_MAP[i][j]);
            if (strcmp(mappedNote, note) == 0)
            {
                finger = (Finger_t)(i + 1); // Fingers are 1-7
                break;
            }
        }
        if (finger != Unknown)
        {
            break;
        }
    }

    // Turn off all LEDs first
    GPIO_WritePin_2(LED_B_THUMB_PIN, LOW);
    GPIO_WritePin_2(LED_THUMB_PIN, LOW);
    GPIO_WritePin_2(LED_INDEX_PIN, LOW);
    GPIO_WritePin_2(LED_MIDDLE_PIN, LOW);
    GPIO_WritePin_2(LED_RING_PIN, LOW);
    GPIO_WritePin_2(LED_PINKY_PIN, LOW);
    GPIO_WritePin_2(LED_A_PINKY_PIN, LOW);

    // Set the LED for the corresponding finger
    if (finger != Unknown)
    {
        switch (finger)
        {
        case B_Thumb:
            OFF_LEDS();
            GPIO_WritePin_2(LED_B_THUMB_PIN, HIGH);
            break;
        case Thumb:
            OFF_LEDS();
            GPIO_WritePin_2(LED_THUMB_PIN, HIGH);
            break;
        case Index:
            OFF_LEDS();
            GPIO_WritePin_2(LED_INDEX_PIN, HIGH);
            break;
        case Middle:
            OFF_LEDS();
            GPIO_WritePin_2(LED_MIDDLE_PIN, HIGH);
            break;
        case Ring:
            OFF_LEDS();
            GPIO_WritePin_2(LED_RING_PIN, HIGH);
            break;
        case Pinky:
            OFF_LEDS();
            GPIO_WritePin_2(LED_PINKY_PIN, HIGH);
            break;
        case A_Pinky:
            OFF_LEDS();
            GPIO_WritePin_2(LED_A_PINKY_PIN, HIGH);
            break;
        default:
            // Invalid finger, do nothing
            break;
        }
    }
}

int TwinkleTwinkle()
{
    // HAL_Delay(500);
    HAL_Delay(100);
    printf("Welcome to the song! Please press any note\n");
    const int Song_Length = sizeof(TWINKLETWINKLE) / sizeof(TWINKLETWINKLE[0]);
    // const int Song_Length = sizeof(SONG_TEST2) / sizeof(SONG_TEST2[0]);

    int current_note_index = 0;
    GPIO_WritePin_2(LED_B_THUMB_PIN, HIGH);
    GPIO_WritePin_2(LED_THUMB_PIN, HIGH);
    GPIO_WritePin_2(LED_INDEX_PIN, HIGH);
    GPIO_WritePin_2(LED_MIDDLE_PIN, HIGH);
    GPIO_WritePin_2(LED_RING_PIN, HIGH);
    GPIO_WritePin_2(LED_PINKY_PIN, HIGH);
    GPIO_WritePin_2(LED_A_PINKY_PIN, HIGH);

    while (current_note_index < Song_Length)
    {
        const char *expected_note = TWINKLETWINKLE[current_note_index];
        // const char *expected_note = SONG_TEST2[current_note_index];

        printf("Play the next note: %s\n", expected_note);
        char textBuf[20];
        char NOTE = *expected_note;
        sprintf(textBuf, "Play note: %c", NOTE);
        DFRobot_RGBLCD_SetCursor(&myLCD, 0, 10);
        DFRobot_RGBLCD_Print(&myLCD, textBuf);

        SET_LEDS(expected_note); // Set the LED for the current note

        // Wait for the correct note to be played
        int correct_note_played = 0;
        int wrong_note_debounce_flag = 0;

        while (!correct_note_played)
        {
            uint32_t currentTime = TIMERS_GetMilliSeconds();
            // Read all fingers
            uint16_t Piezo_Read_Index[7] =
                {
                    ADC_Read_2(B_THUMB_PIN),
                    ADC_Read_2(THUMB_PIN),
                    ADC_Read_2(INDEX_PIN),
                    ADC_Read_2(MIDDLE_PIN),
                    ADC_Read_2(RING_PIN),
                    ADC_Read_2(PINKY_PIN),
                    ADC_Read_2(A_PINKY_PIN)};

            // Process all fingers
            for (int i = 0; i < 7; i++)
            {
                validPeakDetected = 0;
                uint16_t peak = PiezoMovingPeakDetector(Piezo_Read_Index[i], (Finger_t)i, currentTime);
                // if ((peak > NOISE_THRESHOLD) && (validPeakDetected == 1))
                if (validPeakDetected == 1)
                {
                    // Check if the played note matches the expected note
                    KeyType_t keyType = WhiteOrBlackKey(peak, (Finger_t)i);
                    if ((keyType == 0) || (keyType == 1))
                    {
                        const char *played_note = GetNoteString(NOTE_MAP[i][keyType]);
                        if (strcmp(played_note, expected_note) == 0)
                        {
                            correct_note_played = 1;
                            wrong_note_debounce_flag = 0;
                        }
                        else
                        {
                            if (!wrong_note_debounce_flag)
                            {
                                printf("Wrong note! You played: %s, expected: %s\n", played_note, expected_note);
                                wrong_note_debounce_flag = 1;
                            }
                        }
                        // Reset valid peak flag
                        validPeakDetected = 0;
                    }
                }
            }
            // StopTheVoicesInYourHead();
        }
        // StopTheVoicesInYourHead();
        current_note_index++;
    }
    printf("Congratulations! You played the entire song!\n");

    char textBuf[25];
    DFRobot_RGBLCD_SetCursor(&myLCD, 0, 0);
    sprintf(textBuf, "Congratulations!");
    DFRobot_RGBLCD_Print(&myLCD, textBuf);

    DFRobot_RGBLCD_SetCursor(&myLCD, 0, 10);
    sprintf(textBuf, "You finished the song!");
    DFRobot_RGBLCD_Print(&myLCD, textBuf);

    return 0;
}

#ifdef EXCLUDE_MAIN
int main(void)
{
    BOARD_Init();
    ADC_Init_2();
    TIMER_Init();
    GPIO_Init_2();

#ifdef PIEZO
#ifdef PIEZO_CALIBRATE
    Piezo_Init();
#endif // PIEZO_CALIBRATE

    while (1)
    {

#ifdef PIEZO_FREEPLAY

        uint32_t currentTime = TIMERS_GetMilliSeconds();
        // PiezoMovingPeakDetector(Piezo_Read_Index);

        // Process all fingers
        for (int i = 1; i < 8; i++)
        {
            uint16_t peak = PiezoMovingPeakDetector(FINGER_PIN[i], (Finger_t)i, currentTime);
            // printf("Finger %d Peak: %d\n", i, peak);
        }
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

        HAL_Delay(20);
        // i++;
    }
#endif // PIEZO
}
#endif // EXCLUDE_MAIN
