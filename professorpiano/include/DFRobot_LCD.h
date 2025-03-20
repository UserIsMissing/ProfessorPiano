//Code template given by the DFRobot Github: https://github.com/DFRobot/DFRobot_RGBLCD

#ifndef DFROBOT_RGBLCD_H
#define DFROBOT_RGBLCD_H

#include <stdint.h>

/*  Define to match the DFRobot defaults for your display. */
#define LCD_ADDRESS     (0x7c>>1)
#define RGB_ADDRESS     (0x5A>>1)


/*  Colors. */
#define WHITE  0
#define RED    1
#define GREEN  2
#define BLUE   3

/*  DFRobot "registers" for the RGB driver. */
#define REG_RED         0x04  // pwm2
#define REG_GREEN       0x02  // pwm0
#define REG_BLUE        0x03  // pwm2

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

/*  HD44780 commands. */
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

/*  Flags for display entry mode. */
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*  Flags for display on/off control. */
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

/*  Flags for display/cursor shift. */
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

/*  Flags for function set. */
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief This structure keeps all the “state” that was in the C++ class.
 */
typedef struct
{
    uint8_t lcdAddr;       ///< I2C address for the LCD “commands/data”
    uint8_t rgbAddr;       ///< I2C address for the RGB backlight
    uint8_t cols;          ///< Number of columns on the LCD
    uint8_t rows;          ///< Number of rows on the LCD

    uint8_t displayFunction;  ///< (LCD_4BITMODE | LCD_2LINE | etc.)
    uint8_t displayControl;   ///< (LCD_DISPLAYON | LCD_CURSOROFF | etc.)
    uint8_t displayMode;      ///< (LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT)
    uint8_t currline;
    uint8_t numlines;
} DFRobot_RGBLCD_t;

DFRobot_RGBLCD_t myLCD;

/*!
 * @brief Initialize the DFRobot LCD struct & hardware
 * @param[in] p       Pointer to a DFRobot_RGBLCD_t
 * @param[in] lcdCols Number of columns
 * @param[in] lcdRows Number of rows
 * @param[in] lcdAddr I2C address (7-bit) for the LCD part
 * @param[in] rgbAddr I2C address (7-bit) for the RGB driver
 */
void DFRobot_RGBLCD_Init(DFRobot_RGBLCD_t *p,
                         uint8_t lcdCols,
                         uint8_t lcdRows,
                         uint8_t lcdAddr,
                         uint8_t rgbAddr);

/*!
 * @brief Clear and Home
 */
void DFRobot_RGBLCD_Clear(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_Home(DFRobot_RGBLCD_t *p);

/*!
 * @brief Display on/off
 */
void DFRobot_RGBLCD_DisplayOn(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_DisplayOff(DFRobot_RGBLCD_t *p);

/*!
 * @brief Cursor & blink
 */
void DFRobot_RGBLCD_BlinkOn(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_BlinkOff(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_CursorOn(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_CursorOff(DFRobot_RGBLCD_t *p);

/*!
 * @brief Scroll left/right
 */
void DFRobot_RGBLCD_ScrollLeft(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_ScrollRight(DFRobot_RGBLCD_t *p);

/*!
 * @brief Left-to-right or right-to-left text
 */
void DFRobot_RGBLCD_LeftToRight(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_RightToLeft(DFRobot_RGBLCD_t *p);

/*!
 * @brief Enable or disable auto scroll
 */
void DFRobot_RGBLCD_AutoScrollOn(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_AutoScrollOff(DFRobot_RGBLCD_t *p);

/*!
 * @brief Set the cursor
 */
void DFRobot_RGBLCD_SetCursor(DFRobot_RGBLCD_t *p, uint8_t col, uint8_t row);

/*!
 * @brief Print a single character
 */
void DFRobot_RGBLCD_WriteChar(DFRobot_RGBLCD_t *p, uint8_t ch);

/*!
 * @brief Print a C-string
 */
void DFRobot_RGBLCD_Print(DFRobot_RGBLCD_t *p, const char *str);

/*!
 * @brief Create a custom symbol in CGRAM
 */
void DFRobot_RGBLCD_CreateSymbol(DFRobot_RGBLCD_t *p, uint8_t location, const uint8_t charmap[]);

/*!
 * @brief Set the backlight color using direct RGB
 */
void DFRobot_RGBLCD_SetRGB(DFRobot_RGBLCD_t *p, uint8_t r, uint8_t g, uint8_t b);

/*!
 * @brief Set the backlight by color index
 */
void DFRobot_RGBLCD_SetColor(DFRobot_RGBLCD_t *p, uint8_t color);

/*!
 * @brief Blink the LED or turn it off
 */
void DFRobot_RGBLCD_BlinkLED(DFRobot_RGBLCD_t *p);
void DFRobot_RGBLCD_NoBlinkLED(DFRobot_RGBLCD_t *p);

#ifdef __cplusplus
}
#endif

#endif /* DFROBOT_RGBLCD_H */
