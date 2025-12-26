# Phase 1: PC用GCCビルド基盤構築 作業計画書

## 概要

| 項目 | 内容 |
|------|------|
| **フェーズ** | Phase 1: 基盤構築 |
| **目標** | PC用GCCでVESCファームウェアの一部をビルド可能にする最小限の基盤を構築 |
| **想定工数** | 3-5日 |
| **前提条件** | Linux/macOS/WSL環境、GCC、make、pthread開発ライブラリ |
| **成果物** | `pc_build/` ディレクトリ一式、最小限のスタブ、Makefile |

---

## 1. ディレクトリ構造作成

### 1.1 作成するディレクトリ

```
bldc/
└── pc_build/
    ├── stubs/           # STM32/CMSISスタブ
    ├── chibios_posix/   # ChibiOS POSIX互換レイヤー
    ├── hw_stub/         # ハードウェアスタブ
    └── build/           # ビルド出力（.gitignore対象）
```

### 1.2 実行コマンド

```bash
cd bldc
mkdir -p pc_build/stubs
mkdir -p pc_build/chibios_posix
mkdir -p pc_build/hw_stub
mkdir -p pc_build/build
touch pc_build/build/.gitkeep
```

---

## 2. スタブファイル作成

### 2.1 STM32 PCラッパー (`pc_build/stubs/stm32f4xx_pc.h`)

**目的**: 既存の `lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST/stm32f407xx.h` を再利用し、PC環境で動作させる

**依存関係**:
- 入力: `lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST/stm32f407xx.h` (7954行)
- 出力: PC環境で使用可能なペリフェラル定義

**実装内容**:

```c
#ifndef _STM32F4XX_PC_H_
#define _STM32F4XX_PC_H_

// CMSIS型マクロをPC用に定義（ARM依存を除去）
#ifndef __IO
#define __IO volatile
#endif
#ifndef __I
#define __I  volatile const
#endif
#ifndef __O
#define __O  volatile
#endif

// ARM固有マクロの無効化
#define __INLINE static inline
#define __STATIC_INLINE static inline

// 既存の完全な構造体定義を再利用
// 注: このヘッダはARM intrinsicsを含まないTypedef定義のみ使用
#include "lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST/stm32f407xx.h"

// ペリフェラルインスタンスのextern宣言（stm32f4xx_pc.cで実体定義）
extern GPIO_TypeDef GPIOA_Instance, GPIOB_Instance, GPIOC_Instance;
extern GPIO_TypeDef GPIOD_Instance, GPIOE_Instance, GPIOF_Instance;
extern TIM_TypeDef TIM1_Instance, TIM2_Instance, TIM3_Instance;
extern TIM_TypeDef TIM4_Instance, TIM5_Instance, TIM8_Instance;
extern ADC_TypeDef ADC1_Instance, ADC2_Instance, ADC3_Instance;
extern DMA_Stream_TypeDef DMA1_Stream0_Instance, DMA2_Stream0_Instance;
extern RCC_TypeDef RCC_Instance;
extern SPI_TypeDef SPI1_Instance, SPI2_Instance, SPI3_Instance;
extern USART_TypeDef USART1_Instance, USART2_Instance, USART3_Instance;

// ペリフェラルポインタマクロの再定義
#undef GPIOA
#undef GPIOB
#undef GPIOC
#undef GPIOD
#undef GPIOE
#undef GPIOF
#undef TIM1
#undef TIM2
#undef TIM3
#undef TIM4
#undef TIM5
#undef TIM8
#undef ADC1
#undef ADC2
#undef ADC3
#undef RCC
#undef SPI1
#undef SPI2
#undef SPI3
#undef USART1
#undef USART2
#undef USART3

#define GPIOA  (&GPIOA_Instance)
#define GPIOB  (&GPIOB_Instance)
#define GPIOC  (&GPIOC_Instance)
#define GPIOD  (&GPIOD_Instance)
#define GPIOE  (&GPIOE_Instance)
#define GPIOF  (&GPIOF_Instance)
#define TIM1   (&TIM1_Instance)
#define TIM2   (&TIM2_Instance)
#define TIM3   (&TIM3_Instance)
#define TIM4   (&TIM4_Instance)
#define TIM5   (&TIM5_Instance)
#define TIM8   (&TIM8_Instance)
#define ADC1   (&ADC1_Instance)
#define ADC2   (&ADC2_Instance)
#define ADC3   (&ADC3_Instance)
#define RCC    (&RCC_Instance)
#define SPI1   (&SPI1_Instance)
#define SPI2   (&SPI2_Instance)
#define SPI3   (&SPI3_Instance)
#define USART1 (&USART1_Instance)
#define USART2 (&USART2_Instance)
#define USART3 (&USART3_Instance)

#endif /* _STM32F4XX_PC_H_ */
```

**検証方法**:
```bash
gcc -c -I. -Ipc_build/stubs pc_build/stubs/stm32f4xx_pc.c -o /dev/null
```

---

### 2.2 ペリフェラルインスタンス実体 (`pc_build/stubs/stm32f4xx_pc.c`)

**目的**: ペリフェラルレジスタのメモリ領域を提供

**実装内容**:

```c
#include "stm32f4xx_pc.h"

// GPIOインスタンス
GPIO_TypeDef GPIOA_Instance = {0};
GPIO_TypeDef GPIOB_Instance = {0};
GPIO_TypeDef GPIOC_Instance = {0};
GPIO_TypeDef GPIOD_Instance = {0};
GPIO_TypeDef GPIOE_Instance = {0};
GPIO_TypeDef GPIOF_Instance = {0};

// タイマーインスタンス
TIM_TypeDef TIM1_Instance = {0};
TIM_TypeDef TIM2_Instance = {0};
TIM_TypeDef TIM3_Instance = {0};
TIM_TypeDef TIM4_Instance = {0};
TIM_TypeDef TIM5_Instance = {0};
TIM_TypeDef TIM8_Instance = {0};

// ADCインスタンス
ADC_TypeDef ADC1_Instance = {0};
ADC_TypeDef ADC2_Instance = {0};
ADC_TypeDef ADC3_Instance = {0};

// DMAインスタンス
DMA_Stream_TypeDef DMA1_Stream0_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream0_Instance = {0};

// RCCインスタンス
RCC_TypeDef RCC_Instance = {0};

// SPIインスタンス
SPI_TypeDef SPI1_Instance = {0};
SPI_TypeDef SPI2_Instance = {0};
SPI_TypeDef SPI3_Instance = {0};

// USARTインスタンス
USART_TypeDef USART1_Instance = {0};
USART_TypeDef USART2_Instance = {0};
USART_TypeDef USART3_Instance = {0};
```

---

### 2.3 CMSIS Intrinsicsスタブ (`pc_build/stubs/cmsis_pc.h`)

**目的**: ARM固有のCMSIS intrinsics（`__NOP`, `__WFI`, `__disable_irq` 等）をPC用空実装に置換

**依存関係**:
- 参照: `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/core_cmFunc.h` (640行)
- 参照: `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/core_cmInstr.h`

**実装内容**:

```c
#ifndef _CMSIS_PC_H_
#define _CMSIS_PC_H_

#include <stdint.h>

// 割り込み制御（空実装）
#define __disable_irq()     do {} while(0)
#define __enable_irq()      do {} while(0)
#define __disable_fault_irq() do {} while(0)
#define __enable_fault_irq()  do {} while(0)

// 同期命令（空実装）
#define __NOP()             do {} while(0)
#define __WFI()             do {} while(0)
#define __WFE()             do {} while(0)
#define __SEV()             do {} while(0)
#define __DMB()             do {} while(0)
#define __DSB()             do {} while(0)
#define __ISB()             do {} while(0)

// PRIMASK操作
static inline uint32_t __get_PRIMASK(void) { return 0; }
static inline void __set_PRIMASK(uint32_t primask) { (void)primask; }

// BASEPRI操作
static inline uint32_t __get_BASEPRI(void) { return 0; }
static inline void __set_BASEPRI(uint32_t basepri) { (void)basepri; }

// FAULTMASK操作
static inline uint32_t __get_FAULTMASK(void) { return 0; }
static inline void __set_FAULTMASK(uint32_t faultmask) { (void)faultmask; }

// CONTROL操作
static inline uint32_t __get_CONTROL(void) { return 0; }
static inline void __set_CONTROL(uint32_t control) { (void)control; }

// PSP/MSP操作
static inline uint32_t __get_PSP(void) { return 0; }
static inline void __set_PSP(uint32_t psp) { (void)psp; }
static inline uint32_t __get_MSP(void) { return 0; }
static inline void __set_MSP(uint32_t msp) { (void)msp; }

// ビット操作命令
static inline uint32_t __REV(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x000000FF) << 24);
}

static inline uint32_t __REV16(uint32_t value) {
    return ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
}

static inline int32_t __REVSH(int32_t value) {
    return (int16_t)(((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8));
}

static inline uint32_t __RBIT(uint32_t value) {
    uint32_t result = 0;
    for (int i = 0; i < 32; i++) {
        result |= ((value >> i) & 1) << (31 - i);
    }
    return result;
}

static inline uint8_t __CLZ(uint32_t value) {
    if (value == 0) return 32;
    uint8_t count = 0;
    while ((value & 0x80000000) == 0) {
        value <<= 1;
        count++;
    }
    return count;
}

// 排他アクセス命令（空実装）
static inline uint32_t __LDREXW(volatile uint32_t *addr) { return *addr; }
static inline uint32_t __STREXW(uint32_t value, volatile uint32_t *addr) { *addr = value; return 0; }
static inline void __CLREX(void) {}

// NVIC関連定数
#define NVIC_PRIORITYGROUP_0  0x00000007U
#define NVIC_PRIORITYGROUP_1  0x00000006U
#define NVIC_PRIORITYGROUP_2  0x00000005U
#define NVIC_PRIORITYGROUP_3  0x00000004U
#define NVIC_PRIORITYGROUP_4  0x00000003U

#endif /* _CMSIS_PC_H_ */
```

---

### 2.4 ChibiOS互換ヘッダ (`pc_build/chibios_posix/ch.h`)

**目的**: ChibiOS RTの基本APIをPOSIXスレッドで実装

**依存関係**:
- 参照: `tests/utils_math/ch.h` (9行) を拡張
- 参照: `ChibiOS_3.0.5/os/rt/include/ch.h`

**実装内容**:

```c
#ifndef _CH_H_
#define _CH_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* 基本型定義                                                                 */
/*===========================================================================*/

typedef uint32_t systime_t;
typedef int32_t msg_t;
typedef uint32_t eventmask_t;
typedef uint32_t tprio_t;
typedef void* tfunc_t;

/*===========================================================================*/
/* スレッド型                                                                 */
/*===========================================================================*/

typedef struct thread {
    pthread_t       pthread;
    const char*     name;
    tprio_t         prio;
    eventmask_t     events;
    pthread_mutex_t event_mutex;
    pthread_cond_t  event_cond;
    int             motor_selected;
    void*           p_stklimit;
} thread_t;

/*===========================================================================*/
/* ミューテックス型                                                           */
/*===========================================================================*/

typedef struct {
    pthread_mutex_t mutex;
} mutex_t;

/*===========================================================================*/
/* セマフォ型                                                                 */
/*===========================================================================*/

typedef struct {
    sem_t sem;
} semaphore_t;

typedef semaphore_t binary_semaphore_t;

/*===========================================================================*/
/* 仮想タイマー型                                                             */
/*===========================================================================*/

typedef struct virtual_timer {
    bool            armed;
    systime_t       deadline;
    void            (*callback)(void*);
    void*           arg;
} virtual_timer_t;

/*===========================================================================*/
/* 優先度定数                                                                 */
/*===========================================================================*/

#define IDLEPRIO        1
#define LOWPRIO         64
#define NORMALPRIO      128
#define HIGHPRIO        192
#define ABSPRIO         255

/*===========================================================================*/
/* スレッドマクロ                                                             */
/*===========================================================================*/

#define THD_WORKING_AREA(name, size) \
    static uint8_t name[(size) + sizeof(thread_t)]

#define THD_FUNCTION(name, arg) \
    void* name(void* arg)

/*===========================================================================*/
/* 時間変換マクロ (10kHz tick想定)                                            */
/*===========================================================================*/

#define CH_CFG_ST_FREQUENCY     10000

#define TIME_IMMEDIATE          ((systime_t)0)
#define TIME_INFINITE           ((systime_t)-1)

#define MS2ST(ms)               ((systime_t)((ms) * (CH_CFG_ST_FREQUENCY / 1000)))
#define US2ST(us)               ((systime_t)((us) * (CH_CFG_ST_FREQUENCY / 1000000)))
#define ST2MS(st)               ((uint32_t)((st) * 1000 / CH_CFG_ST_FREQUENCY))
#define ST2US(st)               ((uint32_t)((st) * 1000000 / CH_CFG_ST_FREQUENCY))
#define S2ST(s)                 ((systime_t)((s) * CH_CFG_ST_FREQUENCY))

/*===========================================================================*/
/* メッセージ定数                                                             */
/*===========================================================================*/

#define MSG_OK                  ((msg_t)0)
#define MSG_TIMEOUT             ((msg_t)-1)
#define MSG_RESET               ((msg_t)-2)

/*===========================================================================*/
/* API関数プロトタイプ                                                        */
/*===========================================================================*/

/* システム初期化 */
void chSysInit(void);

/* スレッド管理 */
thread_t* chThdCreateStatic(void *wsp, size_t size, tprio_t prio,
                            void* (*pf)(void*), void *arg);
void chThdExit(msg_t msg);
msg_t chThdWait(thread_t *tp);
void chThdSleep(systime_t time);
void chThdSleepMilliseconds(uint32_t ms);
void chThdSleepMicroseconds(uint32_t us);
void chThdSleepUntil(systime_t time);
void chThdYield(void);
thread_t* chThdGetSelfX(void);
void chThdSetPriority(tprio_t newprio);

/* システムロック */
void chSysLock(void);
void chSysUnlock(void);
void chSysLockFromISR(void);
void chSysUnlockFromISR(void);

/* ミューテックス */
void chMtxObjectInit(mutex_t *mp);
void chMtxLock(mutex_t *mp);
void chMtxLockS(mutex_t *mp);
void chMtxUnlock(mutex_t *mp);
void chMtxUnlockS(mutex_t *mp);
void chMtxUnlockAll(void);
bool chMtxTryLock(mutex_t *mp);
bool chMtxTryLockS(mutex_t *mp);

/* セマフォ */
void chSemObjectInit(semaphore_t *sp, int32_t n);
msg_t chSemWait(semaphore_t *sp);
msg_t chSemWaitS(semaphore_t *sp);
msg_t chSemWaitTimeout(semaphore_t *sp, systime_t timeout);
msg_t chSemWaitTimeoutS(semaphore_t *sp, systime_t timeout);
void chSemSignal(semaphore_t *sp);
void chSemSignalI(semaphore_t *sp);
void chSemReset(semaphore_t *sp, int32_t n);
void chSemResetI(semaphore_t *sp, int32_t n);

/* バイナリセマフォ */
void chBSemObjectInit(binary_semaphore_t *bsp, bool taken);
msg_t chBSemWait(binary_semaphore_t *bsp);
msg_t chBSemWaitTimeout(binary_semaphore_t *bsp, systime_t timeout);
void chBSemSignal(binary_semaphore_t *bsp);
void chBSemSignalI(binary_semaphore_t *bsp);
void chBSemReset(binary_semaphore_t *bsp, bool taken);

/* イベント */
void chEvtSignal(thread_t *tp, eventmask_t mask);
void chEvtSignalI(thread_t *tp, eventmask_t mask);
eventmask_t chEvtWaitAny(eventmask_t mask);
eventmask_t chEvtWaitAnyTimeout(eventmask_t mask, systime_t timeout);
eventmask_t chEvtGetAndClearEvents(eventmask_t mask);

/* 仮想タイマー */
void chVTObjectInit(virtual_timer_t *vtp);
void chVTSet(virtual_timer_t *vtp, systime_t delay,
             void (*vtfunc)(void*), void *par);
void chVTSetI(virtual_timer_t *vtp, systime_t delay,
              void (*vtfunc)(void*), void *par);
void chVTReset(virtual_timer_t *vtp);
void chVTResetI(virtual_timer_t *vtp);
bool chVTIsArmed(virtual_timer_t *vtp);
bool chVTIsArmedI(virtual_timer_t *vtp);

/* 時間管理 */
systime_t chVTGetSystemTime(void);
systime_t chVTGetSystemTimeX(void);
bool chVTTimeElapsedSinceX(systime_t start, systime_t time);

#ifdef __cplusplus
}
#endif

#endif /* _CH_H_ */
```

---

### 2.5 ChibiOS HAL互換ヘッダ (`pc_build/chibios_posix/hal.h`)

**目的**: ChibiOS HALの基本機能をPC上でエミュレート

**依存関係**:
- 参照: `tests/utils_math/hal.h` (26行) を拡張

**実装内容**:

```c
#ifndef _HAL_H_
#define _HAL_H_

#include "ch.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* PAL (GPIO) スタブ                                                          */
/*===========================================================================*/

typedef uint32_t ioportid_t;
typedef uint32_t ioportmask_t;
typedef uint32_t iomode_t;

#define PAL_MODE_INPUT              0x00
#define PAL_MODE_OUTPUT_PUSHPULL    0x01
#define PAL_MODE_OUTPUT_OPENDRAIN   0x02
#define PAL_MODE_ALTERNATE          0x03

/* GPIO操作関数 */
void palSetPadMode(ioportid_t port, uint8_t pad, iomode_t mode);
void palSetPad(ioportid_t port, uint8_t pad);
void palClearPad(ioportid_t port, uint8_t pad);
void palTogglePad(ioportid_t port, uint8_t pad);
uint8_t palReadPad(ioportid_t port, uint8_t pad);
void palWritePad(ioportid_t port, uint8_t pad, uint8_t value);

/* ポート操作 */
void palWritePort(ioportid_t port, ioportmask_t value);
ioportmask_t palReadPort(ioportid_t port);
ioportmask_t palReadLatch(ioportid_t port);

/*===========================================================================*/
/* ホールセンサースタブ (tests/utils_math/hal.hから)                          */
/*===========================================================================*/

int READ_HALL1(void);
int READ_HALL2(void);
int READ_HALL3(void);
int READ_HALL1_2(void);
int READ_HALL2_2(void);
int READ_HALL3_2(void);

/*===========================================================================*/
/* シリアル通信スタブ                                                         */
/*===========================================================================*/

typedef struct {
    void* dummy;
} SerialDriver;

typedef struct {
    uint32_t speed;
} SerialConfig;

extern SerialDriver SD1;
extern SerialDriver SD2;
extern SerialDriver SD3;

void sdStart(SerialDriver *sdp, const SerialConfig *config);
void sdStop(SerialDriver *sdp);
size_t sdWrite(SerialDriver *sdp, const uint8_t *bp, size_t n);
size_t sdRead(SerialDriver *sdp, uint8_t *bp, size_t n);
size_t sdAsynchronousRead(SerialDriver *sdp, uint8_t *bp, size_t n);

/*===========================================================================*/
/* ADCスタブ                                                                  */
/*===========================================================================*/

typedef struct {
    void* dummy;
} ADCDriver;

typedef struct {
    void* dummy;
} ADCConversionGroup;

extern ADCDriver ADCD1;
extern ADCDriver ADCD2;
extern ADCDriver ADCD3;

void adcStart(ADCDriver *adcp, void *config);
void adcStop(ADCDriver *adcp);
void adcStartConversion(ADCDriver *adcp, const ADCConversionGroup *grpp,
                        uint16_t *samples, size_t depth);

/*===========================================================================*/
/* PWM/ICUスタブ                                                              */
/*===========================================================================*/

typedef struct {
    void* dummy;
} PWMDriver;

typedef struct {
    void* dummy;
} ICUDriver;

extern PWMDriver PWMD1;
extern PWMDriver PWMD8;
extern ICUDriver ICUD3;
extern ICUDriver ICUD4;

/*===========================================================================*/
/* HAL初期化                                                                  */
/*===========================================================================*/

void halInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_H_ */
```

---

### 2.6 ChibiOS実装ファイル (`pc_build/chibios_posix/ch_core.c`)

**目的**: ChibiOS RT APIのPOSIX実装

**実装内容** (主要部分):

```c
#include "ch.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* グローバルシステムロック */
static pthread_mutex_t sys_lock = PTHREAD_MUTEX_INITIALIZER;

/* 現在のスレッド（TLS） */
static __thread thread_t* current_thread = NULL;

/* 起動時刻 */
static struct timespec start_time;

/*===========================================================================*/
/* システム初期化                                                             */
/*===========================================================================*/

void chSysInit(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

/*===========================================================================*/
/* スレッド管理                                                               */
/*===========================================================================*/

thread_t* chThdCreateStatic(void *wsp, size_t size, tprio_t prio,
                            void* (*pf)(void*), void *arg) {
    (void)size;
    
    thread_t* tp = (thread_t*)wsp;
    memset(tp, 0, sizeof(thread_t));
    tp->prio = prio;
    tp->p_stklimit = wsp;
    
    pthread_mutex_init(&tp->event_mutex, NULL);
    pthread_cond_init(&tp->event_cond, NULL);
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    pthread_create(&tp->pthread, &attr, pf, arg);
    pthread_attr_destroy(&attr);
    
    return tp;
}

void chThdSleep(systime_t time) {
    uint32_t us = ST2US(time);
    usleep(us);
}

void chThdSleepMilliseconds(uint32_t ms) {
    usleep(ms * 1000);
}

void chThdSleepMicroseconds(uint32_t us) {
    usleep(us);
}

thread_t* chThdGetSelfX(void) {
    return current_thread;
}

void chThdYield(void) {
    sched_yield();
}

/*===========================================================================*/
/* システムロック                                                             */
/*===========================================================================*/

void chSysLock(void) {
    pthread_mutex_lock(&sys_lock);
}

void chSysUnlock(void) {
    pthread_mutex_unlock(&sys_lock);
}

void chSysLockFromISR(void) {
    chSysLock();
}

void chSysUnlockFromISR(void) {
    chSysUnlock();
}

/*===========================================================================*/
/* ミューテックス                                                             */
/*===========================================================================*/

void chMtxObjectInit(mutex_t *mp) {
    pthread_mutex_init(&mp->mutex, NULL);
}

void chMtxLock(mutex_t *mp) {
    pthread_mutex_lock(&mp->mutex);
}

void chMtxLockS(mutex_t *mp) {
    chMtxLock(mp);
}

void chMtxUnlock(mutex_t *mp) {
    pthread_mutex_unlock(&mp->mutex);
}

void chMtxUnlockS(mutex_t *mp) {
    chMtxUnlock(mp);
}

bool chMtxTryLock(mutex_t *mp) {
    return pthread_mutex_trylock(&mp->mutex) == 0;
}

/*===========================================================================*/
/* セマフォ                                                                   */
/*===========================================================================*/

void chSemObjectInit(semaphore_t *sp, int32_t n) {
    sem_init(&sp->sem, 0, (unsigned int)n);
}

msg_t chSemWait(semaphore_t *sp) {
    if (sem_wait(&sp->sem) == 0) {
        return MSG_OK;
    }
    return MSG_RESET;
}

void chSemSignal(semaphore_t *sp) {
    sem_post(&sp->sem);
}

void chSemSignalI(semaphore_t *sp) {
    chSemSignal(sp);
}

/*===========================================================================*/
/* イベント                                                                   */
/*===========================================================================*/

void chEvtSignal(thread_t *tp, eventmask_t mask) {
    pthread_mutex_lock(&tp->event_mutex);
    tp->events |= mask;
    pthread_cond_broadcast(&tp->event_cond);
    pthread_mutex_unlock(&tp->event_mutex);
}

void chEvtSignalI(thread_t *tp, eventmask_t mask) {
    chEvtSignal(tp, mask);
}

eventmask_t chEvtWaitAny(eventmask_t mask) {
    thread_t* tp = chThdGetSelfX();
    if (tp == NULL) return 0;
    
    pthread_mutex_lock(&tp->event_mutex);
    while ((tp->events & mask) == 0) {
        pthread_cond_wait(&tp->event_cond, &tp->event_mutex);
    }
    eventmask_t result = tp->events & mask;
    tp->events &= ~result;
    pthread_mutex_unlock(&tp->event_mutex);
    
    return result;
}

/*===========================================================================*/
/* 時間管理                                                                   */
/*===========================================================================*/

systime_t chVTGetSystemTimeX(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    int64_t diff_ns = (now.tv_sec - start_time.tv_sec) * 1000000000LL +
                      (now.tv_nsec - start_time.tv_nsec);
    
    /* 10kHz tickに変換 */
    return (systime_t)(diff_ns / 100000);
}

systime_t chVTGetSystemTime(void) {
    return chVTGetSystemTimeX();
}
```

---

### 2.7 HAL実装ファイル (`pc_build/chibios_posix/hal_stub.c`)

**目的**: HAL APIのスタブ実装

**実装内容**:

```c
#include "hal.h"
#include <string.h>

/* ドライバインスタンス */
SerialDriver SD1 = {0};
SerialDriver SD2 = {0};
SerialDriver SD3 = {0};

ADCDriver ADCD1 = {0};
ADCDriver ADCD2 = {0};
ADCDriver ADCD3 = {0};

PWMDriver PWMD1 = {0};
PWMDriver PWMD8 = {0};

ICUDriver ICUD3 = {0};
ICUDriver ICUD4 = {0};

/* GPIO状態（シミュレーション用） */
static uint32_t gpio_odr[16] = {0};
static uint32_t gpio_idr[16] = {0};

/*===========================================================================*/
/* HAL初期化                                                                  */
/*===========================================================================*/

void halInit(void) {
    memset(gpio_odr, 0, sizeof(gpio_odr));
    memset(gpio_idr, 0, sizeof(gpio_idr));
}

/*===========================================================================*/
/* PAL (GPIO)                                                                 */
/*===========================================================================*/

void palSetPadMode(ioportid_t port, uint8_t pad, iomode_t mode) {
    (void)port; (void)pad; (void)mode;
}

void palSetPad(ioportid_t port, uint8_t pad) {
    if (port < 16 && pad < 32) {
        gpio_odr[port] |= (1U << pad);
    }
}

void palClearPad(ioportid_t port, uint8_t pad) {
    if (port < 16 && pad < 32) {
        gpio_odr[port] &= ~(1U << pad);
    }
}

void palTogglePad(ioportid_t port, uint8_t pad) {
    if (port < 16 && pad < 32) {
        gpio_odr[port] ^= (1U << pad);
    }
}

uint8_t palReadPad(ioportid_t port, uint8_t pad) {
    if (port < 16 && pad < 32) {
        return (gpio_idr[port] >> pad) & 1;
    }
    return 0;
}

void palWritePad(ioportid_t port, uint8_t pad, uint8_t value) {
    if (value) {
        palSetPad(port, pad);
    } else {
        palClearPad(port, pad);
    }
}

void palWritePort(ioportid_t port, ioportmask_t value) {
    if (port < 16) {
        gpio_odr[port] = value;
    }
}

ioportmask_t palReadPort(ioportid_t port) {
    if (port < 16) {
        return gpio_idr[port];
    }
    return 0;
}

ioportmask_t palReadLatch(ioportid_t port) {
    if (port < 16) {
        return gpio_odr[port];
    }
    return 0;
}

/*===========================================================================*/
/* ホールセンサースタブ                                                       */
/*===========================================================================*/

static int hall_state[6] = {0, 0, 0, 0, 0, 0};

int READ_HALL1(void) { return hall_state[0]; }
int READ_HALL2(void) { return hall_state[1]; }
int READ_HALL3(void) { return hall_state[2]; }
int READ_HALL1_2(void) { return hall_state[3]; }
int READ_HALL2_2(void) { return hall_state[4]; }
int READ_HALL3_2(void) { return hall_state[5]; }

/*===========================================================================*/
/* シリアル通信スタブ                                                         */
/*===========================================================================*/

void sdStart(SerialDriver *sdp, const SerialConfig *config) {
    (void)sdp; (void)config;
}

void sdStop(SerialDriver *sdp) {
    (void)sdp;
}

size_t sdWrite(SerialDriver *sdp, const uint8_t *bp, size_t n) {
    (void)sdp; (void)bp;
    return n;
}

size_t sdRead(SerialDriver *sdp, uint8_t *bp, size_t n) {
    (void)sdp; (void)bp; (void)n;
    return 0;
}

size_t sdAsynchronousRead(SerialDriver *sdp, uint8_t *bp, size_t n) {
    return sdRead(sdp, bp, n);
}

/*===========================================================================*/
/* ADCスタブ                                                                  */
/*===========================================================================*/

void adcStart(ADCDriver *adcp, void *config) {
    (void)adcp; (void)config;
}

void adcStop(ADCDriver *adcp) {
    (void)adcp;
}

void adcStartConversion(ADCDriver *adcp, const ADCConversionGroup *grpp,
                        uint16_t *samples, size_t depth) {
    (void)adcp; (void)grpp; (void)samples; (void)depth;
}
```

---

## 3. Makefile作成

### 3.1 PC用Makefile (`pc_build/Makefile`)

**目的**: PC環境でのビルド構成

**実装内容**:

```makefile
# pc_build/Makefile
# PC用GCCビルド設定

# ルートディレクトリ
ROOT := ..
PC_BUILD := .

# コンパイラ設定
CC := gcc
AR := ar
CFLAGS := -Wall -Wextra -Wundef -std=gnu99 -g -O2
CFLAGS += -fsingle-precision-constant
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DNO_STM32
CFLAGS += -DUSE_PC_BUILD

# インクルードパス（PC用スタブを優先）
CFLAGS += -I$(PC_BUILD)/stubs
CFLAGS += -I$(PC_BUILD)/chibios_posix
CFLAGS += -I$(PC_BUILD)/hw_stub
CFLAGS += -I$(ROOT)
CFLAGS += -I$(ROOT)/util
CFLAGS += -I$(ROOT)/motor
CFLAGS += -I$(ROOT)/comm
CFLAGS += -I$(ROOT)/applications
CFLAGS += -I$(ROOT)/encoder
CFLAGS += -I$(ROOT)/hwconf
CFLAGS += -I$(ROOT)/imu
CFLAGS += -I$(ROOT)/driver
CFLAGS += -I$(ROOT)/lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST
CFLAGS += -I$(ROOT)/lispBM/c_libs/stdperiph_stm32f4/CMSIS/include
CFLAGS += -I$(ROOT)/lispBM/c_libs/stdperiph_stm32f4/inc

# リンカフラグ
LDFLAGS := -lpthread -lm -lrt

# ビルド出力ディレクトリ
BUILD_DIR := $(PC_BUILD)/build

# スタブソース
STUB_SRC := \
    $(PC_BUILD)/stubs/stm32f4xx_pc.c

POSIX_SRC := \
    $(PC_BUILD)/chibios_posix/ch_core.c \
    $(PC_BUILD)/chibios_posix/hal_stub.c

# オブジェクトファイル
STUB_OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(STUB_SRC)))
POSIX_OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(POSIX_SRC)))

# Phase 1 検証用: ユーティリティソース
UTIL_SRC := \
    $(ROOT)/util/utils_math.c \
    $(ROOT)/util/buffer.c \
    $(ROOT)/util/crc.c

UTIL_OBJ := $(patsubst $(ROOT)/util/%.c,$(BUILD_DIR)/%.o,$(UTIL_SRC))

# 全オブジェクト
ALL_OBJ := $(STUB_OBJ) $(POSIX_OBJ) $(UTIL_OBJ)

#===========================================================================
# ターゲット
#===========================================================================

.PHONY: all clean test dirs

all: dirs libvesc_pc.a

dirs:
	@mkdir -p $(BUILD_DIR)

# 静的ライブラリ
libvesc_pc.a: $(ALL_OBJ)
	$(AR) rcs $@ $^
	@echo "=== Built: $@ ==="

# スタブオブジェクト
$(BUILD_DIR)/stm32f4xx_pc.o: $(PC_BUILD)/stubs/stm32f4xx_pc.c
	$(CC) $(CFLAGS) -c $< -o $@

# POSIXオブジェクト
$(BUILD_DIR)/ch_core.o: $(PC_BUILD)/chibios_posix/ch_core.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/hal_stub.o: $(PC_BUILD)/chibios_posix/hal_stub.c
	$(CC) $(CFLAGS) -c $< -o $@

# ユーティリティオブジェクト
$(BUILD_DIR)/utils_math.o: $(ROOT)/util/utils_math.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/buffer.o: $(ROOT)/util/buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/crc.o: $(ROOT)/util/crc.c
	$(CC) $(CFLAGS) -c $< -o $@

# 簡易テスト
test: libvesc_pc.a
	@echo "=== Running basic compilation test ==="
	$(CC) $(CFLAGS) -c -xc - -o /dev/null <<< '#include "ch.h"\n#include "hal.h"\nint main(){return 0;}'
	@echo "=== Test passed ==="

clean:
	rm -rf $(BUILD_DIR) libvesc_pc.a
	@echo "=== Cleaned ==="
```

---

## 4. 検証手順

### 4.1 ディレクトリ作成確認

```bash
cd bldc
ls -la pc_build/
# 期待出力:
# stubs/
# chibios_posix/
# hw_stub/
# build/
# Makefile
```

### 4.2 コンパイルテスト

```bash
cd pc_build
make clean
make
# 期待出力:
# === Built: libvesc_pc.a ===
```

### 4.3 ヘッダ互換性テスト

```bash
cd pc_build
gcc -c -I. -Istubs -Ichibios_posix \
    -I../lispBM/c_libs/stdperiph_stm32f4/CMSIS/ST \
    -xc - -o /dev/null << 'EOF'
#include "stm32f4xx_pc.h"
#include "cmsis_pc.h"
#include "ch.h"
#include "hal.h"

int main(void) {
    /* GPIO操作テスト */
    GPIOA->ODR = 0x1234;
    
    /* CMSIS intrinsicsテスト */
    __NOP();
    __DSB();
    
    /* ChibiOS APIテスト */
    chSysInit();
    chSysLock();
    chSysUnlock();
    
    return 0;
}
EOF
echo "Header compatibility test passed!"
```

### 4.4 既存テストとの互換性確認

```bash
cd bldc/tests/utils_math
make clean
make
./test_utils_math
```

---

## 5. 成果物チェックリスト

| 成果物 | ファイルパス | 状態 |
|--------|-------------|------|
| ディレクトリ構造 | `pc_build/stubs/`, `chibios_posix/`, `hw_stub/`, `build/` | ☐ |
| STM32 PCラッパー | `pc_build/stubs/stm32f4xx_pc.h` | ☐ |
| ペリフェラル実体 | `pc_build/stubs/stm32f4xx_pc.c` | ☐ |
| CMSISスタブ | `pc_build/stubs/cmsis_pc.h` | ☐ |
| ChibiOS ch.h | `pc_build/chibios_posix/ch.h` | ☐ |
| ChibiOS hal.h | `pc_build/chibios_posix/hal.h` | ☐ |
| ChibiOS実装 | `pc_build/chibios_posix/ch_core.c` | ☐ |
| HAL実装 | `pc_build/chibios_posix/hal_stub.c` | ☐ |
| Makefile | `pc_build/Makefile` | ☐ |
| コンパイル成功 | `libvesc_pc.a` 生成 | ☐ |
| 基本テスト成功 | ヘッダ互換性テスト | ☐ |

---

## 6. 次フェーズへの引き継ぎ

### 6.1 Phase 2 で必要な追加作業

1. **追加ユーティリティのビルド対象化**
   - `util/digital_filter.c`
   - `util/mempools.c`
   - `util/worker.c`

2. **既存テストの統合**
   - `tests/utils_math/` をPC用Makefileから実行可能に
   - `tests/float_serialization/` の統合

### 6.2 既知の課題

| 課題 | 影響 | 対策予定 |
|------|------|----------|
| `stm32f407xx.h` のARM依存コード | コンパイルエラーの可能性 | `#ifdef __arm__` ガードで対処 |
| 浮動小数点精度差異 | テスト結果の微小な差 | 許容誤差を設定 |
| 仮想タイマー未実装 | 一部API使用不可 | Phase 3で実装 |

---

## 付録A: ファイル一覧と行数見積もり

| ファイル | 推定行数 | 優先度 |
|----------|---------|--------|
| `stm32f4xx_pc.h` | 80-100 | 必須 |
| `stm32f4xx_pc.c` | 40-50 | 必須 |
| `cmsis_pc.h` | 100-120 | 必須 |
| `ch.h` | 180-200 | 必須 |
| `hal.h` | 100-120 | 必須 |
| `ch_core.c` | 200-250 | 必須 |
| `hal_stub.c` | 120-150 | 必須 |
| `Makefile` | 80-100 | 必須 |
| **合計** | **900-1100** | - |

---

## 付録B: トラブルシューティング

### B.1 コンパイルエラー: `stm32f407xx.h` で undefined

**症状**: `__IO` が未定義エラー

**原因**: `stm32f4xx_pc.h` のインクルード順序問題

**対策**:
```c
// stm32f4xx_pc.h の先頭で定義を確認
#ifndef __IO
#define __IO volatile
#endif
```

### B.2 リンクエラー: `pthread_*` が未解決

**症状**: pthread関数が見つからない

**原因**: `-lpthread` フラグ欠落

**対策**:
```makefile
LDFLAGS := -lpthread -lm -lrt
```

### B.3 実行時エラー: セグメンテーション違反

**症状**: ChibiOS API呼び出しでクラッシュ

**原因**: `chSysInit()` 未呼び出し

**対策**:
```c
int main(void) {
    chSysInit();  // 最初に呼び出し必須
    halInit();
    // ...
}
```

---

**作成日**: 2025年12月26日  
**作成者**: GitHub Copilot  
**参照**: [pc_build_strategy.md](../pc_build_strategy.md)
