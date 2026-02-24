# Musical Alarm Clock (STM32F103)

這是一個基於 STM32F103RB (Nucleo-F103RB) 開發的音樂鬧鐘專案。該專案整合了實時時鐘 (RTC)、OLED 顯示器、UART 控制介面，並透過 I2S 介面驅動外部 DAC (PCM5102A) 播放 SD 卡中的 WAV 音樂檔案。

## 1. 硬體架構與接腳分配 (Hardware Pinout)

本專案使用 **STM32F103RBT6** 微控制器。以下是各模組的接腳分配規劃：

| Module | Interface | Pin Name | STM32 Pin | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Console** | UART | USART2_TX | PA2 | Debug / Command Input |
| | | USART2_RX | PA3 | |
| **OLED** | I2C | I2C1_SCL | PB6 | SSD1306 0.96" Display |
| | | I2C1_SDA | PB7 | |
| **RTC** | GPIO | CE (RST) | PB0 | DS1302 Chip Select |
| | | I/O (DAT) | PB1 | DS1302 Data |
| | | SCLK (CLK)| PB10 | DS1302 Clock |
| **SD Card** | SPI | SPI1_SCK | PA5 | MicroSD Card Clock (Shared with LD2) |
| | | SPI1_MISO | PA6 | MicroSD Card Data Out |
| | | SPI1_MOSI | PA7 | MicroSD Card Data In |
| | | CS | PA4 | Chip Select (Software Control) [2] |
| **Audio DAC** | I2S | I2S2_WS | PB12 | PCM5102A LRCK (Left/Right Clock) [1] |
| | | I2S2_CK | PB13 | PCM5102A BCK (Bit Clock) |
| | | I2S2_SD | PB15 | PCM5102A DIN (Data In) |

---

## 2. 軟體架構 (Software Architecture)

*   **Core:** STM32 HAL Library
*   **Middleware:** FatFs (Generic) 用於 SD 卡檔案系統讀取。
*   **Drivers:**
    *   **DS1302:** 使用 GPIO 模擬 (Bit-banging) 3-wire 通訊協議。
    *   **SSD1306:** 基於 I2C 的顯示驅動，負責 Framebuffer 管理與繪圖。
    *   **Audio Engine:** 基於 I2S + DMA 的雙緩衝 (Double Buffering) 音訊播放器。

---

## 3. 開發規劃 (Development Plan)

開發過程分為三個階段，逐步驗證硬體功能與軟體邏輯。

### Phase 1: 基礎顯示與時間 (Basic Display & Clock)
**目標**: 驅動 OLED 與 RTC，並在螢幕上顯示正確的日期與時間。

- [x] **OLED 驅動整合**:
    - [x] 配置 `I2C1` (目前配置為 Standard Mode 100kHz)。
    - [x] 移植 SSD1306 驅動庫。
    - [x] 實作 `OLED_Init()` 與 `OLED_ShowString()`。
- [x] **DS1302 驅動開發**:
    - [x] 配置 PB0, PB1, PB10 為 GPIO Output (PB1 需在讀取時切換為 Input)。
    - [x] 實作 `DS1302_WriteByte()` 與 `DS1302_ReadByte()`。
    - [x] 實作 `DS1302_SetTime()` 與 `DS1302_GetTime()`，處理 BCD 碼轉換。
- [x] **主迴圈整合**:
    - [x] 每秒讀取一次 RTC 時間。
    - [x] 格式化字串並更新 OLED 顯示 (Date, Time, Minute)。

### Phase 2: 互動與控制 (Interaction & Control)
**目標**: 透過 UART 設定時間與鬧鐘，並建立鬧鐘觸發邏輯。

- [ ] **UART 指令解析**:
    - [x] 初始化 `USART2` (TX `printf` 已完成)。
    - [x] 開啟接收中斷 (RXNE) 與實作 Ring Buffer。
    - [x] 設計指令集，例如：
        *   `SET TIME 2023-10-27 12:00:00`
        *   `SET ALARM 07:30`
- [ ] **鬧鐘邏輯**:
    - [ ] 建立全域變數 `AlarmTime`。
    - [ ] 在主迴圈比對 `CurrentTime` 與 `AlarmTime`。
    - [ ] 觸發時，暫時以開發板上的 LED (LD2) 閃爍作為驗證。

### Phase 3: 音樂播放系統 (Audio System)
**目標**: 鬧鐘觸發時，從 SD 卡讀取 WAV 檔案並透過 I2S DAC 播放。
*(註：關於 SPI1 (SD Card) 與板載 LED (PA5) 及 JTAG (PB3) 的腳位衝突處理，將於此階段進行詳細配置與驗證)*

- [ ] **SD 卡與檔案系統**:
    - [ ] 配置 `SPI1`。
    - [ ] 移植 **FatFs** Middleware。
    - [ ] 實作 `user_diskio.c` 中的底層 SPI 讀寫函式。
    - [ ] 測試 `f_mount()` 與 `f_open()` 讀取測試檔案。
- [ ] **I2S 與 DMA 配置**:
    - [ ] 配置 `SPI2` 為 I2S Philips Mode, Master Transmit, 16-bit data [2]。
    - [ ] 配置 `DMA1_Channel5` (SPI2_TX) 為 Circular Mode (或 Normal Mode 配合中斷切換)。
    - [ ] 開啟 DMA Transfer Complete (TC) 與 Half Transfer (HT) 中斷。
- [ ] **WAV 解析與播放**:
    - [ ] 實作 `WAV_ParseHeader()`：讀取檔案前 44 bytes，確認 Sample Rate (如 44.1kHz) 與 Data Size。
    - [ ] **雙緩衝機制 (Ping-Pong Buffer)** [3]：
        *   定義兩個緩衝區 `Buffer_A` 與 `Buffer_B` (例如各 1024 bytes)。
        *   DMA 傳送 `Buffer_A` 時，CPU 讀取 SD 卡填滿 `Buffer_B`。
        *   DMA 完成 `Buffer_A` 後觸發中斷，切換至 `Buffer_B`，CPU 隨即填滿 `Buffer_A`。
- [ ] **系統整合**:
    - [ ] 鬧鐘觸發 -> `f_open("alarm.wav")` -> 解析 Header -> 設定 I2S 頻率 -> 啟動 DMA 播放。

---

## 4. 技術註記 (Technical Notes)

*   **[1] I2S Clock Source & 3.072MHz Oscillator**:
    STM32F103 系列沒有專用的 PLLI2S，I2S 時鐘源來自 `SYSCLK`。為了產生完美的 **48kHz** 取樣率，本專案採用 **3.072MHz 主動振盪器** 作為 HSE 輸入。
    *   **Clock Tree 配置**: HSE (3.072MHz) -> PLL x16 -> **SYSCLK = 49.152MHz**。
    *   **I2S 計算**: 49.152MHz / (48kHz * 16bit * 2ch * 2) = 16 (整數分頻)。
    *   這消除了標準 72MHz 時鐘下的頻率誤差。PCM5102A 需配置為使用內部 PLL (SCK 接地)，僅需 STM32 提供 BCK 與 LRCK。

*   **[2] SPI 資源分配**:
    STM32F103RB (Medium Density) 只有兩組 SPI。
    *   **SPI1**: 不支援 I2S，因此**必須**分配給 SD 卡。注意 PA5 同時連接板載 LED (LD2)，讀寫時 LED 閃爍為正常現象。
    *   **SPI2**: 支援 I2S，因此**必須**分配給 PCM5102A (I2S2)。

*   **[3] DMA & Memory**:
    STM32F103RB 只有 20KB SRAM。
    *   FatFs 需要約 512 bytes + stack。
    *   OLED Framebuffer 需要 1024 bytes (128x64/8)。
    *   Audio Buffers 建議分配 2KB ~ 4KB (例如 2x2048 bytes) 以確保 SD 卡讀取延遲不會造成爆音。
    *   記憶體管理需謹慎，避免 Stack Overflow。

---

## 5. 參考資料 (References)

1.  STM32F103x8/B Datasheet (DS5319)
2.  STM32F10xxx Reference Manual (RM0008)
3.  PCM5102A Datasheet (Texas Instruments)
4.  DS1302 Datasheet (Maxim Integrated)
5.  SSD1306 Datasheet (Solomon Systech)