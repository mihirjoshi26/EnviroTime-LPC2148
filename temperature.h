/*
 * =====================================================================
 * FILE     : temperature.h
 * PURPOSE  : DECLARES functions for reading temperature from LM35.
 *
 * LM35 SENSOR FACTS:
 *   - Gives 10mV per degree Celsius
 *   - At 25 degrees: output = 250mV = 0.25V
 *   - Connected to ADC Channel 1 (P0.28 = AD0.1)
 *   - LPC2148 ADC is 10-bit: 0 to 1023 for 0V to 3.3V
 *
 * FORMULA:
 *   Voltage (mV) = (ADC_value * 3300) / 1023
 *   Temperature  = Voltage / 10    (because 10mV per degree)
 *   Combined: Temperature = (ADC_value * 330) / 1023
 *
 * CONNECTS TO: temperature.c
 * =====================================================================
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/* Initialize ADC for temperature reading - call once at start */
void Temperature_Init(void);

/* Read current temperature from LM35 sensor.
 * Returns temperature as integer in degrees Celsius (e.g., 28 for 28C) */
unsigned int Temperature_ReadCelsius(void);

#endif   /* TEMPERATURE_H */
