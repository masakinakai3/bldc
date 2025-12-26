# Phase 2: ユーティリティビルド実行計画

| 項目 | 内容 |
|------|------|
| **作成日** | 2025年12月26日 |
| **フェーズ** | Phase 2: ユーティリティビルド |
| **前提** | Phase 1 完了（検証レポート: [phase1_verification_report.md](../test/phase1_verification_report.md)） |
| **想定期間** | 3-5日 |
| **目標** | ハードウェア非依存ユーティリティをPC向けにビルド・ライブラリ化 |

---

## 1. 概要

### 1.1 Phase 2の目的

VESCファームウェアの **ハードウェア非依存ユーティリティモジュール** をPC向けGCCでビルドし、静的ライブラリ `libvesc_pc.a` として生成する。

### 1.2 成果物

| 成果物 | 説明 |
|--------|------|
| `pc_build/build/libvesc_pc.a` | ユーティリティ静的ライブラリ |
| `pc_build/test_utils.c` | ユーティリティ統合テストプログラム |
| `pc_build/build/test_utils` | テスト実行ファイル |
| Phase 2検証レポート | `doc/design/test/phase2_verification_report.md` |

### 1.3 スコープ

#### ビルド対象ファイル

| ファイル | 行数 | 依存性 | 既存テスト |
|----------|------|--------|-----------|
| `util/utils_math.c` | 735 | 標準ライブラリのみ | `tests/utils_math/` (Google Test) |
| `util/utils_math.h` | 251 | `<stdbool.h>`, `<stdint.h>`, `<math.h>` | - |
| `util/buffer.c` | 250 | `<math.h>` | `tests/float_serialization/` |
| `util/buffer.h` | 54 | `<stdint.h>` | - |
| `util/crc.c` | 110 | 条件付き `stm32f4xx.h` | - |
| `util/crc.h` | 34 | `<stdint.h>` | - |
| **合計** | **1,434行** | | |

#### スコープ外

| ファイル | 理由 |
|----------|------|
| `util/utils_sys.c` | ChibiOS依存（`chVTGetSystemTimeX`等） |
| `util/mempools.c` | ChibiOS依存（メモリプール） |
| `util/worker.c` | ChibiOS依存（スレッド） |
| `util/digital_filter.c` | Phase 3以降で検討 |

---

## 2. 依存関係分析

### 2.1 utils_math.c

```
utils_math.c
├── utils_math.h
│   ├── <stdbool.h>    ✅ 標準
│   ├── <stdint.h>     ✅ 標準
│   └── <math.h>       ✅ 標準
├── <string.h>         ✅ 標準
└── <stdlib.h>         ✅ 標準
```

**判定**: ✅ 即座にビルド可能

### 2.2 buffer.c

```
buffer.c
├── buffer.h
│   └── <stdint.h>     ✅ 標準
├── <math.h>           ✅ 標準
└── <stdbool.h>        ✅ 標準
```

**判定**: ✅ 即座にビルド可能

### 2.3 crc.c

```
crc.c
├── crc.h
│   └── <stdint.h>     ✅ 標準
└── #ifndef NO_STM32
    └── "stm32f4xx.h"  ⚠️ 条件付き依存
```

**判定**: ✅ `-DNO_STM32` フラグでビルド可能（既存パターン）

---

## 3. 実装タスク

### 3.1 タスク一覧

| ID | タスク | 優先度 | 想定時間 | 依存 |
|----|--------|--------|---------|------|
| 2.1 | Makefile拡張 | 高 | 2h | - |
| 2.2 | utils_math.c統合 | 高 | 1h | 2.1 |
| 2.3 | buffer.c統合 | 高 | 1h | 2.1 |
| 2.4 | crc.c統合 | 高 | 1h | 2.1 |
| 2.5 | libvesc_pc.a生成 | 高 | 1h | 2.2-2.4 |
| 2.6 | テストプログラム作成 | 中 | 4h | 2.5 |
| 2.7 | 既存テスト互換確認 | 中 | 2h | 2.5 |
| 2.8 | ビルド検証・デバッグ | 中 | 2h | 2.6 |
| 2.9 | ドキュメント更新 | 低 | 1h | 2.8 |
| **合計** | | | **15h** | |

### 3.2 詳細設計

#### タスク 2.1: Makefile拡張

**目的**: 既存Makefileにユーティリティソースとライブラリ生成を追加

**変更ファイル**: `pc_build/Makefile`

**追加内容**:

```makefile
# =============================================================================
# Phase 2: Utility sources
# =============================================================================

ROOT = ..

# Utility sources
UTIL_SRC = \
    $(ROOT)/util/utils_math.c \
    $(ROOT)/util/buffer.c \
    $(ROOT)/util/crc.c

# Update include paths
INCLUDES += -I$(ROOT)/util

# Utility objects
UTIL_OBJS = $(patsubst $(ROOT)/%.c,$(BUILDDIR)/%.o,$(UTIL_SRC))

# =============================================================================
# Library target
# =============================================================================

# Static library
$(BUILDDIR)/libvesc_pc.a: $(UTIL_OBJS) $(CHIBIOS_OBJS) $(STUBS_OBJS)
	$(AR) rcs $@ $^
	@echo "Library built: $@"

# Utility object compile rule
$(BUILDDIR)/util/%.o: $(ROOT)/util/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Utility test
$(BUILDDIR)/test_utils: test_utils.c $(BUILDDIR)/libvesc_pc.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILDDIR) -lvesc_pc $(LIBS)

.PHONY: lib test_utils
lib: dirs $(BUILDDIR)/libvesc_pc.a
test_utils: $(BUILDDIR)/test_utils
	@./$(BUILDDIR)/test_utils
```

**成功基準**:
- `make lib` でライブラリ生成成功
- `make test_utils` でテスト実行成功

---

#### タスク 2.2: utils_math.c統合

**目的**: utils_math.cをPC向けビルドに統合

**確認事項**:

| チェック項目 | 状態 |
|--------------|------|
| ハードウェア依存なし | ✅ 確認済み |
| 浮動小数点演算 (`fabsf`, `sinf` 等) | ✅ 標準math.h |
| インライン関数/マクロ | ✅ ヘッダで完結 |

**テストケース** (既存テストより抽出):

```c
// テスト: utils_truncate_number
void test_utils_truncate_number(void) {
    float val = 150.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    assert(val == 100.0f);
    
    val = -150.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    assert(val == -100.0f);
}

// テスト: utils_norm_angle
void test_utils_norm_angle(void) {
    float angle = 370.0f;
    utils_norm_angle(&angle);
    assert(fabsf(angle - 10.0f) < 0.001f);
    
    angle = -10.0f;
    utils_norm_angle(&angle);
    assert(fabsf(angle - 350.0f) < 0.001f);
}

// テスト: utils_fast_atan2
void test_utils_fast_atan2(void) {
    float result = utils_fast_atan2(1.0f, 1.0f);
    float expected = atan2f(1.0f, 1.0f) * (180.0f / M_PI);
    assert(fabsf(result - expected) < 1.0f);  // 1度以内の精度
}
```

---

#### タスク 2.3: buffer.c統合

**目的**: buffer.cをPC向けビルドに統合

**確認事項**:

| チェック項目 | 状態 |
|--------------|------|
| ハードウェア依存なし | ✅ 確認済み |
| エンディアン | ✅ ビッグエンディアン形式（プロトコル互換） |
| float/double変換 | ✅ 標準的なIEEE 754 |

**テストケース** (既存テストより抽出):

```c
// テスト: buffer_append/get_int32
void test_buffer_int32(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    buffer_append_int32(buf, 0x12345678, &idx);
    assert(idx == 4);
    assert(buf[0] == 0x12);
    assert(buf[1] == 0x34);
    assert(buf[2] == 0x56);
    assert(buf[3] == 0x78);
    
    idx = 0;
    int32_t val = buffer_get_int32(buf, &idx);
    assert(val == 0x12345678);
}

// テスト: buffer_float32_auto
void test_buffer_float32_auto(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    float orig = 123.456f;
    buffer_append_float32_auto(buf, orig, &idx);
    
    idx = 0;
    float read = buffer_get_float32_auto(buf, &idx);
    assert(fabsf(read - orig) < 0.001f);
}
```

---

#### タスク 2.4: crc.c統合

**目的**: crc.cをPC向けビルドに統合

**確認事項**:

| チェック項目 | 状態 |
|--------------|------|
| `NO_STM32` マクロ対応 | ✅ 既存対応済み |
| STM32ハードウェアCRC | ⚠️ `crc32()` 関数はSTM32依存 |
| ソフトウェアCRC16 | ✅ ハードウェア非依存 |

**STM32依存の詳細**:

```c
// crc.c 内の crc32() 関数
uint32_t crc32(uint32_t *buf, uint32_t len) {
#ifndef NO_STM32
    // STM32のCRCペリフェラルを使用
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    CRC->CR = 1;
    // ...
#endif
    return 0;  // NO_STM32時は0を返す
}
```

**対応方針**:
- Phase 2では `-DNO_STM32` でビルド（既存動作）
- `crc32()` はソフトウェア実装を後で追加することを検討

**テストケース**:

```c
// テスト: crc16
void test_crc16(void) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc = crc16(data, 4);
    // 既知の正解値と比較
    assert(crc == 0x89C3);  // 事前計算値
}

// テスト: crc16_rolling
void test_crc16_rolling(void) {
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x03, 0x04};
    
    uint16_t crc = 0;
    crc = crc16_rolling(crc, data1, 2);
    crc = crc16_rolling(crc, data2, 2);
    
    uint8_t data_all[] = {0x01, 0x02, 0x03, 0x04};
    assert(crc == crc16(data_all, 4));
}
```

---

#### タスク 2.5: libvesc_pc.a生成

**目的**: 静的ライブラリを生成

**コマンド**:

```bash
cd pc_build
make lib
```

**成功基準**:

| 基準 | 確認方法 |
|------|----------|
| ビルドエラーなし | `make lib` の終了コード 0 |
| 警告なし | stderr に warning 出力なし |
| ライブラリ生成 | `build/libvesc_pc.a` 存在確認 |
| シンボル確認 | `nm build/libvesc_pc.a` で関数確認 |

**期待されるシンボル**:

```
utils_math.o:
  T utils_truncate_number
  T utils_norm_angle
  T utils_fast_atan2
  T utils_fast_sin
  T utils_fast_cos
  ...

buffer.o:
  T buffer_append_int16
  T buffer_append_int32
  T buffer_get_int32
  T buffer_append_float32_auto
  ...

crc.o:
  T crc16
  T crc16_rolling
  T crc32_with_init
  ...
```

---

#### タスク 2.6: テストプログラム作成

**目的**: ユーティリティ統合テストプログラム作成

**ファイル**: `pc_build/test_utils.c`

**構成**:

```c
/*
 * VESC PC Build - Phase 2 Utility Test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "utils_math.h"
#include "buffer.h"
#include "crc.h"

// =============================================================================
// Test counters
// =============================================================================
static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(test_func) do { \
    printf("  %-40s", #test_func); \
    tests_run++; \
    test_func(); \
    tests_passed++; \
    printf(" PASS\n"); \
} while(0)

// =============================================================================
// utils_math tests
// =============================================================================

static void test_utils_truncate_number(void) { /* ... */ }
static void test_utils_norm_angle(void) { /* ... */ }
static void test_utils_fast_atan2(void) { /* ... */ }
static void test_utils_fast_sincos(void) { /* ... */ }
static void test_utils_throttle_curve(void) { /* ... */ }
static void test_utils_map_angle(void) { /* ... */ }
static void test_utils_deadband(void) { /* ... */ }

// =============================================================================
// buffer tests
// =============================================================================

static void test_buffer_int16(void) { /* ... */ }
static void test_buffer_int32(void) { /* ... */ }
static void test_buffer_int64(void) { /* ... */ }
static void test_buffer_float16(void) { /* ... */ }
static void test_buffer_float32(void) { /* ... */ }
static void test_buffer_float32_auto(void) { /* ... */ }
static void test_buffer_double64(void) { /* ... */ }

// =============================================================================
// crc tests
// =============================================================================

static void test_crc16_basic(void) { /* ... */ }
static void test_crc16_rolling(void) { /* ... */ }
static void test_crc32_with_init(void) { /* ... */ }

// =============================================================================
// Main
// =============================================================================

int main(void) {
    printf("=== VESC PC Build - Phase 2 Utility Test ===\n\n");
    
    printf("[utils_math]\n");
    RUN_TEST(test_utils_truncate_number);
    RUN_TEST(test_utils_norm_angle);
    RUN_TEST(test_utils_fast_atan2);
    RUN_TEST(test_utils_fast_sincos);
    RUN_TEST(test_utils_throttle_curve);
    RUN_TEST(test_utils_map_angle);
    RUN_TEST(test_utils_deadband);
    
    printf("\n[buffer]\n");
    RUN_TEST(test_buffer_int16);
    RUN_TEST(test_buffer_int32);
    RUN_TEST(test_buffer_int64);
    RUN_TEST(test_buffer_float16);
    RUN_TEST(test_buffer_float32);
    RUN_TEST(test_buffer_float32_auto);
    RUN_TEST(test_buffer_double64);
    
    printf("\n[crc]\n");
    RUN_TEST(test_crc16_basic);
    RUN_TEST(test_crc16_rolling);
    RUN_TEST(test_crc32_with_init);
    
    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);
    
    return (tests_passed == tests_run) ? 0 : 1;
}
```

**テストケース数目標**: 20+

---

#### タスク 2.7: 既存テスト互換確認

**目的**: 既存テストディレクトリのテストが新インフラで動作することを確認

**対象テスト**:

| テスト | パス | 方式 |
|--------|------|------|
| float_serialization | `tests/float_serialization/` | 純粋C + `-DNO_STM32` |
| utils_math | `tests/utils_math/` | Google Test |

**検証手順**:

```bash
# float_serialization テスト
cd tests/float_serialization
make clean && make
./test

# utils_math テスト (Google Test)
cd tests/utils_math
make clean && make
./test
```

---

#### タスク 2.8: ビルド検証・デバッグ

**目的**: WSL環境でのビルド・実行検証

**検証手順**:

```bash
# WSLでの実行
cd /mnt/c/Users/galax/bldc/bldc/pc_build

# クリーンビルド
make clean
make lib

# ライブラリ確認
ls -la build/libvesc_pc.a
nm build/libvesc_pc.a | head -50

# テスト実行
make test_utils

# Valgrindでメモリチェック (オプション)
valgrind --leak-check=full ./build/test_utils
```

**チェックリスト**:

| 項目 | 確認方法 |
|------|----------|
| コンパイルエラーなし | exit code 0 |
| 警告なし | stderr 確認 |
| 全テストパス | テスト出力確認 |
| メモリリークなし | Valgrind出力 |

---

#### タスク 2.9: ドキュメント更新

**目的**: README.mdとフェーズドキュメント更新

**更新ファイル**:

1. `pc_build/README.md` - Phase 2の使用方法追記
2. `doc/design/test/phase2_verification_report.md` - 検証レポート作成

---

## 4. ディレクトリ構成 (Phase 2完了後)

```
bldc/
├── pc_build/
│   ├── Makefile                    # [更新] lib, test_utils ターゲット追加
│   ├── README.md                   # [更新] Phase 2手順追記
│   ├── test_main.c                 # Phase 1のテスト (維持)
│   ├── test_utils.c                # [新規] Phase 2テストプログラム
│   │
│   ├── build/
│   │   ├── test_pc                 # Phase 1テスト実行ファイル
│   │   ├── test_utils              # [新規] Phase 2テスト実行ファイル
│   │   ├── libvesc_pc.a            # [新規] 静的ライブラリ
│   │   └── util/
│   │       ├── utils_math.o        # [新規]
│   │       ├── buffer.o            # [新規]
│   │       └── crc.o               # [新規]
│   │
│   ├── stubs/                      # Phase 1から継続
│   ├── chibios_posix/              # Phase 1から継続
│   └── hw_stub/                    # Phase 1から継続
│
└── util/                           # 既存 (変更なし)
    ├── utils_math.c
    ├── utils_math.h
    ├── buffer.c
    ├── buffer.h
    ├── crc.c
    └── crc.h
```

---

## 5. 受け入れ基準

### 5.1 必須基準 (Must Have)

| ID | 基準 | 検証方法 |
|----|------|----------|
| M1 | `make lib` がエラー・警告なしで成功 | ビルドログ確認 |
| M2 | `build/libvesc_pc.a` が生成される | ファイル存在確認 |
| M3 | ライブラリに全関数シンボルが含まれる | `nm` コマンド |
| M4 | `make test_utils` が全テストパス | テスト出力 |
| M5 | 既存の `tests/float_serialization` が動作 | テスト実行 |

### 5.2 推奨基準 (Should Have)

| ID | 基準 | 検証方法 |
|----|------|----------|
| S1 | Valgrindでメモリリークなし | Valgrind出力 |
| S2 | 20以上のテストケース | テストカウント |
| S3 | 既存 `tests/utils_math` (Google Test) が動作 | テスト実行 |

### 5.3 オプション基準 (Nice to Have)

| ID | 基準 | 検証方法 |
|----|------|----------|
| N1 | AddressSanitizerでエラーなし | ASan出力 |
| N2 | gcov/lcovでカバレッジ測定 | カバレッジレポート |

---

## 6. リスクと対策

| リスク | 影響度 | 発生確率 | 対策 |
|--------|--------|----------|------|
| 浮動小数点精度の差異 | 低 | 中 | 許容誤差でテスト |
| CRC32 STM32依存 | 低 | 確定 | NO_STM32で除外、Phase 3でソフト実装検討 |
| utils_math.h マクロ衝突 | 中 | 低 | インクルード順序で対応 |
| Google Testビルド問題 | 低 | 低 | 純粋Cテストで代替可能 |

---

## 7. 成功の定義

Phase 2は以下の条件を満たした時点で完了とする:

1. ✅ `libvesc_pc.a` が正常に生成される
2. ✅ `utils_math.c`, `buffer.c`, `crc.c` の全関数がライブラリに含まれる
3. ✅ 統合テスト (`test_utils`) が全てパス
4. ✅ 既存テスト (`tests/float_serialization`) が動作確認
5. ✅ 検証レポートが作成される

---

## 8. Phase 3への準備

Phase 2完了後、以下をPhase 3に引き継ぐ:

| 項目 | 内容 |
|------|------|
| ChibiOS依存ユーティリティ | `utils_sys.c`, `mempools.c`, `worker.c` |
| デジタルフィルタ | `digital_filter.c` |
| CRC32ソフトウェア実装 | STM32ハードウェアCRCの代替 |
| イベントAPI完全実装 | `chEvtBroadcast`等 |

---

## 参考資料

- [Phase 1 仕様書](phase1_pc_build_foundation.md)
- [Phase 1 検証レポート](../test/phase1_verification_report.md)
- [PC Build 戦略設計書](../pc_build_strategy.md)
- [既存テスト: float_serialization](../../../tests/float_serialization/)
- [既存テスト: utils_math](../../../tests/utils_math/)
