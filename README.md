# ProfessorPiano
Professor Piano is an interactive piano training glove designed to teach users how to play the piano through visual and motion-based guidance as well as a freestyle mode where users can explore octaves and chords on their own.

Must Change this file:
.platformio\packages\framework-stm32cubef4\Drivers\STM32F4xx_HAL_Driver\Inc\stm32f4xx_hal_conf.h

Line 95:

from this:
  #define HSE_VALUE              25000000U /*!< Value of the External oscillator in Hz */
  
to this:
  #define HSE_VALUE              8000000U /*!< Value of the External oscillator in Hz */
