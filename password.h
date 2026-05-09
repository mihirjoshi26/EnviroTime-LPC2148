/*
 * =====================================================================
 * FILE     : password.h
 * PURPOSE  : DECLARES password verification and change functions.
 *
 * HOW PASSWORD WORKS IN THIS PROJECT:
 *   - Default password is "1234" (4 digits)
 *   - Before editing RTC or Alarm, user must enter this password
 *   - User can change password in the Password Edit menu
 *   - 3 wrong tries = system locks for 30 seconds
 *
 * CONNECTS TO: password.c
 * =====================================================================
 */

#ifndef PASSWORD_H
#define PASSWORD_H

/* Initialize password system with default password "1234" */
void Password_Init(void);

/* Ask user to enter password via keypad and verify it.
 * Shows "Enter Password:" on LCD.
 * Shows '*' for each digit typed.
 * Returns: 1 = correct password, 0 = wrong password */
unsigned char Password_Verify(void);

/* Same as Password_Verify() but also handles the lockout logic.
 * If wrong 3 times: lock system for 30 seconds.
 * Returns: 1 = access granted, 0 = access denied */
unsigned char Password_CheckWithLockout(void);

/* Allows user to change the password.
 * Steps:
 *   1. Ask for current password
 *   2. Ask for new password
 *   3. Ask to re-enter new password
 *   4. If new and re-entered match: save new password
 *   5. If they don't match: ask to re-enter new password again */
void Password_Change(void);

#endif   /* PASSWORD_H */
