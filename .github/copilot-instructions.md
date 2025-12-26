# Copilot Instructions - VESC Firmware (bldc)

## プロジェクト概要

このリポジトリは **VESC (Vedder Electronic Speed Controller)** ファームウェアです。STM32マイクロコントローラ (主にSTM32F4系) 上で動作し、BLDC/PMSMモータをFOC (Field Oriented Control) で制御するオープンソースプロジェクトです。

- **ターゲットMCU**: STM32F4 (168MHz, Cortex-M4F, 単精度FPU)
- **RTOS**: ChibiOS 3.0.5 ([ChibiOS_3.0.5/readme.txt](ChibiOS_3.0.5/readme.txt))
- **スクリプト**: LispBM組み込みインタプリタ ([lispBM/README.md](lispBM/README.md))
- **CAN通信**: libcanard/DroneCAN ([libcanard/README.md](libcanard/README.md))
- **ライセンス**: GPL v3

## ビルドシステム

**GNU Make** + **arm-none-eabi-gcc** クロスコンパイラ。主要コマンドは [Makefile](Makefile) を参照。

### セットアップ

```bash
# Linux/macOS
sudo apt install git build-essential libgl-dev libxcb-xinerama0 wget  # Ubuntu
brew install stlink openocd  # macOS

# Windows (PowerShell)
choco install make
```

### ビルドコマンド

```bash
# ツールチェーン導入 (初回のみ)
make arm_sdk_install

# ボード一覧表示
make

# ファームウェアビルド
make fw BOARD=<board_name>   # 例: make fw BOARD=100_250
make all_fw                   # 全ボードビルド

# ユニットテスト
make all_ut
make all_ut_run
make all_ut_xml

# Qt Creator IDE環境構築
pip install aqtinstall
make qt_install
```

IDE使用時は [Project/Qt Creator/vesc.pro](Project/Qt Creator/vesc.pro) を開く。

## ディレクトリ構造

### ルートファイル
| ファイル | 説明 |
|----------|------|
| `main.c`, `main.h` | エントリポイント、スレッド初期化 |
| `conf_general.h` | ファームウェアバージョン、全体設定 |
| `conf_custom.c/h` | カスタム設定 |
| `datatypes.h` | 全共通データ型・enum定義 (重要) |
| `timeout.c/h` | タイムアウト/ウォッチドッグ/フェイルセーフ |
| `flash_helper.c/h` | フラッシュメモリ操作 |
| `terminal.c/h` | ターミナルコマンド処理 |
| `bms.c/h` | バッテリー管理システム |
| `events.c/h` | イベント処理 |
| `irq_handlers.c` | 割り込みハンドラ |

### サブディレクトリ
| パス | 説明 |
|------|------|
| `motor/` | **モータ制御コア** - FOC実装 (`mcpwm_foc.c/h`)、インターフェース (`mc_interface.c/h`)、設定 (`mcconf_default.h`) |
| `applications/` | **アプリケーション層** - PPM/ADC/UART/Nunchuk/PAS入力処理 (`app.h`がAPI) |
| `comm/` | **通信** - USB (`comm_usb.c`)、CAN (`comm_can.c`)、コマンド処理 (`commands.c`)、パケット (`packet.c`) |
| `encoder/` | **エンコーダ** - ABI/SPI/BiSS-C/Sin-Cos等の各種エンコーダドライバ |
| `hwconf/` | **ハードウェア定義** - 各ボード固有設定 (`hw_*.h`)、ゲートドライバ (DRV8301/8305/8316/8320s/8323s) |
| `driver/` | **ペリフェラルドライバ** - I2C/SPI (ビットバング)、LED PWM、サーボ、NRF、LoRa |
| `imu/` | IMUドライバ |
| `util/` | **ユーティリティ** - バッファ操作、CRC、数学関数、メモリプール |
| `ChibiOS_3.0.5/` | RTOS (外部依存、原則変更しない) |
| `lispBM/` | LispBMインタプリタ (外部依存) |
| `libcanard/` | DroneCAN実装 (外部依存) |
| `blackmagic/` | Black Magic Probeデバッガ統合 |
| `make/` | ビルドシステム補助 (`fw.mk`, `tools.mk`, OS別設定) |

## 主要API

### モータ制御 (`motor/mc_interface.h`)
```c
void mc_interface_set_duty(float dutyCycle);
void mc_interface_set_current(float current);
void mc_interface_set_pid_speed(float rpm);
void mc_interface_set_pid_pos(float pos);
float mc_interface_get_rpm(void);
mc_fault_code mc_interface_get_fault(void);
```

### アプリケーション (`applications/app.h`)
```c
const app_configuration* app_get_configuration(void);
void app_set_configuration(app_configuration *conf);
void app_ppm_start(void);
void app_adc_start(bool use_rx_tx);
```

### コマンド処理 (`comm/commands.h`)
```c
void commands_process_packet(unsigned char *data, unsigned int len, void(*reply_func)(...));
int commands_printf(const char* format, ...);
```

## コーディング規約 ([CONTRIBUTING](CONTRIBUTING) より)

### 基本スタイル
```c
// タブでインデント (スペース4幅推奨)
// 開き括弧は同一行
int main(void) {
    if (send_func) {
        send_func(data, len);
    } else {
        // 処理
    }
}

// 単一行でもブレース必須
if (condition) {
    single_statement;
}

// C99スタイルコメント
static mc_configuration mcconf; // Static to save stack space
```

### 命名規則
```c
void function_name(void);           // snake_case
#define MACRO_NAME                   // UPPER_SNAKE_CASE
typedef struct { ... } type_name_t; // snake_case_t

// ヘッダガード
#ifndef FILE_H_
#define FILE_H_
#endif
```

### 組み込み制約 (重要)
- **単精度浮動小数点のみ**: STM32F4のFPUは32bit専用。`double`は最大50倍遅い
  - `float` を使用、`sinf()`, `cosf()`, `fabsf()` 等の `f` サフィックス関数を使う
- **動的メモリ禁止**: RAM使用量をコンパイル時に確定させる
- **割り込み安全**: ISRからブロッキング・動的確保・重いログ出力を避ける
- **スタック節約**: 大きな構造体は `static` 宣言を検討
- **警告ゼロ**: コンパイル警告なしでビルドすること

## 重要な注意事項

### 変更時の確認事項
1. **全ボードビルド確認**: `make all_fw` が警告・エラーなく通ること
2. **フラッシュ操作** ([flash_helper.c](flash_helper.c)): 書込み/消去は破壊的。条件分岐を明確化
3. **設定互換性** ([conf_general.h](conf_general.h)): 既存ユーザー設定の後方互換を維持
4. **フォールトコード** ([datatypes.h](datatypes.h) `mc_fault_code`): 新規追加時は末尾に

### 外部依存 (大規模改変しない)
- `ChibiOS_3.0.5/` - RTOS
- `libcanard/` - DroneCAN
- `lispBM/` - LispBMコア

### 生成物 (編集対象外)
- `lispBM/lispBM/test_reports/**`
- `build/`, `builds/`

## ハードウェアリソース割り当て

[main.c](main.c) より:
| リソース | 用途 |
|----------|------|
| TIM1, TIM8 | mcpwm |
| TIM2 | mcpwm_foc |
| TIM3 | servo_dec / Encoder (HW_R2) / pwm_servo |
| TIM4 | WS2811/WS2812 LED / Encoder |
| TIM5 | timer |
| DMA1 Stream 2,7 | I2C1 (Nunchuk, temp) |
| DMA2 Stream 4 | ADC (mcpwm) |

## テスト

### ユニットテスト
```bash
make all_ut_run
```

定義は [make/unittest.mk](make/unittest.mk), [make/unittest-defs.mk](make/unittest-defs.mk) を参照。

### LispBMテスト
- Unit tests, CLANG scan-build, Infer静的解析, カバレッジ (gcovr)
- 詳細: [lispBM/lispBM/test_reports/README.md](lispBM/lispBM/test_reports/README.md)

## デバッグ

```bash
# OpenOCD
openocd -f stm32-bv_openocd.cfg

# GDB
arm-none-eabi-gdb build/vesc.elf
```

ChibiOSステートチェッカー: https://www.chibios.org/dokuwiki/doku.php?id=chibios:documentation:books:rt:kernel_debug#system_state_checks

## 提案時のガイドライン

1. **最小差分**: 変更は必要最小限に。コミットは論理的単位で
2. **影響範囲明記**: ファーム/テスト/設定への影響を説明
3. **ビルド確認**: `make fw BOARD=...` および `make all_fw` が通ること
4. **安全性注記**: リアルタイム性・割り込み安全性・FPU制約への影響を明記
5. **リンク付与**: ワークスペース内ファイル・シンボルへのリンクを含める
6. **`git rebase -i`**: 複数コミットは squash して整理

## 関連ドキュメント

- [README.md](README.md) - セットアップ手順
- [CHANGELOG.md](CHANGELOG.md) - バージョン履歴
- [CONTRIBUTING](CONTRIBUTING) - 貢献ガイドライン (必読)
- [VESC_ファームウェア説明書.md](VESC_ファームウェア説明書.md) - 日本語説明書
- [lispBM/README.md](lispBM/README.md) - LispBM VESC拡張リファレンス
- [documentation/comm_can.md](documentation/comm_can.md) - CAN通信仕様
