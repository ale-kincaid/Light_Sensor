/*
 * adc.c
 *
 *  Created on: Feb 24, 2026
 *      Author: Ale K. & Ben G.
 */

#include "main.h"
#include "adc.h"

extern ADC_HandleTypeDef hadc1;

uint32_t readsensor(sensorval_t *sensors) {
	// read variables
	uint32_t temp_raw;
	uint32_t vrefint_raw;

	if (!sensors){
		return (-1);
	}

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start(&hadc1);

	// getting temp raw value
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	temp_raw = HAL_ADC_GetValue(&hadc1);

	// getting vref raw value
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	vrefint_raw = HAL_ADC_GetValue(&hadc1);

	HAL_ADC_Stop(&hadc1);

	sensors->vdda_value = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vrefint_raw, ADC_RESOLUTION_12B);
	sensors->temperature = __HAL_ADC_CALC_TEMPERATURE(sensors->vdda_value, temp_raw, ADC_RESOLUTION_12B);

	return(0);
}
