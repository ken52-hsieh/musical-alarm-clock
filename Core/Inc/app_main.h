/*
 * app_main.h
 *
 *  Created on: User Request
 *      Author: Gemini
 */

#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_

#include "main.h"

// 應用層初始化 (在 while(1) 之前呼叫)
void App_Init(void);

// 應用層主迴圈 (在 while(1) 內呼叫)
void App_Loop(void);

// --- UART Ring Buffer ---
#define UART_RX_BUFFER_SIZE 128

typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer_t;

#endif /* INC_APP_MAIN_H_ */