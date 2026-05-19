/*
 * flash.h
 *
 *  Created on: Feb 28, 2026
 *      Author: Ale K. & Ben G - Adapted from https://github.iu.edu/SICE-E314/Spring-2026/wiki/Lab6
 *
 *      Holds Flash data record formats
 */

// NOTE: May refactor to more files, MVPing getting this working even if a bit more monolithic

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

// macros for flash init
#define SENTINEL_MARK_BOTTOM	0xDEADBEEFA5A5A5A5
#define SENTINEL_MARK_TOP   	0xFEEDC0DE5A5A5A5A

#define FLASH_FINISH 0x0803FFF0

typedef struct sensordata {
  uint8_t watermark;               // 0x01=populated, 0xFF=unpopulated
  uint8_t status;                  // record type, 01=sensor data, 02=log data;
  uint16_t record_number;          // Which number is this particular record
  uint32_t timestamp;              // Time, bit packed into 32 bits
  uint16_t battery_voltage;        // 16 bit battery voltage
  int16_t temperature;             // STM32 Temperature sensor reading
  uint32_t sensor_period;          // Reading from the light sensor
} sensordata_t;

typedef struct log_data {
  uint8_t watermark;               // 0x01=populated, 0xFF=unpopulated
  uint8_t status;                  // record type, 01=sensor data, 02=log data;
  uint16_t record_number;          // Which number is this particular record
  uint32_t timestamp;              // Time, bit packed into 32 bits
  uint8_t msg[8];                  // String message to make the record seem less cryptic.
} logdata_t;

typedef struct raw {               // Raw structure used to write sensordata and log_data to the flash
  uint64_t data0;
  uint64_t data1;
} raw_t;

// Flash status struct
typedef struct flash_status {
	uint64_t *data_start;
	uint32_t total_records;
	uint32_t max_possible_records;
	uint64_t *next_address;
	uint32_t next_record_number;
} flash_status_t;


/* shared record number*/
extern uint16_t g_record_number;

// returns pointer to bottom of flash
uint64_t* find_sentinel_bottom(void);

// returns pointer to top of flash
uint64_t* find_sentinel_top(void);

// Writes flash record
int write_record(flash_status_t *fs, void *record);

// writes sentinel record
int write_sentinel(uint64_t *pos, raw_t *sentinel);

// inits flash struct
int flash_write_init(flash_status_t *fs);

// NOTE: Might move the date functions
// Packs time into 32 bits
uint32_t pack_time(RTC_TimeTypeDef *time, RTC_DateTypeDef *date);

// Unpacks time into 32 bits
void unpack_time(uint32_t timeval, RTC_TimeTypeDef *time, RTC_DateTypeDef *date);

// Erases upper portion of Flash where we store records
int flash_erase(void);

// Command for erasing flash.
void  flash_erase_command(char *arguments);
#endif /* INC_FLASH_H_ */
