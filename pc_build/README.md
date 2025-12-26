# VESC Firmware PC Build

## 概要

このディレクトリは、VESCファームウェアをPC (x86/x64) 上でビルド・テストするための基盤を提供します。

**目的:**
- ハードウェアなしでユニットテスト実行
- 静的解析・カバレッジ計測
- モータ制御アルゴリズムのシミュレーション
- CI/CDパイプライン統合

## ディレクトリ構造

```
pc_build/
├── Makefile            # ビルドシステム
├── README.md           # このファイル
├── test_main.c         # テストプログラム
├── chibios_posix/      # ChibiOS POSIX互換レイヤー
│   ├── ch.h            # ChibiOS API (pthread実装)
│   ├── ch_core.c       # コア実装
│   ├── hal.h           # HALスタブ
│   └── hal_stub.c      # HALドライバインスタンス
├── stubs/              # STM32ペリフェラルスタブ
│   ├── stm32f4xx_pc.h  # STM32F4 TypeDef定義
│   ├── stm32f4xx_pc.c  # ペリフェラルインスタンス
│   └── cmsis_pc.h      # CMSIS intrinsicsスタブ
├── hw_stub/            # ハードウェア抽象化スタブ (将来用)
└── build/              # ビルド出力
```

## 必要環境

- **Linux/WSL**: Ubuntu 20.04以降推奨
- **GCC**: 9.0以降
- **依存ライブラリ**: pthread, libm, librt (標準インストール済み)

## ビルド方法

```bash
# WSL/Linux環境で実行
cd pc_build

# ビルド
make

# テスト実行
make test

# クリーン
make clean
```

## コンパイルフラグ

| フラグ | 説明 |
|--------|------|
| `-DNO_STM32` | STM32固有コードを無効化 |
| `-DUSE_PC_BUILD` | PCビルド用コードパスを有効化 |

## 実装済み機能 (Phase 1)

### ChibiOS POSIX レイヤー
- **システム**: `chSysInit`, `chSysLock/Unlock`, `chSysHalt`
- **時間**: `chVTGetSystemTime`, `chThdSleep*`
- **スレッド**: `chThdCreateStatic`, `chThdWait`, `chThdYield`
- **Mutex**: `chMtxObjectInit`, `chMtxLock/Unlock`, `chMtxTryLock`
- **セマフォ**: `chSemObjectInit`, `chSemWait/Signal`, timeout対応
- **イベント**: 基本実装 (broadcast/wait)
- **メールボックス**: 基本実装

### STM32 スタブ
- **GPIO**: GPIOA-I
- **Timer**: TIM1-14
- **ADC**: ADC1-3
- **DMA**: DMA1/2 + Streams
- **SPI/I2C/USART/CAN**: 基本構造体

### HAL ドライバスタブ
- ADCDriver, PWMDriver, ICUDriver
- SerialDriver, UARTDriver
- SPIDriver, I2CDriver, CANDriver
- GPTDriver, WDGDriver, EXTDriver

## 使用方法

### 既存コードの条件分岐

```c
#ifdef USE_PC_BUILD
    // PC用コード
    #include "stm32f4xx_pc.h"
#else
    // 実機用コード
    #include "stm32f4xx.h"
#endif
```

### ユニットテストの追加

```c
#include "ch.h"
#include "hal.h"

void test_my_function(void) {
    chSysInit();
    halInit();
    
    // テストコード
    assert(my_function() == expected);
}
```

## 今後の計画

- **Phase 2**: utilソース統合 (`utils_math.c`, `buffer.c`, `crc.c`)
- **Phase 3**: FOC数学関数のPC対応
- **Phase 4**: ユニットテストフレームワーク統合
- **Phase 5**: CI/CD統合

## トラブルシューティング

### Clock skew警告
WSLでWindowsファイルシステムを使用時に発生。機能には影響なし。

### pthread関連エラー
`-lpthread` リンクを確認。

## 関連ドキュメント

- [Phase 1 計画書](../doc/design/plan/phase1_pc_build_foundation.md)
- [本体README](../README.md)
