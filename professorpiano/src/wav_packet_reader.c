#include <wav_packet_reader.h>

int parseWav(const uint8_t *buf, uint32_t bufSize, WavInfo *info)
{
    // Minimal checks for "RIFF", "WAVE"
    // RIFF header:  0..3="RIFF", 4..7 = fileSize-8, 8..11="WAVE"
    if (bufSize < 44) return -1; // too small to be valid .wav
    if (memcmp(buf, "RIFF", 4) != 0) return -2;
    if (memcmp(buf+8, "WAVE", 4) != 0) return -3;

    // Now search for "fmt " and "data" chunks
    // Usually "fmt " is at 12..35, "data" is right after, but not guaranteed.
    uint32_t offset = 12;
    int foundFmt = 0, foundData = 0;

    while (offset + 8 <= bufSize) {
        // Chunk header: 4 bytes for chunk ID, 4 bytes for chunk size
        char chunkId[5];
        memcpy(chunkId, buf + offset, 4);
        chunkId[4] = '\0'; // null-terminate for easy printing

        uint32_t chunkSize = 
            (uint32_t)buf[offset + 4] |
            ((uint32_t)buf[offset + 5] << 8) |
            ((uint32_t)buf[offset + 6] << 16) |
            ((uint32_t)buf[offset + 7] << 24);

        offset += 8; // move past header

        if (strcmp(chunkId, "fmt ") == 0) {
            // parse the wave format
            if (chunkSize < 16) return -4; // PCM fmt chunk should be at least 16
            if (offset + chunkSize > bufSize) return -5; // out of range

            info->audioFormat    = (uint16_t) (buf[offset + 0] | (buf[offset + 1] << 8));
            info->numChannels    = (uint16_t) (buf[offset + 2] | (buf[offset + 3] << 8));
            info->sampleRate     = (uint32_t) (buf[offset + 4] |
                                               (buf[offset + 5] << 8) |
                                               (buf[offset + 6] << 16) |
                                               (buf[offset + 7] << 24));
            // skip byteRate (4 bytes) at offset+8
            // skip blockAlign (2 bytes) at offset+12
            info->bitsPerSample  = (uint16_t) (buf[offset + 14] | (buf[offset + 15] << 8));

            foundFmt = 1;
        }
        else if (strcmp(chunkId, "data") == 0) {
            // "data" chunk has raw PCM
            if (!foundFmt) {
                // we haven’t encountered fmt chunk yet, suspicious
                return -6;
            }
            if (offset + chunkSize > bufSize) return -7;

            info->dataPtr  = buf + offset;
            info->dataSize = chunkSize;
            foundData = 1;
            // we can stop scanning once "data" is found
            break;
        }

        // move to next chunk
        offset += chunkSize;
        // chunkSize is often aligned to 2 bytes. If chunkSize is odd, it might 
        // have a padding byte. Real .wav can have more subchunks after "data" 
        // but typically this is enough for standard PCM.
    }

    if (!foundFmt || !foundData) return -8;
    if (info->audioFormat != 1) {
        // not PCM
        return -9;
    }
    return 0; // success
}
