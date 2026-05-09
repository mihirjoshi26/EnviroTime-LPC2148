/*
 * =====================================================================
 * FILE     : lcd.c
 * PURPOSE  : Contains all functions to control the 16x2 LCD display.
 *            The LCD is connected in 8-bit parallel mode.
 *
 * PIN CONNECTIONS (from pin_config.h):
 *   P0.0-P0.7 = Data bus D0-D7 (8-bit mode)
 *   P0.8 = RS  (0 = command, 1 = data/character)
 *   P0.9 = EN  (Enable pulse: HIGH then LOW to latch data)
 *   RW pin should be tied to Ground externally
 *
 * HOW LCD WORKS in 8-Bit Mode:
 *   1. Set RS = 0 for command OR RS = 1 for character
 *   2. Put full 8 bits of data on D0-D7 pins
 *   3. Pulse EN HIGH then LOW
 *   4. Wait for LCD to process
 * =====================================================================
 */

#include <lpc214x.h>     /* LPC2148 register definitions                */
#include "pin_config.h"  /* Our pin number definitions                  */
#include "delay.h"       /* We need delays for LCD timing               */
#include "lcd.h"         /* Our own header                              */

/* -------------------------------------------------------------------
 * LCD_SendCommand()
 * Sends a command byte to LCD (like clear screen, set cursor, etc.)
 * RS = 0 means we are sending a COMMAND (not a character)
 * -------------------------------------------------------------------*/
void LCD_SendCommand(unsigned char command)
{
    IOCLR0 = (1 << LCD_RS_BIT);      /* RS = 0 for command mode */

    /* Clear 8 data pins */
    IOCLR0 = (0xFF << LCD_DATA_START_BIT);
    
    /* Put 8-bit data on the data pins */
    IOSET0 = ((unsigned int)command << LCD_DATA_START_BIT);
    
    /* Pulse EN HIGH then LOW to latch data */
    IOSET0 = (1 << LCD_EN_BIT);
    delay_us(1);
    IOCLR0 = (1 << LCD_EN_BIT);

    delay_ms(2);                     /* Wait for LCD to process */
}

/* -------------------------------------------------------------------
 * LCD_SendChar()
 * Sends a character (letter, digit, symbol) to display on LCD.
 * RS = 1 means we are sending DATA (a character to display)
 * -------------------------------------------------------------------*/
void LCD_SendChar(unsigned char character)
{
    IOSET0 = (1 << LCD_RS_BIT);      /* RS = 1 for data mode */

    /* Clear 8 data pins */
    IOCLR0 = (0xFF << LCD_DATA_START_BIT);
    
    /* Put 8-bit data on the data pins */
    IOSET0 = ((unsigned int)character << LCD_DATA_START_BIT);
    
    /* Pulse EN HIGH then LOW to latch data */
    IOSET0 = (1 << LCD_EN_BIT);
    delay_us(1);
    IOCLR0 = (1 << LCD_EN_BIT);

    delay_ms(2);                     /* Wait for LCD to process */
}

/* -------------------------------------------------------------------
 * LCD_Init()
 * Initializes the LCD hardware.
 * MUST be called ONCE before using any other LCD function.
 *
 * Steps follow the HD44780 datasheet initialization sequence.
 * -------------------------------------------------------------------*/
void LCD_Init(void)
{
    /* Make LCD pins OUTPUT */
    /* Data pins 8 bits as output */
    IODIR0 |= (0xFF << LCD_DATA_START_BIT);
    /* RS, EN as output */
    IODIR0 |= (1 << LCD_RS_BIT);
    IODIR0 |= (1 << LCD_EN_BIT);

    /* Wait for LCD power to stabilize after turning on */
    delay_ms(20);

    /* Ensure RS is 0 for commands */
    IOCLR0 = (1 << LCD_RS_BIT);

    /* Send initialization commands for 8-bit mode */
    LCD_SendCommand(0x30);    /* 1st initialization attempt              */
    delay_ms(5);
    LCD_SendCommand(0x30);    /* 2nd initialization attempt              */
    delay_us(200);
    LCD_SendCommand(0x30);    /* 3rd initialization attempt - now stable */
    delay_ms(1);

    /* Now configure the LCD with 8-bit interface */
    LCD_SendCommand(LCD_8BIT_2LINE);  /* 8-bit, 2 lines, 5x8 dot font   */
    LCD_SendCommand(LCD_DISPLAY_ON);  /* Turn display ON, cursor OFF     */
    LCD_SendCommand(LCD_CLEAR);       /* Clear display                   */
    delay_ms(2);                      /* Wait after clear (takes time)   */
    LCD_SendCommand(LCD_ENTRY_MODE);  /* Cursor moves right after write  */
}

/* -------------------------------------------------------------------
 * LCD_Clear()
 * Clears all text on the LCD and moves cursor to top-left (0,0)
 * -------------------------------------------------------------------*/
void LCD_Clear(void)
{
    LCD_SendCommand(LCD_CLEAR);   /* Send clear command to LCD           */
    delay_ms(2);                  /* Wait for clear to complete          */
}

/* -------------------------------------------------------------------
 * LCD_SetCursor()
 * Moves the cursor to a specific row and column.
 * row = 0 -> Line 1, row = 1 -> Line 2
 * col = 0 to 15 (for 16-column LCD)
 * -------------------------------------------------------------------*/
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;    /* Will hold the DDRAM address to go to   */

    if (row == 0)
    {
        address = LCD_LINE1 + col;  /* Line 1 starts at address 0x80   */
    }
    else
    {
        address = LCD_LINE2 + col;  /* Line 2 starts at address 0xC0   */
    }

    LCD_SendCommand(address);  /* Send the position command to LCD      */
}

/* -------------------------------------------------------------------
 * LCD_Print()
 * Displays a text string on the LCD starting at current position.
 * The string must end with '\0' (null terminator) - C strings do this.
 *
 * Example: LCD_Print("Hello") displays H-e-l-l-o
 * -------------------------------------------------------------------*/
void LCD_Print(char *text)
{
    /* Keep sending characters until we hit the end of the string */
    while (*text != '\0')          /* '\0' means end of string           */
    {
        LCD_SendChar(*text);       /* Send current character             */
        text++;                    /* Move to next character             */
    }
}

/* -------------------------------------------------------------------
 * LCD_PrintTwoDigit()
 * Displays a number as exactly 2 digits with leading zero.
 * Example: 8 displays as "08", 23 displays as "23"
 * This is needed for time display like "08:05:03"
 * -------------------------------------------------------------------*/
void LCD_PrintTwoDigit(unsigned int number)
{
    unsigned char tensDigit;    /* The tens place digit (left digit)     */
    unsigned char unitsDigit;   /* The units place digit (right digit)   */

    tensDigit  = number / 10;   /* Get tens digit: 23/10 = 2            */
    unitsDigit = number % 10;   /* Get units digit: 23%10 = 3           */

    /* Convert digit to ASCII by adding 48 ('0' = ASCII 48) */
    LCD_SendChar(tensDigit  + 48);   /* Show tens digit                  */
    LCD_SendChar(unitsDigit + 48);   /* Show units digit                 */
}

/* -------------------------------------------------------------------
 * LCD_PrintYear()
 * Displays a 4-digit year like 2024 on LCD.
 * -------------------------------------------------------------------*/
void LCD_PrintYear(unsigned int year)
{
    /* Extract each digit from the 4-digit year */
    LCD_SendChar((year / 1000)       + 48);  /* Thousands digit: 2      */
    LCD_SendChar(((year / 100) % 10) + 48);  /* Hundreds digit: 0       */
    LCD_SendChar(((year / 10)  % 10) + 48);  /* Tens digit: 2           */
    LCD_SendChar((year % 10)         + 48);  /* Units digit: 4          */
}

/* -------------------------------------------------------------------
 * LCD_PrintTemp()
 * Displays temperature as a 2-digit integer.
 * Example: 28 shows as "28"
 * -------------------------------------------------------------------*/
void LCD_PrintTemp(unsigned int tempValue)
{
    /* Handle temperatures up to 99 degrees */
    LCD_SendChar((tempValue / 10) + 48);  /* Tens digit                 */
    LCD_SendChar((tempValue % 10) + 48);  /* Units digit                */
}

/* -------------------------------------------------------------------
 * LCD_ShowInvalidKey()
 * Displays "Please enter a valid key" message for 1 second.
 * This is a convenience function to avoid repeating the same
 * 6-line pattern throughout the codebase.
 * -------------------------------------------------------------------*/
void LCD_ShowInvalidKey(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Please enter a  ");
    LCD_SetCursor(1, 0);
    LCD_Print("valid key       ");
    delay_ms(1000);
}
