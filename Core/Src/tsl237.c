/*
 * tsl237.c
 *
 * TSL237 light-to-frequency sensor interface using TIM2 input capture.
 */

#include "main.h"
#include "tsl237.h"
#include <stdio.h>
#include <stdint.h>

extern TIM_HandleTypeDef htim2;

/*
 * Adjust this if your TIM2 tick frequency changes.
 *
 * Your TIM2 prescaler is 79. If the TIM2 clock is 80 MHz:
 *
 * TIM2 tick = 80 MHz / (79 + 1) = 1 MHz
 *
 * Therefore, 1 timer tick = 1 microsecond.
 */
#define TSL237_TIMER_CLOCK_HZ      50000UL

/*
 * Number of valid periods to average.
 * Higher = smoother reading, but slower response.
 */
#define TSL237_NUM_PERIODS         8

/*
 * Reject extremely short periods as glitches.
 *
 * At 1 MHz timer tick:
 * 50 ticks = 50 us = 20 kHz max accepted signal.
 *
 * You can tune this later depending on expected TSL237 range.
 */
#define TSL237_MIN_PERIOD_TICKS    50UL

/*
 * Timeout for waiting on enough captures.
 */
#define TSL237_TIMEOUT_MS          5000UL

typedef enum
{
    FIRST_EDGE = 0,
    NEXT_EDGE  = 1
} capture_state_t;

/*
 * Capture state variables.
 * These are modified in the interrupt callback, so they must be volatile.
 */
static volatile capture_state_t capture_state = FIRST_EDGE;

static volatile uint32_t last_capture = 0;
static volatile uint32_t period_ticks = 0;

static volatile uint32_t period_sum = 0;
static volatile uint32_t valid_period_count = 0;
static volatile uint32_t raw_capture_count = 0;

static volatile uint8_t tsl237_busy = 0;


/*
 * Optional helper: enable the TSL237.
 *
 * If your enable pin is active-low, change GPIO_PIN_SET to GPIO_PIN_RESET.
 */
static void TSL237_Enable(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(10);
}


/*
 * Reset all capture state before starting a new reading.
 */
static void TSL237_ResetCaptureState(void)
{
    capture_state = FIRST_EDGE;

    last_capture = 0;
    period_ticks = 0;

    period_sum = 0;
    valid_period_count = 0;
    raw_capture_count = 0;

    tsl237_busy = 1;
}


/*
 * TIM input-capture callback.
 *
 * This is called by HAL_TIM_IRQHandler(&htim2) when TIM2_CH1 captures an edge.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    uint32_t current;
    uint32_t delta;

    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        raw_capture_count++;

        current = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        if (capture_state == FIRST_EDGE)
        {
            last_capture = current;
            capture_state = NEXT_EDGE;
        }
        else
        {
            if (current >= last_capture)
            {
                delta = current - last_capture;
            }
            else
            {
                delta = (__HAL_TIM_GET_AUTORELOAD(htim) - last_capture) + current + 1;
            }

            /*
             * Prepare for the next period measurement.
             * This lets us average consecutive periods.
             */
            last_capture = current;

            /*
             * Reject tiny periods caused by noise/glitches.
             */
            if (delta >= TSL237_MIN_PERIOD_TICKS)
            {
                period_sum += delta;
                valid_period_count++;

                if (valid_period_count >= TSL237_NUM_PERIODS)
                {
                    period_ticks = period_sum / TSL237_NUM_PERIODS;

                    tsl237_busy = 0;
                    capture_state = FIRST_EDGE;

                    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
                }
            }
        }
    }
}


/*
 * Read TSL237 period in timer ticks.
 *
 * If TIM2 is configured for 1 MHz, returned ticks are microseconds.
 *
 * Returns:
 *   averaged period in timer ticks
 *   0 on timeout/failure
 */
uint32_t read_tsl237_period(void)
{
    uint32_t start;

    TSL237_Enable();
    TSL237_ResetCaptureState();

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

    if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
    {
        printf("TIM2 IC start failed\r\n");
        tsl237_busy = 0;
        return 0;
    }

    start = HAL_GetTick();

    while (tsl237_busy)
    {
        if ((HAL_GetTick() - start) > TSL237_TIMEOUT_MS)
        {
            printf("TSL237 timeout\r\n");
            printf("raw captures=%lu valid periods=%lu period=%lu\r\n",
                   raw_capture_count,
                   valid_period_count,
                   period_ticks);

            printf("CR1=0x%08lX DIER=0x%08lX SR=0x%08lX CNT=%lu CCR1=%lu\r\n",
                   TIM2->CR1,
                   TIM2->DIER,
                   TIM2->SR,
                   TIM2->CNT,
                   TIM2->CCR1);

            HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);

            tsl237_busy = 0;
            capture_state = FIRST_EDGE;

            return 0;
        }
    }

    return period_ticks;
}


/*
 * Read TSL237 frequency in Hz.
 *
 * Returns:
 *   frequency in Hz
 *   0.0f on timeout/failure
 */
float read_tsl237_frequency(void)
{
    uint32_t period = read_tsl237_period();

    if (period == 0)
    {
        return 0.0f;
    }

    return (float)TSL237_TIMER_CLOCK_HZ / (float)period;
}


/*
 * Terminal command handler.
 *
 * Example command:
 *   tsl237
 */
void tsl237_command(char *arguments)
{
    float freq;
    uint32_t freq_int;
    uint32_t freq_decimal;

    if (arguments != NULL && arguments[0] != '\0')
    {
        printf("NOK\r\n");
        return;
    }

    freq = read_tsl237_frequency();

    if (freq <= 0.0f)
    {
        printf("0.00 Hz\r\n");
        printf("NOK\r\n");
        return;
    }

    /*
     * Integer-style float printing.
     * Avoids needing -u _printf_float.
     */
    freq_int = (uint32_t)freq;
    freq_decimal = (uint32_t)((freq - (float)freq_int) * 100.0f);

    printf("captures=%lu valid=%lu period=%lu ticks\r\n",
           raw_capture_count,
           valid_period_count,
           period_ticks);

    printf("%lu.%02lu Hz\r\n", freq_int, freq_decimal);
    printf("OK\r\n");
}
