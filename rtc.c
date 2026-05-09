/*
 * =====================================================================
 * FILE     : rtc.c
 * PURPOSE  : All Real Time Clock functions for LPC2148.
 *            Uses the built-in RTC peripheral.
 *
 * LPC2148 RTC CLOCK SOURCE:
 *   Uses external 32.768 kHz crystal connected to RTCX1 and RTCX2 pins.
 *   The CCR register bit 4 (CLKSRC) selects this external crystal.
 *   This gives very accurate 1-second ticks for the RTC.
 *
 * KEY REGISTERS:
 *   CCR    = Clock Control Register (start/stop/reset RTC)
 *   SEC    = Seconds register
 *   MIN    = Minutes register
 *   HOUR   = Hours register
 *   DOM    = Day Of Month register
 *   DOW    = Day Of Week register
 *   MONTH  = Month register
 *   YEAR   = Year register
 * =====================================================================
 */

#include <lpc214x.h>     /* LPC2148 register definitions including RTC  */
#include "pin_config.h"  /* Our constants (RTC_CLKEN, RTC_CLKSRC, etc.) */
#include "rtc.h"         /* Our own header                              */
#include "lcd.h"         /* For displaying on LCD                       */

/* Day name strings - index 0=Sunday, 1=Monday, ... 6=Saturday */
char dayNames[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

/* -------------------------------------------------------------------
 * RTC_Init()
 * Initializes the RTC peripheral to start counting time.
 *
 * For LPC2148: We use the external 32.768 kHz crystal connected
 * to RTCX1 and RTCX2 pins as the RTC clock source (CLKSRC = 1).
 * This provides accurate 1-second ticks without needing prescaler
 * configuration (PREINT / PREFRAC are not required).
 *
 * Steps:
 *   1. Reset the RTC tick counter to clear stale time values
 *   2. Select external 32.768 kHz crystal and enable the clock
 * -------------------------------------------------------------------*/
void RTC_Init(void)
{
    /* Step 1: Reset RTC counter (clear any old time values)
     * CTCRST (bit 1) = 1  ->  resets the internal tick counter
     * The clock is not running yet at this point.               */
    CCR = RTC_CTCRST;

    /* Step 2: Start the RTC with the external 32.768 kHz crystal
     * CLKEN  (bit 0) = 1  ->  enables the RTC and starts counting
     * CLKSRC (bit 4) = 1  ->  selects the external 32.768 kHz
     *                         crystal on RTCX1 / RTCX2 pins
     * CTCRST (bit 1) = 0  ->  releases the counter reset
     *
     * Because we are using the external crystal, the prescaler
     * registers (PREINT, PREFRAC) are ignored by the hardware
     * and do not need to be configured.                         */
    CCR = RTC_CLKEN | RTC_CLKSRC;

    /* RTC is now running! It will count seconds automatically
     * using the 32.768 kHz crystal for accurate timekeeping.    */
}

/* -------------------------------------------------------------------
 * RTC_ReadTime()
 * Reads the current time and date from RTC hardware registers.
 * Stores all values into the RTC_Time structure pointed to by parameter.
 * -------------------------------------------------------------------*/
void RTC_ReadTime(RTC_Time *currentTime)
{
    unsigned char sec1;    /* First read of seconds                     */
    unsigned char sec2;    /* Second read to detect rollover            */

    /* Read individual RTC registers instead of consolidated CTIME.
     * On real LPC2148 hardware, CTIME0/CTIME1 can return stale values
     * if read while the RTC counter is updating internally.
     * Reading individual registers and checking for second rollover
     * ensures we always get a consistent snapshot.                    */

    do {
        sec1 = SEC;                         /* Read seconds first      */
        currentTime->minute = MIN;          /* Read minutes            */
        currentTime->hour   = HOUR;         /* Read hours              */
        currentTime->day    = DOW;          /* Read day of week        */
        currentTime->date   = DOM;          /* Read day of month       */
        currentTime->month  = MONTH;        /* Read month              */
        currentTime->year   = YEAR;         /* Read year               */
        sec2 = SEC;                         /* Re-read seconds         */
    } while (sec1 != sec2);                 /* Retry if second changed */

    currentTime->second = sec1;             /* Use verified seconds    */
}

/* -------------------------------------------------------------------
 * RTC_SetHour()
 * Changes the hour in the RTC.
 * -------------------------------------------------------------------*/
void RTC_SetHour(unsigned char newHour)
{
    HOUR = newHour;    /* Write directly to RTC HOUR register            */
}

/* -------------------------------------------------------------------
 * RTC_SetMinute()
 * Changes the minute in the RTC.
 * -------------------------------------------------------------------*/
void RTC_SetMinute(unsigned char newMinute)
{
    MIN = newMinute;   /* Write directly to RTC MIN register             */
}

/* -------------------------------------------------------------------
 * RTC_SetSecond()
 * Changes the second in the RTC.
 * -------------------------------------------------------------------*/
void RTC_SetSecond(unsigned char newSecond)
{
    SEC = newSecond;   /* Write directly to RTC SEC register             */
}

/* -------------------------------------------------------------------
 * RTC_SetDay()
 * Changes the day of week in the RTC (0=Sunday to 6=Saturday).
 * -------------------------------------------------------------------*/
void RTC_SetDay(unsigned char newDay)
{
    DOW = newDay;      /* Write directly to RTC DOW register             */
}

/* -------------------------------------------------------------------
 * RTC_SetDate()
 * Changes the day of month in the RTC (1 to 31).
 * -------------------------------------------------------------------*/
void RTC_SetDate(unsigned char newDate)
{
    DOM = newDate;     /* Write directly to RTC DOM register             */
}

/* -------------------------------------------------------------------
 * RTC_SetMonth()
 * Changes the month in the RTC (1 to 12).
 * -------------------------------------------------------------------*/
void RTC_SetMonth(unsigned char newMonth)
{
    MONTH = newMonth;  /* Write directly to RTC MONTH register           */
}

/* -------------------------------------------------------------------
 * RTC_SetYear()
 * Changes the year in the RTC.
 * -------------------------------------------------------------------*/
void RTC_SetYear(unsigned int newYear)
{
    YEAR = newYear;    /* Write directly to RTC YEAR register            */
}

/* -------------------------------------------------------------------
 * RTC_GetDayName()
 * Returns the 3-letter name for a day number.
 * Example: RTC_GetDayName(0) returns "SUN"
 * -------------------------------------------------------------------*/
char* RTC_GetDayName(unsigned char dayNumber)
{
    if (dayNumber > 6)
    {
        return "ERR";   /* Invalid day number */
    }
    return dayNames[dayNumber];   /* Return pointer to day name string   */
}

/* -------------------------------------------------------------------
 * RTC_DisplayOnLCD()
 * Displays time, date, and temperature on the 16x2 LCD.
 *
 * LCD Line 1: "HH:MM:SS    XX°C"
 * LCD Line 2: "DD/MM/YYYY   DAY"
 *
 * Example:
 * Line 1: "09:45:30    28°C"
 * Line 2: "15/06/2024   SAT"
 * -------------------------------------------------------------------*/
void RTC_DisplayOnLCD(RTC_Time *timeToShow, unsigned int temperature)
{
    /* --- Display Line 1: Time and Temperature --- */
    LCD_SetCursor(0, 0);    /* Move to Line 1, Column 0                  */

    /* Print time: HH:MM:SS */
    LCD_PrintTwoDigit(timeToShow->hour);     /* HH                       */
    LCD_SendChar(':');                        /* Separator                */
    LCD_PrintTwoDigit(timeToShow->minute);   /* MM                       */
    LCD_SendChar(':');                        /* Separator                */
    LCD_PrintTwoDigit(timeToShow->second);   /* SS                       */

    /* Print spaces then right-aligned temperature: "    XX°C" */
    LCD_SendChar(' ');                        /* Col 8                    */
    LCD_SendChar(' ');                        /* Col 9                    */
    LCD_SendChar(' ');                        /* Col 10                   */
    LCD_SendChar(' ');                        /* Col 11                   */
    LCD_PrintTemp(temperature);               /* Col 12-13: XX            */
    LCD_SendChar(0xDF);                       /* Col 14: Degree symbol °  */
    LCD_SendChar('C');                        /* Col 15: Celsius          */

    /* --- Display Line 2: Date and Day --- */
    LCD_SetCursor(1, 0);    /* Move to Line 2, Column 0                  */

    /* Print date: DD/MM/YYYY */
    LCD_PrintTwoDigit(timeToShow->date);     /* DD                       */
    LCD_SendChar('/');                        /* Separator                */
    LCD_PrintTwoDigit(timeToShow->month);    /* MM                       */
    LCD_SendChar('/');                        /* Separator                */
    LCD_PrintYear(timeToShow->year);         /* YYYY                     */

    /* Print spaces then right-aligned day name */
    LCD_SendChar(' ');                        /* Col 10                   */
    LCD_SendChar(' ');                        /* Col 11                   */
    LCD_SendChar(' ');                        /* Col 12                   */
    LCD_Print(RTC_GetDayName(timeToShow->day));  /* Col 13-15: DAY       */
}
