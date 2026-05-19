/*
 * adc.c
 *
 *  Created on: Feb 24, 2026
 *      Author: Ale K. & Ben G.
 */

#include "main.h"
#include "adc.h"
#include <retarget.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern ADC_HandleTypeDef hadc1;

/*
 * Reads one ADC channel using a single conversion.
 *
 * This avoids scan-mode polling issues where the ADC completes the full
 * sequence before the second HAL_ADC_PollForConversion() call.
 */
static HAL_StatusTypeDef ADC_Read_Channel(uint32_t channel, uint32_t *raw_value)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    HAL_StatusTypeDef status;

    if (raw_value == NULL)
    {
        return HAL_ERROR;
    }

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    if (status != HAL_OK)
    {
        printf("HAL_ADC_ConfigChannel failed for channel %lu\r\n", channel);
        return status;
    }

    status = HAL_ADC_Start(&hadc1);
    if (status != HAL_OK)
    {
        printf("HAL_ADC_Start failed for channel %lu\r\n", channel);
        return status;
    }

    status = HAL_ADC_PollForConversion(&hadc1, 1000);
    if (status != HAL_OK)
    {
        printf("HAL_ADC_PollForConversion failed for channel %lu, status=%d, ISR=0x%08lX\r\n",
               channel, status, ADC1->ISR);

        HAL_ADC_Stop(&hadc1);
        return status;
    }

    *raw_value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return HAL_OK;
}


uint32_t readsensor(sensorval_t *sensors)
{
    uint32_t temp_raw = 0;
    uint32_t vrefint_raw = 0;
    HAL_StatusTypeDef status;

    if (sensors == NULL)
    {
        return 1;
    }

    /*
     * Calibration should ideally be done once at startup after MX_ADC1_Init().
     * Keeping it here works, but it is inefficient.
     */
    status = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    if (status != HAL_OK)
    {
        printf("ADC calibration failed, status=%d\r\n", status);
        return 1;
    }

    status = ADC_Read_Channel(ADC_CHANNEL_TEMPSENSOR, &temp_raw);
    if (status != HAL_OK)
    {
        printf("Temperature ADC read failed\r\n");
        return 1;
    }

    status = ADC_Read_Channel(ADC_CHANNEL_VREFINT, &vrefint_raw);
    if (status != HAL_OK)
    {
        printf("VREFINT ADC read failed\r\n");
        return 1;
    }

//    printf("temp_raw = %lu\r\n", temp_raw);
//    printf("vrefint_raw = %lu\r\n", vrefint_raw);

    sensors->vdda_value =
        __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vrefint_raw, ADC_RESOLUTION_12B);

    sensors->temperature =
        __HAL_ADC_CALC_TEMPERATURE(sensors->vdda_value, temp_raw, ADC_RESOLUTION_12B);

//    printf("VDDA = %lu mV\r\n", sensors->vdda_value);
//    printf("Temperature = %ld C\r\n", sensors->temperature);

    return 0;
}
