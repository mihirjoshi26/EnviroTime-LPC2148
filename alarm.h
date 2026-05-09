/*
 * =====================================================================
 * FILE     : alarm.h
 * PURPOSE  : DECLARES the Alarm data type and all alarm-related
 *            functions for the EnviroTime multi-alarm system.
 *
 * ALARM FEATURES:
 *   - Supports up to MAX_ALARMS (5) independent alarms
 *   - Each alarm has hour, minute, second, and enabled/disabled flag
 *   - View: read-only browsing with Prev/Next/Exit
 *   - Manage: Add / Edit (time + on/off toggle) / Delete
 *
 * CONNECTS TO: alarm.c (implementation)
 * =====================================================================
 */

#ifndef ALARM_H
#define ALARM_H

#include "rtc.h"   /* For RTC_Time used by CheckAndRingAlarm */

/* ------------------------------------------------------------------- */
/*  ALARM CONSTANTS                                                     */
/* ------------------------------------------------------------------- */
#define MAX_ALARMS  5   /* Maximum number of alarms the system can hold */

/* ------------------------------------------------------------------- */
/*  ALARM DATA TYPE                                                     */
/* ------------------------------------------------------------------- */
/* Structure to hold one alarm's settings */
typedef struct {
    unsigned char hour;      /* Alarm hour   (0-23)                    */
    unsigned char minute;    /* Alarm minute (0-59)                    */
    unsigned char second;    /* Alarm second (0-59)                    */
    unsigned char enabled;   /* 1 = ON (active), 0 = OFF (inactive)    */
} Alarm;

/* ------------------------------------------------------------------- */
/*  GLOBAL ALARM DATA (defined in alarm.c)                              */
/* ------------------------------------------------------------------- */
extern Alarm alarms[MAX_ALARMS];      /* Array storing all alarms      */
extern unsigned char alarmCount;      /* Current number of alarms      */

/* ------------------------------------------------------------------- */
/*  ALARM FUNCTION DECLARATIONS                                         */
/* ------------------------------------------------------------------- */

/* Check if any enabled alarm matches current time. If so, ring buzzer
 * until user presses the ALARM SWITCH, then disable that alarm. */
void CheckAndRingAlarm(RTC_Time *currentTime);

/* Top-level alarm menu: 1.View  2.Manage  3.Exit */
void HandleAlarmMenu(void);

/* Read-only alarm browser with Prev/Next/Exit navigation */
void HandleAlarmView(void);

/* Alarm management submenu: 1.Add  2.Edit  3.Delete  4.Exit */
void HandleAlarmManage(void);

/* Add a new alarm (prompts for time, defaults to enabled) */
void AlarmAdd(void);

/* Browse and select an alarm, then edit time or toggle ON/OFF */
void AlarmEdit(void);

/* Browse and select an alarm, confirm, then delete it */
void AlarmDelete(void);

/* Display one alarm's time and status on LCD Line 1 */
void DisplayAlarmLine(unsigned char index);

/* Shared browser for Edit/Delete: navigate alarms, select with #
 * Returns '#' if selected (index in *selectedIndex), '*' if cancelled */
char AlarmBrowseSelect(unsigned char *selectedIndex);

#endif   /* ALARM_H */
