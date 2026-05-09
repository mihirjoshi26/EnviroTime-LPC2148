/*
 * =====================================================================
 * FILE     : keypad.c
 * PURPOSE  : Controls the 4x4 matrix keypad connected to Port 1.
 *
 * HOW MATRIX KEYPAD WORKS:
 *   - 4 ROW pins are OUTPUTS driven LOW one by one
 *   - 4 COL pins are INPUTS (pulled HIGH externally)
 *   - When a key is pressed, it connects one ROW to one COL
 *   - We drive ROW LOW, then check which COL goes LOW
 *   - From row+col position, we know which key was pressed
 *
 * SCANNING METHOD:
 *   Step 1: Drive all ROWs LOW. Read all COLs.
 *           If any COL is LOW -> some key is pressed.
 *   Step 2: Drive only ROW0 LOW, others HIGH. Check COLs.
 *           If a COL is LOW -> key is in ROW0.
 *   Step 3: Repeat for ROW1, ROW2, ROW3.
 *   Step 4: Once row is found, check which COL is LOW -> column found.
 *   Step 5: Use lookup table [row][col] to find key character.
 * =====================================================================
 */

#include <lpc214x.h>     /* LPC2148 register definitions                */
#include "pin_config.h"  /* Our pin definitions                         */
#include "delay.h"       /* For debounce delay                          */
#include "lcd.h"         /* To show * on LCD during password entry      */
#include "keypad.h"      /* Our own header                              */

/* -------------------------------------------------------------------
 * KEY LOOKUP TABLE
 * This table maps [row][col] to the key character.
 * When we find row=0, col=2, the key is '3' (from row 0, column 2).
 * -------------------------------------------------------------------*/
char keypadMap[4][4] =
{
    /*  COL0   COL1   COL2   COL3  */
    {   '1',   '2',   '3',   'A'  },  /* ROW 0 - top row    */
    {   '4',   '5',   '6',   'B'  },  /* ROW 1              */
    {   '7',   '8',   '9',   'C'  },  /* ROW 2              */
    {   '*',   '0',   '#',   'D'  }   /* ROW 3 - bottom row */
};

/* -------------------------------------------------------------------
 * Keypad_Init()
 * Sets up GPIO pins for keypad operation.
 * ROW pins: OUTPUT (we control them)
 * COL pins: INPUT (we read them, externally pulled HIGH)
 * -------------------------------------------------------------------*/
void Keypad_Init(void)
{
    /* Set ROW pins (P1.20 to P1.23) as OUTPUT */
    IODIR1 |= (1 << KEYPAD_ROW0);   /* P1.20 = output                  */
    IODIR1 |= (1 << KEYPAD_ROW1);   /* P1.21 = output                  */
    IODIR1 |= (1 << KEYPAD_ROW2);   /* P1.22 = output                  */
    IODIR1 |= (1 << KEYPAD_ROW3);   /* P1.23 = output                  */

    /* Set COL pins (P1.16 to P1.19) as INPUT */
    /* By default LPC2148 GPIO is input, but we clear the bits anyway */
    IODIR1 &= ~(1 << KEYPAD_COL0);  /* P1.16 = input                   */
    IODIR1 &= ~(1 << KEYPAD_COL1);  /* P1.17 = input                   */
    IODIR1 &= ~(1 << KEYPAD_COL2);  /* P1.18 = input                   */
    IODIR1 &= ~(1 << KEYPAD_COL3);  /* P1.19 = input                   */

    /* Drive all ROW pins LOW initially (ground all rows) */
    IOCLR1 = (1 << KEYPAD_ROW0);
    IOCLR1 = (1 << KEYPAD_ROW1);
    IOCLR1 = (1 << KEYPAD_ROW2);
    IOCLR1 = (1 << KEYPAD_ROW3);
}

/* -------------------------------------------------------------------
 * checkAnyKeyPressed()  - PRIVATE (only used inside this file)
 * Checks if ANY key is currently pressed.
 * Returns: 1 = yes a key is pressed, 0 = no key pressed
 *
 * All rows are LOW, so if any key is pressed, its COL will be pulled LOW.
 * -------------------------------------------------------------------*/
static unsigned char checkAnyKeyPressed(void)
{
    unsigned char col0State;   /* State of column 0 pin */
    unsigned char col1State;   /* State of column 1 pin */
    unsigned char col2State;   /* State of column 2 pin */
    unsigned char col3State;   /* State of column 3 pin */

    /* Read each column pin: 0 means key pressed (pulled LOW), 1 means not */
    col0State = (IOPIN1 >> KEYPAD_COL0) & 1;  /* Read P1.16             */
    col1State = (IOPIN1 >> KEYPAD_COL1) & 1;  /* Read P1.17             */
    col2State = (IOPIN1 >> KEYPAD_COL2) & 1;  /* Read P1.18             */
    col3State = (IOPIN1 >> KEYPAD_COL3) & 1;  /* Read P1.19             */

    /* If ANY column is LOW (0), a key is pressed */
    if (col0State == 0 || col1State == 0 || col2State == 0 || col3State == 0)
    {
        return 1;   /* YES, some key is pressed */
    }

    return 0;       /* No key is pressed */
}

/* -------------------------------------------------------------------
 * findPressedRow()  - PRIVATE helper function
 * Finds which ROW has the pressed key.
 * Returns: 0, 1, 2, or 3 for the row number.
 *
 * Method: Drive one row LOW at a time, keep others HIGH.
 *         If columns show a LOW, that row has the pressed key.
 * -------------------------------------------------------------------*/
static unsigned char findPressedRow(void)
{
    unsigned char rowIndex;   /* Which row we are currently testing      */

    /* Test each row one by one */
    for (rowIndex = 0; rowIndex < 4; rowIndex++)
    {
        /* Drive all rows HIGH first (deactivate all) */
        IOSET1 = (1 << KEYPAD_ROW0);
        IOSET1 = (1 << KEYPAD_ROW1);
        IOSET1 = (1 << KEYPAD_ROW2);
        IOSET1 = (1 << KEYPAD_ROW3);

        /* Now drive only the current row LOW (activate it) */
        IOCLR1 = (1 << (KEYPAD_ROW0 + rowIndex));

        /* Small delay for signal to stabilize */
        delay_us(5);

        /* Check if any column went LOW (key pressed in this row?) */
        if (checkAnyKeyPressed() == 1)
        {
            break;   /* Found the row! Stop searching. */
        }
    }

    /* Restore all rows to LOW (normal scanning state) */
    IOCLR1 = (1 << KEYPAD_ROW0);
    IOCLR1 = (1 << KEYPAD_ROW1);
    IOCLR1 = (1 << KEYPAD_ROW2);
    IOCLR1 = (1 << KEYPAD_ROW3);

    return rowIndex;   /* Return which row had the key */
}

/* -------------------------------------------------------------------
 * findPressedCol()  - PRIVATE helper function
 * Finds which COLUMN has the pressed key.
 * Returns: 0, 1, 2, or 3 for the column number.
 *
 * Method: Check each column pin. The one that is LOW is the pressed column.
 * -------------------------------------------------------------------*/
static unsigned char findPressedCol(void)
{
    unsigned char colIndex;   /* Which column we are checking */

    for (colIndex = 0; colIndex < 4; colIndex++)
    {
        /* Read the column pin state (0 = LOW = key in this column) */
        if (((IOPIN1 >> (KEYPAD_COL0 + colIndex)) & 1) == 0)
        {
            break;   /* Found the column! */
        }
    }

    return colIndex;   /* Return which column had the key */
}

/* -------------------------------------------------------------------
 * Keypad_GetKey()
 * Waits for a key press and returns the character of the key.
 * This function BLOCKS until user presses a key.
 *
 * Returns: character like '0' to '9', '*', '#', 'A' to 'D'
 * -------------------------------------------------------------------*/
char Keypad_GetKey(void)
{
    unsigned char rowFound;    /* Row number of pressed key              */
    unsigned char colFound;    /* Column number of pressed key           */
    char keyPressed;           /* The actual character of the key        */

    while (1)
    {
        /* Step 1: Wait until a key is pressed */
        while (checkAnyKeyPressed() == 0)
        {
            /* Keep looping until user presses something */
        }

        /* Step 2: Debounce - wait 20ms and check again */
        delay_ms(20);

        if (checkAnyKeyPressed() == 0)
        {
            /* Just noise (bouncing) - restart loop */
            continue;
        }
        
        /* Step 3: Find which row has the pressed key */
        rowFound = findPressedRow();

        /* Step 4: Find which column has the pressed key */
        colFound = findPressedCol();

        /* SAFETY CHECK: If user released the key exactly during the scan,
           rowFound or colFound will be 4 (out of bounds). 
           If so, ignore this phantom press and restart polling. */
        if (rowFound >= 4 || colFound >= 4)
        {
            continue;
        }

        /* Step 5: Look up the character from our table */
        keyPressed = keypadMap[rowFound][colFound];

        /* Step 6: Wait for key to be RELEASED before returning */
        while (checkAnyKeyPressed() == 1)
        {
            /* Wait until user lifts their finger */
        }

        delay_ms(10);    /* Small delay after release */

        return keyPressed;   /* Return the legitimately pressed key */
    }
}

/* -------------------------------------------------------------------
 * Keypad_ReadNumber()
 * Reads a multi-digit number from keypad.
 * Used for entering passwords, time values, etc.
 *
 * Parameters:
 *   maxDigits  = maximum digits to accept (e.g., 4 for password)
 *   showStars  = 1: show '*' on LCD (for password)
 *                0: show actual digits on LCD (for time entry)
 *
 * '#' key = confirm/enter (finish input)
 * '*' key = backspace (delete last digit)
 *
 * Returns: the number entered (e.g., if user pressed 1,2,3,4 returns 1234)
 * -------------------------------------------------------------------*/
unsigned int Keypad_ReadNumber(unsigned char maxDigits, unsigned char showStars)
{
    char keyPressed;                /* Current key user pressed          */
    unsigned char digitBuffer[10];  /* Store each digit separately       */
    unsigned char digitCount = 0;   /* How many digits entered so far    */
    unsigned int  finalNumber = 0;  /* Final number to return            */
    unsigned char i;                /* Loop counter                      */

    /* Keep getting keys until user presses '#' or reaches max digits */
    while (1)
    {
        keyPressed = Keypad_GetKey();   /* Wait for and get a key press  */

        /* Check if user pressed '#' = ENTER/CONFIRM */
        if (keyPressed == '#')
        {
            break;   /* User is done entering, exit the loop */
        }

        /* Check if user pressed '*' = BACKSPACE */
        if (keyPressed == '*')
        {
            if (digitCount > 0)   /* Only backspace if something was entered */
            {
                digitCount--;     /* Remove last digit from count            */
                /* Update LCD: go back one position and print space to erase */
                /* Caller should handle LCD cursor position */
            }
            continue;   /* Go back to waiting for next key */
        }

        /* Check if it is a digit key '0' to '9' */
        if (keyPressed >= '0' && keyPressed <= '9')
        {
            /* Only accept if we haven't reached the maximum digits */
            if (digitCount < maxDigits)
            {
                /* Store the digit (convert char to number: '5' - '0' = 5) */
                digitBuffer[digitCount] = keyPressed - '0';
                digitCount++;   /* Increment digit count */

                /* Show on LCD */
                if (showStars == 1)
                {
                    LCD_SendChar('*');  /* Show star for password privacy */
                }
                else
                {
                    LCD_SendChar(keyPressed);  /* Show actual digit       */
                }
            }
        }
        /* If any other key pressed (A, B, C, D etc.) - ignore it */
    }

    /* Calculate the final number from the digit array
     * Example: digits [1,2,3,4] -> 1*1000 + 2*100 + 3*10 + 4 = 1234 */
    finalNumber = 0;
    for (i = 0; i < digitCount; i++)
    {
        finalNumber = (finalNumber * 10) + digitBuffer[i];
    }

    return finalNumber;   /* Return the complete number entered */
}
