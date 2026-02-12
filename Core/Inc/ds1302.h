/*
 * ds1302.h
 *
 *  Created on: Phase 1
 *      Author: Gemini
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

#include "main.h"

// 時間結構體
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;   // 1-31
    uint8_t month; // 1-12
    uint8_t year;  // 0-99 (代表 2000-2099)
    uint8_t week;  // 1-7 (1=Sunday, or user defined)
} DS1302_Time_t;

// 函式原型
void DS1302_Init(void);
void DS1302_SetTime(DS1302_Time_t *time);
void DS1302_GetTime(DS1302_Time_t *time);

#endif /* INC_DS1302_H_ */
