/*
 * =====================================================================
 * FILE     : temperature.c
 * PURPOSE  : Reads temperature from LM35 sensor via ADC.
 *
 * ADC STEPS TO READ:
 *   1. Configure P0.28 as analog input (not GPIO) via PINSEL1
 *   2. Power on ADC via PCONP register
 *   3. Set ADC clock divider (ADC clock must be less than 4.5 MHz)
 *   4. Select channel 1, start conversion
 *   5. Wait for DONE bit in AD0GDR to become 1
 *   6. Read 10-bit result from AD0GDR bits [15:6]
 *   7. Convert raw value to temperature using formula
 *
 * ADC CLOCK CALCULATION:
 *   PCLK = 15 MHz (peripheral clock)
 *   We want ADC clock = 3 MHz
 *   CLKDIV = (PCLK / ADC_CLK) - 1 = (15/3) - 1 = 4
 * =====================================================================
 */

#include <lpc214x.h>      /* LPC2148 registers (AD0CR, AD0GDR, etc.)    */
#include "pin_config.h"   /* ADC_PDN_BIT, ADC_START_BIT, etc.           */
#include "delay.h"        /* For short delay after starting conversion   */
#include "temperature.h"  /* Our own header                             */

/* -------------------------------------------------------------------
 * Temperature_Init()
 * Configures the ADC to read from the LM35 sensor on P0.28 (AD0.1).
 *
 * Three things to configure:
 * 1. PINSEL1: Tell P0.28 to work as AD0.1 (not as GPIO)
 * 2. PCONP:   Turn on ADC peripheral power
 * 3. AD0CR:   Set ADC clock speed and power on ADC
 * -------------------------------------------------------------------*/
void Temperature_Init(void)
{
    /* Step 1: Configure P0.28 as AD0.1 (analog input)
     * PINSEL1 controls pins P0.16 to P0.31
     * P0.28 is bits [25:24] of PINSEL1
     * Setting bits [25:24] = 01 makes P0.28 = AD0.1 */
    PINSEL1 &= ~(0x3 << 24);   /* Clear bits 25:24 first               */
    PINSEL1 |=  (0x1 << 24);   /* Set to 01 = AD0.1 function           */

    /* Step 2: Enable ADC power in PCONP register
     * Bit 12 of PCONP controls ADC power */
    PCONP |= (1 << 12);        /* Bit 12 = 1: Turn on ADC power        */

    /* Step 3: Configure ADC0 control register (AD0CR)
     * Bits [7:0]  = channel select: bit 1 = channel 1 (AD0.1)
     * Bits [15:8] = CLKDIV: set to 4 (PCLK=15MHz, ADC=3MHz)
     * Bit 21      = PDN: Power Down = 0 means ACTIVE (powered on)
     *
     * AD0CR = (channel 1) | (clock divider 4) | (power on) */
    AD0CR = (1 << 1)            /* Select channel 1 (AD0.1)             */
          | (4 << 8)            /* CLKDIV = 4 -> ADC clock = 3MHz       */
          | (1 << ADC_PDN_BIT); /* PDN = 1 -> ADC is powered and active */
}

/* -------------------------------------------------------------------
 * Temperature_ReadCelsius()
 * Performs one ADC conversion and returns temperature in Celsius.
 *
 * Returns: temperature as integer (e.g., 28 for 28 degrees Celsius)
 * -------------------------------------------------------------------*/
unsigned int Temperature_ReadCelsius(void)
{
    unsigned int adcRawValue;       /* Raw 10-bit ADC result (0-1023)    */
    unsigned int temperatureCelsius; /* Calculated temperature           */

    /* Step 1: Start ADC conversion on channel 1
     * Setting bits [26:24] = 001 in AD0CR means "start now" */
    AD0CR |= (1 << ADC_START_BIT);  /* Set START bits to begin conversion */

    /* Step 2: Wait for conversion to complete
     * Bit 31 (DONE bit) of AD0GDR becomes 1 when conversion is done */
    while (((AD0GDR >> ADC_DONE_BIT) & 1) == 0)
    {
        /* Keep waiting in this loop until DONE bit = 1 */
    }

    /* Step 3: Stop the conversion trigger
     * Clear all 3 START bits [26:24] using a 0x7 mask */
    AD0CR &= ~(0x7 << ADC_START_BIT);

    /* Step 4: Extract 10-bit result from AD0GDR
     * Result is in bits [15:6], so shift right by 6 and mask with 0x3FF */
    adcRawValue = (AD0GDR >> ADC_RESULT_SHIFT) & ADC_RESULT_MASK;

    /* Step 5: Convert raw ADC value to temperature in Celsius
     * Formula: temp = (raw * 330) / 1023
     *
     * Derivation:
     *   Voltage (mV) = raw * 3300 / 1023
     *   Temperature (C) = Voltage / 10
     *   Combined: temp = raw * 3300 / (1023 * 10) = raw * 330 / 1023 */
    temperatureCelsius = (adcRawValue * 330) / 1023;

    return temperatureCelsius;   /* Return temperature as integer        */
}
