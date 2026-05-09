/*
 * =====================================================================
 * FILE     : menu.h
 * PURPOSE  : DECLARES all menu-related functions for the EnviroTime
 *            project: main menu, RTC editing, password menu, and the
 *            shared GetValidInput helper.
 *
 * MENU STRUCTURE:
 *   Main Menu:
 *     1. RTC Edit  -> edit hour/min/sec/day/date/month/year
 *     2. Alarm     -> view/manage multiple alarms (see alarm.h)
 *     3. Password  -> change password
 *     4. Exit
 *
 * CONNECTS TO: menu.c (implementation)
 * =====================================================================
 */

#ifndef MENU_H
#define MENU_H

#include "rtc.h"   /* For RTC_Time used by HandleRTCParameter */

/* ------------------------------------------------------------------- */
/*  MENU FUNCTION DECLARATIONS                                          */
/* ------------------------------------------------------------------- */

/* Display the main edit menu on LCD (1.RTC  2.Alarm  3.Pwd  4.Exit) */
void ShowMainMenu(void);

/* RTC edit sub-menu: select hour/min/sec/day/date/month/year to edit */
void HandleRTCEdit(void);

/* Handle editing one specific RTC parameter (hour, min, sec, etc.)
 * paramChoice: 1=hour, 2=min, 3=sec, 4=day, 5=date, 6=month, 7=year */
void HandleRTCParameter(unsigned char paramChoice);

/* Password sub-menu: 1.Edit Password  2.Exit */
void HandlePasswordMenu(void);

/* Get a validated numeric input from keypad.
 * Shows prompts on LCD, accepts digits, validates range.
 * Returns the validated number entered by user. */
unsigned int GetValidInput(unsigned char maxValue, unsigned char minValue,
                           unsigned char maxDigits,
                           char *promptLine1, char *promptLine2);

#endif   /* MENU_H */
