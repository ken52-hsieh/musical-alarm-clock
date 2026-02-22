/*
 * ds1302.c
 *
 *  Created on: Phase 1
 *      Author: Gemini
 */

#include "ds1302.h"

// 使用 main.h 中定義的腳位
// RTC_CE_Pin (PB0), RTC_IO_Pin (PB1), RTC_SCLK_Pin (PB10)

#define CMD_WRITE_SEC   0x80
#define CMD_READ_SEC    0x81
#define CMD_CTRL_WP     0x8E // Write Protect

// 簡單延遲
static void DS1302_Delay(volatile uint32_t count) {
    while (count--) {
        __asm("nop");
    }
}

// IO 方向控制
static void DS1302_SetGPIO_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = RTC_IO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RTC_IO_GPIO_Port, &GPIO_InitStruct);
}

static void DS1302_SetGPIO_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = RTC_IO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RTC_IO_GPIO_Port, &GPIO_InitStruct);
}

static void DS1302_WriteByte(uint8_t data) {
    DS1302_SetGPIO_Output();
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(RTC_IO_GPIO_Port, RTC_IO_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        DS1302_Delay(5);
        HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_SET);
        DS1302_Delay(5);
        HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_RESET);
        DS1302_Delay(5);
        data >>= 1;
    }
}

static uint8_t DS1302_ReadByte(void) {
    uint8_t data = 0;
    DS1302_SetGPIO_Input();
    for (int i = 0; i < 8; i++) {
        data >>= 1;
        if (HAL_GPIO_ReadPin(RTC_IO_GPIO_Port, RTC_IO_Pin) == GPIO_PIN_SET) {
            data |= 0x80;
        }
        HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_SET);
        DS1302_Delay(5);
        HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_RESET);
        DS1302_Delay(5);
    }
    return data;
}

static void DS1302_WriteReg(uint8_t reg, uint8_t data) {
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_RESET);
    DS1302_Delay(5);
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_SET);
    DS1302_Delay(5);
    DS1302_WriteByte(reg);
    DS1302_WriteByte(data);
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_RESET);
}

static uint8_t DS1302_ReadReg(uint8_t reg) {
    uint8_t data;
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RTC_SCLK_GPIO_Port, RTC_SCLK_Pin, GPIO_PIN_RESET);
    DS1302_Delay(5);
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_SET);
    DS1302_Delay(5);
    DS1302_WriteByte(reg);
    data = DS1302_ReadByte();
    HAL_GPIO_WritePin(RTC_CE_GPIO_Port, RTC_CE_Pin, GPIO_PIN_RESET);
    return data;
}

static uint8_t DEC2BCD(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }
static uint8_t BCD2DEC(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

void DS1302_Init(void) {
    // 初始化 CE 與 SCLK 腳位為輸出模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = RTC_CE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RTC_CE_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RTC_SCLK_Pin;
    HAL_GPIO_Init(RTC_SCLK_GPIO_Port, &GPIO_InitStruct);

    DS1302_WriteReg(CMD_CTRL_WP, 0x00); // 解除寫保護
    uint8_t sec = DS1302_ReadReg(CMD_READ_SEC);
    if (sec & 0x80) DS1302_WriteReg(CMD_WRITE_SEC, 0x00); // 啟動震盪器
}

void DS1302_SetTime(DS1302_Time_t *time) {
    DS1302_WriteReg(CMD_CTRL_WP, 0x00);
    DS1302_WriteReg(0x80, DEC2BCD(time->seconds));
    DS1302_WriteReg(0x82, DEC2BCD(time->minutes));
    DS1302_WriteReg(0x84, DEC2BCD(time->hours));
    DS1302_WriteReg(0x86, DEC2BCD(time->day));
    DS1302_WriteReg(0x88, DEC2BCD(time->month));
    DS1302_WriteReg(0x8A, DEC2BCD(time->week));
    DS1302_WriteReg(0x8C, DEC2BCD(time->year));
    DS1302_WriteReg(CMD_CTRL_WP, 0x80);
}

void DS1302_GetTime(DS1302_Time_t *time) {
    time->seconds = BCD2DEC(DS1302_ReadReg(0x81));
    time->minutes = BCD2DEC(DS1302_ReadReg(0x83));
    time->hours   = BCD2DEC(DS1302_ReadReg(0x85));
    time->day     = BCD2DEC(DS1302_ReadReg(0x87));
    time->month   = BCD2DEC(DS1302_ReadReg(0x89));
    time->week    = BCD2DEC(DS1302_ReadReg(0x8B));
    time->year    = BCD2DEC(DS1302_ReadReg(0x8D));
}
