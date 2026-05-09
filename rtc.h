/*
 * =====================================================================
 * FILE     : rtc.h
 * PURPOSE  : DECLARES all RTC (Real Time Clock) functions.
 *            The LPC2148 has a built-in RTC - no external chip needed!
 *
 * RTC REGISTERS (built into LPC2148):
 *   SEC   = current second (0-59)
 *   MIN   = current minute (0-59)
 *   HOUR  = current hour (0-23)
 *   DOM   = day of month (1-31)
 *   DOW   = day of week (0=Sunday, 1=Monday ... 6=Saturday)
 *   MONTH = current month (1-12)
 *   YEAR  = current year (e.g., 2024)
 *
 * CONNECTS TO: rtc.c
 * =====================================================================
 */

#ifndef RTC_H
#define RTC_H

/* Structure to hold all time and date info in one place */
typedef struct
{
    unsigned char second;   /* 0 to 59   */
    unsigned char minute;   /* 0 to 59   */
    unsigned char hour;     /* 0 to 23   */
    unsigned char date;     /* 1 to 31   */
    unsigned char day;      /* 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat */
    unsigned char month;    /* 1 to 12   */
    unsigned int  year;     /* e.g. 2024 */
} RTC_Time;

/* Initialize RTC hardware - call once at start */
void RTC_Init(void);

/* Read current time and date from RTC registers into a structure */
void RTC_ReadTime(RTC_Time *currentTime);

/* Write new hour to RTC (0-23) */
void RTC_SetHour(unsigned char newHour);

/* Write new minute to RTC (0-59) */
void RTC_SetMinute(unsigned char newMinute);

/* Write new second to RTC (0-59) */
void RTC_SetSecond(unsigned char newSecond);

/* Write new day-of-week to RTC (0=Sun to 6=Sat) */
void RTC_SetDay(unsigned char newDay);

/* Write new date (day of month) to RTC (1-31) */
void RTC_SetDate(unsigned char newDate);

/* Write new month to RTC (1-12) */
void RTC_SetMonth(unsigned char newMonth);

/* Write new year to RTC */
void RTC_SetYear(unsigned int newYear);

/* Display time and date on LCD in format:
 * Line 1: HH:MM:SS  T:XXC
 * Line 2: DD/MM/YYYY  DAY  */
void RTC_DisplayOnLCD(RTC_Time *timeToShow, unsigned int temperature);

/* Returns the 3-letter day name string for a day number
 * Example: RTC_GetDayName(1) returns "MON" */
char* RTC_GetDayName(unsigned char dayNumber);

#endif   /* RTC_H */
