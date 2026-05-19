#ifndef TSL237_H
#define TSL237_H

#include "main.h"
#include <stdint.h>

extern volatile uint32_t period;
extern volatile uint32_t tsl237_flag;


extern TIM_HandleTypeDef htim2;

float read_tsl237_frequency(void);
uint32_t read_tsl237_period(void);
void  tsl237_command(char *arguments);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);


#endif
