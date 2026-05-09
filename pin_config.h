/*
 * =====================================================================
 * FILE     : pin_config.h
 * PURPOSE  : This file contains ALL pin numbers used in this project.
 *            If you want to change any hardware connection, just
 *            change the number here. No need to search other files.
 *
 * HOW TO USE: #include "pin_config.h" in every file that   needs pins.
 * =====================================================================
 *
 * HARDWARE CONNECTIONS SUMMARY:
 * ------------------------------
 * LCD (8-bit mode connected to Port 0):
 *   P0.0  to P0.7 = LCD Data pins D0 to D7
 *   P0.8  = LCD RS  (Register Select)
 *   P0.9  = LCD EN  (Enable)
 *   (LCD RW pin should be tied LOW to Ground)
 *
 * KEYPAD (4x4 matrix connected to Port 1):
 *   P1.20 = ROW0, P1.21 = ROW1, P1.22 = ROW2, P1.23 = ROW3
 *   P1.16 = COL0, P1.17 = COL1, P1.18 = COL2, P1.19 = COL3
 *
 * OTHER PINS (Port 0):
 *   P0.10 = EDIT SWITCH  (Press to enter menu)
 *   P0.11 = ALARM SWITCH (Press to stop alarm buzzer)
 *   P0.12 = BUZZER       (HIGH = buzzer ON)
 *   P0.28 = LM35 sensor  (ADC Channel 1 - analog input)
 * =====================================================================
 */

#ifndef PIN_CONFIG_H      /* Guard: prevents this file being included twice */
#define PIN_CONFIG_H

#include <lpc214x.h>      /* LPC2148 register definitions (IODIR0, etc.) */

/* ------------------------------------------------------------------- */
/*  LCD PIN DEFINITIONS  (Port 0)                                       */
/* ------------------------------------------------------------------- */
#define LCD_DATA_START_BIT   0    /* P0.0 = start of 8-bit data bus (D0 to D7) */
#define LCD_RS_BIT           8    /* P0.8 = Register Select pin         */
#define LCD_EN_BIT           9    /* P0.9 = Enable pin                  */

/* ------------------------------------------------------------------- */
/*  KEYPAD PIN DEFINITIONS  (Port 1)                                    */
/* ------------------------------------------------------------------- */
#define KEYPAD_ROW0         20    /* P1.20 = Keypad Row 0 (top row)      */
#define KEYPAD_ROW1         21    /* P1.21 = Keypad Row 1                */
#define KEYPAD_ROW2         22    /* P1.22 = Keypad Row 2                */
#define KEYPAD_ROW3         23    /* P1.23 = Keypad Row 3 (bottom row)   */
#define KEYPAD_COL0         16    /* P1.16 = Keypad Column 0 (leftmost)  */
#define KEYPAD_COL1         17    /* P1.17 = Keypad Column 1             */
#define KEYPAD_COL2         18    /* P1.18 = Keypad Column 2             */
#define KEYPAD_COL3         19    /* P1.19 = Keypad Column 3 (rightmost) */

/* ------------------------------------------------------------------- */
/*  SWITCH AND BUZZER PIN DEFINITIONS  (Port 0)                         */
/* ------------------------------------------------------------------- */
#define EDIT_SWITCH_BIT     10    /* P0.10 = Edit switch (active LOW)    */
#define ALARM_SWITCH_BIT    11    /* P0.11 = Alarm stop switch (active LOW)*/
#define BUZZER_BIT          12    /* P0.12 = Buzzer control pin          */

/* ------------------------------------------------------------------- */
/*  ADC / LM35 TEMPERATURE SENSOR                                       */
/* ------------------------------------------------------------------- */
#define LM35_ADC_CHANNEL     1    /* ADC Channel 1 = P0.28 = AD0.1      */
/*
 * LM35 formula:
 *   ADC gives 0-1023 for 0V to 3.3V
 *   LM35 gives 10mV per degree Celsius
 *   Temperature = (ADC_value * 3300) / (1023 * 10)
 *   Simplified = (ADC_value * 330) / 1023
 */

/* ------------------------------------------------------------------- */
/*  PASSWORD SETTINGS                                                    */
/* ------------------------------------------------------------------- */
#define PASSWORD_LENGTH      4    /* Password is 4 digits                */
#define MAX_WRONG_TRIES      3    /* Lock after 3 wrong attempts         */
#define LOCKOUT_TIME_MS  30000    /* Lock for 30 seconds = 30000 ms      */

/* ------------------------------------------------------------------- */
/*  LCD COMMAND CODES  (HD44780 standard)                               */
/* ------------------------------------------------------------------- */
#define LCD_CLEAR           0x01  /* Clear display, cursor goes to home  */
#define LCD_HOME            0x02  /* Move cursor to position (0,0)       */
#define LCD_8BIT_2LINE      0x38  /* 8-bit mode, 2 lines, 5x8 font      */
#define LCD_DISPLAY_ON      0x0C  /* Display ON, cursor OFF              */
#define LCD_ENTRY_MODE      0x06  /* Cursor moves right after each char  */
#define LCD_LINE1           0x80  /* DDRAM address of Line 1, Column 0  */
#define LCD_LINE2           0xC0  /* DDRAM address of Line 2, Column 0  */

/* ------------------------------------------------------------------- */
/*  RTC CCR REGISTER BITS                                               */
/* ------------------------------------------------------------------- */
#define RTC_CLKEN           0x01  /* Bit 0: Start RTC counting           */
#define RTC_CTCRST          0x02  /* Bit 1: Reset RTC counter            */
#define RTC_CLKSRC          0x10  /* Bit 4: Use external 32.768kHz xtal  */

/* ------------------------------------------------------------------- */
/*  ADC REGISTER BITS                                                   */
/* ------------------------------------------------------------------- */
#define ADC_PDN_BIT         21    /* Bit 21 of AD0CR: power on ADC       */
#define ADC_START_BIT       24    /* Bit 24 of AD0CR: start conversion   */
#define ADC_DONE_BIT        31    /* Bit 31 of AD0GDR: conversion done   */
#define ADC_RESULT_SHIFT     6    /* Result is in bits [15:6] of AD0GDR  */
#define ADC_RESULT_MASK   1023    /* 10-bit mask (0x3FF)                 */

/* ------------------------------------------------------------------- */
/*  SIMPLE TRUE/FALSE DEFINITIONS                                       */
/* ------------------------------------------------------------------- */
#define TRUE                 1    /* Used for yes/correct/success        */
#define FALSE                0    /* Used for no/wrong/failure           */
#define CORRECT              1    /* Password matched                    */
#define WRONG                0    /* Password did not match              */

#endif   /* PIN_CONFIG_H */
