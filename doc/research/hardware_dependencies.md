# VESC ファームウェア ハードウェア依存箇所調査レポート

## 概要

本ドキュメントは、VESCファームウェアリポジトリにおけるハードウェア依存箇所を調査した結果をまとめたものです。

**調査日**: 2025年12月25日  
**対象MCU**: STM32F4xx (STM32F405/407)  
**動作クロック**: 168 MHz

---

## 1. ターゲットMCU

### 1.1 サポートMCU

| MCU | コア | Flash | RAM | FPU |
|-----|------|-------|-----|-----|
| STM32F405RGT6 | Cortex-M4 | 1 MB | 192 KB | 単精度 |
| STM32F407VGT6 | Cortex-M4 | 1 MB | 192 KB | 単精度 |

### 1.2 MCU設定ファイル

| ファイル | 説明 |
|----------|------|
| [hwconf/mcuconf.h](../hwconf/mcuconf.h) | クロック/PLL/ペリフェラル設定 |
| [hwconf/stm32f4xx_conf.h](../hwconf/stm32f4xx_conf.h) | STM32 Standard Peripheral Library設定 |
| [ld_eeprom_emu.ld](../ld_eeprom_emu.ld) | メモリレイアウト/リンカースクリプト |

### 1.3 クロック設定

[hwconf/mcuconf.h](../hwconf/mcuconf.h) より:

```c
// 外部クリスタル (8MHz or 25MHz)
#define STM32_PLLM_VALUE    8      // 8MHz XTALの場合
#define STM32_PLLN_VALUE    336
#define STM32_PLLP_VALUE    2
#define STM32_PLLQ_VALUE    7

// 結果: SYSCLK = 168 MHz
//       APB1 = 42 MHz (PPRE1_DIV4)
//       APB2 = 84 MHz (PPRE2_DIV2)
```

オプション: 内部RC発振器 (`HW_USE_INTERNAL_RC`) または 25MHz外部クロック (`HW_USE_25MHZ_EXT_CLOCK`)

---

## 2. ハードウェア抽象化レイヤー (HAL)

### 2.1 ハードウェア設定システム

VESCは各ボード向けに個別のハードウェア設定ファイルを持つ柔軟なシステムを採用しています。

```
hwconf/
├── hw.h              ← 共通インターフェース (全HW定義をインクルード)
├── hw.c              ← 共通実装
├── hwconf.mk         ← ビルドシステム統合
├── vesc/             ← VESC公式ハードウェア
│   ├── basic/        ← VESC Basic
│   ├── duet/         ← Duet
│   └── ...
├── flipsky/          ← Flipsky製品
├── stormcore/        ← Stormcore製品
├── trampa/           ← TRAMPA製品
└── other/            ← その他のハードウェア
```

### 2.2 ビルド時のハードウェア選択

```bash
# Makefile経由で選択
make fw BOARD=basic

# カスタムハードウェア
make fw_custom HW_SRC=hwconf/custom/hw_custom.c HW_HEADER=hwconf/custom/hw_custom.h
```

ビルドマクロ ([Makefile](../Makefile)):
```makefile
-DHW_SOURCE=\"$(HW_SRC_FILE)\" -DHW_HEADER=\"$(HW_HEADER)\"
```

---

## 3. GPIO / ピン割り当て

### 3.1 PWM出力ピン (モータ制御)

6相PWM出力 (3相ハイサイド + 3相ローサイド):

| 機能 | GPIO | タイマ | チャンネル |
|------|------|--------|------------|
| Phase A High | PA8 | TIM1 | CH1 |
| Phase A Low | PA7 | TIM1 | CH1N |
| Phase B High | PA9 | TIM1 | CH2 |
| Phase B Low | PB0 | TIM1 | CH2N |
| Phase C High | PA10 | TIM1 | CH3 |
| Phase C Low | PB1 | TIM1 | CH3N |

※ボードにより異なる場合があります

### 3.2 ADC入力ピン

典型的なADC割り当て ([hwconf/vesc/basic/hw_basic_core.h](../hwconf/vesc/basic/hw_basic_core.h)):

| 機能 | ADCチャンネル | GPIO |
|------|---------------|------|
| 電流センサ1 | IN10 | PC0 |
| 電流センサ2 | IN11 | PC1 |
| 電流センサ3 | IN12 | PC2 |
| 相電圧センサ1 | IN0 | PA0 |
| 相電圧センサ2 | IN1 | PA1 |
| 相電圧センサ3 | IN2 | PA2 |
| 入力電圧 | IN13 | PC3 |
| MOSFET温度 | IN15 | PC5 |
| モータ温度 | IN9 | PB1 |
| 内部Vref | Vrefint | - |

### 3.3 ハードウェア固有マクロ

[hwconf/hw.h](../hwconf/hw.h) で定義される共通マクロ:

```c
// GPIO定義例 (hw_xxx.h でオーバーライド)
#define LED_GREEN_GPIO      GPIOB
#define LED_GREEN_PIN       7
#define LED_RED_GPIO        GPIOB
#define LED_RED_PIN         5

// LED制御マクロ
#define LED_GREEN_ON()      palSetPad(LED_GREEN_GPIO, LED_GREEN_PIN)
#define LED_GREEN_OFF()     palClearPad(LED_GREEN_GPIO, LED_GREEN_PIN)

// ゲートドライバ制御
#define ENABLE_GATE()       // ハードウェア固有実装
#define DISABLE_GATE()      // ハードウェア固有実装

// フォールト検出
#define IS_DRV_FAULT()      0  // ドライバフォールト状態
```

---

## 4. ペリフェラル使用状況

### 4.1 タイマ割り当て

[main.c](../main.c#L78-L89) より:

| タイマ | 用途 | 備考 |
|--------|------|------|
| TIM1 | PWM生成 (モータ1) | 6chコンプリメンタリPWM |
| TIM2 | FOC制御タイミング | mcpwm_foc用 |
| TIM3 | サーボデコーダ/エンコーダ | HW依存で切替 |
| TIM4 | WS2812 LED/エンコーダ | HW依存で切替 |
| TIM5 | 汎用タイマ | システムタイミング |
| TIM8 | PWM生成 (モータ2) | デュアルモーター時 |

### 4.2 DMA使用状況

| DMAコントローラ | ストリーム | 用途 |
|-----------------|------------|------|
| DMA1 | Stream 2 | I2C1 RX (Nunchuk/温度) |
| DMA1 | Stream 7 | I2C1 TX (Nunchuk/温度) |
| DMA2 | Stream 4 | ADC (電流/電圧サンプリング) |

### 4.3 ADC設定

```c
// 典型的なADC設定
#define HW_ADC_CHANNELS         18    // 総ADCチャンネル数
#define HW_ADC_INJ_CHANNELS     3     // インジェクテッドチャンネル数
#define HW_ADC_NBR_CONV         6     // 変換数/シーケンス
```

ADCサンプリングはTIM8からトリガされ、DMA2 Stream4で転送されます。

---

## 5. 通信インターフェース

### 5.1 USB (CDC)

| 項目 | 設定 |
|------|------|
| ペリフェラル | USB OTG FS |
| モード | CDC (仮想シリアル) |
| 関連ファイル | [comm/comm_usb.c](../comm/comm_usb.c) |

### 5.2 CAN

| 項目 | 設定 |
|------|------|
| ペリフェラル | CAN1 (CAN2はオプション) |
| ボーレート | 設定可能 (デフォルト500kbps) |
| 関連ファイル | [comm/comm_can.c](../comm/comm_can.c) |

```c
// CANなしハードウェア
#define HW_HAS_NO_CAN   // このマクロでCAN機能を無効化
```

### 5.3 UART

ハードウェア固有設定例:

```c
#define HW_UART_DEV         SD3              // USART3
#define HW_UART_GPIO_AF     GPIO_AF_USART3
#define HW_UART_TX_PORT     GPIOB
#define HW_UART_TX_PIN      10
#define HW_UART_RX_PORT     GPIOB
#define HW_UART_RX_PIN      11
```

### 5.4 I2C

```c
#define HW_I2C_DEV          I2CD2            // I2C2
#define HW_I2C_GPIO_AF      GPIO_AF_I2C2
#define HW_I2C_SCL_PORT     GPIOB
#define HW_I2C_SCL_PIN      10
#define HW_I2C_SDA_PORT     GPIOB
#define HW_I2C_SDA_PIN      11
```

### 5.5 SPI

ソフトウェアSPI (ビットバング) とハードウェアSPI両対応:

```c
// ハードウェアSPI例
#define HW_SPI_DEV          SPID1
#define HW_SPI_GPIO_AF      GPIO_AF_SPI1
#define HW_SPI_PORT_NSS     GPIOB
#define HW_SPI_PIN_NSS      11
#define HW_SPI_PORT_SCK     GPIOA
#define HW_SPI_PIN_SCK      5
#define HW_SPI_PORT_MOSI    GPIOB
#define HW_SPI_PIN_MOSI     2
#define HW_SPI_PORT_MISO    GPIOA
#define HW_SPI_PIN_MISO     6
```

---

## 6. ゲートドライバ

### 6.1 サポートドライバ

| ドライバ | ファイル | インターフェース |
|----------|----------|------------------|
| DRV8301 | [hwconf/drv8301.c](../hwconf/drv8301.c) | SPI |
| DRV8305 | [hwconf/drv8305.c](../hwconf/drv8305.c) | SPI |
| DRV8313 | (内蔵ロジック) | GPIO |
| DRV8316 | [hwconf/drv8316.c](../hwconf/drv8316.c) | SPI |
| DRV8320S | [hwconf/drv8320s.c](../hwconf/drv8320s.c) | SPI |
| DRV8323S | [hwconf/drv8323s.c](../hwconf/drv8323s.c) | SPI |

### 6.2 ドライバ有効化

ハードウェアヘッダで定義:

```c
#define HW_HAS_DRV8301        // DRV8301使用
// または
#define HW_HAS_DRV8323S       // DRV8323S使用
```

SPIピン定義例:

```c
#define DRV8301_MOSI_GPIO   GPIOC
#define DRV8301_MOSI_PIN    12
#define DRV8301_MISO_GPIO   GPIOC
#define DRV8301_MISO_PIN    11
#define DRV8301_SCK_GPIO    GPIOC
#define DRV8301_SCK_PIN     10
#define DRV8301_CS_GPIO     GPIOC
#define DRV8301_CS_PIN      9
```

---

## 7. センサインターフェース

### 7.1 電流センサ

| 構成 | マクロ | 説明 |
|------|--------|------|
| 2シャント | デフォルト | 相BとC電流から相Aを計算 |
| 3シャント | `HW_HAS_3_SHUNTS` | 全3相を直接測定 |
| 相シャント | `HW_HAS_PHASE_SHUNTS` | モータ相と直列配置 |

```c
// 電流取得マクロ
#define GET_CURRENT1()    ((float)ADC_Value[ADC_IND_CURR1])
#define GET_CURRENT2()    ((float)ADC_Value[ADC_IND_CURR2])
#define GET_CURRENT3()    ((float)ADC_Value[ADC_IND_CURR3])  // 3シャント時

// 極性反転オプション
#define INVERTED_SHUNT_POLARITY
```

### 7.2 エンコーダ

サポートエンコーダ一覧 ([encoder/](../encoder/)):

| タイプ | ファイル | 説明 |
|--------|----------|------|
| ABI | enc_abi.c/h | インクリメンタルエンコーダ |
| AS504x | enc_as504x.c/h | AMS磁気エンコーダ (SPI) |
| AS5x47U | enc_as5x47u.c/h | AMS磁気エンコーダ (SPI) |
| AD2S1205 | enc_ad2s1205.c/h | Analog Devices RDC |
| BiSS-C | enc_bissc.c/h | BiSSプロトコル |
| MT6816 | enc_mt6816.c/h | MagnTek磁気エンコーダ |
| TLE5012 | enc_tle5012.c/h | Infineon磁気エンコーダ |
| Sin/Cos | enc_sincos.c/h | アナログSin/Cos |
| PWM | enc_pwm.c/h | PWMエンコード出力 |
| TS5700N8501 | enc_ts5700n8501.c/h | Tamagawa絶対値エンコーダ |

エンコーダピン設定例:

```c
#define HW_HALL_ENC_GPIO1   GPIOC
#define HW_HALL_ENC_PIN1    6
#define HW_HALL_ENC_GPIO2   GPIOC
#define HW_HALL_ENC_PIN2    7
#define HW_HALL_ENC_GPIO3   GPIOC
#define HW_HALL_ENC_PIN3    8
#define HW_ENC_TIM          TIM3
#define HW_ENC_TIM_AF       GPIO_AF_TIM3
```

### 7.3 IMU (慣性計測装置)

サポートIMU一覧 ([imu/](../imu/)):

| センサ | ファイル | インターフェース |
|--------|----------|------------------|
| MPU9150/9250 | mpu9150.c/h | I2C |
| BMI160 | bmi160_wrapper.c/h | SPI |
| LSM6DS3 | lsm6ds3.c/h | SPI |
| ICM20948 | icm20948.c/h | SPI |

IMUピン設定例:

```c
#define LSM6DS3_NSS_GPIO    GPIOC
#define LSM6DS3_NSS_PIN     4
#define LSM6DS3_SCK_GPIO    GPIOA
#define LSM6DS3_SCK_PIN     6
#define LSM6DS3_MOSI_GPIO   GPIOA
#define LSM6DS3_MOSI_PIN    5
#define LSM6DS3_MISO_GPIO   GPIOB
#define LSM6DS3_MISO_PIN    2
```

---

## 8. メモリマップ

### 8.1 フラッシュメモリレイアウト

[ld_eeprom_emu.ld](../ld_eeprom_emu.ld) より:

| 領域 | アドレス | サイズ | 用途 |
|------|----------|--------|------|
| flash | 0x08000000 | 16 KB | ブートローダ |
| flash2 | 0x0800C000 | ~464 KB | アプリケーション |
| crcinfo | 0x0807FFF0 | 8 bytes | CRC情報 |

### 8.2 RAMレイアウト

| 領域 | アドレス | サイズ | 用途 |
|------|----------|--------|------|
| ram0 | 0x20000000 | 128 KB | SRAM1+SRAM2 (メイン) |
| ram1 | 0x20000000 | 112 KB | SRAM1 |
| ram2 | 0x2001C000 | 16 KB | SRAM2 |
| ram4 | 0x10000000 | 62 KB | CCM SRAM (高速) |
| libif | 0x1000F800 | 2 KB | Cライブラリ I/F |
| ram5 | 0x40024000 | 4 KB | バックアップSRAM |

---

## 9. ハードウェア機能フラグ

### 9.1 主要なコンパイル時フラグ

| マクロ | 説明 |
|--------|------|
| `HW_NAME` | ハードウェア名 (文字列) |
| `HW_HAS_DRV8301` | DRV8301ドライバ使用 |
| `HW_HAS_DRV8305` | DRV8305ドライバ使用 |
| `HW_HAS_DRV8316` | DRV8316ドライバ使用 |
| `HW_HAS_DRV8320S` | DRV8320Sドライバ使用 |
| `HW_HAS_DRV8323S` | DRV8323Sドライバ使用 |
| `HW_HAS_3_SHUNTS` | 3電流シャント構成 |
| `HW_HAS_PHASE_SHUNTS` | 相電流シャント配置 |
| `HW_HAS_PHASE_FILTERS` | 相フィルタ搭載 |
| `HW_HAS_DUAL_MOTORS` | デュアルモーター対応 |
| `HW_HAS_DUAL_PARALLEL` | 並列デュアル構成 |
| `HW_HAS_NO_CAN` | CANバスなし |
| `HW_HAS_PERMANENT_NRF` | 内蔵NRF24モジュール |
| `HW_USE_INTERNAL_RC` | 内部RC発振器使用 |
| `HW_USE_25MHZ_EXT_CLOCK` | 25MHz外部クロック |
| `HW_USE_BRK` | ブレーキ入力使用 |
| `INVERTED_SHUNT_POLARITY` | シャント極性反転 |

### 9.2 電気的パラメータ

各ハードウェアで定義される電気的制限:

```c
// 電流制限
#define HW_LIM_CURRENT        -150.0, 150.0    // モータ電流
#define HW_LIM_CURRENT_IN     -130.0, 130.0    // 入力電流
#define HW_LIM_CURRENT_ABS    0.0, 160.0       // 絶対最大電流

// 電圧制限
#define HW_LIM_VIN            11.0, 94.0       // 入力電圧範囲

// その他
#define HW_LIM_ERPM           -200e3, 200e3    // 電気的RPM
#define HW_LIM_DUTY_MIN       0.0, 0.1
#define HW_LIM_DUTY_MAX       0.0, 0.99
#define HW_LIM_TEMP_FET       -40.0, 110.0     // FET温度限界

// デッドタイム
#define HW_DEAD_TIME_NSEC     200.0            // ns
```

---

## 10. 回路パラメータ

### 10.1 電流測定

```c
#define V_REG                 3.3              // ADC基準電圧
#define CURRENT_AMP_GAIN      20.0             // 電流アンプゲイン
#define CURRENT_SHUNT_RES     0.0005           // シャント抵抗 (Ω)

// ゲイン係数計算
// I = ADC_V / (CURRENT_SHUNT_RES * CURRENT_AMP_GAIN)
```

### 10.2 電圧測定

```c
#define VIN_R1                150000.0         // 上側抵抗 (Ω)
#define VIN_R2                4700.0           // 下側抵抗 (Ω)

// 入力電圧計算
#define GET_INPUT_VOLTAGE()   ((V_REG / 4095.0) * ADC_Value[ADC_IND_VIN_SENS] * ((VIN_R1 + VIN_R2) / VIN_R2))
```

### 10.3 温度測定

```c
// NTC計算 (10kΩ NTC, β=3380)
#define NTC_RES(adc_val)      (10000.0 / ((4095.0 / (float)adc_val) - 1.0))
#define NTC_TEMP(adc_ind)     (1.0 / ((logf(NTC_RES(ADC_Value[adc_ind]) / 10000.0) / 3380.0) + (1.0 / 298.15)) - 273.15)
```

---

## 11. ハードウェアボード一覧

### 11.1 VESC公式

| ボード | ディレクトリ | 特徴 |
|--------|--------------|------|
| VESC Basic | vesc/basic/ | 標準設計 |
| VESC Duet | vesc/duet/ | デュアルモーター |
| VESC Pronto | vesc/pronto/ | 小型版 |
| VESC Str365 | vesc/str365/ | 高出力版 |

### 11.2 サードパーティ

| メーカー | ディレクトリ | 製品例 |
|----------|--------------|--------|
| Flipsky | flipsky/ | FSESC 4.12/6.6等 |
| Stormcore | stormcore/ | Stormcore 60D/100D |
| TRAMPA | trampa/ | VESC 6 |
| Makerbase | makerbase/ | VESC改良版 |
| Luna | luna/ | BBSHD用 |

---

## 12. 新規ハードウェア追加手順

### 12.1 必要ファイル

1. **hw_yourboard.h** - ピン定義、パラメータ
2. **hw_yourboard.c** - GPIO初期化、ハードウェア固有関数

### 12.2 最小限の定義項目

```c
// 必須定義
#define HW_NAME               "YOUR_BOARD"

// ADC関連
#define HW_ADC_CHANNELS       18
#define HW_ADC_INJ_CHANNELS   3
#define HW_ADC_NBR_CONV       6
#define ADC_IND_CURR1         0
#define ADC_IND_CURR2         1
#define ADC_IND_VIN_SENS      11
// ... その他ADCインデックス

// 電流/電圧計算
#define GET_INPUT_VOLTAGE()   (...)
#define CURRENT_AMP_GAIN      20.0
#define CURRENT_SHUNT_RES     0.0005

// GPIO初期化関数
void hw_init_gpio(void);

// 制限値
#define HW_LIM_CURRENT        -100.0, 100.0
#define HW_LIM_VIN            10.0, 60.0
// ...
```

---

## 13. まとめ

### ハードウェア依存箇所の分類

| カテゴリ | 該当箇所 | 抽象化レベル |
|----------|----------|--------------|
| MCU固有 | レジスタ/ペリフェラル | ChibiOS HAL |
| ピン割り当て | GPIO定義 | マクロ定義 |
| ゲートドライバ | DRV8xxx系 | 条件コンパイル |
| センサ | エンコーダ/IMU | プラグイン構造 |
| 通信 | USB/CAN/UART/SPI/I2C | ChibiOS HAL |
| 電気的パラメータ | 電流/電圧係数 | マクロ定義 |

### ポータビリティ

- **同一MCU (STM32F4)**: ハードウェアヘッダ作成のみで対応可能
- **異なるMCU**: ChibiOS HALポート + ペリフェラル設定変更が必要
- **異なるアーキテクチャ**: 大規模な移植作業が必要

VESCファームウェアは、ハードウェア抽象化レイヤーを通じて多様なボードをサポートする設計になっています。新しいハードウェアへの対応は、適切なhw_xxx.h/cファイルを作成することで実現できます。
