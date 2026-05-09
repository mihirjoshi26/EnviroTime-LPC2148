/*
 * =====================================================================
 * FILE     : keypad.h
 * PURPOSE  : DECLARES keypad functions so other files can use them.
 *
 * KEYPAD LAYOUT (physical buttons):
 *   +---+---+---+---+
 *   | 1 | 2 | 3 | A |   <- Row 0
 *   +---+---+---+---+
 *   | 4 | 5 | 6 | B |   <- Row 1
 *   +---+---+---+---+
 *   | 7 | 8 | 9 | C |   <- Row 2
 *   +---+---+---+---+
 *   | * | 0 | # | D |   <- Row 3
 *   +---+---+---+---+
 *
 * IN THIS PROJECT:
 *   '0'-'9' = Digit input for time, alarm, password
 *   '*'     = Backspace (delete last digit)
 *   '#'     = ENTER / Confirm
 * =====================================================================
 */

#ifndef KEYPAD_H
#define KEYPAD_H

/* Initialize keypad GPIO pins - call once at start */
void Keypad_Init(void);

/* Wait until a key is pressed, then return which key was pressed.
 * Returns: character like '0','1',...'9','*','#','A','B','C','D'
 * This function BLOCKS (waits) until a key is pressed. */
char Keypad_GetKey(void);

/* Read a number entered via keypad (up to maxDigits digits).
 * Shows '*' on LCD for each digit pressed (for password entry).
 * '#' key confirms, '*' key acts as backspace.
 * Returns the number entered as unsigned int.
 * 
 * Parameter: maxDigits = maximum digits to accept (e.g., 4 for password)
 * Parameter: showStars = 1 means show '*' (for password), 0 shows digits */
unsigned int Keypad_ReadNumber(unsigned char maxDigits, unsigned char showStars);

#endif   /* KEYPAD_H */
