/*
 * =====================================================================
 * FILE     : lcd.h
 * PURPOSE  : DECLARES all LCD functions so other files can use them.
 *            Include this file wherever you need to display on LCD.
 *
 * HARDWARE: 16x2 LCD in 8-bit mode connected to Port 0 of LPC2148
 * CONNECTS TO: lcd.c (actual LCD code)
 * =====================================================================
 */

#ifndef LCD_H
#define LCD_H

/* Initialize LCD - call this ONCE at the start of your program */
void LCD_Init(void);

/* Clear the entire LCD display */
void LCD_Clear(void);

/* Move cursor to a specific position
 * row = 0 means Line 1, row = 1 means Line 2
 * col = 0 to 15 (16 columns available) */
void LCD_SetCursor(unsigned char row, unsigned char col);

/* Send a command to LCD (like clear, move cursor, etc.) */
void LCD_SendCommand(unsigned char command);

/* Display a single character at current cursor position */
void LCD_SendChar(unsigned char character);

/* Display a whole string (text) starting at current cursor */
void LCD_Print(char *text);

/* Display a 2-digit number with leading zero
 * Example: LCD_PrintTwoDigit(8) shows "08" on LCD */
void LCD_PrintTwoDigit(unsigned int number);

/* Display a 4-digit year number
 * Example: LCD_PrintYear(2024) shows "2024" on LCD */
void LCD_PrintYear(unsigned int year);

/* Display temperature as integer on LCD
 * Example: LCD_PrintTemp(28) shows "28" on LCD */
void LCD_PrintTemp(unsigned int tempValue);

/* Display "Please enter a valid key" message for 1 second.
 * Convenience function to replace repeated 6-line patterns. */
void LCD_ShowInvalidKey(void);

#endif   /* LCD_H */
