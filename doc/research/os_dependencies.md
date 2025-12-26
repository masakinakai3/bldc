# OS依存関係調査報告書

## 概要

本ドキュメントは、VESCファームウェアにおけるOS（オペレーティングシステム）依存関係を調査した結果をまとめたものです。本ファームウェアは**ChibiOS 3.0.5**をリアルタイムオペレーティングシステム（RTOS）として使用しています。

---

## 1. ChibiOS概要

### 1.1 バージョンとライセンス

- **バージョン**: ChibiOS 3.0.5
- **ライセンス**: Apache License 2.0
- **配置場所**: [ChibiOS_3.0.5/](../../ChibiOS_3.0.5/)

### 1.2 ディレクトリ構造

```
ChibiOS_3.0.5/
├── os/
│   ├── common/     # 共通ユーティリティ
│   ├── ext/        # 外部ライブラリ
│   ├── hal/        # Hardware Abstraction Layer
│   ├── nil/        # NIL Kernel (軽量カーネル)
│   ├── rt/         # RT Kernel (フル機能カーネル)
│   └── various/    # 各種ユーティリティ (chprintf, shellなど)
├── ext/            # 外部ライブラリ
├── license.txt
└── readme.txt
```

---

## 2. カーネル構成 (chconf.h)

[chconf.h](../../chconf.h) にてChibiOSカーネルの動作設定が定義されています。

### 2.1 システムタイマー設定

```c
// システムタイマー解像度
#define CH_CFG_ST_RESOLUTION                32

// システムティック周波数 (10kHz = 100μs精度)
#define CH_CFG_ST_FREQUENCY                 10000

// ティックレスモード用デルタ定数
#define CH_CFG_ST_TIMEDELTA                 0
```

### 2.2 カーネルオプション

```c
// ラウンドロビン時間量子 (4ティック)
#define CH_CFG_TIME_QUANTUM                 4

// メモリコアサイズ (0 = 利用可能な全RAM)
#define CH_CFG_MEMCORE_SIZE                 0

// スピード最適化有効
#define CH_CFG_OPTIMIZE_SPEED               TRUE
```

### 2.3 サブシステム有効化

| サブシステム | 有効化 | 説明 |
|-------------|--------|------|
| `CH_CFG_USE_TM` | TRUE | 時間計測API |
| `CH_CFG_USE_REGISTRY` | TRUE | スレッドレジストリ |
| `CH_CFG_USE_WAITEXIT` | TRUE | スレッド同期 |
| `CH_CFG_USE_SEMAPHORES` | TRUE | セマフォ |
| `CH_CFG_USE_MUTEXES` | TRUE | ミューテックス |
| `CH_CFG_USE_CONDVARS` | TRUE | 条件変数 |
| `CH_CFG_USE_EVENTS` | TRUE | イベントフラグ |
| `CH_CFG_USE_MESSAGES` | TRUE | 同期メッセージ |

### 2.4 移植性関連設定

```c
// 浮動小数点対応chprintf
#define CHPRINTF_USE_FLOAT              TRUE

// 簡素化優先度方式
#define CORTEX_SIMPLIFIED_PRIORITY      TRUE

// アイドルスレッドスタックサイズ
#define PORT_IDLE_THREAD_STACK_SIZE     64

// 割り込み要求スタック
#define PORT_INT_REQUIRED_STACK         128
```

---

## 3. HAL構成 (halconf.h)

[halconf.h](../../halconf.h) にてChibiOS HAL（Hardware Abstraction Layer）の設定が定義されています。

### 3.1 有効化されたドライバ

| ドライバ | 設定 | 用途 |
|----------|------|------|
| `HAL_USE_PAL` | TRUE | Port Abstraction Layer (GPIO) |
| `HAL_USE_CAN` | TRUE | CAN通信 |
| `HAL_USE_I2C` | TRUE | I2C通信 |
| `HAL_USE_ICU` | TRUE | Input Capture Unit |
| `HAL_USE_SERIAL` | TRUE | UART通信 |
| `HAL_USE_SERIAL_USB` | TRUE | USB CDC通信 |
| `HAL_USE_SPI` | TRUE | SPI通信 |

### 3.2 無効化されたドライバ

| ドライバ | 設定 | 備考 |
|----------|------|------|
| `HAL_USE_ADC` | FALSE | 直接レジスタアクセス使用 |
| `HAL_USE_DAC` | FALSE | 未使用 |
| `HAL_USE_EXT` | FALSE | 未使用 |
| `HAL_USE_GPT` | FALSE | 直接タイマー制御使用 |
| `HAL_USE_PWM` | FALSE | 直接タイマー制御使用 |

---

## 4. スレッド管理

### 4.1 スレッド定義マクロ

```c
// スレッドスタック領域の静的確保
static THD_WORKING_AREA(timeout_thread_wa, 256);

// スレッド関数定義
static THD_FUNCTION(timeout_thread, arg);
```

### 4.2 主要スレッド一覧

| ファイル | スレッド名 | スタックサイズ | 優先度 | 目的 |
|----------|-----------|---------------|--------|------|
| [main.c](../../main.c) | led_thread | - | NORMALPRIO | LED制御 |
| [main.c](../../main.c) | periodic_thread | - | NORMALPRIO | 定期処理 |
| [main.c](../../main.c) | flash_integrity_check_thread | - | LOWPRIO | フラッシュ整合性チェック |
| [timeout.c](../../timeout.c) | timeout_thread | 256 | NORMALPRIO | タイムアウト監視 |
| [mc_interface.c](../../motor/mc_interface.c) | timer_thread | - | NORMALPRIO | タイマー処理 |
| [mc_interface.c](../../motor/mc_interface.c) | sample_send_thread | - | NORMALPRIO-1 | サンプルデータ送信 |
| [mc_interface.c](../../motor/mc_interface.c) | fault_stop_thread | - | HIGHPRIO-3 | フォールト停止処理 |
| [mc_interface.c](../../motor/mc_interface.c) | stat_thread | - | NORMALPRIO | 統計情報収集 |
| [mcpwm_foc.c](../../motor/mcpwm_foc.c) | timer_thread | - | NORMALPRIO | FOCタイマー |
| [mcpwm_foc.c](../../motor/mcpwm_foc.c) | hfi_thread | - | NORMALPRIO | HFI処理 |
| [mcpwm_foc.c](../../motor/mcpwm_foc.c) | pid_thread | - | NORMALPRIO | PID制御 |
| [worker.c](../../util/worker.c) | work_thread | - | NORMALPRIO | ワーカータスク |

### 4.3 スレッド作成

```c
// 静的スレッド作成
chThdCreateStatic(timeout_thread_wa, sizeof(timeout_thread_wa), 
                  NORMALPRIO, timeout_thread, NULL);
```

### 4.4 優先度定数

| 定数 | 説明 |
|------|------|
| `LOWPRIO` | 低優先度 |
| `NORMALPRIO` | 通常優先度 |
| `HIGHPRIO` | 高優先度 |

---

## 5. 同期プリミティブ

### 5.1 ミューテックス (Mutex)

**使用ファイル**: [mempools.c](../../util/mempools.c), [utils_sys.c](../../util/utils_sys.c) など

```c
// 初期化
static mutex_t packet_buffer_mutex;
chMtxObjectInit(&packet_buffer_mutex);

// ロック/アンロック
chMtxLock(&packet_buffer_mutex);
// クリティカルセクション
chMtxUnlock(&packet_buffer_mutex);
```

### 5.2 クリティカルセクション

**使用ファイル**: [utils_sys.c](../../util/utils_sys.c), [terminal.c](../../terminal.c), [mc_interface.c](../../motor/mc_interface.c)

```c
// 通常コンテキスト用
chSysLock();
// クリティカルセクション
chSysUnlock();

// ISRコンテキスト用
chSysLockFromISR();
// クリティカルセクション
chSysUnlockFromISR();
```

**ネスト対応ヘルパー** ([utils_sys.c](../../util/utils_sys.c)):

```c
// ネストされたロック/アンロック
void utils_sys_lock_cnt(void);
void utils_sys_unlock_cnt(void);
```

### 5.3 イベントフラグ

**使用ファイル**: [mc_interface.c](../../motor/mc_interface.c), [comm_usb.c](../../comm/comm_usb.c)

```c
// イベント通知
chEvtSignal(sample_send_tp, (eventmask_t) 1);
chEvtSignalI(fault_stop_tp, (eventmask_t) 1);  // ISRから

// イベント待機
chEvtWaitAny((eventmask_t) 1);
```

### 5.4 仮想タイマー

**使用ファイル**: [app.c](../../applications/app.c), [app_dpv.c](../../applications/app_dpv.c)

```c
// タイマー型
static virtual_timer_t output_vt = {0};

// タイマー設定
chVTSet(&output_vt, MS2ST(time_ms), output_vt_cb, 0);

// タイマーリセット
chVTReset(&output_vt);

// ISRコンテキスト用
chVTSetI(&dpv_vt, MS2ST(1), update, NULL);
```

---

## 6. 時間管理

### 6.1 時間変換マクロ

| マクロ | 説明 | 使用例 |
|--------|------|--------|
| `MS2ST(ms)` | ミリ秒→システムティック | `MS2ST(100)` |
| `US2ST(us)` | マイクロ秒→システムティック | `US2ST(1000)` |
| `S2ST(s)` | 秒→システムティック | `S2ST(1)` |
| `ST2MS(st)` | システムティック→ミリ秒 | `ST2MS(ticks)` |

### 6.2 時間取得

```c
// 現在のシステム時刻取得
systime_t now = chVTGetSystemTimeX();

// 経過時間計算
if (chVTTimeElapsedSinceX(last_update_time) > MS2ST(timeout_msec)) {
    // タイムアウト処理
}
```

### 6.3 スリープ関数

```c
// ミリ秒スリープ
chThdSleepMilliseconds(10);

// マイクロ秒スリープ
chThdSleepMicroseconds(100);
```

---

## 7. HALドライバ使用

### 7.1 PAL (Port Abstraction Layer)

**GPIO操作の抽象化**

```c
// ピンモード設定
palSetPadMode(BOOT_OK_GPIO, BOOT_OK_PIN, PAL_MODE_OUTPUT_PUSHPULL);
palSetPadMode(HW_UART_TX_PORT, HW_UART_TX_PIN, PAL_MODE_ALTERNATE(HW_UART_GPIO_AF));

// ピン出力操作
palSetPad(BOOT_OK_GPIO, BOOT_OK_PIN);    // High
palClearPad(BOOT_OK_GPIO, BOOT_OK_PIN);  // Low

// ピン入力読み取り
int value = palReadPad(HW_ICU_GPIO, HW_ICU_PIN);
```

### 7.2 Serial Driver (UART)

**使用ファイル**: [si8900.c](../../hwconf/si8900.c), [lispif_vesc_extensions.c](../../lispBM/lispif_vesc_extensions.c)

```c
// ドライバ開始
sdStart(&HW_UART_DEV, &uart_cfg);

// 送信
sdWrite(&HW_UART_DEV, data, size);

// タイムアウト付き送受信
sdWriteTimeout(&HW_SI8900_DEV, txb, tx_len, MS2ST(10));
sdReadTimeout(&HW_SI8900_DEV, rxb, rx_len, MS2ST(10));
```

### 7.3 CAN Driver

**使用ファイル**: [comm_can.c](../../comm/comm_can.c)

```c
// ドライバ開始
canStart(&CAND1, &cancfg);
canStart(&CAND2, &cancfg);

// 送信
msg_t ret = canTransmit(&HW_CAN_DEV, CAN_ANY_MAILBOX, &txmsg, MS2ST(5));

// 受信
msg_t result = canReceive(&HW_CAN_DEV, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE);
```

### 7.4 I2C Driver

**使用ファイル**: 各種ハードウェア設定ファイル ([hw_ENNOID_150V_mk8.c](../../hwconf/ENNOID/MK8/hw_ENNOID_150V_mk8.c) など)

```c
// ドライバ開始
i2cStart(&HW_I2C_DEV, &i2cfg);

// マスター送信
i2cMasterTransmit(&HW_I2C_DEV, addr, txbuf, txsize, rxbuf, rxsize);
```

### 7.5 USB Serial Driver

**使用ファイル**: [comm_usb_serial.c](../../comm/comm_usb_serial.c)

```c
// グローバルドライバインスタンス
SerialUSBDriver SDU1;

// ドライバ開始
usbStart(serusbcfg.usbp, &usbcfg);

// 送信
written = SDU1.vmt->writet(&SDU1, buffer, len, MS2ST(100));
```

### 7.6 ICU (Input Capture Unit)

**使用ファイル**: [servo_dec.c](../../driver/servo_dec.c), [enc_pwm.c](../../encoder/enc_pwm.c)

```c
// コールバック関数
static void icuwidthcb(ICUDriver *icup) {
    // パルス幅取得処理
}

// ドライバ開始
icuStart(&HW_ICU_DEV, &icucfg);
icuStartCapture(&HW_ICU_DEV);
icuEnableNotifications(&HW_ICU_DEV);
```

---

## 8. 割り込み処理

### 8.1 割り込みハンドラマクロ

**使用ファイル**: [irq_handlers.c](../../irq_handlers.c)

```c
CH_IRQ_HANDLER(ADC1_2_3_IRQHandler) {
    CH_IRQ_PROLOGUE();
    
    // 割り込み処理
    
    CH_IRQ_EPILOGUE();
}
```

### 8.2 登録されている割り込みハンドラ

| ハンドラ | ファイル | 目的 |
|----------|----------|------|
| `ADC1_2_3_IRQHandler` | [irq_handlers.c](../../irq_handlers.c) | ADC変換完了 |
| `HW_ENC_EXTI_ISR_VEC` | [irq_handlers.c](../../irq_handlers.c) | エンコーダEXTI |
| `HW_ENC_TIM_ISR_VEC` | [irq_handlers.c](../../irq_handlers.c) | エンコーダタイマー |
| `TIM2_IRQHandler` | [irq_handlers.c](../../irq_handlers.c) | TIM2割り込み |
| `PVD_IRQHandler` | [irq_handlers.c](../../irq_handlers.c) | 電圧検出 |
| `NMI_Handler` | [irq_handlers.c](../../irq_handlers.c) | NMI例外 |
| `HardFault_Handler` | [irq_handlers.c](../../irq_handlers.c) | ハードフォールト |
| `MemManage_Handler` | [irq_handlers.c](../../irq_handlers.c) | メモリ管理例外 |
| `BusFault_Handler` | [irq_handlers.c](../../irq_handlers.c) | バスフォールト |
| `UsageFault_Handler` | [irq_handlers.c](../../irq_handlers.c) | 使用フォールト |

---

## 9. システム初期化

### 9.1 初期化シーケンス

**ファイル**: [main.c](../../main.c)

```c
int main(void) {
    halInit();      // HAL初期化 (ChibiOS HAL)
    chSysInit();    // カーネル初期化 (ChibiOS RT)
    
    // ハードウェア早期初期化
    HW_EARLY_INIT();
    
    // 各サブシステム初期化
    mempools_init();
    events_init();
    timer_init();
    hw_init_gpio();
    // ...
}
```

### 9.2 ウォッチドッグタイマー

**ファイル**: [timeout.c](../../timeout.c)

```c
// IWDG設定 (ChibiOS非依存、STM32ペリフェラル直接操作)
IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
IWDG_SetPrescaler(IWDG_Prescaler_4);
IWDG_SetReload(140);  // 約12ms
IWDG_ReloadCounter();
IWDG_Enable();
```

---

## 10. ChibiOS固有データ型

### 10.1 システム型

| 型 | 説明 | 定義場所 |
|----|------|----------|
| `systime_t` | システム時刻 | ChibiOS RT |
| `thread_t` | スレッド構造体 | ChibiOS RT |
| `msg_t` | メッセージ型 | ChibiOS RT |
| `stkalign_t` | スタックアライメント型 | ChibiOS RT |
| `eventmask_t` | イベントマスク | ChibiOS RT |

### 10.2 同期オブジェクト型

| 型 | 説明 |
|----|------|
| `mutex_t` | ミューテックス |
| `semaphore_t` | セマフォ |
| `binary_semaphore_t` | バイナリセマフォ |
| `virtual_timer_t` | 仮想タイマー |
| `event_listener_t` | イベントリスナー |
| `memory_pool_t` | メモリプール |

---

## 11. ユーティリティ関数

### 11.1 chprintf

**ビルド設定**: [make/fw.mk](../../make/fw.mk)

```makefile
$(CHIBIOS)/os/hal/lib/streams/chprintf.c
```

**使用例**:
```c
#include "chprintf.h"
chprintf(chp, "Value: %d\r\n", value);
```

### 11.2 スレッドレジストリ

**使用ファイル**: [terminal.c](../../terminal.c)

```c
// スレッド一覧の取得
thread_t *tp = chRegFirstThread();
while (tp != NULL) {
    // スレッド情報表示
    tp = chRegNextThread(tp);
}
```

---

## 12. 移植性に関する考慮事項

### 12.1 ChibiOS依存APIの集中箇所

| カテゴリ | 主要ファイル |
|----------|-------------|
| スレッド管理 | main.c, timeout.c, mc_interface.c, mcpwm_foc.c |
| 同期プリミティブ | mempools.c, utils_sys.c |
| HALドライバ | comm_can.c, comm_usb_serial.c, hwconf/*.c |
| 割り込み処理 | irq_handlers.c |
| 時間管理 | 全体に分散 |

### 12.2 他RTOSへの移植時の影響

他のRTOSへ移植する場合、以下の変更が必要になります：

1. **スレッドAPI置換**
   - `THD_WORKING_AREA` / `THD_FUNCTION` / `chThdCreateStatic`
   - スレッド優先度定数 (LOWPRIO, NORMALPRIO, HIGHPRIO)

2. **同期プリミティブ置換**
   - `chMtx*` → ターゲットRTOSのミューテックスAPI
   - `chEvt*` → ターゲットRTOSのイベントAPI
   - `chVT*` → ターゲットRTOSのソフトウェアタイマーAPI

3. **クリティカルセクション置換**
   - `chSysLock/Unlock` → ターゲットRTOSの割り込み禁止API

4. **時間管理置換**
   - `MS2ST`, `chThdSleepMilliseconds` → ターゲットRTOSの時間API

5. **HALドライバ置換**
   - PAL, Serial, CAN, I2C, ICU等のドライバ → ターゲットプラットフォームのドライバ

### 12.3 抽象化レイヤー

現在、一部のChibiOS依存部分は以下のファイルでラップされています：

- [utils_sys.c](../../util/utils_sys.c) - システムロックのネスト対応
- [mempools.c](../../util/mempools.c) - メモリプール管理

---

## 13. まとめ

VESCファームウェアはChibiOS 3.0.5に深く依存しており、以下の機能を使用しています：

1. **RTカーネル** - スレッド管理、スケジューリング、同期プリミティブ
2. **HAL** - GPIO、UART、CAN、I2C、USB、ICUドライバ
3. **ユーティリティ** - chprintf、スレッドレジストリ

他のRTOSへの移植には大規模な変更が必要ですが、ハードウェア抽象化層（HAL）の設計により、ペリフェラルアクセスは比較的整理されています。

---

## 参考資料

- [ChibiOS Documentation](http://www.chibios.org/dokuwiki/doku.php)
- [ChibiOS 3.0.5 readme.txt](../../ChibiOS_3.0.5/readme.txt)
- [VESC Firmware Japanese Documentation](../../VESC_ファームウェア説明書.md)
