#include "DFRobot_LCD.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#include "I2C_2.h"

/*  A small 2D array of color definitions, as in the original code. */
static const uint8_t color_define[4][3] = {
    {255, 255, 255}, /* white */
    {255, 0,   0  }, /* red */
    {0,   255, 0  }, /* green */
    {0,   0,   255}  /* blue */
};

/*!
 * @brief Low-level helper: send multiple bytes in ONE i2c transaction
 *        (replicates Wire.beginTransmission() / write() / endTransmission()).
 */
static void i2cSendBytes(uint8_t i2cAddr, const uint8_t *buf, size_t len)
{
    /*  i2cAddr is the 7-bit address; HAL expects 8-bit in hi2cX calls -> shift left 1. */
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(&hi2c2, (i2cAddr << 1), (uint8_t*)buf, len, 100);
    if(ret != HAL_OK)
    {
        //error
    }
}

/*!
 * @brief Send command vs. data to the LCD. The DFRobot “backpack” expects
 *        first byte = control (0x80 for cmd, 0x40 for data), second byte = payload
 */
static void lcdCommand(DFRobot_RGBLCD_t *p, uint8_t cmd)
{
    uint8_t data[2];
    data[0] = 0x80;   /* indicate “command” */
    data[1] = cmd;
    i2cSendBytes(p->lcdAddr, data, 2);
}

static void lcdWrite(DFRobot_RGBLCD_t *p, uint8_t value)
{
    uint8_t data[2];
    data[0] = 0x40;   /* indicate “data” */
    data[1] = value;
    i2cSendBytes(p->lcdAddr, data, 2);
}

/*!
 * @brief Write to the RGB device’s registers
 */
static void setReg(DFRobot_RGBLCD_t *p, uint8_t addr, uint8_t data)
{
    /*  The DFRobot board’s RGB chip is memory-mapped, so we can do:
     *    I2C_WriteReg_2() or HAL_I2C_Mem_Write() with the “register address” = addr
     */
    I2C_WriteReg_2(p->rgbAddr, addr, data);
    /*  or do the full HAL call directly, e.g.:
     *  HAL_I2C_Mem_Write(&hi2c2, (p->rgbAddr << 1),
     *                    addr, I2C_MEMADD_SIZE_8BIT,
     *                    &data, 1, 100);
     */
}

/*!
 * @brief Internal “begin” used by the init function
 */
static void lcdBegin(DFRobot_RGBLCD_t *p, uint8_t cols, uint8_t lines, uint8_t dotsize)
{
    if(lines > 1) {
        p->displayFunction |= LCD_2LINE;
    }
    p->numlines = lines;
    p->currline = 0;

    if((dotsize != 0) && (lines == 1)) {
        p->displayFunction |= LCD_5x10DOTS;
    }

    HAL_Delay(50); /* Wait for >40ms after power rises above 2.7V */

    /*  Send function set command 3x per HD44780 datasheet. */
    lcdCommand(p, LCD_FUNCTIONSET | p->displayFunction);
    HAL_Delay(5);

    lcdCommand(p, LCD_FUNCTIONSET | p->displayFunction);
    HAL_Delay(5);

    lcdCommand(p, LCD_FUNCTIONSET | p->displayFunction);

    /*  Turn the display on with no cursor or blinking default */
    p->displayControl = (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    DFRobot_RGBLCD_DisplayOn(p);

    /*  Clear it off */
    DFRobot_RGBLCD_Clear(p);

    /*  Initialize to default text direction (for romance languages) */
    p->displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    lcdCommand(p, LCD_ENTRYMODESET | p->displayMode);

    /*  Now setup the RGB backlight defaults. */
    setReg(p, REG_MODE1, 0x00);
    /*  set LEDs controllable by both PWM and GRPPWM registers */
    setReg(p, REG_OUTPUT, 0xFF);
    /*  set MODE2: 0x20 means DMBLNK=1 => “blinky mode” (per DFRobot docs) */
    setReg(p, REG_MODE2, 0x20);

    /*  Default color = white */
    DFRobot_RGBLCD_SetRGB(p, 255, 255, 255);
}

void DFRobot_RGBLCD_Init(DFRobot_RGBLCD_t *p, uint8_t lcdCols, uint8_t lcdRows,
                         uint8_t lcdAddr, uint8_t rgbAddr)
{
    /*  User might need to initialize I2C if not done already: */
    I2C_Init_2();
    I2C_ScanBus_LCD_ADDRESS();
    I2C_ScanBus_RGB_ADDRESS();

    p->lcdAddr = lcdAddr;
    p->rgbAddr = rgbAddr;
    p->cols = lcdCols;
    p->rows = lcdRows;

    p->displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    p->displayControl  = 0;
    p->displayMode     = 0;
    p->currline        = 0;
    p->numlines        = lcdRows;

    /*  Kick off the HD44780 init sequence. */
    lcdBegin(p, lcdCols, lcdRows, 0);
}

/* --------------------------------------------------------------------------
 * BASIC LCD COMMANDS
 * --------------------------------------------------------------------------*/
void DFRobot_RGBLCD_Clear(DFRobot_RGBLCD_t *p)
{
    lcdCommand(p, LCD_CLEARDISPLAY);
    /*  The data sheet says clearing can take >1ms, so wait at least 2ms. */
    HAL_Delay(2);
}

void DFRobot_RGBLCD_Home(DFRobot_RGBLCD_t *p)
{
    lcdCommand(p, LCD_RETURNHOME);
    /*  This also takes >1ms. */
    HAL_Delay(2);
}

void DFRobot_RGBLCD_DisplayOn(DFRobot_RGBLCD_t *p)
{
    p->displayControl |= LCD_DISPLAYON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_DisplayOff(DFRobot_RGBLCD_t *p)
{
    p->displayControl &= ~LCD_DISPLAYON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_BlinkOn(DFRobot_RGBLCD_t *p)
{
    p->displayControl |= LCD_BLINKON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_BlinkOff(DFRobot_RGBLCD_t *p)
{
    p->displayControl &= ~LCD_BLINKON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_CursorOn(DFRobot_RGBLCD_t *p)
{
    p->displayControl |= LCD_CURSORON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_CursorOff(DFRobot_RGBLCD_t *p)
{
    p->displayControl &= ~LCD_CURSORON;
    lcdCommand(p, LCD_DISPLAYCONTROL | p->displayControl);
}

void DFRobot_RGBLCD_ScrollLeft(DFRobot_RGBLCD_t *p)
{
    lcdCommand(p, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void DFRobot_RGBLCD_ScrollRight(DFRobot_RGBLCD_t *p)
{
    lcdCommand(p, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void DFRobot_RGBLCD_LeftToRight(DFRobot_RGBLCD_t *p)
{
    p->displayMode |= LCD_ENTRYLEFT;
    lcdCommand(p, LCD_ENTRYMODESET | p->displayMode);
}

void DFRobot_RGBLCD_RightToLeft(DFRobot_RGBLCD_t *p)
{
    p->displayMode &= ~LCD_ENTRYLEFT;
    lcdCommand(p, LCD_ENTRYMODESET | p->displayMode);
}

void DFRobot_RGBLCD_AutoScrollOn(DFRobot_RGBLCD_t *p)
{
    p->displayMode |= LCD_ENTRYSHIFTINCREMENT;
    lcdCommand(p, LCD_ENTRYMODESET | p->displayMode);
}

void DFRobot_RGBLCD_AutoScrollOff(DFRobot_RGBLCD_t *p)
{
    p->displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
    lcdCommand(p, LCD_ENTRYMODESET | p->displayMode);
}

void DFRobot_RGBLCD_SetCursor(DFRobot_RGBLCD_t *p, uint8_t col, uint8_t row)
{
    /*  On a typical 16x2: row=0 => 0x80 + col, row=1 => 0xC0 + col. */
    /*  Adjust for your own LCD if it has more lines or custom mapping. */
    if(row > 1) row = 1; /* or handle bigger? */

    uint8_t val = (row == 0) ? (0x80 | col) : (0xC0 | col);
    uint8_t data[2];
    data[0] = 0x80;  /* command byte */
    data[1] = val;
    i2cSendBytes(p->lcdAddr, data, 2);
}

void DFRobot_RGBLCD_WriteChar(DFRobot_RGBLCD_t *p, uint8_t ch)
{
    lcdWrite(p, ch);
}

void DFRobot_RGBLCD_Print(DFRobot_RGBLCD_t *p, const char *str)
{
    while(*str)
    {
        lcdWrite(p, (uint8_t)(*str));
        str++;
    }
}

void DFRobot_RGBLCD_CreateSymbol(DFRobot_RGBLCD_t *p, uint8_t location, const uint8_t charmap[])
{
    location &= 0x7; /* Only 8 locations 0..7 */
    lcdCommand(p, LCD_SETCGRAMADDR | (location << 3));

    /*  According to the original code, it writes 8 bytes to CGRAM. 
     *  Each of those writes must have the 0x40 “data prefix”. 
     */
    for(int i = 0; i < 8; i++)
    {
        lcdWrite(p, charmap[i]);
    }
}

void DFRobot_RGBLCD_SetRGB(DFRobot_RGBLCD_t *p, uint8_t r, uint8_t g, uint8_t b)
{
    setReg(p, REG_RED,   r);
    setReg(p, REG_GREEN, g);
    setReg(p, REG_BLUE,  b);
}

void DFRobot_RGBLCD_SetColor(DFRobot_RGBLCD_t *p, uint8_t color)
{
    if(color > 3) return;
    DFRobot_RGBLCD_SetRGB(p,
        color_define[color][0],
        color_define[color][1],
        color_define[color][2]
    );
}

void DFRobot_RGBLCD_BlinkLED(DFRobot_RGBLCD_t *p)
{
    /*  Blink period = (reg7 + 1)/24. On/off ratio = reg6/256, from DFRobot doc. */
    setReg(p, 0x07, 0x17); /* blink every ~1 second */
    setReg(p, 0x06, 0x7F); /* half on, half off */
}

void DFRobot_RGBLCD_NoBlinkLED(DFRobot_RGBLCD_t *p)
{
    setReg(p, 0x07, 0x00);
    setReg(p, 0x06, 0xFF);
}
