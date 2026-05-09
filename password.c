/*
 * =====================================================================
 * FILE     : password.c
 * PURPOSE  : All password-related logic.
 *            Verifying password, lockout, and changing password.
 *
 * STORAGE:
 *   Password is stored in RAM (lost on power off).
 *   Default password at startup = "1234"
 *   To make it permanent: external EEPROM would be needed (not in scope).
 *
 * PASSWORD ENTRY FLOW:
 *   - User presses digit keys one by one
 *   - '*' is shown for each digit (privacy)
 *   - '#' key confirms entry ONLY when exactly 4 digits entered
 *   - System compares entered password with stored password
 * =====================================================================
 */

#include "password.h"    /* Our own header                              */
#include "keypad.h"      /* For getting key presses                     */
#include "lcd.h"         /* For showing messages on LCD                 */
#include "delay.h"       /* For delays in messages and lockout          */
#include "pin_config.h"  /* For PASSWORD_LENGTH, MAX_WRONG_TRIES, etc.  */
#include <lpc214x.h>     /* For IOSET0, IOCLR0 (buzzer control)         */

/* The stored password - this is a global variable (lives whole program) */
/* Index 0 = first digit, Index 3 = last digit of 4-digit password       */
unsigned char storedPassword[PASSWORD_LENGTH] = {1, 2, 3, 4};  /* "1234" */

/* Counter for wrong password attempts */
unsigned char wrongAttemptCount = 0;

/* -------------------------------------------------------------------
 * Password_Init()
 * Sets the default password to "1234" at program start.
 * -------------------------------------------------------------------*/
void Password_Init(void)
{
    storedPassword[0] = 1;   /* First digit = 1  */
    storedPassword[1] = 2;   /* Second digit = 2 */
    storedPassword[2] = 3;   /* Third digit = 3  */
    storedPassword[3] = 4;   /* Fourth digit = 4 */
    wrongAttemptCount = 0;   /* Reset wrong attempts counter            */
}

/* -------------------------------------------------------------------
 * Password_Verify()
 * Shows password prompt, gets exactly 4 digit input from user,
 * compares with stored password.
 *
 * FIX: '#' only accepted when exactly PASSWORD_LENGTH digits entered.
 *      Prevents empty or partial password from being accepted.
 *
 * Returns: CORRECT (1) if matched, WRONG (0) if not matched
 * -------------------------------------------------------------------*/
unsigned char Password_Verify(void)
{
    char  keyPressed;                   /* Current key from keypad       */
    unsigned char enteredDigits[PASSWORD_LENGTH]; /* Store entered digits */
    unsigned char digitCount = 0;       /* How many digits entered       */
    unsigned char i;                    /* Loop counter for comparison   */
    unsigned char allMatch;             /* Flag: do all digits match?    */

    /* Show password prompt on LCD */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Enter Password: ");   /* Line 1 prompt                   */
    LCD_SetCursor(1, 0);
    LCD_Print("                ");   /* Line 2 blank (will fill with *) */
    LCD_SetCursor(1, 0);            /* Move cursor to start of Line 2   */

    /* Clear the entered digits array */
    for (i = 0; i < PASSWORD_LENGTH; i++)
    {
        enteredDigits[i] = 0;
    }
    digitCount = 0;

    /* Keep getting keys until exactly 4 digits entered and '#' pressed */
    while (1)
    {
        keyPressed = Keypad_GetKey();   /* Wait for key press            */

        /* FIX: '#' only accepted when exactly 4 digits entered          */
        if (keyPressed == '#' && digitCount == PASSWORD_LENGTH)
        {
            break;
        }
        else if (keyPressed == '*')
        {
            /* Backspace: remove last digit */
            if (digitCount > 0)
            {
                digitCount--;
                /* Move cursor back one and print space to erase '*' */
                LCD_SetCursor(1, digitCount);
                LCD_SendChar(' ');
                LCD_SetCursor(1, digitCount);  /* Move cursor back       */
            }
        }
        /* FIX: digitCount < PASSWORD_LENGTH ensures max 4 digits        */
        else if (keyPressed >= '0' && keyPressed <= '9' && digitCount < PASSWORD_LENGTH)
        {
            enteredDigits[digitCount] = keyPressed - '0'; /* Save digit  */
            digitCount++;                    /* Increment count           */
            LCD_SendChar('*');               /* Show '*' on LCD           */
        }
        /* Ignore any other keys (A, B, C, D) */
    }

    /* Now compare entered digits with stored password */
    allMatch = 1;   /* Assume they match, prove wrong below             */

    /* No length check needed - digitCount guaranteed == PASSWORD_LENGTH */
    for (i = 0; i < PASSWORD_LENGTH; i++)
    {
        if (enteredDigits[i] != storedPassword[i])
        {
            allMatch = 0;   /* Found a mismatch!                        */
            break;          /* No need to check further                 */
        }
    }

    return allMatch;   /* Return CORRECT or WRONG                       */
}

/* -------------------------------------------------------------------
 * Password_CheckWithLockout()
 * Verifies password and handles the 3-attempt lockout.
 *
 * If wrong 3 times:
 *   - Shows "System Locked!" message
 *   - Beeps buzzer 3 times
 *   - Waits 30 seconds
 *   - Returns to normal (wrong count reset)
 *
 * Returns: 1 = access granted, 0 = access denied
 * -------------------------------------------------------------------*/
unsigned char Password_CheckWithLockout(void)
{
    unsigned char result;         /* Result of password check            */
    unsigned char i;              /* Loop counter                        */

    result = Password_Verify();   /* Ask for password and check it       */

    if (result == CORRECT)
    {
        wrongAttemptCount = 0;    /* Reset counter on success            */
        return 1;                 /* Access granted                      */
    }
    else
    {
        /* Wrong password */
        wrongAttemptCount++;      /* Increment wrong attempt counter     */

        /* Show "Access Denied" message */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Access Denied!  ");
        LCD_SetCursor(1, 0);
        LCD_Print("Attempts: ");
        LCD_SendChar('0' + wrongAttemptCount);  /* Show count (1, 2, 3) */
        LCD_Print("/3          ");

        /* Beep buzzer once */
        IOSET0 = (1 << BUZZER_BIT);    /* Buzzer ON                     */
        delay_ms(200);                  /* Beep for 200ms                */
        IOCLR0 = (1 << BUZZER_BIT);    /* Buzzer OFF                    */

        delay_ms(1500);    /* Show message for 1.5 seconds               */

        /* Check if 3 wrong attempts reached */
        if (wrongAttemptCount >= MAX_WRONG_TRIES)
        {
            /* System Lockout! */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print(" System Locked! ");
            LCD_SetCursor(1, 0);
            LCD_Print(" Wait 30 sec... ");

            /* Beep buzzer 3 times to indicate lockout */
            for (i = 0; i < 3; i++)
            {
                IOSET0 = (1 << BUZZER_BIT);   /* Buzzer ON              */
                delay_ms(300);
                IOCLR0 = (1 << BUZZER_BIT);   /* Buzzer OFF             */
                delay_ms(200);
            }

            /* Wait 30 seconds (lockout period) */
            delay_ms(LOCKOUT_TIME_MS);

            /* Reset counter after lockout period */
            wrongAttemptCount = 0;
        }

        return 0;   /* Access denied */
    }
}

/* -------------------------------------------------------------------
 * Password_Change()
 * Guides user through changing the password.
 *
 * Step 1: Enter current password (verify)
 * Step 2: Enter new password (exactly 4 digits, then '#')
 * Step 3: Re-enter new password to confirm (exactly 4 digits, then '#')
 * Step 4: If Step 2 and Step 3 match -> save new password
 *         If they don't match -> go back to Step 2 and ask again
 *
 * FIX: '#' only accepted when exactly PASSWORD_LENGTH digits entered
 *      in both Step 2 and Step 3.
 * -------------------------------------------------------------------*/
void Password_Change(void)
{
    unsigned char newPassword[PASSWORD_LENGTH];     /* New password buffer  */
    unsigned char confirmPassword[PASSWORD_LENGTH]; /* Confirm buffer        */
    char  keyPressed;                               /* Current key pressed   */
    unsigned char digitCount;                       /* Digits entered count  */
    unsigned char i;                                /* Loop counter          */
    unsigned char passwordsMatch;                   /* Do new pwd and confirm match? */
    unsigned char confirmCount;                     /* Confirm digits count  */

    /* Step 1: Verify current password first */
    if (Password_CheckWithLockout() == 0)
    {
        /* Wrong current password - go back to main menu */
        return;
    }

    /* Current password was correct. Now ask for new password. */
    /* This loop continues until new password and confirm password match */
    while (1)
    {
        /* ---- Step 2: Enter New Password ---- */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("New Password:   ");
        LCD_SetCursor(1, 0);
        LCD_Print("                ");
        LCD_SetCursor(1, 0);

        digitCount = 0;

        while (1)
        {
            keyPressed = Keypad_GetKey();

            /* FIX: '#' only accepted when exactly 4 digits entered      */
            if (keyPressed == '#' && digitCount == PASSWORD_LENGTH)
            {
                break;
            }
            else if (keyPressed == '*' && digitCount > 0)
            {
                digitCount--;
                LCD_SetCursor(1, digitCount);
                LCD_SendChar(' ');
                LCD_SetCursor(1, digitCount);
            }
            /* FIX: digitCount < PASSWORD_LENGTH ensures max 4 digits    */
            else if (keyPressed >= '0' && keyPressed <= '9' && digitCount < PASSWORD_LENGTH)
            {
                newPassword[digitCount] = keyPressed - '0';
                digitCount++;
                LCD_SendChar('*');
            }
        }

        /* ---- Step 3: Re-enter New Password (Confirm) ---- */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Confirm Pwd:    ");
        LCD_SetCursor(1, 0);
        LCD_Print("                ");
        LCD_SetCursor(1, 0);

        confirmCount = 0;

        while (1)
        {
            keyPressed = Keypad_GetKey();

            /* FIX: '#' only accepted when exactly 4 digits entered      */
            if (keyPressed == '#' && confirmCount == PASSWORD_LENGTH)
            {
                break;
            }
            else if (keyPressed == '*' && confirmCount > 0)
            {
                confirmCount--;
                LCD_SetCursor(1, confirmCount);
                LCD_SendChar(' ');
                LCD_SetCursor(1, confirmCount);
            }
            /* FIX: confirmCount < PASSWORD_LENGTH ensures max 4 digits  */
            else if (keyPressed >= '0' && keyPressed <= '9' && confirmCount < PASSWORD_LENGTH)
            {
                confirmPassword[confirmCount] = keyPressed - '0';
                confirmCount++;
                LCD_SendChar('*');
            }
        }

        /* ---- Step 4: Compare new password with confirmation ---- */
        passwordsMatch = 1;

        /* No length check needed - both guaranteed == PASSWORD_LENGTH   */
        for (i = 0; i < PASSWORD_LENGTH; i++)
        {
            if (newPassword[i] != confirmPassword[i])
            {
                passwordsMatch = 0;   /* Found mismatch                 */
                break;
            }
        }

        if (passwordsMatch == 1)
        {
            /* Passwords match! Save the new password */
            for (i = 0; i < PASSWORD_LENGTH; i++)
            {
                storedPassword[i] = newPassword[i];   /* Update stored  */
            }

            /* Show success message */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Password Changed");
            LCD_SetCursor(1, 0);
            LCD_Print("Successfully!   ");
            delay_ms(2000);    /* Show for 2 seconds                    */
            break;             /* Exit the while loop - done!           */
        }
        else
        {
            /* Passwords did NOT match - show error and try again */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Pwd Mismatch!   ");
            LCD_SetCursor(1, 0);
            LCD_Print("Re-enter New Pwd");
            delay_ms(1500);    /* Show message for 1.5 seconds          */
            /* Loop continues - goes back to Step 2 (enter new password) */
        }
    }
}
