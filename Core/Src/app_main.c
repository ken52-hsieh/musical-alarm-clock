/*
 * app_main.c
 *
 *  Created on: User Request
 *      Author: Gemini
 */

#include "app_main.h"

// 未來可以在這裡 include 其他驅動，例如：
// #include "ds1302.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

void App_Init(void)
{
    // 在這裡進行應用層的初始化
    // 例如: DS1302_Init();

    ssd1306_Init();
    ssd1306_Fill(White);
    ssd1306_SetCursor(1,1);
    ssd1306_WriteString("OLED Init OK", Font_7x10, White);
    ssd1306_UpdateScreen();
}

void App_Loop(void)
{
    // 在這裡撰寫主要邏輯
    // 測試：閃爍板載 LED (LD2) 證明程式有在跑
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(500); // 延遲 500ms
}
