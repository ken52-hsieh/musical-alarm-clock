/*
 * app_main.c
 *
 *  Created on: User Request
 *      Author: Gemini
 */

#include "app_main.h"

// 未來可以在這裡 include 其他驅動，例如：
// #include "ds1302.h"
#include "ds1302.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>

void App_Init(void)
{
    // 在這裡進行應用層的初始化
    DS1302_Init();

    // --- 時間校正 (請取消註解以設定時間，燒錄一次後再註解回去) ---
//    DS1302_Time_t initialTime;
//    initialTime.year = 26;    // 2024 年 (只填後兩位)
//    initialTime.month = 2;    // 1 月
//    initialTime.day = 13;      // 1 日
//    initialTime.week = 5;     // 星期一 (1~7)
//    initialTime.hours = 9;   // 12 點
//    initialTime.minutes = 25;  // 00 分
//    initialTime.seconds = 0;  // 00 秒
//    DS1302_SetTime(&initialTime);
    // -------------------------------------------------------

    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(1,1);
    ssd1306_WriteString("RTC OLED Init OK", Font_7x10, White);
    ssd1306_UpdateScreen();

}

void App_Loop(void)
{
    DS1302_Time_t currentTime;
    char dateString[16];
    char timeString[16];

    DS1302_GetTime(&currentTime);

    // 格式化日期 (月/日) 與 時間 (時:分)
    sprintf(dateString, "%02d/%02d", currentTime.month, currentTime.day);
    sprintf(timeString, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);

    // 輸出到 Serial Port
     printf("Current Date: %s\r\n", dateString);
     printf("Current Time: %s\r\n", timeString);

    ssd1306_Fill(Black);
    
    // 日期顯示 (小字體 Font_7x10), 置中: (128 - 5*7) / 2 = 46
    ssd1306_SetCursor(46, 10);
    ssd1306_WriteString(dateString, Font_7x10, White);
    
    // 時間顯示 (大字體 Font_16x26), 置中: (128 - 8*16) / 2 = 0
    ssd1306_SetCursor(0, 30);
    ssd1306_WriteString(timeString, Font_16x26, White);
    
    ssd1306_UpdateScreen();

    // 在這裡撰寫主要邏輯
    // 測試：閃爍板載 LED (LD2) 證明程式有在跑
    //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(500); // 延遲 200ms
}
