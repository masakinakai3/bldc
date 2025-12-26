# VESC ファームウェア コンパイラ依存箇所調査レポート

## 概要

本ドキュメントは、VESCファームウェアリポジトリにおけるコンパイラ依存箇所を調査した結果をまとめたものです。

**調査日**: 2025年12月25日  
**対象コンパイラ**: GNU ARM Embedded Toolchain (arm-none-eabi-gcc 7-2018-q2-update)  
**ターゲットMCU**: STM32F4 (Cortex-M4)

---

## 1. ツールチェーン依存

### 1.1 指定コンパイラ

プロジェクトは **arm-none-eabi-gcc** (GNU ARM Embedded Toolchain) を前提としています。

| ファイル | 内容 |
|----------|------|
| [make/tools.mk](../../make/tools.mk) | ARM SDK バージョン 7-2018-q2-update を指定 |
| [make/fw.mk](../../make/fw.mk#L218) | `TRGT = $(TCHAIN_PREFIX)` でGCCツールチェーンを使用 |

```makefile
# make/tools.mk:14
ARM_SDK_DIR := $(TOOLS_DIR)/gcc-arm-none-eabi-7-2018-q2-update

# make/tools.mk:227
ARM_SDK_PREFIX := $(ARM_SDK_DIR)/bin/arm-none-eabi-
```

### 1.2 ChibiOS RTOSのコンパイラサポート

ChibiOSは複数コンパイラをサポートしていますが、本プロジェクトはGCC専用です。

```
ChibiOS_3.0.5/os/common/ports/ARMCMx/compilers/
├── GCC/   ← 使用中
├── IAR/   ← 未使用
└── RVCT/  ← 未使用
```

---

## 2. GCC固有のプラグマ

### 2.1 最適化プラグマ

GCC固有の `#pragma GCC optimize` を使用して、関数/ファイル単位で最適化レベルを制御しています。

| ファイル | 行 | 内容 |
|----------|-----|------|
| [terminal.c](../../terminal.c#L20) | 20 | `#pragma GCC optimize ("Os")` |
| [main.c](../../main.c#L20) | 20 | `#pragma GCC optimize ("Os")` |
| [motor/mcpwm_foc.c](../../motor/mcpwm_foc.c#L166-L167) | 166-167 | `#pragma GCC push_options` / `#pragma GCC optimize ("Os")` |
| [motor/mc_interface.c](../../motor/mc_interface.c#L20-L21) | 20-21 | `#pragma GCC push_options` / `#pragma GCC optimize ("Os")` |
| [libcanard/canard.c](../../libcanard/canard.c#L27) | 27 | `#pragma GCC optimize ("Os")` |
| [lispBM/lispif.c](../../lispBM/lispif.c#L21) | 21 | `#pragma GCC optimize ("Os")` |
| [lispBM/lispif_vesc_extensions.c](../../lispBM/lispif_vesc_extensions.c#L21) | 21 | `#pragma GCC optimize ("Os")` |
| [lispBM/lispif_c_lib.c](../../lispBM/lispif_c_lib.c#L20) | 20 | `#pragma GCC optimize ("Os")` |
| [lispBM/lispBM/src/fundamental.c](../../lispBM/lispBM/src/fundamental.c#L35) | 35 | `#pragma GCC optimize ("-Os")` |

**影響**: これらのプラグマは他のコンパイラ (Clang, IAR, ARM Compiler等) では無視されるか、エラーとなります。

---

## 3. GCC固有の属性 (`__attribute__`)

### 3.1 セクション配置 (`__attribute__((section(...)))`)

メモリ配置の最適化のため、特定の変数/関数をカスタムセクションに配置しています。

| ファイル | セクション | 用途 |
|----------|------------|------|
| [motor/mc_interface.c](../../motor/mc_interface.c#L109-L119) | `.ram4` | ADCサンプルバッファ (CCM SRAM) |
| [comm/comm_can.c](../../comm/comm_can.c#L64-L67) | `.ram4` | CAN通信スレッドワークエリア |
| [comm/comm_usb.c](../../comm/comm_usb.c#L34) | `.ram4` | USBシリアルスレッドワークエリア |
| [applications/app_nunchuk.c](../../applications/app_nunchuk.c#L45-L47) | `.ram4` | Nunchukスレッドワークエリア |
| [applications/app_ppm.c](../../applications/app_ppm.c#L41) | `.ram4` | PPMスレッドワークエリア |
| [applications/app_adc.c](../../applications/app_adc.c#L52) | `.ram4` | ADCアプリスレッドワークエリア |
| [applications/app_pas.c](../../applications/app_pas.c#L45) | `.ram4` | PASスレッドワークエリア |
| [applications/app_uartcomm.c](../../applications/app_uartcomm.c#L47) | `.ram4` | UARTパケット処理スレッド |
| [terminal.c](../../terminal.c#L70) | `.text2` | ターミナル処理関数 |
| [lispBM/lispif.c](../../lispBM/lispif.c#L55-L68) | `.ram4` | LispBM ヒープ/ビットマップ等 |
| [lispBM/lispif_c_lib.c](../../lispBM/lispif_c_lib.c#L69) | `.libif` | C ライブラリインターフェース |

**コード例**:
```c
// motor/mc_interface.c:109
__attribute__((section(".ram4"))) static volatile int16_t m_curr0_samples[ADC_SAMPLE_MAX_LEN];
```

### 3.2 アライメント (`__attribute__((aligned(...)))`)

| ファイル | 用途 |
|----------|------|
| [lispBM/lispif.c](../../lispBM/lispif.c#L55) | LispBM ヒープを8バイト境界に配置 |

```c
__attribute__((section(".ram4"))) static lbm_cons_t heap[HEAP_SIZE] __attribute__ ((aligned (8)));
```

### 3.3 パッキング (`__attribute__((packed))`)

| ファイル | 用途 |
|----------|------|
| [applications/finn/app_finn_types.h](../../applications/finn/app_finn_types.h#L73-L85) | CANパケット構造体のパッキング |

```c
typedef struct __attribute__((packed)) CAN_PACKET_POD_REQUEST { ... };
typedef struct __attribute__((packed)) CAN_PACKET_POD_STATUS { ... };
```

---

## 4. GCC固有のコンパイラフラグ

### 4.1 FPU関連フラグ

[make/fw.mk](../../make/fw.mk#L15) および [ChibiOS_3.0.5/os/common/ports/ARMCMx/compilers/GCC/rules.mk](../../ChibiOS_3.0.5/os/common/ports/ARMCMx/compilers/GCC/rules.mk#L35):

| フラグ | 用途 |
|--------|------|
| `-mfloat-abi=hard` | ハードウェアFPU使用 |
| `-mfpu=fpv4-sp-d16` | 単精度FPU指定 (Cortex-M4用) |
| `-fsingle-precision-constant` | 浮動小数点定数を単精度として扱う |
| `-Wdouble-promotion` | double への暗黙変換を警告 |

```makefile
# make/fw.mk:15
USE_OPT += -fsingle-precision-constant -Wdouble-promotion -specs=nosys.specs
```

### 4.2 Thumb/ARMモード関連

| フラグ | ファイル | 用途 |
|--------|----------|------|
| `-mthumb` | [make/fw.mk](../../make/fw.mk#L241) | Thumb命令セット使用 |
| `-DTHUMB` | [make/fw.mk](../../make/fw.mk#L241) | THUMBモード定義 |

### 4.3 C言語標準

```makefile
# make/fw.mk:10
USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16 -std=gnu99 -D_GNU_SOURCE
```

- `-std=gnu99`: GNU拡張付きC99標準
- `-D_GNU_SOURCE`: GNU拡張機能有効化

---

## 5. CMSIS インラインアセンブリ

### 5.1 ARM CMSIS Intrinsics

CMSIS (Cortex Microcontroller Software Interface Standard) が提供する組み込み関数を使用しています。これらはGCCのインラインアセンブリで実装されています。

| 関数 | 使用箇所 | 用途 |
|------|----------|------|
| `__NOP()` | 多数 | CPU待機 (タイミング調整) |
| `__WFI()` | ChibiOS | Wait For Interrupt |
| `__DMB()` | ChibiOS | Data Memory Barrier |
| `__DSB()` | ChibiOS | Data Synchronization Barrier |
| `__ISB()` | ChibiOS | Instruction Synchronization Barrier |

**使用例** ([hwconf/drv8316.c](../../hwconf/drv8316.c#L330-L336)):
```c
__NOP();
__NOP();
__NOP();
__NOP();
```

### 5.2 CMSISヘッダファイル

| ファイル | 内容 |
|----------|------|
| `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/core_cmFunc.h` | コアレジスタアクセス関数 |
| `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/core_cmInstr.h` | SIMD/DSP命令 |
| `lispBM/c_libs/stdperiph_stm32f4/CMSIS/include/core_cm4_simd.h` | Cortex-M4 SIMD命令 |

これらのファイルには `__ASM` マクロを使用したインラインアセンブリが含まれています:

```c
// core_cm4_simd.h:467
__ASM ("ssat16 %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) );
```

---

## 6. ChibiOS RTOS のコンパイラ依存

### 6.1 割り込みハンドラマクロ

[irq_handlers.c](../../irq_handlers.c) で使用:

```c
CH_IRQ_HANDLER(ADC1_2_3_IRQHandler) {
    CH_IRQ_PROLOGUE();
    // 処理
    CH_IRQ_EPILOGUE();
}
```

これらのマクロは `ChibiOS_3.0.5/os/rt/ports/ARMCMx/compilers/GCC/` 配下のヘッダで定義されており、GCCのインラインアセンブリを使用しています。

### 6.2 ポートアセンブリファイル

ChibiOSはGCC用のアセンブリファイル (`.s`) を使用:

- `ChibiOS_3.0.5/os/rt/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.s`
- `ChibiOS_3.0.5/os/common/ports/ARMCMx/compilers/GCC/crt0_v7m.s`

---

## 7. リンカースクリプト

[ld_eeprom_emu.ld](../../ld_eeprom_emu.ld) はGNU ldリンカー用のスクリプトです。

### 7.1 カスタムセクション定義

| セクション | アドレス | 用途 |
|------------|----------|------|
| `.ram4` | 0x10000000 (CCM SRAM) | 高速アクセス用データ |
| `.libif` | 0x1000F800 | C ライブラリインターフェース |
| `.text2` | flash2 | セカンダリコードセクション |

---

## 8. 外部ライブラリのコンパイラ依存

### 8.1 LZO圧縮ライブラリ

[util/lzo/lzodefs.h](../../util/lzo/lzodefs.h) と [util/lzo/minilzo.c](../../util/lzo/minilzo.c) は、多数のコンパイラをサポートする抽象化レイヤーを持っています:

- `__GNUC__`, `__clang__`, `_MSC_VER` 等のマクロで分岐
- `__attribute__((always_inline))`, `__attribute__((noinline))` 等の抽象化
- `__builtin_expect` (likely/unlikely)

### 8.2 LispBM

[lispBM/](../../lispBM/) サブモジュールもGCC前提:

- `__BYTE_ORDER__`, `__ORDER_LITTLE_ENDIAN__` でエンディアン判定
- `__attribute__((aligned))` でメモリアライメント

---

## 9. 移植性に関する注意事項

### 9.1 他コンパイラへの移植時の課題

| 項目 | 難易度 | 対応方法 |
|------|--------|----------|
| `#pragma GCC optimize` | 低 | 条件コンパイルでラップ |
| `__attribute__((section))` | 中 | コンパイラ固有構文に変換 |
| `__attribute__((packed))` | 低 | 多くのコンパイラがサポート |
| CMSIS インラインアセンブリ | 高 | CMSISの別コンパイラ版を使用 |
| ChibiOS RTOSポート | 高 | ChibiOSの該当コンパイラポートを使用 |
| リンカースクリプト | 高 | コンパイラ固有形式に書き換え |

### 9.2 推奨事項

1. **GCC使用を維持**: 本プロジェクトはGCC前提で深く統合されており、他コンパイラへの移植は大規模な作業となります
2. **GCCバージョン**: arm-none-eabi-gcc 7.x 以降を推奨 (7-2018-q2-update が公式サポート)
3. **Clang対応**: 将来的にClang対応を検討する場合、LZOライブラリの抽象化レイヤーを参考に

---

## 10. まとめ

### コンパイラ依存箇所の分類

| カテゴリ | 該当ファイル数 | 影響度 |
|----------|----------------|--------|
| GCC プラグマ | 約10ファイル | 中 |
| `__attribute__` | 約20ファイル | 高 |
| インラインアセンブリ | CMSIS + ChibiOS | 高 |
| コンパイラフラグ | Makefile 系 | 高 |
| リンカースクリプト | 1ファイル | 高 |

### 結論

VESCファームウェアは **GNU ARM Embedded Toolchain (arm-none-eabi-gcc)** に強く依存しています。この依存は以下の理由から意図的なものです:

1. **パフォーマンス最適化**: GCC固有の最適化プラグマで、メモリ制約のある組み込み環境でコードサイズを削減
2. **メモリ配置制御**: CCM SRAMへの配置でリアルタイム性能を確保
3. **ChibiOS統合**: RTOSがGCC前提で実装されている
4. **CMSIS準拠**: ARM公式のCMSISライブラリがGCCをプライマリターゲットとしている

他のコンパイラへの移植は理論的には可能ですが、ChibiOSのポート作業を含む大規模な変更が必要となります。
