/*
 * sample.c
 *
 *  Created on: Mar 1, 2026
 *      Author: Ale K. & Ben G.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "flash.h"
#include "adc.h"
#include "tsl237.h"

extern RTC_HandleTypeDef hrtc;

void log_func(const char *message) {
	logdata_t ourlog = {0};
	flash_status_t fs1;
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	uint32_t time_stamp;

	if (message == NULL) {
		return;
	}

	if (flash_write_init(&fs1) != 0) {
			return;
		}

	HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
	time_stamp = pack_time(&current_time, &current_date);

	ourlog.watermark = 0x01;
	ourlog.status = 0x02;
	ourlog.record_number = fs1.next_record_number;
	ourlog.timestamp = time_stamp;

	memset(ourlog.msg, 0, sizeof(ourlog.msg));
	strncpy((char *)ourlog.msg, message, sizeof(ourlog.msg));

	if (write_record(&fs1, &ourlog) != 0) {
		return;
	}
}

void log_command(char *arguments) {
	if (arguments) {
		printf("NOK\n\r");
		return;
	}
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
	uint64_t *top = (uint64_t *) FLASH_FINISH;
	logdata_t *ld = (logdata_t *) top;
	ld--;
	while ((ld->watermark!=0xFF) && (ld->watermark!=0xa5)) {
		if (ld->status == 0x02) {
			unpack_time(ld->timestamp, &current_time, &current_date);

			printf("L,%d,%02d/%02d/20%02d,%02d:%02d:%02d,%.8s\n\r",
					ld->record_number,
					current_date.Month,
					current_date.Date,
					current_date.Year,
					current_time.Hours,
					current_time.Minutes,
					current_time.Seconds,
					ld->msg);
		}
		ld--;
	}
	printf("OK\n\r");

}

void data_command(char *arguments){
	if (arguments){
		printf("NOK\n\r");
	}
	else{
		// time setup
		RTC_TimeTypeDef current_time;
		RTC_DateTypeDef current_date;

		HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
		// first getting the starting address of flash records
		uint64_t *top = (uint64_t *) FLASH_FINISH;
		sensordata_t *sd = (sensordata_t *) top;
		sd--; // pointing to first data record
		// iterating through potential saved records in flash by looking at what's written there in mem
		while ((sd->watermark!=0xFF)&&(sd->watermark!=0xa5)) {
			// printing out the vals at that point
			// getting unpack time
			if (sd->status == 0x01) {
				unpack_time(sd->timestamp, &current_time, &current_date);
				// D, Number, Date, Time, Battery, Temperature, Sensor Period
				printf("D,%d,%02d/%02d/20%02d,%02d:%02d:%02d,%d,%d, %lu\n\r",
						sd->record_number, current_date.Month, current_date.Date, current_date.Year,
						current_time.Hours, current_time.Minutes, current_time.Seconds,
						sd->battery_voltage, sd->temperature, sd->sensor_period);
			}
			sd--;
		}
		printf("OK\n\r");
	}
}

void sample(void) {
	sensordata_t our_record = {0};
	// todo: Get sensor data, date, time
	uint8_t water_mark = 0x01;
	uint8_t status_sample  = 0x01;
	sensorval_t my_sensor;
	uint32_t read_status;
	uint32_t sensor_period = read_tsl237_period();
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	flash_status_t fs1;

	HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);

	uint32_t time_stamp = pack_time(&current_time, &current_date);
	if (flash_write_init(&fs1) != 0) {
			printf("Could not initialize flash\n\r");
			return;
	}

	read_status = readsensor(&my_sensor);
	if (read_status != 0){
		printf("NOK\n\r");
		return;
	}

	// write battery and temp
	our_record.watermark = water_mark;
	our_record.status = status_sample;
	our_record.battery_voltage = (uint16_t)my_sensor.vdda_value / 1000;
	our_record.temperature = (int16_t)my_sensor.temperature;
	our_record.sensor_period = sensor_period;
	our_record.timestamp = time_stamp;
	our_record.record_number = fs1.next_record_number;

	// then todo: write with write record command from flash
	if (write_record(&fs1, &our_record) != 0) {
		printf("Could not write record\n\r");
		return;
	}
	log_func("test");
	printf("OK\n\r");
}
void sample_command(char *arguments){
	// Check that no args are passed too
	if (arguments){
		printf("NOK\n\r");
	}
	// Else do sampling
	else{
		sample();
	}
}


