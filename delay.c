/*
 * =====================================================================
 * FILE     : delay.c
 * PURPOSE  : Provides simple busy-wait delay functions.
 *            The CPU keeps running in a loop to waste time.
 *
 * CLOCK INFO:
 *   Crystal = 12 MHz
 *   CPU Clock (CCLK) = 12 MHz x 5 (PLL) = 60 MHz
 *   Each loop iteration takes about 5 CPU cycles
 *   So 1 microsecond = 60 cycles / 5 = 12 loop iterations
 *   So 1 millisecond = 12 x 1000 = 12000 loop iterations
 *
 * NOTE: Compile at Optimization Level 0 in Keil (no optimization).
 *       Higher optimization may remove the empty loop!
 * =====================================================================
 */

#include "delay.h"   /* Include our own header */

/* -------------------------------------------------------------------
 * delay_ms()
 * Makes CPU wait for 'milliseconds' number of milliseconds.
 * 
 * How: 1 ms = 12000 iterations of the loop
 *      So if you want 5 ms, loop runs 5 x 12000 = 60000 times
 * -------------------------------------------------------------------*/
void delay_ms(unsigned int milliseconds)
{
    unsigned int loopCount;   /* Counter variable for the loop */

    /* Calculate total loop iterations needed */
    loopCount = milliseconds * 12000;

    /* Run empty loop - CPU just counts down, wasting time */
    while (loopCount > 0)
    {
        loopCount--;   /* Decrease counter each iteration */
    }
}

/* -------------------------------------------------------------------
 * delay_us()
 * Makes CPU wait for 'microseconds' number of microseconds.
 *
 * How: 1 us = 12 iterations of the loop
 *      So if you want 100 us, loop runs 100 x 12 = 1200 times
 * -------------------------------------------------------------------*/
void delay_us(unsigned int microseconds)
{
    unsigned int loopCount;   /* Counter variable */

    /* Calculate total loop iterations needed */
    loopCount = microseconds * 12;

    /* Run empty loop to waste exact amount of time */
    while (loopCount > 0)
    {
        loopCount--;
    }
}
