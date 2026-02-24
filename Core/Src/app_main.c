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
#include <string.h> // For strcmp, strncmp

// 引用 main.c 中的 huart2
extern UART_HandleTypeDef huart2;

// UART 接收相關變數
RingBuffer_t uartRxBuffer = {0};
uint8_t rxByte; // 單字元接收緩衝

// 宣告內部函式
void Process_UART_Command(void);

// --- UART Callback ---
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // 將接收到的 byte 寫入 Ring Buffer
        uint16_t next_head = (uartRxBuffer.head + 1) % UART_RX_BUFFER_SIZE;
        if (next_head != uartRxBuffer.tail) { // 檢查是否滿了
            uartRxBuffer.buffer[uartRxBuffer.head] = rxByte;
            uartRxBuffer.head = next_head;
        }
        // 重新啟動接收中斷，準備收下一個 byte
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
}

// --- Ring Buffer Helper ---
int RB_Read(uint8_t *byte) {
    if (uartRxBuffer.head == uartRxBuffer.tail) return 0; // Empty
    *byte = uartRxBuffer.buffer[uartRxBuffer.tail];
    uartRxBuffer.tail = (uartRxBuffer.tail + 1) % UART_RX_BUFFER_SIZE;
    return 1;
}

void App_Init(void)
{
    // 在這裡進行應用層的初始化
    printf("System Clock (App_Init): %lu Hz\r\n", SystemCoreClock); // 啟動時再次確認
    printf("Console Ready. Type 'HELP' for commands.\r\n");

    // 啟動 UART 接收中斷
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);

    DS1302_Init();

    // --- 時間校正 (請取消註解以設定時間，燒錄一次後再註解回去) ---
//   DS1302_Time_t initialTime;
//   initialTime.year = 26;    // 2024 年 (只填後兩位)
//   initialTime.month = 2;    // 1 月
//   initialTime.day = 23;      // 1 日
//   initialTime.week = 1;     // 星期一 (1~7)
//   initialTime.hours = 14;   // 12 點
//   initialTime.minutes = 44;  // 00 分
//   initialTime.seconds = 0;  // 00 秒
//   DS1302_SetTime(&initialTime);
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
    uint32_t startTick, endTick; // 用於測量執行時間
    
    // 使用非阻塞方式管理時間更新 (每 1000ms 更新一次)
    static uint32_t lastUpdateTick = 0;
    
    // 1. 處理 UART 指令 (最高優先級，每次 Loop 都檢查)
    Process_UART_Command();

    DS1302_GetTime(&currentTime);
    // 2. 定時更新顯示與時間 (每秒一次)
    if (HAL_GetTick() - lastUpdateTick >= 1000) {
        lastUpdateTick = HAL_GetTick();

    // 格式化日期 (月/日) 與 時間 (時:分)
    sprintf(dateString, "%02d/%02d", currentTime.month, currentTime.day);
    sprintf(timeString, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
        DS1302_GetTime(&currentTime);

    // 驗證 DS1302 時間是否合理
    if ((currentTime.seconds > 59) || (currentTime.minutes > 59) || (currentTime.hours > 23) ||
        (currentTime.day > 31) || (currentTime.month > 12) || (currentTime.year > 99)) {
        printf("RTC Time Invalid! Please Check!\r\n");
        // 在這裡可以加入 Error Handler 或 LED 錯誤提示
        // 格式化日期 (月/日) 與 時間 (時:分)
        sprintf(dateString, "%02d/%02d", currentTime.month, currentTime.day);
        sprintf(timeString, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);

        // 驗證 DS1302 時間是否合理
        if ((currentTime.seconds > 59) || (currentTime.minutes > 59) || (currentTime.hours > 23) ||
            (currentTime.day > 31) || (currentTime.month > 12) || (currentTime.year > 99)) {
            // printf("RTC Time Invalid! Please Check!\r\n"); // 避免刷屏，暫時註解
        }

        // 輸出到 Serial Port (Debug)
        // printf("Current Time: %s\r\n", timeString);

        ssd1306_Fill(Black);
        
        // 日期顯示 (小字體 Font_7x10), 置中: (128 - 5*7) / 2 = 46
        ssd1306_SetCursor(46, 10);
        ssd1306_WriteString(dateString, Font_7x10, White);
        
        // 時間顯示 (大字體 Font_16x26), 置中: (128 - 8*16) / 2 = 0
        ssd1306_SetCursor(0, 30);
        ssd1306_WriteString(timeString, Font_16x26, White);
        
        ssd1306_UpdateScreen();

        // 測試：閃爍板載 LED (LD2) 證明程式有在跑
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
}

// 簡單的指令處理器
void Process_UART_Command(void) {
    static char cmdBuffer[32];
    static uint8_t cmdIndex = 0;
    uint8_t ch;

    // 從 Ring Buffer 讀取所有可用字元
    while (RB_Read(&ch)) {
        // Echo back (回顯，讓使用者看到自己打了什麼)
        putchar(ch);

    // 輸出到 Serial Port
    printf("Current Date: %s\r\n", dateString);
    printf("Current Time: %s\r\n", timeString);
    printf("System Clock: %lu Hz\r\n", SystemCoreClock); // 確認系統時鐘變數
        if (ch == '\n' || ch == '\r') {
            if (cmdIndex > 0) {
                cmdBuffer[cmdIndex] = '\0'; // Null terminate
                printf("\r\nCMD Recv: %s\r\n", cmdBuffer);

    ssd1306_Fill(Black);
    
    // 日期顯示 (小字體 Font_7x10), 置中: (128 - 5*7) / 2 = 46
    ssd1306_SetCursor(46, 10);
    ssd1306_WriteString(dateString, Font_7x10, White);
    
    // 時間顯示 (大字體 Font_16x26), 置中: (128 - 8*16) / 2 = 0
    ssd1306_SetCursor(0, 30);
    ssd1306_WriteString(timeString, Font_16x26, White);
    
    // 測量 OLED 更新耗時
    startTick = HAL_GetTick();
    ssd1306_UpdateScreen();
    endTick = HAL_GetTick();tak
    printf("OLED Update Cost: %lu ms\r\n", endTick - startTick); // 印出耗時
                // --- 指令解析邏輯 ---
                if (strcasecmp(cmdBuffer, "HELP") == 0) {
                    printf("Available Commands:\r\n");
                    printf("  HELP       - Show this list\r\n");
                    printf("  TEST       - Print Test Message\r\n");
                    printf("  SET TIME   - (TODO) Set RTC Time\r\n");
                }
                else if (strcasecmp(cmdBuffer, "TEST") == 0) {
                    printf("Test OK! System is running.\r\n");
                }
                else {
                    printf("Unknown Command.\r\n");
                }
                // ------------------

    // 在這裡撰寫主要邏輯
    // 測試：閃爍板載 LED (LD2) 證明程式有在跑
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(500); // 延遲 500ms
                cmdIndex = 0; // 重置緩衝區
            } else {
                printf("\r\n"); // 只是換行
            }
        } else {
            // 存入緩衝區
            if (cmdIndex < sizeof(cmdBuffer) - 1) {
                cmdBuffer[cmdIndex++] = ch;
            } else {
                // 緩衝區溢位，重置
                printf("\r\nCommand too long!\r\n");
                cmdIndex = 0;
            }
        }
    }
}
