#include "I2S.h"

int16_t sineTable[WAVE_TABLE_SIZE];
int16_t i2sTxBuffer[AUDIO_BUFFER_SIZE];
Voice_t voices[NUM_VOICES];

I2S_HandleTypeDef hi2s1;  // I2S handle

// I2S Initialization
void I2S_Init(void) {
    // Enable the I2S clock
    __HAL_RCC_SPI2_CLK_ENABLE();

    // Configure I2S
    hi2s1.Instance = SPI2;
    hi2s1.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE; // Enable MCLK (optional)
    hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_48K;
    hi2s1.Init.CPOL = I2S_CPOL_LOW;
    hi2s1.Init.ClockSource = I2S_CLOCK_PLL;
    hi2s1.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

        // Typically in your main.c or Board_Init or similar:
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);


    memset(voices, 0, sizeof(voices));
    memset(i2sTxBuffer, 0, sizeof(i2sTxBuffer));

    if (HAL_I2S_Init(&hi2s1) != HAL_OK) {
        Error_Handler_3();
    }
}
// I2S MSP Initialization (GPIO and clock setup)
DMA_HandleTypeDef hdma_spi2_tx;

void HAL_I2S_MspInit(I2S_HandleTypeDef* hi2s) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hi2s->Instance == SPI2) {
        // Enable GPIO clocks
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();  // For MCLK (optional)
        __HAL_RCC_DMA1_CLK_ENABLE();    // Enable DMA clock

        // Configure I2S pins: PB12 (WS), PB13 (SCK), PB15 (SD)
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Configure DMA for SPI2_TX
        hdma_spi2_tx.Instance = DMA1_Stream4;  // Adjust stream as necessary
        hdma_spi2_tx.Init.Channel = DMA_CHANNEL_0;  // Adjust channel as necessary
        hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;  // Circular mode for continuous transmission
        // hdma_spi2_tx.Init.Mode = DMA_NORMAL;  // Normal mode for 1 transmission
        hdma_spi2_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;


        
        if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
        {
            Error_Handler_3();
        }
        __HAL_LINKDMA(hi2s, hdmatx, hdma_spi2_tx);
    }
}

void InitSineTable(void)
{
    // Fill sineTable[] with one full cycle of sine, scaled to 16-bit
    for (int i = 0; i < WAVE_TABLE_SIZE; i++)
    {
        float theta = (2.0f * (float)M_PI * i) / (float)WAVE_TABLE_SIZE;
        float val   = sinf(theta);
        // Scale to int16 range
        sineTable[i] = (int16_t)(val * 32767.0f);
        // printf("sample[%d] = %d\n", i, sineTable[i]);
    }
}


/**
 * @brief Start or "note on" a voice at the given frequency and amplitude.
 * @param voiceIndex Index of the voice [0..NUM_VOICES-1].
 * @param freq Desired frequency in Hz.
 * @param amplitude Volume scale [0..1.0].
 */
void startVoice(int voiceIndex, float freq, float amplitude)
{
    if (voiceIndex < 0 || voiceIndex >= NUM_VOICES) return;
    voices[voiceIndex].phase          = 0.0f;
    voices[voiceIndex].phaseIncrement = (freq * (float)WAVE_TABLE_SIZE) / (float)SAMPLE_RATE;
    voices[voiceIndex].amplitude      = amplitude;  // e.g. 0.2 for 20%
    voices[voiceIndex].active         = true;
}

/**
 * @brief Stop or "note off" a voice.
 */
void stopVoice(int voiceIndex)
{
    if (voiceIndex < 0 || voiceIndex >= NUM_VOICES) return;
    voices[voiceIndex].active = false;
}
/**
 * @brief Fill half of the I2S buffer with newly mixed audio
 * 
 * @param pBuffer Pointer to start of the half buffer we're filling
 * @param numSamples Number of 16-bit samples to fill (stereo means each frame has 2 samples)
 */
void fillAudioBuffer(int16_t *pBuffer, int numSamples)
{
    for (int i = 0; i < numSamples; i += 2)
    {
        int32_t mix = 0;

        for (int v = 0; v < NUM_VOICES; v++)  //Loop through all active voices
        {
            if (voices[v].active)
            {
                int idx = (int)voices[v].phase;


                // Grab the sample
                int16_t sample = sineTable[idx];
                float scaled    = sample * voices[v].amplitude;  //scale to avoid clipping when mixing signals
                mix += (int32_t)scaled;

                // Advance phase
                voices[v].phase += voices[v].phaseIncrement;
                if (voices[v].phase >= (float)WAVE_TABLE_SIZE)
                {
                    voices[v].phase -= (float)WAVE_TABLE_SIZE;
                }
            }
        }

        // Clipping
        if (mix > 32767)  mix = 32767;
        else if (mix < -32768) mix = -32768;

        // Final sample
        int16_t outSample = (int16_t)mix;
        pBuffer[i]   = outSample;
        pBuffer[i+1] = outSample;
    }
}

void fillAudioBuffer_square(int16_t *pBuffer, int numSamples)  //TEST_FUNCTION
{
    for (int i = 0; i < numSamples; i += 2)
    {
        int16_t val = ((i % 256) < 128) ? 30000 : -30000; 
        pBuffer[i]   = val; // L
        pBuffer[i+1] = val; // R
    }
}


// Called when first half of i2sTxBuffer is done transmitting
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        // Fill the FIRST half
        fillAudioBuffer(&i2sTxBuffer[0], AUDIO_BUFFER_SIZE / 2);
    }
}

// Called when second half is done transmitting
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        // Fill the SECOND half
        fillAudioBuffer(&i2sTxBuffer[AUDIO_BUFFER_SIZE / 2], AUDIO_BUFFER_SIZE / 2);
    }
}










void Error_Handler_3(void)
{
    __disable_irq();
    static int halfBlinkPeriod = 500000;
    static int timer = 0;
    while (1)
    {
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        while (timer < halfBlinkPeriod)
        {
            timer++;
        }
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        timer = 0;
        while (timer < halfBlinkPeriod)
        {
            timer++;
        }
        timer = 0;
    }
}


void DMA1_Stream4_IRQHandler(void)
{
    // This calls into the HAL so it can detect half-complete / complete
    HAL_DMA_IRQHandler(&hdma_spi2_tx);
}