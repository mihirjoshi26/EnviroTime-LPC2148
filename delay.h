/*
 * =====================================================================
 * FILE     : delay.h
 * PURPOSE  : This file DECLARES (announces) the delay functions.
 *            Any file that wants to use delay must include this file.
 *
 * CONNECTS TO: delay.c (which has the actual delay code)
 * =====================================================================
 */

#ifndef DELAY_H
#define DELAY_H

/*
 * delay_ms()
 * ----------
 * Makes the CPU wait for the given number of milliseconds.
 * Example: delay_ms(1000) waits for 1 second.
 *
 * Parameter: milliseconds - how many ms to wait
 */
void delay_ms(unsigned int milliseconds);

/*
 * delay_us()
 * ----------
 * Makes the CPU wait for the given number of microseconds.
 * Example: delay_us(500) waits for 0.5 milliseconds.
 *
 * Parameter: microseconds - how many us to wait
 */
void delay_us(unsigned int microseconds);

#endif   /* DELAY_H */
