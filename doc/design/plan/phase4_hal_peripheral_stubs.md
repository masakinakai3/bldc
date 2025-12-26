# Phase 4 実行計画: HAL/ペリフェラルスタブ

## 概要

| 項目 | 内容 |
|------|------|
| **フェーズ** | Phase 4 |
| **目的** | HAL/ペリフェラルスタブの構築とutils_sys.c統合 |
| **前提条件** | Phase 3完了（95%達成、55/55テスト合格） |
| **想定期間** | 3-5日 |
| **ベースライン** | ライブラリ280KB、188関数、テスト55件 |

---

## 目標

1. **hw_stub.h/c** - ハードウェア抽象化スタブの作成
2. **conf_general_pc.h** - PC用設定ヘッダの作成
3. **utils_sys.c** - Phase 3で延期されたモジュールの統合
4. **app_stub.c** - アプリケーション層スタブの作成
5. **統合テスト** - Phase 4統合テストの実装

---

## タスク詳細

### Task 4.1: hw_stub.h/c の作成

**目的**: `hwconf/hw.h` に依存するコードのPC向けスタブ

**依存関係分析** (`hwconf/hw.h`):
```c
#include "stm32f4xx_conf.h"
#include HW_HEADER  // ボード固有ヘッダ

// ドライバインクルード (DRV8301, DRV8305, DRV8316, DRV8320S, DRV8323S)
// GPIOピン定義、ADCチャンネル定義、タイマー設定等
```

**utils_sys.c が必要とするhw定義**:
- `READ_HALL1()`, `READ_HALL2()`, `READ_HALL3()` - ホールセンサ読み取り
- `READ_HALL1_2()`, `READ_HALL2_2()`, `READ_HALL3_2()` - 第2モーター用
- `HW_HAS_DUAL_MOTORS` - デュアルモーターフラグ

**作成ファイル**: `pc_build/hw_stub/hw_stub.h`

```c
// 必要な定義
#ifndef HW_STUB_H_
#define HW_STUB_H_

#include <stdint.h>
#include <stdbool.h>

// ホールセンサスタブ
#define READ_HALL1()    hw_stub_read_hall1()
#define READ_HALL2()    hw_stub_read_hall2()
#define READ_HALL3()    hw_stub_read_hall3()
#define READ_HALL1_2()  hw_stub_read_hall1_2()
#define READ_HALL2_2()  hw_stub_read_hall2_2()
#define READ_HALL3_2()  hw_stub_read_hall3_2()

int hw_stub_read_hall1(void);
int hw_stub_read_hall2(void);
int hw_stub_read_hall3(void);
int hw_stub_read_hall1_2(void);
int hw_stub_read_hall2_2(void);
int hw_stub_read_hall3_2(void);

// シミュレーション用ホール値設定
void hw_stub_set_hall_values(int h1, int h2, int h3);
void hw_stub_set_hall_values_2(int h1, int h2, int h3);

// HW設定定数
#define HW_NAME "PC_STUB"
#define HW_SOURCE "hw_stub.h"

// スタックアライメント型 (ChibiOS互換)
typedef uint32_t stkalign_t;

#endif
```

**作成ファイル**: `pc_build/hw_stub/hw_stub.c`

```c
#include "hw_stub.h"

static int hall1 = 0, hall2 = 0, hall3 = 0;
static int hall1_2 = 0, hall2_2 = 0, hall3_2 = 0;

int hw_stub_read_hall1(void)   { return hall1; }
int hw_stub_read_hall2(void)   { return hall2; }
int hw_stub_read_hall3(void)   { return hall3; }
int hw_stub_read_hall1_2(void) { return hall1_2; }
int hw_stub_read_hall2_2(void) { return hall2_2; }
int hw_stub_read_hall3_2(void) { return hall3_2; }

void hw_stub_set_hall_values(int h1, int h2, int h3) {
    hall1 = h1; hall2 = h2; hall3 = h3;
}

void hw_stub_set_hall_values_2(int h1, int h2, int h3) {
    hall1_2 = h1; hall2_2 = h2; hall3_2 = h3;
}
```

**成果物**:
- [x] `pc_build/hw_stub/hw_stub.h`
- [x] `pc_build/hw_stub/hw_stub.c`
- [x] ホールセンサシミュレーション機能

**検証**:
```bash
gcc -c pc_build/hw_stub/hw_stub.c -I pc_build/hw_stub -o build/hw_stub.o
```

---

### Task 4.2: conf_general_pc.h の作成

**目的**: `conf_general.h` に依存するコードのPC向け設定

**依存関係分析** (`conf_general.h`):
- ファームウェアバージョン定義
- ハードウェア設定マクロ
- `app.h` から `app_configuration` 型が必要

**utils_sys.c が必要とする定義**:
- `app_get_configuration()` 関数（`app.h`経由）
- `app_configuration` 型（`datatypes.h`で定義済み）

**作成ファイル**: `pc_build/stubs/conf_general_pc.h`

```c
#ifndef CONF_GENERAL_PC_H_
#define CONF_GENERAL_PC_H_

#include "datatypes.h"

// ファームウェアバージョン（PC向けスタブ）
#define FW_VERSION_MAJOR    6
#define FW_VERSION_MINOR    0
#define FW_VERSION_PATCH    0

// テスト・シミュレーション向け設定
#define PC_BUILD            1

#endif
```

**成果物**:
- [x] `pc_build/stubs/conf_general_pc.h`

---

### Task 4.3: app_stub.c の作成

**目的**: `applications/app.h` 関数のスタブ実装

**依存関係分析** (`app.h`):
```c
const app_configuration* app_get_configuration(void);
void app_set_configuration(app_configuration *conf);
// ... その他のアプリケーション関数
```

**utils_sys.c が必要とする関数**:
- `app_get_configuration()` - コントローラID取得に使用

**作成ファイル**: `pc_build/stubs/app_stub.h`

```c
#ifndef APP_STUB_H_
#define APP_STUB_H_

#include "datatypes.h"

// アプリケーション設定スタブ
const app_configuration* app_get_configuration(void);
void app_set_configuration(app_configuration *conf);

// テスト用設定変更
void app_stub_set_controller_id(uint8_t id);

#endif
```

**作成ファイル**: `pc_build/stubs/app_stub.c`

```c
#include "app_stub.h"
#include <string.h>

static app_configuration app_conf;
static bool initialized = false;

static void init_default_conf(void) {
    if (!initialized) {
        memset(&app_conf, 0, sizeof(app_conf));
        app_conf.controller_id = 0;
        // 他のデフォルト値設定
        initialized = true;
    }
}

const app_configuration* app_get_configuration(void) {
    init_default_conf();
    return &app_conf;
}

void app_set_configuration(app_configuration *conf) {
    if (conf) {
        memcpy(&app_conf, conf, sizeof(app_conf));
    }
}

void app_stub_set_controller_id(uint8_t id) {
    init_default_conf();
    app_conf.controller_id = id;
}
```

**成果物**:
- [x] `pc_build/stubs/app_stub.h`
- [x] `pc_build/stubs/app_stub.c`

**検証**:
```bash
gcc -c pc_build/stubs/app_stub.c -I pc_build/stubs -I .. -o build/app_stub.o
```

---

### Task 4.4: ChibiOS POSIX拡張（utils_sys.c向け）

**目的**: `utils_sys.c` が必要とする追加ChibiOS API

**依存関係分析** (`utils_sys.c`):
```c
#include "hal.h"           // chSysLock/Unlock (既存)
#include "app.h"           // app_get_configuration (Task 4.3で対応)

// utils_stack_left_now() で使用:
__get_PSP()                // PSPレジスタ取得 (ARM固有)
chThdGetSelfX()            // 現在スレッド取得 (既存)
thread_t->p_stklimit       // スタックリミット (既存)
stkalign_t                 // スタックアライメント型 (Task 4.1で定義)
struct port_intctx         // ポートコンテキスト (PC向け定義が必要)
```

**追加定義**: `pc_build/chibios_posix/ch.h`

```c
// PC向け: __get_PSP() は使用不可のため代替実装
// port_intctx型定義
struct port_intctx {
    uint32_t dummy;
};

// PC向け: スタック計算は概算を返す
#define __get_PSP() ((uint32_t)0)
```

**成果物**:
- [x] `ch.h` への追加定義 (`struct port_intctx`, `__get_PSP`)
- [x] `chVTTimeElapsedSinceX` マクロ（UTILS_AGE_S用）

---

### Task 4.5: utils_sys.c の統合

**目的**: Phase 3で延期された `util/utils_sys.c` をライブラリに統合

**依存関係チェックリスト**:
- [x] `hal.h` - `chSysLock()`/`chSysUnlock()` (既存)
- [x] `app.h` - `app_get_configuration()` (Task 4.3)
- [x] `READ_HALLx()` マクロ (Task 4.1)
- [x] `HW_TYPE` enum - `datatypes.h` (既存)
- [x] `thread_t->p_stklimit` (既存)
- [x] `stkalign_t` (Task 4.1)
- [x] `__get_PSP()` (Task 4.4)
- [x] `struct port_intctx` (Task 4.4)

**Makefile変更**:

```makefile
# Phase 4 sources
PHASE4_SRC = \
    $(ROOT)/util/utils_sys.c

# Phase 4 stubs
PHASE4_STUB_SRC = \
    hw_stub/hw_stub.c \
    stubs/app_stub.c

# Object files
PHASE4_OBJS = $(BUILDDIR)/util/utils_sys.o
PHASE4_STUB_OBJS = $(BUILDDIR)/hw_stub/hw_stub.o $(BUILDDIR)/stubs/app_stub.o

# Include path更新
INCLUDES += -Ihw_stub
```

**ビルドルール**:
```makefile
$(BUILDDIR)/util/utils_sys.o: $(ROOT)/util/utils_sys.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -include hw_stub.h -include app_stub.h -c $< -o $@

$(BUILDDIR)/hw_stub/hw_stub.o: hw_stub/hw_stub.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/stubs/app_stub.o: stubs/app_stub.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
```

**成果物**:
- [x] `utils_sys.c` がビルド可能
- [x] ライブラリに統合

**検証**:
```bash
make clean && make lib
nm build/libvesc_pc.a | grep -E "utils_sys|utils_read_hall|utils_second_motor"
```

---

### Task 4.6: Phase 4 統合テスト

**目的**: Phase 4で追加した機能のテスト

**テスト対象関数** (`utils_sys.h`):
| 関数 | テスト内容 |
|------|-----------|
| `utils_sys_lock_cnt()` | ロックカウンタのインクリメント |
| `utils_sys_unlock_cnt()` | ロックカウンタのデクリメント |
| `utils_second_motor_id()` | 第2モーターID計算 |
| `utils_read_hall()` | ホールセンサ読み取り |
| `utils_hw_type_to_string()` | HW_TYPE文字列変換 |
| `utils_check_min_stack_left()` | スタック残量チェック |
| `utils_is_func_valid()` | 関数アドレス検証 |

**テストファイル**: `pc_build/test_phase4.c`

```c
// テストケース例
void test_utils_sys(void) {
    // utils_sys_lock_cnt / unlock_cnt
    utils_sys_lock_cnt();
    utils_sys_lock_cnt();
    utils_sys_unlock_cnt();
    utils_sys_unlock_cnt();
    // 正常終了すれば成功
    
    // utils_second_motor_id
    app_stub_set_controller_id(10);
    uint8_t id2 = utils_second_motor_id();
    TEST_ASSERT(id2 == 11 || id2 == 0);  // HW_HAS_DUAL_MOTORS依存
    
    // utils_read_hall
    hw_stub_set_hall_values(1, 0, 1);
    int hall = utils_read_hall(false, 0);
    TEST_ASSERT(hall == 5);  // b101 = 5
    
    // utils_hw_type_to_string
    const char* name = utils_hw_type_to_string(HW_TYPE_VESC);
    TEST_ASSERT(strcmp(name, "HW_TYPE_VESC") == 0);
    
    // utils_is_func_valid - PC環境では常にfalse想定
    bool valid = utils_is_func_valid((void*)0x08000000);
    // PC環境では実装依存
}
```

**成果物**:
- [x] `pc_build/test_phase4.c`
- [x] Makefileに `test_phase4` ターゲット追加

**検証**:
```bash
make test_phase4
```

---

## 実装順序

```
Task 4.1 (hw_stub)
      │
      ▼
Task 4.2 (conf_general_pc.h)
      │
      ▼
Task 4.3 (app_stub) ──────────┐
      │                       │
      ▼                       ▼
Task 4.4 (ChibiOS拡張) ───► Task 4.5 (utils_sys.c統合)
                                     │
                                     ▼
                              Task 4.6 (テスト)
```

---

## 成果物一覧

| ファイル | 状態 | 説明 |
|----------|------|------|
| `pc_build/hw_stub/hw_stub.h` | 新規作成 | ハードウェアスタブヘッダ |
| `pc_build/hw_stub/hw_stub.c` | 新規作成 | ハードウェアスタブ実装 |
| `pc_build/stubs/conf_general_pc.h` | 新規作成 | PC用設定ヘッダ |
| `pc_build/stubs/app_stub.h` | 新規作成 | アプリスタブヘッダ |
| `pc_build/stubs/app_stub.c` | 新規作成 | アプリスタブ実装 |
| `pc_build/chibios_posix/ch.h` | 更新 | port_intctx追加 |
| `pc_build/Makefile` | 更新 | Phase 4ソース追加 |
| `pc_build/test_phase4.c` | 新規作成 | Phase 4テスト |

---

## 期待される成果

| 項目 | Phase 3終了時 | Phase 4目標 |
|------|--------------|-------------|
| ライブラリサイズ | 280KB | ~290KB |
| 関数数 | 188 | ~196 (+8) |
| テスト数 | 55 | ~70 (+15) |
| 達成率 | 95% | 100% |

---

## リスクと対策

| リスク | 影響度 | 対策 |
|--------|--------|------|
| `utils_stack_left_now()` がPC環境で機能しない | 低 | ダミー値を返す実装 |
| `utils_is_func_valid()` がPC環境で誤判定 | 低 | PC用のアドレス範囲に調整またはスキップ |
| `HW_HAS_DUAL_MOTORS` 未定義時の動作 | 低 | 条件コンパイルでシングルモーター動作 |

---

## 次フェーズへの準備

Phase 4完了後、Phase 5（モーター制御ロジック）に向けて以下が可能になる：

1. **mc_interface.c** のビルド準備
   - `utils_sys.h` が使用可能
   - `app.h` スタブが利用可能

2. **mcpwm_foc.c** のビルド準備
   - タイマー/ADCスタブの拡張が必要（Phase 5で対応）

3. **datatypes.h** の全型が利用可能
   - `mc_configuration`, `motor_state` 等

---

## チェックリスト

- [ ] Task 4.1: hw_stub.h/c 作成
- [ ] Task 4.2: conf_general_pc.h 作成
- [ ] Task 4.3: app_stub.h/c 作成
- [ ] Task 4.4: ChibiOS POSIX拡張
- [ ] Task 4.5: utils_sys.c 統合
- [ ] Task 4.6: Phase 4テスト作成・実行
- [ ] ライブラリビルド確認
- [ ] 全テスト合格確認
