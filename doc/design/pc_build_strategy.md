# PC用GCCビルド戦略設計書

## 1. 概要

### 1.1 目的

VESCファームウェアをPC用GCC（x86/x64）でビルド可能にし、以下を実現する：

- **ユニットテスト**: ホストPC上でロジック検証
- **シミュレーション**: ハードウェアなしでのアルゴリズム検証
- **静的解析**: PCツールチェーンでのコード品質チェック
- **開発効率向上**: 高速なビルド・デバッグサイクル

### 1.2 基本方針

**ソースコードは変更しない**という制約の下、以下のアプローチを採用：

1. **追加ヘッダファイル**による抽象化レイヤー提供
2. **スタブ/モック実装**の別ファイル提供
3. **専用Makefile**によるPC向けビルド構成
4. **条件コンパイル**の活用（既存マクロの利用）

### 1.3 検証ステータス

> **検証日**: リポジトリ詳細調査により以下を確認
>
> ✅ **実現可能性**: 高い（既存のインフラストラクチャを活用可能）
> 
> 📁 **既存リソース活用**: 当初想定より作業量を大幅に削減可能
> 
> ⚠️ **要修正事項**: 本ドキュメントの後半「11. 検証結果と修正事項」を参照

---

## 2. 依存関係の分析と対策

### 2.1 コンパイラ依存対策

#### 2.1.1 GCCプラグマ

| 依存箇所 | 対策 |
|----------|------|
| `#pragma GCC optimize ("Os")` | PC GCCでも有効（互換性あり） |
| `#pragma GCC push_options` / `pop_options` | PC GCCでも有効（互換性あり） |

**結論**: GCCプラグマはPC用GCCでもそのまま動作するため、対策不要。

#### 2.1.2 GCC属性 (`__attribute__`)

| 属性 | ARM用途 | PC対策 |
|------|---------|--------|
| `__attribute__((section(".ram4")))` | CCM SRAMに配置 | **無視**（通常メモリに配置） |
| `__attribute__((section(".libif")))` | 特定アドレスに配置 | **無視** |
| `__attribute__((aligned(N)))` | メモリアライメント | PC GCCでも有効 |
| `__attribute__((packed))` | 構造体パッキング | PC GCCでも有効 |

**対策**: リンカースクリプトでセクションを定義しないことで、自動的に通常セクションに配置される。

#### 2.1.3 コンパイラフラグ

| ARMフラグ | PC代替 |
|-----------|--------|
| `-mthumb` | 削除 |
| `-mfloat-abi=hard` | 削除 |
| `-mfpu=fpv4-sp-d16` | 削除 |
| `-fsingle-precision-constant` | 維持（PC GCCでも有効） |
| `-mcpu=cortex-m4` | 削除 |
| `-std=gnu99` | 維持 |

### 2.2 ハードウェア依存対策

#### 2.2.1 ペリフェラルレジスタアクセス

**課題**: STM32F4のペリフェラルレジスタ（GPIO, TIM, ADC, DMA等）への直接アクセス

**対策**: スタブヘッダで空のレジスタ構造体を定義

> ✅ **検証結果**: `lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST/stm32f407xx.h` に完全なペリフェラルTypedef定義が存在
> 
> このファイル（7954行）には `GPIO_TypeDef`, `TIM_TypeDef`, `ADC_TypeDef`, `DMA_Stream_TypeDef`, `RCC_TypeDef`, `SPI_TypeDef`, `USART_TypeDef` など全てのペリフェラル構造体が定義済み。
> 
> **推奨アプローチ**: 新規スタブを作成するのではなく、既存ヘッダをPC向けに適応させるラッパーヘッダを作成

```c
// pc_build/stubs/stm32f4xx_pc.h
#ifndef _STM32F4XX_PC_H_
#define _STM32F4XX_PC_H_

// CMSISの__IO等のマクロをPC用に定義
#define __IO volatile
#define __I  volatile const
#define __O  volatile

// 既存の完全な構造体定義を再利用
#include "lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST/stm32f407xx.h"

// ペリフェラルインスタンスのextern宣言（スタブ側で実体を定義）
extern GPIO_TypeDef GPIOA_Instance, GPIOB_Instance, GPIOC_Instance;
extern GPIO_TypeDef GPIOD_Instance, GPIOE_Instance, GPIOF_Instance;
// ... 他のペリフェラルも同様

#define GPIOA (&GPIOA_Instance)
#define GPIOB (&GPIOB_Instance)
// ...

#endif
```

#### 2.2.2 STM32 Standard Peripheral Library

> ✅ **検証結果**: `lispBM/c_libs/stdperiph_stm32f4/inc/` に以下のヘッダが存在
> - `stm32f4xx_tim.h` (1151行) - `TIM_TimeBaseInitTypeDef`, `TIM_OCInitTypeDef` 等
> - `stm32f4xx_adc.h` - ADC設定構造体
> - `stm32f4xx_dma.h` - DMA設定構造体
> - `stm32f4xx_gpio.h` (AF定義: `stm32f4_gpio_af.h`)
> - `stm32f4xx_rcc.h`, `stm32f4xx_flash.h`, `stm32f4xx_iwdg.h`, `misc.h`(NVIC)
>
> **推奨アプローチ**: 型定義は既存ヘッダを再利用し、関数のみスタブを提供

**対策**: スタブ関数を提供

```c
// stubs/stm32f4xx_gpio_stub.c
void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct) {
    // 空実装またはログ出力
}

void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    GPIOx->ODR |= GPIO_Pin;
}
```

#### 2.2.3 CMSIS Intrinsics

> ✅ **検証結果**: `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/` に以下が存在
> - `core_cm4.h` (1773行) - Cortex-M4コア定義
> - `core_cmFunc.h` (640行) - `__get_PRIMASK()`, `__set_PRIMASK()` 等の関数
> - `core_cmInstr.h` - `__NOP()`, `__WFI()`, `__DMB()`, `__DSB()`, `__ISB()` 等
> - `core_cm4_simd.h` - SIMD命令
> - `arm_math.h` - DSP関数
>
> **課題**: これらはインラインアセンブリを含むため、PC用には空の実装が必要

**対策**: 空マクロまたは互換関数で置換

```c
// stubs/cmsis_stub.h
#define __NOP()         do {} while(0)
#define __WFI()         do {} while(0)
#define __DMB()         do {} while(0)
#define __DSB()         do {} while(0)
#define __ISB()         do {} while(0)
#define __disable_irq() do {} while(0)
#define __enable_irq()  do {} while(0)

static inline uint32_t __get_PRIMASK(void) { return 0; }
static inline void __set_PRIMASK(uint32_t val) { (void)val; }
```

### 2.3 OS (ChibiOS) 依存対策

#### 2.3.1 POSIX互換レイヤー

ChibiOS APIをPOSIXスレッド/同期プリミティブにマッピング：

| ChibiOS API | POSIX代替 |
|-------------|-----------|
| `chThdCreateStatic` | `pthread_create` |
| `chThdSleepMilliseconds` | `usleep` / `nanosleep` |
| `chMtxLock` / `chMtxUnlock` | `pthread_mutex_lock/unlock` |
| `chSemWait` / `chSemSignal` | `sem_wait` / `sem_post` |
| `chEvtSignal` / `chEvtWaitAny` | 条件変数 + ミューテックス |
| `chSysLock` / `chSysUnlock` | `pthread_mutex_lock/unlock` (グローバル) |
| `chVTGetSystemTimeX` | `clock_gettime(CLOCK_MONOTONIC)` |

#### 2.3.2 ChibiOS HALドライバ

| ChibiOS HAL | PC代替 |
|-------------|--------|
| PAL (GPIO) | メモリ上の変数操作 |
| Serial | ファイルI/O または pty |
| CAN | SocketCAN または スタブ |
| I2C/SPI | スタブ |
| USB | libusb または スタブ |
| ICU | スタブ |

---

## 3. ファイル構成

### 3.0 既存テストインフラストラクチャ（検証結果）

> ✅ **重要な発見**: 既存の `tests/` ディレクトリと `make/` ディレクトリに再利用可能なインフラが存在

#### 既存テストディレクトリ

| ディレクトリ | 内容 | テスト方式 |
|-------------|------|-----------|
| `tests/utils_math/` | utils_math.c の単体テスト | Google Test + gcov |
| `tests/angles/` | 角度計算のテスト | 純粋C (`-DNO_STM32`) |
| `tests/packet_recovery/` | パケット処理のテスト | 純粋C (`-DNO_STM32`) |
| `tests/float_serialization/` | buffer.c のシリアライズテスト | 純粋C (`-DNO_STM32`) |

#### 既存スタブ実装 (`tests/utils_math/`)

**ch.h** (9行):
```c
#ifndef CH_H
#define CH_H
typedef int systime_t;
typedef struct {
   uint32_t *p_stklimit;
} thread_t;
#endif
```

**hal.h** (26行):
```c
#ifndef HAL_H
#define HAL_H
void chSysLock(){}
void chSysUnlock(){}
int READ_HALL1(){ return 0; }
int READ_HALL2(){ return 0; }
int READ_HALL3(){ return 0; }
int READ_HALL1_2(){ return 0; }
int READ_HALL2_2(){ return 0; }
int READ_HALL3_2(){ return 0; }
#endif
```

#### 既存ビルドシステム (`make/`)

| ファイル | 機能 |
|----------|------|
| `make/unittest-defs.mk` | コンパイラ定義、gcov設定 |
| `make/unittest.mk` | Google Test統合、`override THUMB :=` でARMモード無効化 |

**重要**: `unittest.mk` の `override THUMB :=` により、THUMBモードフラグを空文字列に上書きしてPC向けビルドを可能にしている。

### 3.1 新規追加ファイル

> 📝 **更新**: 既存リソースを活用することで、当初計画より少ないファイルで実現可能

```
bldc/
├── pc_build/                          # PC向けビルド用ディレクトリ
│   ├── Makefile                       # PC用Makefile
│   ├── stubs/                         # スタブ実装
│   │   ├── stm32f4xx_pc.h             # PC用STM32定義ラッパー（既存ヘッダを再利用）
│   │   ├── stm32f4xx_pc.c             # ペリフェラルインスタンス実体
│   │   ├── stm32f4xx_gpio_stub.c      # GPIOスタブ関数
│   │   ├── stm32f4xx_tim_stub.c       # タイマースタブ関数
│   │   ├── stm32f4xx_adc_stub.c       # ADCスタブ関数
│   │   ├── stm32f4xx_dma_stub.c       # DMAスタブ関数
│   │   ├── stm32f4xx_rcc_stub.c       # RCCスタブ関数
│   │   ├── cmsis_pc.h                 # CMSIS intrinsicsスタブ
│   │   └── misc_stub.c                # NVIC等のスタブ
│   │
│   ├── chibios_posix/                 # ChibiOS POSIX互換レイヤー
│   │   ├── ch.h                       # ChibiOS RT API互換ヘッダ
│   │   ├── hal.h                      # ChibiOS HAL互換ヘッダ（tests/utils_math/を拡張）
│   │   ├── chconf.h                   # 設定（PC用）
│   │   ├── halconf.h                  # HAL設定（PC用）
│   │   ├── ch_core.c                  # スレッド/同期実装
│   │   ├── ch_time.c                  # 時間管理実装
│   │   └── hal_pal.c                  # GPIO (PAL)実装
│   │
│   └── hw_stub/                       # ハードウェアスタブ
│       ├── hw_stub.h                  # hw.h互換スタブ
│       └── hw_stub.c                  # 初期化関数等
│
├── lispBM/c_libs/stdperiph_stm32f4/   # ★既存リソース（再利用）
│   ├── CMSIS/ST/stm32f407xx.h         # ペリフェラルTypedef定義
│   ├── CMSIS/include/core_cm4.h       # CMSIS定義
│   └── inc/stm32f4xx_*.h              # Standard Peripheral Library型定義
│
├── tests/utils_math/                   # ★既存リソース（参考実装）
│   ├── ch.h                           # 簡易ChibiOSスタブ
│   └── hal.h                          # 簡易HALスタブ
│
└── make/                              # ★既存リソース（再利用）
    ├── unittest-defs.mk               # コンパイラ定義
    └── unittest.mk                    # Google Test統合
```

### 3.2 既存ファイルとの関係

```
bldc/
├── *.c, *.h                   # 既存ソース（変更なし）
├── motor/                      # モーター制御（変更なし）
├── comm/                       # 通信（変更なし）
├── applications/               # アプリケーション（変更なし）
├── ChibiOS_3.0.5/             # ChibiOS（使用しない）
│
├── pc_build/                  # ← 新規追加
│   └── ...
│
└── tests/                     # 既存テスト
    └── utils_math/            # 参考：既存のスタブ実装
        ├── ch.h               # ← 簡易ChibiOSスタブの例
        └── hal.h
```

---

## 4. ビルドシステム設計

### 4.1 PC用Makefile

```makefile
# pc_build/Makefile

# コンパイラ設定
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g -O2
CFLAGS += -fsingle-precision-constant
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DUSE_PC_BUILD

# インクルードパス（PC用スタブを優先）
CFLAGS += -I$(PC_BUILD)/stubs
CFLAGS += -I$(PC_BUILD)/chibios_posix
CFLAGS += -I$(PC_BUILD)/hw_stub
CFLAGS += -I$(PC_BUILD)/config
CFLAGS += -I$(ROOT)
CFLAGS += -I$(ROOT)/motor
CFLAGS += -I$(ROOT)/comm
CFLAGS += -I$(ROOT)/applications
CFLAGS += -I$(ROOT)/util
CFLAGS += -I$(ROOT)/encoder
CFLAGS += -I$(ROOT)/hwconf
CFLAGS += -I$(ROOT)/imu
CFLAGS += -I$(ROOT)/driver

# リンカフラグ
LDFLAGS = -lpthread -lm -lrt

# ソースファイル
STUB_SRC = $(wildcard $(PC_BUILD)/stubs/*.c)
POSIX_SRC = $(wildcard $(PC_BUILD)/chibios_posix/*.c)
HW_STUB_SRC = $(wildcard $(PC_BUILD)/hw_stub/*.c)

# ビルド対象の既存ソース（選択的）
VESC_SRC = \
    $(ROOT)/util/utils_math.c \
    $(ROOT)/util/utils_sys.c \
    $(ROOT)/util/buffer.c \
    $(ROOT)/util/crc.c \
    # ... 必要なファイルを追加

# ビルドターゲット
all: libvesc_pc.a

libvesc_pc.a: $(STUB_OBJ) $(POSIX_OBJ) $(HW_STUB_OBJ) $(VESC_OBJ)
	$(AR) rcs $@ $^

# テスト用実行ファイル
test: libvesc_pc.a
	$(CC) $(CFLAGS) -o test_runner tests/*.c -L. -lvesc_pc $(LDFLAGS)

clean:
	rm -f *.o *.a test_runner
```

### 4.2 インクルード順序の制御

PC用スタブヘッダを元のヘッダより先に検索させることで、既存ソースを変更せずにスタブを使用：

```
インクルード検索順序:
1. pc_build/stubs/          ← STM32レジスタスタブ
2. pc_build/chibios_posix/  ← ChibiOS POSIX互換
3. pc_build/hw_stub/        ← ハードウェアスタブ
4. bldc/                    ← 既存ソース
5. bldc/motor/              ← 既存モーター制御
...
```

### 4.3 条件コンパイルの活用

> ✅ **検証結果**: 重要な既存マクロ `NO_STM32` を発見
> 
> `util/crc.c` で以下のパターンが使用されている：
> ```c
> #ifndef NO_STM32
> #include "stm32f4xx.h"
> #endif
> ```
> 
> `tests/angles/Makefile`, `tests/packet_recovery/Makefile`, `tests/float_serialization/Makefile` で使用：
> ```makefile
> CFLAGS = -O2 -g -Wall -Wextra -Wundef -std=gnu99 -I../../util -DNO_STM32
> ```
>
> **推奨**: 新規 `USE_PC_BUILD` より、既存の `NO_STM32` マクロを優先使用

既存コードの `#ifdef` を活用：

| マクロ | 用途 | 状態 |
|--------|------|------|
| `NO_STM32` | STM32依存コード無効化 | ✅ 既存・使用中 |
| `USE_PC_BUILD` | PC向けビルドフラグ（新規定義） | 📝 必要に応じて追加 |
| `HW_HAS_NO_CAN` | CAN無効化（既存） | ✅ 既存 |
| `COMM_USE_USB` | USB有効/無効（既存） | ✅ 既存 |

---

## 5. 主要スタブ実装設計

### 5.1 ChibiOS RT スタブ (`chibios_posix/ch.h`)

```c
#ifndef _CH_H_
#define _CH_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// システム型
typedef uint32_t systime_t;
typedef int32_t msg_t;
typedef uint32_t eventmask_t;

// スレッド型
typedef struct {
    pthread_t thread;
    int motor_selected;
    void *p_stklimit;
} thread_t;

// ミューテックス型
typedef struct {
    pthread_mutex_t mutex;
} mutex_t;

// セマフォ型
typedef struct {
    sem_t sem;
} semaphore_t;

// 仮想タイマー型
typedef struct {
    bool armed;
    systime_t deadline;
    void (*callback)(void *);
    void *arg;
} virtual_timer_t;

// 優先度定数
#define LOWPRIO     1
#define NORMALPRIO  128
#define HIGHPRIO    255

// スレッドマクロ
#define THD_WORKING_AREA(name, size) \
    static uint8_t name[size + sizeof(thread_t)]

#define THD_FUNCTION(name, arg) \
    void* name(void* arg)

// API関数宣言
thread_t* chThdCreateStatic(void *wsp, size_t size, int prio,
                            void* (*pf)(void*), void *arg);
void chThdSleepMilliseconds(uint32_t ms);
void chThdSleepMicroseconds(uint32_t us);
thread_t* chThdGetSelfX(void);

void chMtxObjectInit(mutex_t *mp);
void chMtxLock(mutex_t *mp);
void chMtxUnlock(mutex_t *mp);

void chSysLock(void);
void chSysUnlock(void);
void chSysLockFromISR(void);
void chSysUnlockFromISR(void);

systime_t chVTGetSystemTimeX(void);
void chEvtSignal(thread_t *tp, eventmask_t mask);
void chEvtSignalI(thread_t *tp, eventmask_t mask);
eventmask_t chEvtWaitAny(eventmask_t mask);

void chVTSet(virtual_timer_t *vtp, systime_t delay,
             void (*vtfunc)(void *), void *par);
void chVTSetI(virtual_timer_t *vtp, systime_t delay,
              void (*vtfunc)(void *), void *par);
void chVTReset(virtual_timer_t *vtp);

// 時間変換マクロ (10kHz tick想定)
#define MS2ST(ms)   ((systime_t)((ms) * 10))
#define US2ST(us)   ((systime_t)((us) / 100))
#define ST2MS(st)   ((uint32_t)((st) / 10))
#define S2ST(s)     ((systime_t)((s) * 10000))

#define CH_CFG_ST_FREQUENCY 10000

#endif /* _CH_H_ */
```

### 5.2 ChibiOS HAL PAL スタブ (`chibios_posix/hal_pal.c`)

```c
#include "hal.h"
#include <string.h>

// GPIOインスタンス
static GPIO_TypeDef gpio_instances[16];

GPIO_TypeDef* _pal_get_port(int port_id) {
    return &gpio_instances[port_id];
}

void palSetPadMode(GPIO_TypeDef* port, uint8_t pad, uint32_t mode) {
    (void)port; (void)pad; (void)mode;
    // モード設定をシミュレート（必要に応じてログ出力）
}

void palSetPad(GPIO_TypeDef* port, uint8_t pad) {
    if (port && pad < 16) {
        port->ODR |= (1U << pad);
    }
}

void palClearPad(GPIO_TypeDef* port, uint8_t pad) {
    if (port && pad < 16) {
        port->ODR &= ~(1U << pad);
    }
}

uint8_t palReadPad(GPIO_TypeDef* port, uint8_t pad) {
    if (port && pad < 16) {
        return (port->IDR >> pad) & 1;
    }
    return 0;
}
```

### 5.3 STM32 レジスタスタブ (`stubs/stm32f4xx_stub.h`)

```c
#ifndef _STM32F4XX_STUB_H_
#define _STM32F4XX_STUB_H_

#include <stdint.h>

// GPIO
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

// タイマー
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

// ADC
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

// DMA
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t NDTR;
    volatile uint32_t PAR;
    volatile uint32_t M0AR;
    volatile uint32_t M1AR;
    volatile uint32_t FCR;
} DMA_Stream_TypeDef;

// 外部宣言
extern GPIO_TypeDef GPIOA_Instance, GPIOB_Instance, GPIOC_Instance;
extern GPIO_TypeDef GPIOD_Instance, GPIOE_Instance, GPIOF_Instance;
extern TIM_TypeDef TIM1_Instance, TIM2_Instance, TIM3_Instance;
extern TIM_TypeDef TIM4_Instance, TIM5_Instance, TIM8_Instance;
extern ADC_TypeDef ADC1_Instance, ADC2_Instance, ADC3_Instance;

#define GPIOA (&GPIOA_Instance)
#define GPIOB (&GPIOB_Instance)
#define GPIOC (&GPIOC_Instance)
#define GPIOD (&GPIOD_Instance)
#define GPIOE (&GPIOE_Instance)
#define GPIOF (&GPIOF_Instance)
#define TIM1  (&TIM1_Instance)
#define TIM2  (&TIM2_Instance)
#define TIM3  (&TIM3_Instance)
#define TIM4  (&TIM4_Instance)
#define TIM5  (&TIM5_Instance)
#define TIM8  (&TIM8_Instance)
#define ADC1  (&ADC1_Instance)
#define ADC2  (&ADC2_Instance)
#define ADC3  (&ADC3_Instance)

#endif /* _STM32F4XX_STUB_H_ */
```

---

## 6. 段階的実装計画

> 📝 **更新**: 既存インフラストラクチャの活用により、工数を大幅に削減

### Phase 1: 基盤構築（3-5日） ← 当初1-2週間から短縮

1. **ディレクトリ構造作成**
   - `pc_build/` 以下のフォルダ作成
   
2. **最小限のスタブ作成**
   - `stm32f4xx_pc.h` - 既存 `stm32f407xx.h` を再利用するラッパー
   - `cmsis_pc.h` - CMSIS intrinsics空実装
   - `ch.h` - `tests/utils_math/ch.h` を拡張
   - `hal.h` - `tests/utils_math/hal.h` を拡張

3. **PC用Makefile作成**
   - 既存 `make/unittest-defs.mk`, `make/unittest.mk` を参考に構築
   - `override THUMB :=` パターンを活用

### Phase 2: ユーティリティビルド（3-5日） ← 当初1週間から短縮

1. **ビルド対象** (ハードウェア非依存で既に検証済み)
   - `util/utils_math.c` - 既存テスト `tests/utils_math/` あり
   - `util/buffer.c` - 既存テスト `tests/float_serialization/` あり (`-DNO_STM32`)
   - `util/crc.c` - 既存の `NO_STM32` 条件コンパイルあり

2. **テスト**
   - 既存テストをそのまま実行して動作確認
   - Google Test形式への統一は任意

### Phase 3: ChibiOS互換レイヤー（1-2週間） ← 当初2-3週間から短縮

1. **スレッド管理**
   - `ch_core.c` - pthread ベースの実装
   - 既存の `tests/utils_math/ch.h` の型定義を拡張

2. **同期プリミティブ**
   - ミューテックス: `pthread_mutex_t` ラップ
   - セマフォ: `sem_t` ラップ
   - イベント: 条件変数で実装

3. **時間管理**
   - `ch_time.c` - `clock_gettime(CLOCK_MONOTONIC)` ベース

### Phase 4: HAL/ペリフェラルスタブ（1週間） ← 当初2週間から短縮

> ✅ **検証結果**: 型定義は既存ヘッダを再利用できるため、関数スタブのみ実装

1. **GPIO (PAL)**
   - メモリ上の変数操作

2. **タイマー・ADC・DMA**
   - 空関数または基本シミュレーション

3. **割り込みハンドラ**
   - 空関数として定義

### Phase 5: モーター制御ロジック（2-3週間）

1. **ビルド対象拡大**
   - `motor/mc_interface.c` （ハードウェア非依存部分）
   - `motor/mcpwm_foc.c` （アルゴリズム部分）

2. **ADC/PWMシミュレーション**
   - 入力データをファイルから読み込み
   - 出力データをファイルに記録

### Phase 6: 統合テスト（1週間） ← 当初1-2週間から短縮

1. **アプリケーション層**
   - 選択的なアプリケーションのビルド

2. **シミュレーション実行**
   - モーター制御ループのシミュレーション

---

## 7. 既存コードとの互換性維持

### 7.1 ヘッダガード方式

既存ヘッダと同名のスタブヘッダを作成し、インクルードパスの優先順位で制御：

```c
// pc_build/stubs/stm32f4xx_conf.h
#ifndef _STM32F4XX_CONF_H_
#define _STM32F4XX_CONF_H_

// PC用の空定義または最小限の定義
#include "stm32f4xx_stub.h"

#endif
```

### 7.2 条件コンパイルの活用

```c
// 既存コードで使用されている条件
#ifdef HW_HAS_NO_CAN
    // CAN無効
#endif

// PC向けビルドで定義
// CFLAGS += -DHW_HAS_NO_CAN
```

### 7.3 リンク時のシンボル解決

未解決シンボルはスタブ関数で解決：

```c
// stubs/unresolved_stub.c
void DMA2_Stream0_IRQHandler(void) { /* 空 */ }
void TIM1_UP_TIM10_IRQHandler(void) { /* 空 */ }
// 必要に応じて追加
```

---

## 8. テスト戦略

### 8.1 ユニットテスト

```c
// tests/test_utils_math.c
#include "utils_math.h"
#include <assert.h>

void test_utils_truncate_number(void) {
    float val = 150.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    assert(val == 100.0f);
}

int main(void) {
    test_utils_truncate_number();
    printf("All tests passed!\n");
    return 0;
}
```

### 8.2 シミュレーションテスト

```c
// tests/test_foc_simulation.c
// ADC値をファイルから読み込み、制御ループを実行
void run_foc_simulation(const char* adc_data_file) {
    // ADCデータ読み込み
    // FOC計算実行
    // PWMデューティ出力をファイルに記録
}
```

---

## 9. 制限事項と注意点

### 9.1 サポート外の機能

| 機能 | 理由 |
|------|------|
| 実タイムPWM出力 | ハードウェアタイマーが必要 |
| 実際の電流測定 | ADCハードウェアが必要 |
| 実際のCAN通信 | CANコントローラが必要（SocketCAN除く） |
| USB CDC通信 | USBハードウェアが必要 |
| DMAトランザクション | DMAコントローラが必要 |
| ウォッチドッグ | ハードウェアウォッチドッグ |

### 9.2 動作の差異

| 項目 | ARM実機 | PC |
|------|---------|-----|
| 浮動小数点精度 | 単精度 (FPU) | 倍精度→単精度変換 |
| タイミング | リアルタイム | 非リアルタイム |
| 割り込み | ハードウェア割り込み | シグナル/コールバック |
| メモリ配置 | CCM SRAM分離 | 統一メモリ空間 |

### 9.3 注意点

1. **浮動小数点の差異**
   - PC GCCはデフォルトで倍精度を使用
   - `-fsingle-precision-constant` を指定しても内部計算で差異が生じる可能性

2. **エンディアン**
   - STM32F4 (ARM Cortex-M4): リトルエンディアン
   - x86/x64: リトルエンディアン
   - → 互換性あり

3. **アライメント**
   - x86はアライメント制約が緩いため、ARM上で問題となるコードを検出できない可能性

---

## 10. まとめ

### 10.1 実現可能性

ソースコードを変更せずにPC用GCCでビルドするアプローチは**実現可能**です。主な手法は：

1. インクルードパスの優先順位制御によるスタブヘッダの注入
2. リンク時のスタブ関数によるシンボル解決
3. ChibiOS POSIX互換レイヤーの提供

### 10.2 想定工数（検証後に更新）

| フェーズ | 当初見積もり | 検証後見積もり | 削減理由 |
|----------|-------------|---------------|----------|
| 基盤構築 | 1-2週間 | 3-5日 | 既存ヘッダ再利用 |
| ユーティリティ | 1週間 | 3-5日 | 既存テスト活用 |
| ChibiOS互換 | 2-3週間 | 1-2週間 | 既存スタブ拡張 |
| HALスタブ | 2週間 | 1週間 | 型定義再利用 |
| モーター制御 | 2-3週間 | 2-3週間 | 変更なし |
| 統合テスト | 1-2週間 | 1週間 | 既存インフラ活用 |
| **合計** | **9-13週間** | **6-9週間** | **約30-40%削減** |

### 10.3 期待される効果

1. **開発効率向上**: ホストPC上での高速ビルド・デバッグ
2. **テスト容易性**: ユニットテストの実行環境提供
3. **静的解析**: PC向けツール（Valgrind, AddressSanitizer等）の活用
4. **CI/CD統合**: クラウドビルドサービスでの自動テスト

---

## 11. 検証結果と修正事項

### 11.1 発見された既存リソース

リポジトリの詳細調査により、以下の再利用可能なリソースを発見：

#### 11.1.1 STM32ペリフェラル定義（`lispBM/c_libs/stdperiph_stm32f4/`）

| ファイル | 内容 | 行数 |
|----------|------|------|
| `CMSIS/ST/stm32f407xx.h` | 全ペリフェラルのTypedef定義 | 7954 |
| `CMSIS/ST/stm32f4xx.h` | デバイス選択ラッパー | 279 |
| `CMSIS/include/core_cm4.h` | Cortex-M4コア定義 | 1773 |
| `CMSIS/include/core_cmFunc.h` | コア関数定義 | 640 |
| `CMSIS/include/core_cmInstr.h` | 命令定義 | - |
| `inc/stm32f4xx_tim.h` | TIM型定義・定数 | 1151 |
| `inc/stm32f4xx_adc.h` | ADC型定義・定数 | - |
| `inc/stm32f4xx_dma.h` | DMA型定義・定数 | - |
| `inc/stm32f4xx_rcc.h` | RCC型定義・定数 | - |
| `inc/misc.h` | NVIC型定義・定数 | - |

**影響**: 当初計画していた`stm32f4xx_stub.h`での構造体定義が不要。ラッパーヘッダでの再利用が可能。

#### 11.1.2 既存テストインフラ（`tests/`, `make/`）

| コンポーネント | 場所 | 内容 |
|---------------|------|------|
| Google Test統合 | `make/unittest.mk` | テストフレームワーク、gcovサポート |
| ARMモード無効化 | `make/unittest.mk` | `override THUMB :=` |
| コンパイラ定義 | `make/unittest-defs.mk` | gcov、リンク設定 |
| ChibiOSスタブ | `tests/utils_math/ch.h` | 基本的な型定義 |
| HALスタブ | `tests/utils_math/hal.h` | chSysLock/Unlock、READ_HALL |

**影響**: 新規ビルドシステムを最初から作成する必要がなく、既存システムを拡張すれば良い。

#### 11.1.3 既存の条件コンパイルパターン

| マクロ | 使用箇所 | 用途 |
|--------|----------|------|
| `NO_STM32` | `util/crc.c`, `tests/*/Makefile` | STM32依存を除外 |

**影響**: 新規マクロ `USE_PC_BUILD` の追加は任意。既存の `NO_STM32` パターンを優先使用可能。

### 11.2 当初計画からの変更点

#### 削除/簡略化した項目

1. **`stm32f4xx_stub.h` の構造体定義**
   - 理由: `stm32f407xx.h` に完全な定義が存在
   - 変更: ラッパーヘッダで既存定義を再利用

2. **`stm32f4xx_flash_stub.c`, `stm32f4xx_iwdg_stub.c` の個別ファイル**
   - 理由: 型定義は既存ヘッダから取得可能
   - 変更: 必要に応じて1つの `peripheral_stubs.c` に統合

3. **`config/conf_general_pc.h`, `mcconf_default_pc.h`**
   - 理由: 条件コンパイルで対応可能
   - 変更: Phase 5以降で必要に応じて追加

4. **`hal_serial.c`, `hal_can.c`, `hal_usb.c` の個別実装**
   - 理由: 基本ビルドでは不要
   - 変更: Phase 6以降でシミュレーション用途として検討

#### 追加した項目

1. **`__IO`, `__I`, `__O` マクロの定義**
   - 既存の `stm32f407xx.h` を使用するために必要
   - `cmsis_pc.h` または `stm32f4xx_pc.h` で定義

2. **既存テストの互換性確認**
   - `tests/float_serialization/` が `-DNO_STM32` で動作することを確認

### 11.3 リスクと対策

| リスク | 影響度 | 対策 |
|--------|--------|------|
| 既存ヘッダのARM依存コード | 中 | `#ifdef __arm__` 等のガード確認、必要に応じてパッチ |
| ChibiOS API使用箇所の多さ | 高 | 段階的に必要なAPIのみ実装、未使用APIはリンクエラーで検出 |
| CMSIS intrinsicsの依存 | 中 | 空マクロで置換、実行時の動作には影響なし |

### 11.4 次のステップ

1. **Phase 1 プロトタイプ作成**
   - `pc_build/stubs/stm32f4xx_pc.h` の作成
   - `pc_build/stubs/cmsis_pc.h` の作成
   - `pc_build/Makefile` の作成（`make/unittest.mk` ベース）

2. **既存テストでの検証**
   - `tests/utils_math/` の新インフラでのビルド確認
   - `tests/float_serialization/` の動作確認

3. **ChibiOSスタブの段階的拡張**
   - `chThdCreateStatic` 等の主要APIから実装

## 参考資料

### 調査ドキュメント
- [compiler_dependencies.md](../research/compiler_dependencies.md) - コンパイラ依存調査
- [hardware_dependencies.md](../research/hardware_dependencies.md) - ハードウェア依存調査
- [os_dependencies.md](../research/os_dependencies.md) - OS依存調査

### 既存リソース（リポジトリ内）
- [tests/utils_math/](../../tests/utils_math/) - 既存のテストスタブ実装例
- [tests/float_serialization/](../../tests/float_serialization/) - `NO_STM32`使用テスト例
- [make/unittest.mk](../../make/unittest.mk) - Google Test統合Makefile
- [make/unittest-defs.mk](../../make/unittest-defs.mk) - コンパイラ定義
- [lispBM/c_libs/stdperiph_stm32f4/](../../lispBM/c_libs/stdperiph_stm32f4/) - STM32ペリフェラル定義

### 外部参考資料
- [ChibiOS Documentation](http://www.chibios.org/dokuwiki/doku.php) - ChibiOS公式ドキュメント
- [STM32F4 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020.pdf) - STM32F4リファレンス
- [CMSIS Documentation](https://arm-software.github.io/CMSIS_5/Core/html/index.html) - CMSIS公式
