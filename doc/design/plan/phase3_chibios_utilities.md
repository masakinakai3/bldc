# Phase 3: ChibiOS依存ユーティリティ統合 実行計画

| 項目 | 内容 |
|------|------|
| **作成日** | 2025年12月27日 |
| **フェーズ** | Phase 3: ChibiOS依存ユーティリティ統合 |
| **前提** | Phase 2 完了（libvesc_pc.a: 192KB, 161関数, 32/32テスト合格） |
| **想定期間** | 1-2週間 |
| **目標** | ChibiOS POSIXレイヤーを拡張し、ChibiOS依存ユーティリティをPCビルドに統合 |

---

## 1. 概要

### 1.1 Phase 3の目的

VESCファームウェアの **ChibiOS依存ユーティリティモジュール** をPC向けGCCでビルドするため、ChibiOS POSIXレイヤーを拡張し、必要なスタブを追加する。

### 1.2 成果物

| 成果物 | 説明 |
|--------|------|
| `pc_build/chibios_posix/ch.h` | 拡張済みChibiOS API宣言 |
| `pc_build/chibios_posix/ch_core.c` | 不足API（chEvtSignal等）の実装追加 |
| `pc_build/stubs/app_stub.c/h` | アプリケーション層スタブ |
| `pc_build/stubs/packet_stub.h` | パケット定義スタブ |
| `pc_build/build/libvesc_pc.a` | 拡張ユーティリティ統合ライブラリ |
| `pc_build/test_phase3.c` | Phase 3統合テストプログラム |
| Phase 3検証レポート | `doc/design/test/phase3_verification_report.md` |

### 1.3 ビルド対象ファイル

| ファイル | 行数 | ChibiOS依存 | 追加スタブ必要 |
|----------|------|-------------|----------------|
| `util/digital_filter.c` | 266 | なし | なし |
| `util/worker.c` | 54 | スレッドAPI | 既存で対応可 |
| `util/mempools.c` | 145 | mutex_t | packet.h |
| `util/utils_sys.c` | 169 | chSysLock/Unlock | app.h, hal.h拡張 |
| **合計** | **634行** | | |

### 1.4 ChibiOS POSIXレイヤー現状

**現在の実装状況** (`pc_build/chibios_posix/`):

| ファイル | 行数 | 状態 |
|----------|------|------|
| `ch.h` | 452 | ✅ 完了（API宣言） |
| `ch_core.c` | 663 | ⚠️ 一部不足 |
| `hal.h` | 66 | ✅ 基本実装あり |
| `hal_stub.c` | 50 | ✅ 基本実装あり |

**不足している関数**:

| 関数 | 使用箇所 | 優先度 |
|------|----------|--------|
| `chEvtSignal()` | mc_interface.c, mcpwm_foc.c | 高 |
| `chEvtSignalI()` | mc_interface.c, mcpwm_foc.c | 高 |

---

## 2. タスク分解

### Task 3.1: digital_filter.c の統合（難易度: 低）

#### 2.1.1 目的

純粋数学ライブラリである `digital_filter.c` をPCビルドに追加する。

#### 2.1.2 依存関係

```
digital_filter.c
├── digital_filter.h
│   ├── <math.h>       ✅ 標準
│   └── <stdint.h>     ✅ 標準
├── <math.h>           ✅ 標準
└── <string.h>         ✅ 標準
```

#### 2.1.3 実装手順

1. `pc_build/Makefile` に `digital_filter.c` を追加
2. ビルド確認（警告ゼロ）
3. 単体テスト作成（FFT、フィルター関数）

#### 2.1.4 受け入れ基準

- [ ] コンパイル成功（警告ゼロ）
- [ ] `nm libvesc_pc.a | grep filter` で関数シンボル確認
- [ ] FFT関数の基本テスト成功

#### 2.1.5 工数見積もり

**0.5日**

---

### Task 3.2: worker.c の統合（難易度: 中）

#### 2.2.1 目的

バックグラウンドワーカースレッド管理の `worker.c` を統合する。

#### 2.2.2 依存関係

```
worker.c
├── worker.h
│   ├── <stdbool.h>    ✅ 標準
│   └── <stdint.h>     ✅ 標準
├── ch.h               ⚠️ ChibiOS POSIX
│   ├── THD_WORKING_AREA    ✅ 実装済み
│   ├── THD_FUNCTION        ✅ 実装済み
│   ├── chThdCreateStatic   ✅ 実装済み
│   ├── chThdWait           ✅ 実装済み
│   ├── chThdGetSelfX       ✅ 実装済み
│   └── chRegSetThreadName  ✅ 実装済み
└── hal.h              ⚠️ 要確認
    └── palSetPad      ✅ 実装済み（空）
```

#### 2.2.3 実装手順

1. 既存ChibiOS POSIXレイヤーでの動作確認
2. `pc_build/Makefile` に `worker.c` を追加
3. ワーカースレッドテスト作成

#### 2.2.4 受け入れ基準

- [ ] コンパイル成功（警告ゼロ）
- [ ] `worker_execute()` による関数実行テスト成功
- [ ] スレッド生成・待機・終了の正常動作確認

#### 2.2.5 工数見積もり

**1日**

---

### Task 3.3: ChibiOS POSIXレイヤー拡張（難易度: 中）

#### 2.3.1 目的

不足している `chEvtSignal` / `chEvtSignalI` 関数を実装する。

#### 2.3.2 不足API詳細

本家ChibiOS定義 (`ChibiOS_3.0.5/os/rt/include/chevents.h`):

```c
void chEvtSignal(thread_t *tp, eventmask_t events);
void chEvtSignalI(thread_t *tp, eventmask_t events);
```

**用途**: 特定スレッドにイベントを送信する。ISRコンテキストからも使用可能。

#### 2.3.3 実装設計

```c
// pc_build/chibios_posix/ch_core.c に追加

void chEvtSignal(thread_t *tp, eventmask_t events) {
    if (!tp) return;
    
    pthread_mutex_lock(&event_mutex);
    tp->events |= events;
    pthread_cond_broadcast(&event_cond);
    pthread_mutex_unlock(&event_mutex);
}

void chEvtSignalI(thread_t *tp, eventmask_t events) {
    // ISRコンテキストではないPCでは通常版と同じ
    chEvtSignal(tp, events);
}
```

**thread_t構造体拡張**（ch.hで必要な場合）:

```c
typedef struct thread_t {
    pthread_t thread;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    eventmask_t events;      // ← 追加
    int motor_selected;
    void *p_stklimit;
} thread_t;
```

#### 2.3.4 ch.h 宣言追加

```c
// 関数宣言
void chEvtSignal(thread_t *tp, eventmask_t events);
void chEvtSignalI(thread_t *tp, eventmask_t events);
```

#### 2.3.5 実装手順

1. `ch.h` に `chEvtSignal` / `chEvtSignalI` 宣言を追加
2. `thread_t` 構造体に `events` フィールドがあることを確認
3. `ch_core.c` に実装追加
4. 単体テスト作成

#### 2.3.6 受け入れ基準

- [ ] `chEvtSignal` / `chEvtSignalI` がビルド可能
- [ ] イベントシグナル送受信テスト成功
- [ ] 複数スレッド間のイベント通信テスト成功

#### 2.3.7 工数見積もり

**1日**

---

### Task 3.4: mempools.c の統合（難易度: 中）

#### 2.4.1 目的

設定メモリプール管理の `mempools.c` を統合する。

#### 2.4.2 依存関係

```
mempools.c
├── mempools.h
│   ├── mc_interface.h     ⚠️ 要スタブ
│   │   └── mc_configuration  ⚠️ datatypes.h
│   └── <stdint.h>         ✅ 標準
├── ch.h                   ⚠️ ChibiOS POSIX
│   ├── chMtxObjectInit    ✅ 実装済み
│   ├── chMtxLock          ✅ 実装済み
│   └── chMtxUnlock        ✅ 実装済み
├── packet.h               ⚠️ 要スタブ
│   └── PACKET_MAX_PL_LEN  ⚠️ 定数
└── datatypes.h            ⚠️ 大規模構造体
    ├── mc_configuration   ⚠️ ~500行
    └── app_configuration  ⚠️ ~200行
```

#### 2.4.3 スタブ設計

**packet_stub.h**:

```c
#ifndef PACKET_STUB_H_
#define PACKET_STUB_H_

#define PACKET_MAX_PL_LEN    512

// パケット関連の最小定義
typedef struct {
    void (*send_func)(unsigned char *data, unsigned int len);
} PACKET_STATE_t;

#endif
```

**mempools依存解決戦略**:

`mempools.c` は `mc_configuration` と `app_configuration` の大規模構造体に依存。
2つのアプローチが可能:

1. **最小スタブアプローチ**: 空の構造体定義
2. **完全型アプローチ**: `datatypes.h` をそのまま使用

`datatypes.h` は純粋なヘッダーなので、そのまま使用することを推奨。

#### 2.4.4 実装手順

1. `packet_stub.h` を作成
2. `datatypes.h` のインクルードパス設定確認
3. `pc_build/Makefile` に `mempools.c` を追加
4. ビルド・リンク確認
5. メモリプールテスト作成

#### 2.4.5 受け入れ基準

- [ ] コンパイル成功（警告ゼロ）
- [ ] `mempools_init()` 呼び出し成功
- [ ] `mempools_alloc_mcconf()` / `mempools_free_mcconf()` テスト成功

#### 2.4.6 工数見積もり

**1.5日**

---

### Task 3.5: utils_sys.c の統合（難易度: 高）

#### 2.5.1 目的

システムユーティリティ関数（ロックカウント、ホール読み取り）の `utils_sys.c` を統合する。

#### 2.5.2 依存関係

```
utils_sys.c
├── utils_sys.h
│   └── <stdint.h>         ✅ 標準
├── ch.h                   ⚠️ ChibiOS POSIX
│   ├── chSysLock          ✅ 実装済み
│   └── chSysUnlock        ✅ 実装済み
├── hal.h                  ⚠️ 要拡張
│   ├── READ_HALL1/2/3     ⚠️ 要追加
│   └── READ_HALL1_2/2_2/3_2  ⚠️ 要追加
└── app.h                  ⚠️ 要スタブ
    └── app_get_configuration()  ⚠️ 関数
```

#### 2.5.3 utils_sys.c 関数一覧

```c
// ロック管理（ChibiOS依存）
void utils_sys_lock_cnt(void);
void utils_sys_unlock_cnt(void);

// ホールセンサ読み取り（ハードウェア依存）
int utils_read_hall(bool is_second_motor, int hall_extra_samples);
```

#### 2.5.4 スタブ設計

**hal.h 拡張**:

```c
// 既存の hal.h に追加

// ホールセンサ読み取りマクロ（スタブ）
#ifndef READ_HALL1
#define READ_HALL1()    (0)
#define READ_HALL2()    (0)
#define READ_HALL3()    (0)
#define READ_HALL1_2()  (0)
#define READ_HALL2_2()  (0)
#define READ_HALL3_2()  (0)
#endif
```

**app_stub.h/c**:

```c
// pc_build/stubs/app_stub.h
#ifndef APP_STUB_H_
#define APP_STUB_H_

#include "datatypes.h"  // app_configuration定義

// アプリケーション設定スタブ
const app_configuration* app_get_configuration(void);
void app_set_configuration(app_configuration *conf);

#endif

// pc_build/stubs/app_stub.c
#include "app_stub.h"
#include <string.h>

static app_configuration app_conf;

const app_configuration* app_get_configuration(void) {
    return &app_conf;
}

void app_set_configuration(app_configuration *conf) {
    if (conf) {
        memcpy(&app_conf, conf, sizeof(app_configuration));
    }
}
```

#### 2.5.5 実装手順

1. `hal.h` に `READ_HALL*` マクロを追加
2. `app_stub.h/c` を作成
3. `pc_build/Makefile` に `utils_sys.c` と `app_stub.c` を追加
4. ビルド・リンク確認
5. ロック関数テスト作成

#### 2.5.6 受け入れ基準

- [ ] コンパイル成功（警告ゼロ）
- [ ] `utils_sys_lock_cnt()` / `utils_sys_unlock_cnt()` ペア動作確認
- [ ] `utils_read_hall()` がスタブ値（0）を返すことを確認

#### 2.5.7 工数見積もり

**1.5日**

---

### Task 3.6: 統合テスト・検証（難易度: 中）

#### 2.6.1 目的

Phase 3で追加した全モジュールの統合テストを実施する。

#### 2.6.2 テスト項目

| テスト | 対象 | 説明 |
|--------|------|------|
| FFTテスト | digital_filter.c | 既知信号のFFT変換・逆変換 |
| フィルターテスト | digital_filter.c | ローパスフィルター応答確認 |
| ワーカーテスト | worker.c | スレッド実行・待機 |
| イベントテスト | ch_core.c | chEvtSignal/Wait動作 |
| メモリプールテスト | mempools.c | alloc/free サイクル |
| ロックテスト | utils_sys.c | lock_cnt/unlock_cnt |

#### 2.6.3 テストプログラム構成

```c
// pc_build/test_phase3.c

#include <stdio.h>
#include "digital_filter.h"
#include "worker.h"
#include "mempools.h"
#include "utils_sys.h"

// テスト関数群
int test_digital_filter(void);
int test_worker(void);
int test_mempools(void);
int test_utils_sys(void);
int test_evt_signal(void);

int main(void) {
    int failures = 0;
    
    failures += test_digital_filter();
    failures += test_worker();
    failures += test_mempools();
    failures += test_utils_sys();
    failures += test_evt_signal();
    
    printf("\n=== Phase 3 Test Summary ===\n");
    printf("Failures: %d\n", failures);
    
    return failures;
}
```

#### 2.6.4 受け入れ基準

- [ ] 全テスト合格（failures == 0）
- [ ] `libvesc_pc.a` サイズ増加確認（Phase 2: 192KB → 目標: ~220KB）
- [ ] 新規関数シンボル確認

#### 2.6.5 工数見積もり

**1日**

---

## 3. スケジュール

### 3.1 全体スケジュール

| 日程 | タスク | 成果物 |
|------|--------|--------|
| Day 1 | Task 3.1: digital_filter.c統合 | ビルド成功、基本テスト |
| Day 2 | Task 3.2: worker.c統合 | スレッドテスト成功 |
| Day 3 | Task 3.3: ChibiOS POSIXレイヤー拡張 | chEvtSignal実装 |
| Day 4-5 | Task 3.4: mempools.c統合 | メモリプールテスト成功 |
| Day 6-7 | Task 3.5: utils_sys.c統合 | スタブ作成、テスト成功 |
| Day 8 | Task 3.6: 統合テスト・検証 | Phase 3検証レポート |

### 3.2 クリティカルパス

```
Task 3.3 (ChibiOS拡張) → Task 3.4 (mempools) → Task 3.5 (utils_sys) → Task 3.6 (統合テスト)
```

`mempools.c` と `utils_sys.c` は `datatypes.h` に依存するため、順序に注意。

### 3.3 並列実行可能タスク

- Task 3.1 (digital_filter) は独立して実行可能
- Task 3.2 (worker) は Task 3.3 と並行可能

---

## 4. リスクと対策

### 4.1 技術的リスク

| リスク | 影響 | 確率 | 対策 |
|--------|------|------|------|
| datatypes.h の間接依存が深い | mempools/utils_sys ビルド失敗 | 中 | 追加スタブで対応 |
| chEvtSignal実装が複雑化 | 期間延長 | 低 | 簡易実装で開始 |
| worker.cのスレッド動作不安定 | テスト失敗 | 低 | pthreadデバッグ |

### 4.2 依存関係リスク

**datatypes.h 依存チェーン**:

```
datatypes.h
├── mc_configuration
│   ├── motor_state_t
│   ├── foc_sensor_mode
│   └── ... (多数のenum)
└── app_configuration
    ├── app_use_case_mode
    └── ...
```

`datatypes.h` は純粋なデータ型定義のみなので、そのまま使用可能。
ただし、以下のヘッダが必要になる可能性あり:

- `conf_general.h` - バージョン定義
- 各種 `_default.h` - デフォルト値

**対策**: インクルードパスを適切に設定し、必要に応じて最小スタブを作成。

---

## 5. 検証方法

### 5.1 ビルド検証

```bash
cd /mnt/c/Users/galax/bldc/bldc/pc_build
make clean
make all

# ライブラリ確認
ls -la build/libvesc_pc.a
nm build/libvesc_pc.a | grep -E 'filter_|worker_|mempools_|utils_sys_'
```

### 5.2 テスト実行

```bash
make test_phase3
./build/test_phase3
```

### 5.3 関数数カウント

```bash
# Phase 2: 161関数
# Phase 3目標: ~180関数
nm build/libvesc_pc.a | grep ' T ' | wc -l
```

---

## 6. Makefile修正案

### 6.1 追加ソースファイル

```makefile
# Phase 3 追加分
UTIL_SRC += $(ROOT)/util/digital_filter.c
UTIL_SRC += $(ROOT)/util/worker.c
UTIL_SRC += $(ROOT)/util/mempools.c
UTIL_SRC += $(ROOT)/util/utils_sys.c

# 追加スタブ
STUB_SRC += $(PC_BUILD)/stubs/app_stub.c
STUB_SRC += $(PC_BUILD)/stubs/packet_stub.c
```

### 6.2 追加インクルードパス

```makefile
# datatypes.h等のためのルートパス
CFLAGS += -I$(ROOT)
CFLAGS += -I$(ROOT)/applications   # app.h
CFLAGS += -I$(ROOT)/comm           # packet.h (本来)
```

### 6.3 テストターゲット

```makefile
test_phase3: $(OBJS)
	$(CC) $(CFLAGS) -o build/test_phase3 test_phase3.c -L build -lvesc_pc $(LDFLAGS)
```

---

## 7. 完了条件

### 7.1 必須条件

- [ ] `make all` が警告ゼロで成功
- [ ] 4つのユーティリティ（digital_filter, worker, mempools, utils_sys）がライブラリに含まれる
- [ ] Phase 3テスト全項目合格
- [ ] `libvesc_pc.a` が正常に生成される

### 7.2 推奨条件

- [ ] 関数数が180以上に増加
- [ ] メモリリーク検出なし（valgrind）
- [ ] ドキュメント更新完了

---

## 8. 参照ドキュメント

| ドキュメント | 説明 |
|--------------|------|
| [pc_build_strategy.md](../pc_build_strategy.md) | 全体戦略 |
| [phase1_pc_build_foundation.md](phase1_pc_build_foundation.md) | Phase 1計画 |
| [phase2_utility_build.md](phase2_utility_build.md) | Phase 2計画 |
| [phase2_verification_report.md](../test/phase2_verification_report.md) | Phase 2検証結果 |
| [ChibiOS_3.0.5/os/rt/include/chevents.h](../../../ChibiOS_3.0.5/os/rt/include/chevents.h) | 本家イベントAPI |

---

## 付録A: ファイル間依存関係図

```
                    ┌─────────────────┐
                    │   datatypes.h   │
                    │  (型定義のみ)    │
                    └────────┬────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│   mempools.c    │ │   utils_sys.c   │ │   app_stub.c    │
│ (mutex, config) │ │ (lock, hall)    │ │ (config stub)   │
└────────┬────────┘ └────────┬────────┘ └─────────────────┘
         │                   │
         ▼                   ▼
┌─────────────────┐ ┌─────────────────┐
│   ch.h/ch_core  │ │    hal.h        │
│   (mutex API)   │ │ (READ_HALL*)    │
└─────────────────┘ └─────────────────┘

独立モジュール:
┌─────────────────┐ ┌─────────────────┐
│digital_filter.c │ │    worker.c     │
│  (純粋数学)      │ │ (pthread wrapper)│
└─────────────────┘ └─────────────────┘
```

---

## 付録B: ChibiOS API使用頻度（VESCファームウェア）

モータ制御コード（mc_interface.c, mcpwm_foc.c, app.c）での使用頻度:

| API | 使用回数 | Phase 3実装状態 |
|-----|----------|-----------------|
| `chThdSleepMilliseconds` | 42 | ✅ 実装済み |
| `chThdSleepMicroseconds` | 10 | ✅ 実装済み |
| `chThdSleep` | 10 | ✅ 実装済み |
| `chVTGetSystemTimeX` | 9 | ✅ 実装済み |
| `chThdCreateStatic` | 7 | ✅ 実装済み |
| `chRegSetThreadName` | 7 | ✅ 実装済み |
| `chThdGetSelfX` | 5 | ✅ 実装済み |
| `chSysUnlockFromISR` | 5 | ✅ 実装済み |
| `chSysLockFromISR` | 5 | ✅ 実装済み |
| `chEvtSignalI` | 5 | ⚠️ **要追加** |
| `chEvtWaitAny` | 2 | ✅ 実装済み |
| `chEvtSignal` | 2 | ⚠️ **要追加** |
| `chVTSet` | 1 | ✅ 実装済み |
| `chVTReset` | 1 | ✅ 実装済み |
| `chVTObjectInit` | 1 | ✅ 実装済み |

**結論**: `chEvtSignal` / `chEvtSignalI` の追加のみでPhase 3の主要APIカバレッジ達成。
