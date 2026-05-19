/*
 * adc.h
 *
 *  Created on: Feb 24, 2026
 *      Author: Ale K. & Ben G.
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "main.h"

// Typedef for reading temp sensor vals
typedef struct sensorval {
	uint32_t vdda_value;
	uint32_t temperature;
} sensorval_t;

// Read the sensor values and convert to voltage, stored in sensorval struct.
uint32_t readsensor(sensorval_t *sensors);

#endif /* INC_ADC_H_ */
