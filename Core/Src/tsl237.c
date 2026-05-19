#include "main.h"
#include <stdio.h>
#include <string.h>
#include "tsl237.h"

volatile uint32_t period;
volatile uint32_t tsl237_flag = 0;
extern TIM_HandleTypeDef htim2;

enum states {FIRST_EDGE, SECOND_EDGE};

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	static uint32_t last = 0;
	static uint32_t state = FIRST_EDGE;
	uint32_t current;

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		switch (state) {
		case FIRST_EDGE:
			if (tsl237_flag) {
				last = htim->Instance->CCR1;
				state = SECOND_EDGE;
			}
			break;
		case SECOND_EDGE:
			current = htim->Instance->CCR1;
			if (last <= current) {
				period = current - last;
			}
			else {
				period = TIM2->ARR - last + current;
			}
			tsl237_flag = 0;
			HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
			state=FIRST_EDGE;
			break;
		default:
			state=FIRST_EDGE;
		}
	}
}

float read_tsl237_frequency(void) {
	float sensor_period;
	tsl237_flag = 1;
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	while (tsl237_flag);
	sensor_period = (1/(float)SystemCoreClock) * (float) period;
	return (1/sensor_period);
}

uint32_t read_tsl237_period(void) {
	uint32_t sensor_period;
	tsl237_flag = 1;
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	while (tsl237_flag);
	sensor_period = (1/SystemCoreClock) * period;
	return period;
}

void tsl237_command(char *arguments) {
	if (arguments != NULL && arguments[0] != '\0') {
		printf("NOK\r\n");
	}

	float freq = read_tsl237_frequency();
	printf("%.2f hz\r\n", freq);
	printf("OK\r\n");
}
