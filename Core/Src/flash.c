/*
 * flash.c
 *
 *  Created on: Feb 28, 2026
 *      Author: Ale K. & Ben G.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "flash.h"
#include "rtc.h"
#include "adc.h"
#include "tsl237.h"

extern RTC_HandleTypeDef hrtc;

// Linker (LD) script externs
// Starts init data in RAM
extern int _sdata;
// End of init data in RAM
extern int _edata;
extern int __fini_array_end;

uint16_t g_record_number = 0;

uint64_t* find_sentinel_bottom(void) {
	uint64_t *p = (uint64_t *)&__fini_array_end + ((uint64_t *) &_edata - (uint64_t *)&_sdata);
	p += 2; // increment pointer by 16
	p = (uint64_t *) ((uintptr_t) p & ~(uintptr_t)0xF);
	while (p <= ((uint64_t*)FLASH_FINISH)) {
		if (*p == SENTINEL_MARK_BOTTOM) {
			return (p); // return address of sentinel
		}
		p += 2; // moves 16 bytes each iteration
	}
	return(0); // if not located, return null pointer
}

uint64_t* find_sentinel_top(void) {
	uint64_t *p = (uint64_t *) FLASH_FINISH; // sets pointer to start of flash address

	if (*p == SENTINEL_MARK_TOP) {
		return (p);
	}
	else {
		return (0);
	}
}

int write_sentinel(uint64_t *pos, raw_t *sentinel) {
	if (!pos || !sentinel) {
		return (-1);
	}
	HAL_FLASH_Unlock();
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uintptr_t)pos, sentinel->data0) != HAL_OK){
			HAL_FLASH_Lock();
			return (-1);
		}
	pos++;
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uintptr_t)pos, sentinel->data1) != HAL_OK){
			HAL_FLASH_Lock();
			return (-1);
		}
	HAL_FLASH_Lock();
	return (0);
}


int write_record(flash_status_t *fs, void *record){
	raw_t *write_data;

	if ((!fs) || (!record)){
		return (-1);
	}

	if (fs->total_records >= fs->max_possible_records){
		printf("FLASH IS FULL!\n\r");
		return (-1);
	}

	// This is likely where we could see a hard fault
	write_data = (raw_t *) record;

	HAL_FLASH_Unlock();
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uintptr_t) fs->next_address, write_data->data0) != HAL_OK){
		HAL_FLASH_Lock();
		return (-1);
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uintptr_t) (fs->next_address + 1), write_data->data1) != HAL_OK){
		HAL_FLASH_Lock();
		return (-1);
	}

	HAL_FLASH_Lock();
	fs->next_address-=2;
	fs->next_record_number++;
	fs->total_records++;
	return (0);
}

// inits flash struct
int flash_write_init(flash_status_t *fs){
	sensordata_t *sd = 0;
	uint16_t record_counter = 0;
	raw_t sentinel_top = {SENTINEL_MARK_TOP, 0};
	raw_t sentinel_bottom = {SENTINEL_MARK_BOTTOM, 0};

	// Address at the end of the program
	uint64_t *program_end = (uint64_t *) ((uint32_t)&__fini_array_end + (uint32_t)&_edata - (uint32_t)&_sdata);

	// Address at top of flash, based on MCU
	uint64_t *top = (uint64_t *) FLASH_FINISH;

	// Address at bottom of flash for records
	uint64_t *bottom = (uint64_t *) (((uint32_t)program_end & ~0x7FF) + 0x800);

	// Top mem pointer: located at very top of flash
	uint64_t *pt = find_sentinel_top();

	// Bottom mem pointer: located at first open page after software
	uint64_t *pb = find_sentinel_bottom();
	/*
	 * 4 case for top and bottom memory
	 * 1. pt = 0, pb = 0: Memory not initialized
	 * Action: Erase memory if needed and add top and bottom sentinels
	 * 2. pt > 0, pb = 0: Lower sentinel has been corrupted
	 * Action: restore lower sentinel. Accept data that may have been lost
	 * 3. pt = 0, pb > 0: Upper sentinel has been corrupted
	 * Action: Erase memory, add top and bottom sentinels
	 * 4. pt > 0, pb > 0. Memory is initialized
	 * Action: No memory changes necessary
	 */

//	printf("top = %p\n\r", top);
//	printf("bottom = %p\n\r", bottom);
//	printf("program end = %p\n\r", program_end);
//	printf("pt = %p\n\r", pt);
//	printf("pb = %p\n\r", pb);

	if ((!pt) && (!pb)){
		printf("Memory Uninitialized\n\r");
		if (write_sentinel(top, &sentinel_top)) {
			return (-1);
		}
		if (write_sentinel(bottom, &sentinel_bottom)) {
			printf("Could not write top sentinel\n\r");
			return (-1);
		}
		printf("Done\n\r");
	}
	else if ((!pt) && (pb)) {
		printf("Memory corrupted, Erase not implemented for this stage\n\r");
		return (-1);
	}
	else if ((pt) && (pb)) {
		if (pb != bottom) {
			printf("Sentinel is not at the bottom of lowest flash");
			return (-1);
		}
	}
	else {
		printf("Unknown memory condition\n\r");
		return (-1);
	}
	// if the program makes it this far, both sentinels are in place
	// Next step is to load the flash data structure used for writing/reading records
	fs->data_start = top - 2; // *top is sentinel. 1 below is the first record address
	fs->max_possible_records = (((uint32_t)top-(uint32_t)bottom)>>4)-1;
	sd = (sensordata_t *) top; // sd is not pointing at top sentinel
	sd--; // pointing to first data record
	while ((sd->watermark!=0xFF)&&(sd->watermark!=0xa5)) {
		record_counter++;
		sd--;
	}
	fs->next_record_number = record_counter;
	fs->total_records = record_counter;
	fs->next_address = (uint64_t *) sd;
//	printf("data_start=%p\n\r", fs->data_start);
//	printf("max_possible_records=%d\n\r", (int) fs->max_possible_records);
//	printf("next_record_number=%d\n\r", (int) fs->next_record_number);
//	printf("total_records=%d\n\r", (int) fs->total_records);
//	printf("next_address=%p\n\r", fs->next_address);
	return (0);
}

// NOTE: Might move the date functions
uint32_t pack_time(RTC_TimeTypeDef *time, RTC_DateTypeDef *date){
	  uint32_t out = 0;
	  uint32_t temp = 0;
	  temp = (((uint32_t) date->Date) & 0b00011111) << 27;           // Add the date
	  out = temp;
	  //   printf("temp=0x%08x,date=%d\n\r",(unsigned int) temp, date->Date);
	  temp = (((uint32_t) date->Month) & 0b00001111) << 22;          // Add the month
	  out |= temp;
	  //   printf("temp=0x%08x,date=%d\n\r",(unsigned int) temp, date->Month);
	  temp = ((((uint32_t) date->Year) - 19) & 0b00000111) << 19;  // Add the year
	  out |= temp;
	  //   printf("temp=0x%08x,date=%d\n\r",(unsigned int) temp, date->Year);
	  temp = (((uint32_t) time->Hours) & 0b00011111) << 13;           // Add the hour
	  out |= temp;
	  //   printf("temp=0x%08x,hours=%d\n\r",(unsigned int) temp, time->Hours);
	  temp = (((uint32_t) time->Minutes) & 0b00111111) << 6;          // Add the minute
	  out |= temp;
	  //   printf("temp=0x%08x,minutes=%d\n\r",(unsigned int) temp, time->Minutes);
	  temp = (((uint32_t) time->Seconds) & 0b00111111);               // Add the second
	  out |= temp;
	  //  printf("temp=0x%08x,seconds=%d\n\r",(unsigned int) temp, time->Seconds);
	  //   printf("Done Packing Time/Date\n\r");
	  return out;
}

void  unpack_time(uint32_t timeval, RTC_TimeTypeDef *time, RTC_DateTypeDef *date){
	uint32_t temp = timeval;

	  // Seconds
	  temp &= 0x3F;
	  time->Seconds = (uint8_t) temp;

	  // Minutes
	  temp = timeval;
	  temp >>= 6;
	  temp &= 0x3F;
	  time->Minutes = (uint8_t) temp;

	  // Hours
	  temp = timeval;
	  temp >>= 13;
	  temp &= 0x1F;
	  time->Hours = (uint8_t) temp;

	  // Year
	  temp = timeval;
	  temp >>= 19;
	  temp &= 0x07;
	  date->Year = (uint8_t) temp + 19;

	  // Year
	  temp = timeval;
	  temp >>= 22;
	  temp &= 0x0F;
	  date->Month = (uint8_t) temp;

	  // Day
	  temp = timeval;
	  temp >>= 27;
	  temp &= 0x1F;
	  date->Date = (uint8_t) temp;
}


// NOTE: Listing static functions before flash erase
// This is to keep the static functions internal and not include in flash.h
// I.e. limiting scope to translation unit
static uint32_t GetPage(uint32_t Addr){
	uint32_t page = 0;

	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)){
		// Bank 1
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else{
		// Bank 2
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}
	return page;
}

// Gets bank based on addr arg
static uint32_t GetBank(uint32_t Addr){
	return FLASH_BANK_1;
}

int flash_erase(void){
	uint32_t FirstPage = 0;
	uint32_t NbOfPages = 0;
	uint32_t BankNumber = 0;
	uint32_t PAGEError = 0;

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t program_end = ((uint32_t)&__fini_array_end
							+ (uint32_t)&_edata
							- (uint32_t)&_sdata
							);
	uint32_t bottom = (((uint32_t)program_end & ~0x7FF) + 0x800);
	uint32_t top = FLASH_FINISH;

	HAL_FLASH_Unlock();

	// Clear Opt bit
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	FirstPage = GetPage(bottom);
	// # of pages to erase starting from the first page
	NbOfPages = GetPage(top) - FirstPage + 1;
	BankNumber = GetBank(bottom);
	// Filling EraseInit struct
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;  // declaration says this is uint32_t
	EraseInitStruct.Banks = BankNumber;
	EraseInitStruct.Page = FirstPage;
	EraseInitStruct.NbPages = NbOfPages;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
		HAL_FLASH_Lock();
		return (-1);
	}
	HAL_FLASH_Lock();
	return (0);
}



void  flash_erase_command(char *arguments){
	// Starting off not taking arguments, can add stuff like "all" later
	if (arguments != NULL && strcmp(arguments, "all") == 0) {
		flash_erase();
		printf("OK\n\r");
	}
	else{
		printf("NOK\n\r");
	}
}
