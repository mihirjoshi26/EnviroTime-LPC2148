/*
 * =====================================================================
 * FILE     : alarm.c
 * PURPOSE  : Implements the multi-alarm system for EnviroTime.
 *            Supports up to 5 alarms with Add, Edit, Delete, and
 *            read-only View with navigable browsing.
 *
 * ALARM MENU HIERARCHY:
 *   HandleAlarmMenu (top level):
 *     1. View   -> HandleAlarmView (read-only browse)
 *     2. Manage -> HandleAlarmManage:
 *        1. Add    -> AlarmAdd
 *        2. Edit   -> AlarmEdit (time + on/off toggle)
 *        3. Delete -> AlarmDelete (with confirmation)
 *        4. Exit
 *     3. Exit
 *
 * CONNECTS TO: alarm.h (declarations)
 * =====================================================================
 */

#include <lpc214x.h>      /* LPC2148 hardware registers                 */
#include "pin_config.h"   /* Pin numbers and constants (BUZZER_BIT, etc) */
#include "delay.h"        /* delay_ms() function                        */
#include "lcd.h"          /* LCD display functions                       */
#include "keypad.h"       /* Keypad reading functions                    */
#include "alarm.h"        /* Our own header (Alarm struct, prototypes)   */
#include "menu.h"         /* For GetValidInput used by AlarmAdd/AlarmEdit*/

/* =================================================================== */
/*  GLOBAL ALARM DATA                                                   */
/* =================================================================== */
Alarm alarms[MAX_ALARMS];      /* Array to store all alarms              */
unsigned char alarmCount = 0;  /* Current number of stored alarms        */

/* =================================================================== */
/*  ALARM CHECK FUNCTION                                                */
/* =================================================================== */

/* Forward declaration for IsAlarmSwitchPressed (defined in main.c) */
unsigned char IsAlarmSwitchPressed(void);

/* -------------------------------------------------------------------
 * CheckAndRingAlarm()
 * Checks if current time matches ANY enabled alarm in the array.
 * If it matches: turns on buzzer until user presses ALARM SWITCH.
 * After dismissal, the triggered alarm is disabled to prevent
 * re-triggering in the same second.
 * -------------------------------------------------------------------*/
void CheckAndRingAlarm(RTC_Time *currentTime)
{
    unsigned char i;   /* Loop index for alarm array */

    for (i = 0; i < alarmCount; i++)
    {
        /* Skip disabled alarms */
        if (alarms[i].enabled == 0)
        {
            continue;
        }

        /* Check if current time matches this alarm */
        if (currentTime->hour   == alarms[i].hour   &&
            currentTime->minute == alarms[i].minute &&
            currentTime->second == alarms[i].second)
        {
            /* *** ALARM TIME REACHED! *** */

            /* Show alarm message on LCD */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("*** ALARM!  *** ");
            LCD_SetCursor(1, 0);
            LCD_Print("Press SW2 Stop  ");

            /* Turn buzzer ON */
            IOSET0 = (1 << BUZZER_BIT);   /* P0.12 HIGH = Buzzer ON   */

            /* Keep buzzer ON until ALARM SWITCH (SW2) is pressed */
            while (IsAlarmSwitchPressed() == 0)
            {
                /* Stay in this loop - buzzer keeps ringing */
            }

            /* ALARM SWITCH was pressed - turn buzzer OFF */
            IOCLR0 = (1 << BUZZER_BIT);   /* P0.12 LOW = Buzzer OFF   */

            /* Clear alarm display */
            LCD_Clear();

            /* Disable this alarm so it doesn't re-trigger */
            alarms[i].enabled = 0;

            /* Only handle one alarm trigger per check */
            break;
        }
    }
}

/* =================================================================== */
/*  ALARM DISPLAY HELPER                                                */
/* =================================================================== */

/* -------------------------------------------------------------------
 * DisplayAlarmLine()
 * Displays one alarm's time and status on LCD Line 1.
 * Format: "HH:MM:SS OFF    " or "HH:MM:SS ON     "
 *
 * Parameter: index = which alarm in the array to display
 * -------------------------------------------------------------------*/
void DisplayAlarmLine(unsigned char index)
{
    LCD_SetCursor(0, 0);
    LCD_PrintTwoDigit(alarms[index].hour);      /* HH                  */
    LCD_SendChar(':');                           /* :                   */
    LCD_PrintTwoDigit(alarms[index].minute);    /* MM                  */
    LCD_SendChar(':');                           /* :                   */
    LCD_PrintTwoDigit(alarms[index].second);    /* SS                  */
    LCD_SendChar(' ');                           /* space separator     */
    if (alarms[index].enabled == 1)
    {
        LCD_Print("ON     ");   /* "ON" + 5 spaces = 7 chars to fill   */
    }
    else
    {
        LCD_Print("OFF    ");   /* "OFF" + 4 spaces = 7 chars to fill  */
    }
}

/* =================================================================== */
/*  ALARM BROWSE & SELECT (shared by Edit and Delete)                   */
/* =================================================================== */

/* -------------------------------------------------------------------
 * AlarmBrowseSelect()
 * Shared alarm browser used by Edit and Delete functions.
 * Shows alarms one at a time with Prev/Next/#=Select/Star=Exit nav.
 *
 * Parameters:
 *   selectedIndex = pointer to store the index of selected alarm
 *
 * Returns:
 *   '#' if user selected an alarm (index stored in *selectedIndex)
 *   '*' if user chose to exit without selecting
 * -------------------------------------------------------------------*/
char AlarmBrowseSelect(unsigned char *selectedIndex)
{
    unsigned char idx = 0;   /* Current alarm being displayed          */
    char key;                /* Key pressed by user                    */

    if (alarmCount == 0)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("No Alarms Set   ");
        LCD_SetCursor(1, 0);
        LCD_Print("                ");
        delay_ms(2000);
        return '*';   /* Nothing to select */
    }

    while (1)
    {
        LCD_Clear();
        DisplayAlarmLine(idx);
        LCD_SetCursor(1, 0);

        if (alarmCount == 1)
        {
            /* Only one alarm: Select or Exit */
            LCD_Print("#:Select  *:Exit");
            key = Keypad_GetKey();
            if (key == '#')
            {
                *selectedIndex = idx;
                return '#';
            }
            else if (key == '*')
            {
                return '*';
            }
        }
        else if (idx == 0)
        {
            /* First alarm, more after it */
            LCD_Print("2> #:Select *:Ex");
            key = Keypad_GetKey();
            if (key == '2')      { idx++; }
            else if (key == '#') { *selectedIndex = idx; return '#'; }
            else if (key == '*') { return '*'; }
        }
        else if (idx == alarmCount - 1)
        {
            /* Last alarm */
            LCD_Print("1< #:Select *:Ex");
            key = Keypad_GetKey();
            if (key == '1')      { idx--; }
            else if (key == '#') { *selectedIndex = idx; return '#'; }
            else if (key == '*') { return '*'; }
        }
        else
        {
            /* Middle alarm */
            LCD_Print("1< 2> #:Sel *:Ex");
            key = Keypad_GetKey();
            if (key == '1')      { idx--; }
            else if (key == '2') { idx++; }
            else if (key == '#') { *selectedIndex = idx; return '#'; }
            else if (key == '*') { return '*'; }
        }
    }
}

/* =================================================================== */
/*  READ-ONLY ALARM VIEWER                                              */
/* =================================================================== */

/* -------------------------------------------------------------------
 * HandleAlarmView()
 * Read-only alarm browser. Displays each alarm with its status.
 * Navigation: Prev / Next / Exit. No editing possible here.
 * -------------------------------------------------------------------*/
void HandleAlarmView(void)
{
    unsigned char idx = 0;   /* Current alarm being displayed          */
    char key;                /* Key pressed by user                    */

    if (alarmCount == 0)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("No Alarms Set   ");
        LCD_SetCursor(1, 0);
        LCD_Print("                ");
        delay_ms(2000);
        return;
    }

    while (1)
    {
        LCD_Clear();
        DisplayAlarmLine(idx);
        LCD_SetCursor(1, 0);

        if (alarmCount == 1)
        {
            /* Only one alarm: just Exit */
            LCD_Print("     *:Exit     ");
            key = Keypad_GetKey();
            if (key == '*') { break; }
        }
        else if (idx == 0)
        {
            /* First alarm, more exist after */
            LCD_Print("2:>       *:Exit");
            key = Keypad_GetKey();
            if (key == '2')      { idx++; }
            else if (key == '*') { break; }
        }
        else if (idx == alarmCount - 1)
        {
            /* Last alarm */
            LCD_Print("1:<       *:Exit");
            key = Keypad_GetKey();
            if (key == '1')      { idx--; }
            else if (key == '*') { break; }
        }
        else
        {
            /* Middle alarm */
            LCD_Print("1:<  2:>  *:Exit");
            key = Keypad_GetKey();
            if (key == '1')      { idx--; }
            else if (key == '2') { idx++; }
            else if (key == '*') { break; }
        }
    }
    LCD_Clear();
}

/* =================================================================== */
/*  ALARM ADD                                                           */
/* =================================================================== */

/* -------------------------------------------------------------------
 * AlarmAdd()
 * Prompts user to enter a new alarm time (hour, minute, second).
 * New alarm defaults to ENABLED. Refuses if alarm list is full.
 * -------------------------------------------------------------------*/
void AlarmAdd(void)
{
    unsigned int inputHour;     /* User-entered alarm hour               */
    unsigned int inputMinute;   /* User-entered alarm minute             */
    unsigned int inputSecond;   /* User-entered alarm second             */

    /* Check if alarm list is full */
    if (alarmCount >= MAX_ALARMS)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("Alarm List Full!");
        LCD_SetCursor(1, 0);
        LCD_Print("Max 5 alarms    ");
        delay_ms(2000);
        return;
    }

    /* Prompt for alarm time using existing GetValidInput */
    inputHour   = GetValidInput(23, 0, 2, "Set Alarm Hour  ", "Enter(0-23):# ");
    inputMinute = GetValidInput(59, 0, 2, "Set Alarm Min.  ", "Enter(0-59):# ");
    inputSecond = GetValidInput(59, 0, 2, "Set Alarm Sec.  ", "Enter(0-59):# ");

    /* Store the new alarm */
    alarms[alarmCount].hour    = (unsigned char)inputHour;
    alarms[alarmCount].minute  = (unsigned char)inputMinute;
    alarms[alarmCount].second  = (unsigned char)inputSecond;
    alarms[alarmCount].enabled = 1;   /* Default: enabled */
    alarmCount++;

    /* Confirmation message */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Alarm Added!    ");
    LCD_SetCursor(1, 0);
    LCD_PrintTwoDigit((unsigned char)inputHour);
    LCD_SendChar(':');
    LCD_PrintTwoDigit((unsigned char)inputMinute);
    LCD_SendChar(':');
    LCD_PrintTwoDigit((unsigned char)inputSecond);
    LCD_Print(" ON     ");
    delay_ms(2000);
}

/* =================================================================== */
/*  ALARM EDIT                                                          */
/* =================================================================== */

/* -------------------------------------------------------------------
 * AlarmEdit()
 * Lets user browse alarms, select one, then either:
 *   1. Edit alarm time (hour/min/sec)
 *   2. Toggle alarm ON/OFF (enable/disable)
 *   3. Exit back to manage menu
 * -------------------------------------------------------------------*/
void AlarmEdit(void)
{
    unsigned char selIdx;       /* Index of alarm user selected           */
    char result;                /* Result from browse-select              */
    char editKey;               /* Key pressed in edit sub-menu           */
    unsigned int inputHour;     /* New alarm hour                        */
    unsigned int inputMinute;   /* New alarm minute                      */
    unsigned int inputSecond;   /* New alarm second                      */

    /* Let user browse and select an alarm */
    result = AlarmBrowseSelect(&selIdx);
    if (result == '*')
    {
        return;   /* User cancelled */
    }

    /* User selected alarm at selIdx - show edit options */
    while (1)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("1:Time  2:OnOff ");
        LCD_SetCursor(1, 0);
        LCD_Print("3:Exit          ");

        editKey = Keypad_GetKey();

        if (editKey == '1')
        {
            /* Edit alarm time */
            inputHour   = GetValidInput(23, 0, 2, "Set Alarm Hour  ", "Enter(0-23):# ");
            inputMinute = GetValidInput(59, 0, 2, "Set Alarm Min.  ", "Enter(0-59):# ");
            inputSecond = GetValidInput(59, 0, 2, "Set Alarm Sec.  ", "Enter(0-59):# ");

            alarms[selIdx].hour   = (unsigned char)inputHour;
            alarms[selIdx].minute = (unsigned char)inputMinute;
            alarms[selIdx].second = (unsigned char)inputSecond;

            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Alarm Updated!  ");
            LCD_SetCursor(1, 0);
            LCD_PrintTwoDigit(alarms[selIdx].hour);
            LCD_SendChar(':');
            LCD_PrintTwoDigit(alarms[selIdx].minute);
            LCD_SendChar(':');
            LCD_PrintTwoDigit(alarms[selIdx].second);
            LCD_Print("        ");
            delay_ms(2000);
        }
        else if (editKey == '2')
        {
            /* Toggle ON/OFF */
            if (alarms[selIdx].enabled == 1)
            {
                alarms[selIdx].enabled = 0;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Alarm Disabled  ");
            }
            else
            {
                alarms[selIdx].enabled = 1;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Alarm Enabled   ");
            }
            LCD_SetCursor(1, 0);
            LCD_PrintTwoDigit(alarms[selIdx].hour);
            LCD_SendChar(':');
            LCD_PrintTwoDigit(alarms[selIdx].minute);
            LCD_SendChar(':');
            LCD_PrintTwoDigit(alarms[selIdx].second);
            if (alarms[selIdx].enabled == 1)
            {
                LCD_Print(" ON     ");
            }
            else
            {
                LCD_Print(" OFF    ");
            }
            delay_ms(2000);
        }
        else if (editKey == '3')
        {
            /* Exit edit sub-menu */
            LCD_Clear();
            break;
        }
        else
        {
            /* Invalid key */
            LCD_ShowInvalidKey();
        }
    }
}

/* =================================================================== */
/*  ALARM DELETE                                                        */
/* =================================================================== */

/* -------------------------------------------------------------------
 * AlarmDelete()
 * Lets user browse alarms, select one, confirm deletion.
 * Removes the alarm by shifting remaining alarms down in the array.
 * -------------------------------------------------------------------*/
void AlarmDelete(void)
{
    unsigned char selIdx;   /* Index of alarm user selected               */
    char result;            /* Result from browse-select                  */
    char confirmKey;        /* Key pressed for confirmation               */
    unsigned char i;        /* Loop index for array shift                 */

    /* Let user browse and select an alarm */
    result = AlarmBrowseSelect(&selIdx);
    if (result == '*')
    {
        return;   /* User cancelled */
    }

    /* Confirm deletion */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Delete alarm?   ");
    LCD_SetCursor(1, 0);
    LCD_Print("1.Yes     2.No  ");

    while (1)
    {
        confirmKey = Keypad_GetKey();

        if (confirmKey == '1')
        {
            /* Delete: shift all alarms after selIdx down by one */
            for (i = selIdx; i < alarmCount - 1; i++)
            {
                alarms[i] = alarms[i + 1];
            }
            alarmCount--;

            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Alarm Deleted!  ");
            LCD_SetCursor(1, 0);
            LCD_Print("                ");
            delay_ms(2000);
            break;
        }
        else if (confirmKey == '2')
        {
            /* Cancel deletion */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Delete Cancelled");
            LCD_SetCursor(1, 0);
            LCD_Print("                ");
            delay_ms(1000);
            break;
        }
        /* Ignore other keys, keep waiting for 1 or 2 */
    }
}

/* =================================================================== */
/*  ALARM MANAGEMENT SUBMENU                                            */
/* =================================================================== */

/* -------------------------------------------------------------------
 * HandleAlarmManage()
 * Shows alarm management submenu: Add, Edit, Delete, or Exit.
 * Loops until user selects Exit (key 4).
 * -------------------------------------------------------------------*/
void HandleAlarmManage(void)
{
    char subKey;   /* Key pressed in manage submenu */

    while (1)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("1:Add    2:Edit ");
        LCD_SetCursor(1, 0);
        LCD_Print("3:Delete 4:Exit ");

        subKey = Keypad_GetKey();

        if (subKey == '1')
        {
            AlarmAdd();
        }
        else if (subKey == '2')
        {
            AlarmEdit();
        }
        else if (subKey == '3')
        {
            AlarmDelete();
        }
        else if (subKey == '4')
        {
            LCD_Clear();
            break;
        }
        else
        {
            /* Invalid key */
            LCD_ShowInvalidKey();
        }
    }
}

/* =================================================================== */
/*  TOP-LEVEL ALARM MENU                                                */
/* =================================================================== */

/* -------------------------------------------------------------------
 * HandleAlarmMenu()
 * Top-level alarm menu: View alarms (read-only) or Manage alarms.
 * Loops until user selects Exit (key 3).
 * -------------------------------------------------------------------*/
void HandleAlarmMenu(void)
{
    char subKey;   /* Key pressed in alarm menu */

    while (1)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("1:View  2:Manage");
        LCD_SetCursor(1, 0);
        LCD_Print("3:Exit          ");

        subKey = Keypad_GetKey();

        if (subKey == '1')
        {
            HandleAlarmView();
        }
        else if (subKey == '2')
        {
            HandleAlarmManage();
        }
        else if (subKey == '3')
        {
            LCD_Clear();
            break;
        }
        else
        {
            /* Invalid key */
            LCD_ShowInvalidKey();
        }
    }
}

/* =================================================================== */
/*  END OF alarm.c                                                      */
/* =================================================================== */
