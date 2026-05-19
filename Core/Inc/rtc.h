#ifndef RTC_H
#define RTC_H

#include "main.h"

extern RTC_HandleTypeDef hrtc;

void ds_command(char *arguments);
void ts_command(char *arguments);
void dr_command(char *arguments);
void prompt(void);

#endif
