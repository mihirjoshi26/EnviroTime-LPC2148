/*
 * =====================================================================
 * FILE     : main.c
 * PURPOSE  : Main program - this is where everything comes together.
 *            Controls the overall flow of the EnviroTime project.
 *
 * PROGRAM FLOW:
 *   1. Initialize all hardware (LCD, RTC, Keypad, ADC, Buzzer, Switch)
 *   2. Show startup message
 *   3. Enter infinite main loop:
 *      a. Read RTC time and date
 *      b. Read temperature from LM35
 *      c. Display both on LCD
 *      d. Check if alarm time matches -> if yes, ring buzzer
 *      e. Check if EDIT SWITCH pressed -> if yes, show main menu
 *      f. Repeat from (a)
 *
 * MAIN MENU (when EDIT SWITCH pressed):
 *   Key 1 = RTC Edit   (set time/date with password)
 *   Key 2 = Alarm Menu (view/manage multiple alarms with password)
 *           -> View:   read-only browse with Prev/Next/Exit
 *           -> Manage: Add / Edit / Delete alarms
 *   Key 3 = Pwd Edit   (change password)
 *   Key 4 = Exit       (go back to normal display)
 *
 * HOW FILES CONNECT:
 *   main.c  -> alarm.h, menu.h, lcd.h, keypad.h, rtc.h, temperature.h
 *   alarm.c -> menu.h (for GetValidInput)
 *   menu.c  -> alarm.h (for HandleAlarmMenu), password.h, rtc.h
 *   All module files use: pin_config.h, delay.h
 * =====================================================================
 */

#include <lpc214x.h>      /* LPC2148 hardware registers                 */
#include "pin_config.h"   /* All pin numbers and constants              */
#include "delay.h"        /* delay_ms() function                        */
#include "lcd.h"          /* All LCD functions                          */
#include "keypad.h"       /* Keypad reading functions                    */
#include "rtc.h"          /* RTC read/write/display functions           */
#include "temperature.h"  /* LM35 temperature reading                   */
#include "password.h"     /* Password verify and change functions       */
#include "alarm.h"        /* Alarm data and alarm menu functions        */
#include "menu.h"         /* Main menu, RTC edit, password menu, input  */

/* =================================================================== */
/*  FUNCTION DECLARATIONS (prototypes for functions in this file)       */
/* =================================================================== */
void PLL_Init(void);
void System_Init(void);
void ShowStartupMessage(void);
unsigned char IsEditSwitchPressed(void);
unsigned char IsAlarmSwitchPressed(void);

/* =================================================================== */
/*  MAIN FUNCTION                                                       */
/* =================================================================== */
int main(void)
{
    RTC_Time currentTime;        /* Holds current time/date from RTC    */
    unsigned int temperature;    /* Current temperature in Celsius      */
    char menuKey;                /* Key pressed in main menu            */
    unsigned char lastSecond;    /* Tracks previous second for change   */
    unsigned char forceRefresh;  /* Flag to force LCD update            */

    /* Step 1: Initialize all hardware */
    System_Init();

    /* Step 2: Show startup message briefly */
    ShowStartupMessage();

    /* Step 3: Initialize tracking variables */
    lastSecond = 0xFF;           /* Invalid value to force first update */
    forceRefresh = 1;            /* Force update on first iteration     */

    /* Step 4: Infinite loop - the system runs forever */
    while (1)
    {
        /* --- A: Read current time from RTC --- */
        RTC_ReadTime(&currentTime);

        /* --- B: Only update LCD when second has changed --- */
        /* This prevents constant LCD rewrites that cause the
           display to appear frozen on real hardware.         */
        if (currentTime.second != lastSecond || forceRefresh == 1)
        {
            /* Read temperature from LM35 sensor */
            temperature = Temperature_ReadCelsius();

            /* Display everything on LCD */
            RTC_DisplayOnLCD(&currentTime, temperature);

            /* Remember this second so we don't redraw until it changes */
            lastSecond = currentTime.second;
            forceRefresh = 0;
        }

        /* --- C: Check if alarm should ring --- */
        CheckAndRingAlarm(&currentTime);

        /* --- D: Check if EDIT SWITCH is pressed --- */
        if (IsEditSwitchPressed() == 1)
        {
            /* EDIT SWITCH was pressed - enter menu loop */
            /* Stay in menu until user presses '4' (Exit) */
            while (1)
            {
                ShowMainMenu();
                menuKey = Keypad_GetKey();

                if (menuKey == '1')
                {
                    /* Option 1: RTC Edit */
                    if (Password_CheckWithLockout() == 1)
                    {
                        HandleRTCEdit();
                    }
                }
                else if (menuKey == '2')
                {
                    /* Option 2: Alarm Menu (View / Manage) */
                    if (Password_CheckWithLockout() == 1)
                    {
                        HandleAlarmMenu();
                    }
                }
                else if (menuKey == '3')
                {
                    /* Option 3: Password Menu */
                    HandlePasswordMenu();
                }
                else if (menuKey == '4')
                {
                    /* Option 4: Exit to home screen */
                    LCD_Clear();
                    break;
                }
                else
                {
                    /* Invalid key: show message, stay in menu */
                    LCD_ShowInvalidKey();
                }
            }

            /* After exiting menu, force a display refresh */
            lastSecond = 0xFF;
            forceRefresh = 1;
        }

        /* Short polling delay - just enough to prevent APB bus overload */
        delay_ms(50);

    }  /* End of while(1) - never exits */
}

/* =================================================================== */
/*  SYSTEM INITIALIZATION                                               */
/* =================================================================== */

/* -------------------------------------------------------------------
 * PLL_Init()
 * Configures Phase Locked Loop (PLL0) to generate 60 MHz CPU clock
 * from the 12 MHz external crystal. (M=5, P=2)
 * VPBDIV remains default 0x00, so PCLK = CCLK/4 = 15 MHz.
 * -------------------------------------------------------------------*/
void PLL_Init(void)
{
    PLL0CON = 0x01;           /* Enable PLL0                     */
    PLL0CFG = 0x24;           /* Multiplier M=5, Divider P=2     */
    PLL0FEED = 0xAA;          /* Feed sequence to lock PLL       */
    PLL0FEED = 0x55;
    
    while (!(PLL0STAT & 0x0400)) /* Wait for lock (PLOCK bit 10) */
    {
        /* wait */
    }
    
    PLL0CON = 0x03;           /* Enable and connect PLL0         */
    PLL0FEED = 0xAA;          /* Feed sequence to connect        */
    PLL0FEED = 0x55;
}

/* -------------------------------------------------------------------
 * System_Init()
 * Initializes ALL hardware peripherals in the correct order.
 * -------------------------------------------------------------------*/
void System_Init(void)
{
    /* Step 0: Initialize PLL for 60 MHz CCLK */
    PLL_Init();

    /* Step 1: Configure EDIT SWITCH pin (P0.10) as INPUT */
    IODIR0 &= ~(1 << EDIT_SWITCH_BIT);   /* Clear bit = INPUT          */

    /* Step 2: Configure ALARM SWITCH pin (P0.11) as INPUT */
    IODIR0 &= ~(1 << ALARM_SWITCH_BIT);  /* Clear bit = INPUT          */

    /* Force P0.10 and P0.11 to GPIO mode via PINSEL0 register.
     * PINSEL0 controls pin functions for P0.0 to P0.15.
     * P0.10 is bits [21:20], P0.11 is bits [23:22] of PINSEL0.
     * Clearing these bits to 00 selects standard GPIO function,
     * ensuring the switches work regardless of default PINSEL state. */
    PINSEL0 &= ~(3 << 20);  /* P0.10 = GPIO */
    PINSEL0 &= ~(3 << 22);  /* P0.11 = GPIO */

    /* Step 3: Configure BUZZER pin (P0.12) as OUTPUT and turn OFF */
    IODIR0 |= (1 << BUZZER_BIT);         /* Set bit = OUTPUT           */
    IOCLR0  = (1 << BUZZER_BIT);         /* Drive LOW = Buzzer OFF     */

    /* Step 4: Initialize LCD (must be done early) */
    LCD_Init();

    /* Step 5: Initialize Keypad */
    Keypad_Init();

    /* Step 6: Initialize RTC */
    RTC_Init();

    /* Step 7: Initialize Temperature/ADC */
    Temperature_Init();

    /* Step 8: Initialize Password with default "1234" */
    Password_Init();

    /* Optional: Set initial RTC time if needed (first time only)
     * In a real project, you would check if RTC is already running.
     * For now, we set a default starting time. */
    RTC_SetHour(12);     /* Start at 12:00:00  */
    RTC_SetMinute(0);
    RTC_SetSecond(0);
    RTC_SetDate(3);      /* 3rd May 2026   */
    RTC_SetMonth(5);
    RTC_SetYear(2026);
    RTC_SetDay(1);       /* Monday             */
}

/* -------------------------------------------------------------------
 * ShowStartupMessage()
 * Shows the project name on LCD for 2 seconds when power is turned on.
 * -------------------------------------------------------------------*/
void ShowStartupMessage(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("  EnviroTime    ");   /* Project name on Line 1          */
    LCD_SetCursor(1, 0);
    LCD_Print(" Initializing.. ");   /* Status on Line 2                */
    delay_ms(2000);                  /* Show for 2 seconds              */
    LCD_Clear();
}

/* =================================================================== */
/*  SWITCH POLLING FUNCTIONS                                            */
/* =================================================================== */

/* -------------------------------------------------------------------
 * IsEditSwitchPressed()
 * Checks if the EDIT switch (P0.10) is being pressed.
 * Switch is ACTIVE LOW: pressed = pin reads 0, not pressed = reads 1
 *
 * Returns: 1 if pressed, 0 if not pressed
 * -------------------------------------------------------------------*/
unsigned char IsEditSwitchPressed(void)
{
    /* Read P0.10: if bit is 0, switch is pressed */
    if (((IOPIN0 >> EDIT_SWITCH_BIT) & 1) == 0)
    {
        delay_ms(20);   /* Debounce: wait 20ms and check again          */

        /* Confirm it's still pressed (not just electrical noise) */
        if (((IOPIN0 >> EDIT_SWITCH_BIT) & 1) == 0)
        {
            /* Wait until switch is released before returning */
            while (((IOPIN0 >> EDIT_SWITCH_BIT) & 1) == 0)
            {
                /* Keep waiting - user might still be pressing */
            }
            return 1;   /* Switch was pressed                           */
        }
    }
    return 0;   /* Switch was not pressed                               */
}

/* -------------------------------------------------------------------
 * IsAlarmSwitchPressed()
 * Checks if the ALARM STOP switch (P0.11) is being pressed.
 * Returns: 1 if pressed, 0 if not pressed
 * -------------------------------------------------------------------*/
unsigned char IsAlarmSwitchPressed(void)
{
    if (((IOPIN0 >> ALARM_SWITCH_BIT) & 1) == 0)
    {
        delay_ms(20);   /* Debounce */
        if (((IOPIN0 >> ALARM_SWITCH_BIT) & 1) == 0)
        {
            return 1;   /* Alarm switch pressed */
        }
    }
    return 0;
}

/* =================================================================== */
/*  END OF main.c                                                       */
/* =================================================================== */
