/*
 * =====================================================================
 * FILE     : menu.c
 * PURPOSE  : Implements all menu-related functions for the EnviroTime
 *            project: main menu display, RTC editing sub-menus,
 *            password menu, and the shared GetValidInput helper.
 *
 * MENU STRUCTURE:
 *   ShowMainMenu    -> displays 1.RTC  2.Alarm  3.Pwd  4.Exit
 *   HandleRTCEdit   -> sub-menu for editing time/date parameters
 *   HandlePasswordMenu -> sub-menu for changing password
 *   GetValidInput   -> shared helper for validated numeric input
 *
 * CONNECTS TO: menu.h (declarations)
 * =====================================================================
 */

#include <lpc214x.h>      /* LPC2148 hardware registers                 */
#include "pin_config.h"   /* Pin numbers and constants                  */
#include "delay.h"        /* delay_ms() function                        */
#include "lcd.h"          /* LCD display functions                       */
#include "keypad.h"       /* Keypad reading functions                    */
#include "rtc.h"          /* RTC read/write functions                    */
#include "password.h"     /* Password change function                   */
#include "menu.h"         /* Our own header                             */

/* =================================================================== */
/*  MAIN MENU                                                           */
/* =================================================================== */

/* -------------------------------------------------------------------
 * ShowMainMenu()
 * Displays the main edit menu on LCD.
 * User selects option by pressing 1, 2, 3, or 4 on keypad.
 * -------------------------------------------------------------------*/
void ShowMainMenu(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("1:RTC    2:Alarm");    /* Line 1: options 1 and 2       */
    LCD_SetCursor(1, 0);
    LCD_Print("3:Pass   4:Exit ");    /* Line 2: options 3 and 4       */
}

/* =================================================================== */
/*  RTC EDIT FUNCTIONS                                                  */
/* =================================================================== */

/* -------------------------------------------------------------------
 * HandleRTCEdit()
 * Shows the RTC edit sub-menu and lets user select which parameter
 * to edit (hour, minute, second, day, date, month, year).
 * Stays in sub-menu until user presses '8' to exit.
 * -------------------------------------------------------------------*/
void HandleRTCEdit(void)
{
    char subMenuKey;          /* Key user presses in sub-menu           */

    /* Keep showing sub-menu until user exits */
    while (1)
    {
        /* Show RTC edit sub-menu */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("1Hr 2Mn 3Sc 8Ext");   /* Line 1: options 1,2,3,8   */
        LCD_SetCursor(1, 0);
        LCD_Print("4Dy 5Dt 6Mo 7Yr ");   /* Line 2: options 4,5,6,7   */

        /* Wait for user to press an option */
        subMenuKey = Keypad_GetKey();

        if (subMenuKey == '8')
        {
            /* Exit back to main menu */
            LCD_Clear();
            break;
        }
        else if (subMenuKey >= '1' && subMenuKey <= '7')
        {
            /* Call function to handle the selected parameter */
            HandleRTCParameter(subMenuKey - '0');
        }
        else
        {
            LCD_ShowInvalidKey();
        }
    }
}

/* -------------------------------------------------------------------
 * HandleRTCParameter()
 * Handles editing one specific RTC parameter (hour, min, sec, etc.)
 *
 * Parameters:
 *   paramChoice = 1 (hour), 2 (min), 3 (sec), 4 (day),
 *                 5 (date), 6 (month), 7 (year)
 *   currentTime = pointer to current RTC time structure
 * -------------------------------------------------------------------*/
void HandleRTCParameter(unsigned char paramChoice)
{
    unsigned int newValue;    /* The new value user enters              */

    if (paramChoice == 1)
    {
        /* Edit HOUR */
        newValue = GetValidInput(23, 0, 2, "Set Hour (0-23) ", "Enter Value:# ");
        RTC_SetHour((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Hour Saved!     ");
        delay_ms(1000);
    }
    else if (paramChoice == 2)
    {
        /* Edit MINUTE */
        newValue = GetValidInput(59, 0, 2, "Set Min. (0-59) ", "Enter Value:# ");
        RTC_SetMinute((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Minute Saved!   ");
        delay_ms(1000);
    }
    else if (paramChoice == 3)
    {
        /* Edit SECOND */
        newValue = GetValidInput(59, 0, 2, "Set Sec. (0-59) ", "Enter Value:# ");
        RTC_SetSecond((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Second Saved!   ");
        delay_ms(1000);
    }
    else if (paramChoice == 4)
    {
        /* Edit DAY OF WEEK */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Set Day (0-6)   ");
        LCD_SetCursor(1, 0);
        LCD_Print("0=Sun 6=Sat:#   ");
        newValue = GetValidInput(6, 0, 1, "Set Day (0-6)   ", "0=Sun 6=Sat:# ");
        RTC_SetDay((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Day Saved!      ");
        delay_ms(1000);
    }
    else if (paramChoice == 5)
    {
        /* Edit DATE (day of month) */
        newValue = GetValidInput(31, 1, 2, "Set Date (1-31) ", "Enter Value:# ");
        RTC_SetDate((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Date Saved!     ");
        delay_ms(1000);
    }
    else if (paramChoice == 6)
    {
        /* Edit MONTH */
        newValue = GetValidInput(12, 1, 2, "Set Month(1-12) ", "Enter Value:# ");
        RTC_SetMonth((unsigned char)newValue);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Month Saved!    ");
        delay_ms(1000);
    }
    else if (paramChoice == 7)
    {
        /* Edit YEAR - special case: need 4 digits */
        unsigned int yearValue;
        char key;
        unsigned char digitCount = 0;

        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Edit Year:      ");
        LCD_SetCursor(1, 0);

        yearValue = 0;
        digitCount = 0;

        while (1)
        {
            key = Keypad_GetKey();
            if (key == '#')
            {
                if (digitCount == 0)
                {
                    LCD_ShowInvalidKey();
                    /* Redraw year prompt */
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_Print("Edit Year:      ");
                    LCD_SetCursor(1, 0);
                    continue;   /* Stay in inner loop */
                }
                break;
            }
            else if (key >= '0' && key <= '9' && digitCount < 4)
            {
                yearValue = (yearValue * 10) + (key - '0');
                digitCount++;
                LCD_SendChar(key);   /* Show actual digit for year      */
            }
            else if (key == '*' && digitCount > 0)
            {
                yearValue = yearValue / 10;
                digitCount--;
                LCD_SetCursor(1, 0);
                LCD_Print("                ");
                LCD_SetCursor(1, 0);
                if (digitCount == 1) {
                    LCD_SendChar(yearValue + '0');
                } else if (digitCount == 2) {
                    LCD_SendChar((yearValue / 10) + '0');
                    LCD_SendChar((yearValue % 10) + '0');
                } else if (digitCount == 3) {
                    LCD_SendChar((yearValue / 100) + '0');
                    LCD_SendChar(((yearValue / 10) % 10) + '0');
                    LCD_SendChar((yearValue % 10) + '0');
                }
            }
        }

        /* Basic validation: year should be reasonable */
        if (yearValue >= 2000 && yearValue <= 4095)
        {
            RTC_SetYear(yearValue);
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Year Saved!     ");
        }
        else
        {
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Invalid Year!   ");
            LCD_SetCursor(1, 0);
            LCD_Print("Use 2000-4095   ");
        }
        delay_ms(1000);
    }
}

/* =================================================================== */
/*  PASSWORD MENU FUNCTION                                              */
/* =================================================================== */

/* -------------------------------------------------------------------
 * HandlePasswordMenu()
 * Shows password submenu: Edit Password or Exit.
 * Loops until user selects Exit (key 2).
 * -------------------------------------------------------------------*/
void HandlePasswordMenu(void)
{
    char subKey;   /* Key pressed in password submenu */

    while (1)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("1:Edit Password ");
        LCD_SetCursor(1, 0);
        LCD_Print("2:Exit          ");

        subKey = Keypad_GetKey();

        if (subKey == '1')
        {
            /* Edit password (has its own internal verification) */
            Password_Change();
        }
        else if (subKey == '2')
        {
            /* Exit password menu */
            LCD_Clear();
            break;
        }
        else
        {
            LCD_ShowInvalidKey();
        }
    }
}

/* =================================================================== */
/*  HELPER FUNCTION                                                     */
/* =================================================================== */

/* -------------------------------------------------------------------
 * GetValidInput()
 * Gets a numeric input from the keypad with validation.
 * Keeps asking until user enters a value in valid range.
 *
 * Parameters:
 *   maxValue   = maximum allowed value
 *   minValue   = minimum allowed value
 *   maxDigits  = maximum number of digits to accept
 *   promptLine1 = text to show on LCD Line 1
 *   promptLine2 = text to show on LCD Line 2
 *
 * Returns: the validated number entered by user
 * -------------------------------------------------------------------*/
unsigned int GetValidInput(unsigned char maxValue, unsigned char minValue,
                           unsigned char maxDigits,
                           char *promptLine1, char *promptLine2)
{
    unsigned int enteredValue;   /* Value entered by user               */
    char  key;                   /* Current key pressed                 */
    unsigned char digitCount;    /* Number of digits entered            */

    /* Keep asking until valid input received */
    while (1)
    {
        /* Show prompt on LCD */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print(promptLine1);      /* Show first line prompt          */
        LCD_SetCursor(1, 0);         /* Move cursor to start of line 2  */
        LCD_Print(promptLine2);      /* Show second line prompt         */

        enteredValue = 0;
        digitCount   = 0;

        /* Get digits from user */
        while (1)
        {
            key = Keypad_GetKey();

            if (key == '#')
            {
                if (digitCount == 0)
                {
                    LCD_ShowInvalidKey();
                    break;   /* Break inner loop; outer loop re-prompts */
                }
                break;   /* User confirmed input with digits */
            }
            else if (key >= '0' && key <= '9' && digitCount < maxDigits)
            {
                /* Accept up to maxDigits digits */
                enteredValue = (enteredValue * 10) + (key - '0');
                digitCount++;
                LCD_SendChar(key);   /* Show digit on LCD              */
            }
            else if (key == '*' && digitCount > 0)
            {
                /* Backspace: remove last digit */
                enteredValue = enteredValue / 10;   /* Remove last digit */
                digitCount--;
                /* We cannot easily erase from LCD here, so clear and redraw */
                LCD_SetCursor(1, 0);
                LCD_Print("                ");   /* Clear line 2        */
                LCD_SetCursor(1, 0);
                LCD_Print(promptLine2);          /* Show prompt again   */
                if (digitCount == 1) {
                    LCD_SendChar(enteredValue + '0');
                } else if (digitCount == 2) {
                    LCD_SendChar((enteredValue / 10) + '0');
                    LCD_SendChar((enteredValue % 10) + '0');
                }
            }
        }

        /* If no digits were entered, outer loop restarts (re-prompts) */
        if (digitCount == 0)
        {
            continue;
        }

        /* Validate the entered value */
        if (enteredValue >= minValue && enteredValue <= maxValue)
        {
            return enteredValue;   /* Return the valid number           */
        }
        else
        {
            LCD_ShowInvalidKey();
            /* Loop will show prompt again */
        }
    }
}

/* =================================================================== */
/*  END OF menu.c                                                       */
/* =================================================================== */
