/**
 * @file    wav_packet_reader.h
 * 
 * 
 * 
 *
 * @author  Ryan Taylor
 *
 * @date    1 Mar 2025
 * 
 **/

 #ifndef wav
 #define wav


#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint16_t audioFormat;     // e.g. 1 = PCM
    uint16_t numChannels;     // e.g. 2 = stereo
    uint32_t sampleRate;      // e.g. 8000, 44100, 48000
    uint16_t bitsPerSample;   // e.g. 16
    const uint8_t *dataPtr;   // pointer to start of PCM data
    uint32_t dataSize;        // number of bytes of PCM data
} WavInfo;


int parseWav(const uint8_t *buf, uint32_t bufSize, WavInfo *info);

#endif