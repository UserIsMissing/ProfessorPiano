#ifndef NOTE_FREQUENCIES_H
#define NOTE_FREQUENCIES_H

#include <Piezo_File.h>

// Define the number of octaves and notes
#define NUM_OCTAVES 8  // From Octave 0 to Octave 7
#define NUM_NOTES 12   // 12 notes per octave (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)

// Frequency lookup table for each note and octave
static const float NOTE_FREQUENCIES[NUM_OCTAVES][NUM_NOTES] = {
    // Octave 0
    {16.35f, 17.32f, 18.35f, 19.45f, 20.60f, 21.83f, 23.12f, 24.50f, 25.96f, 27.50f, 29.14f, 30.87f},
    // Octave 1
    {32.70f, 34.65f, 36.71f, 38.89f, 41.20f, 43.65f, 46.25f, 49.00f, 51.91f, 55.00f, 58.27f, 61.74f},
    // Octave 2
    {65.41f, 69.30f, 73.42f, 77.78f, 82.41f, 87.31f, 92.50f, 98.00f, 103.83f, 110.00f, 116.54f, 123.47f},
    // Octave 3
    {130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 185.00f, 196.00f, 207.65f, 220.00f, 233.08f, 246.94f},
    // Octave 4
    {261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f},
    // Octave 5
    {523.25f, 554.37f, 587.33f, 622.25f, 659.25f, 698.46f, 739.99f, 783.99f, 830.61f, 880.00f, 932.33f, 987.77f},
    // Octave 6
    {1046.50f, 1108.73f, 1174.66f, 1244.51f, 1318.51f, 1396.91f, 1479.98f, 1567.98f, 1661.22f, 1760.00f, 1864.66f, 1975.53f},
    // Octave 7
    {2093.00f, 2217.46f, 2349.32f, 2489.02f, 2637.02f, 2793.83f, 2959.96f, 3135.96f, 3322.44f, 3520.00f, 3729.31f, 3951.07f}
};

// Function to get the frequency of a note at a given octave
static float GetNoteFrequency(Note_t note, uint8_t octave) {
    if (octave >= NUM_OCTAVES || note < Note_C || note > Note_B) {
        return 0.0f; // Invalid octave or note
    }
    return NOTE_FREQUENCIES[octave][note - 1]; // Note_C is index 0 in the table
};

#endif // NOTE_FREQUENCIES_H