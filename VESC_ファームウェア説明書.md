# VESC ファームウェア リポジトリ 説明書

## 概要

このリポジトリは **VESC (Vedder Electronic Speed Controller)** のオープンソースファームウェアプロジェクトです。DC/BLDC/FOCモーター制御用の完全な組み込みシステムを提供します。

**公式サイト**: [https://vesc-project.com/](https://vesc-project.com/)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Travis CI Status](https://travis-ci.com/vedderb/bldc.svg?branch=master)](https://travis-ci.com/vedderb/bldc)

---

## 主要機能

### モーター制御
- **BLDC (ブラシレスDC)** モーター制御
- **FOC (Field Oriented Control)** - 高性能ベクトル制御
- **DCモーター** 制御
- センサレス制御 / ホールセンサー対応
- エンコーダー対応（ABI、SinCos、Resolver等）

### リアルタイム制御
- **ChibiOS 3.0.5** RTOS ベース
- 高速PWM制御（最大30kHz制御ループ）
- デッドタイム補償
- 電流・電圧・温度保護

### 通信インターフェース
- UART / USB
- CAN bus (`libcanard` - DroneCAN対応)
- Bluetooth (BLE)
- I2C / SPI

---

## ディレクトリ構成

### コア制御

| ディレクトリ | 説明 |
|-------------|------|
| `motor/` | モーター制御ロジック（BLDC, FOC, DC） |
| `motor/mcpwm.c` | BLDC/DC PWM制御 |
| `motor/mcpwm_foc.c` | FOC制御実装 |
| `motor/foc_math.c` | FOC演算・PID制御 |
| `motor/mc_interface.c` | モーター制御統合API |

### ハードウェア設定

| ディレクトリ | 説明 |
|-------------|------|
| `hwconf/` | **各ESCボードの設定** (100種類以上対応) |
| `hwconf/hw.h` | 共通ハードウェア定義 |
| `hwconf/stormcore/100D/` | Stormcore 100D設定 |
| `hwconf/trampa/vesc6/` | VESC 6設定 |

### アプリケーション層

| ディレクトリ | 説明 |
|-------------|------|
| `applications/` | カスタムアプリケーション |
| `applications/app_adc.c` | ADC入力制御 (スロットル等) |
| `applications/app_balance.c` | バランス制御 (電動一輪車等) |
| `applications/app_pas.c` | ペダルアシスト (e-bike) |

### 通信

| ディレクトリ | 説明 |
|-------------|------|
| `comm/` | 通信プロトコル実装 |
| `comm/commands.c` | コマンド処理 (VESC Tool連携) |
| `comm/comm_can.c` | CAN通信 |
| `comm/comm_usb.c` | USB通信 |

### LispBM スクリプトエンジン

| ディレクトリ | 説明 |
|-------------|------|
| `lispBM/` | **組み込みLisp処理系** |
| `lispBM/README.md` | LispBM完全ドキュメント |
| `lispBM/lispif_vesc_extensions.c` | VESC拡張関数 |

**主な用途**:
- ランタイムでの制御ロジック変更
- カスタムアプリケーション開発
- 設定変更 (`conf-set`, `conf-store`)
- モーター制御API (`set-current`, `get-speed` 等)

### センサー・IMU

| ディレクトリ | 説明 |
|-------------|------|
| `imu/` | IMUセンサー統合 |
| `imu/Fusion/` | センサーフュージョンアルゴリズム |
| `imu/BMI160_driver/` | BMI160 IMUドライバ |
| `encoder/` | エンコーダードライバ群 |

### デバッグ・フラッシング

| ディレクトリ | 説明 |
|-------------|------|
| `blackmagic/` | **Black Magic Probe統合** |
| `blackmagic/README.md` | SWDプログラマ・デバッガ機能 |

対応ターゲット: STM32F0/F1/F2/F3/F4/F7, NRF51/52 等

### RTOS・HAL

| ディレクトリ | 説明 |
|-------------|------|
| `ChibiOS_3.0.5/` | **リアルタイムOS** |
| `ChibiOS_3.0.5/os/hal/` | ハードウェア抽象化層 |
| `ChibiOS_3.0.5/os/rt/` | リアルタイムカーネル |

---

## 制御パラメータ

### ランタイム変更可能

**ビルド不要**で以下のパラメータを変更可能:

| 分類 | 例 | LispBM経由 | VESC Tool経由 |
|-----|---|-----------|--------------|
| **速度PID** | Pゲイン (`s_pid_kp`) | `(conf-set 's-pid-kp 0.01)` | GUI設定画面 |
| **電流制限** | `l_current_max` | `(conf-set 'l-current-max 80.0)` | ✓ |
| **FOC設定** | `foc_f_zv` (制御周波数) | `(conf-set 'foc-f-zv 25000.0)` | ✓ |
| **BLDC PWM** | `m_bldc_f_sw_max` | `(conf-set 'm-bldc-f-sw-max 35000.0)` | ✓ |

**変更手順**:
1. LispBMで `(conf-set 'パラメータ名 値)` 実行
2. `(conf-store)` で永続化 (フラッシュ書き込み)

**C API経由**:
```c
mc_configuration mcconf;
conf_general_read_mc_configuration(&mcconf, false);
mcconf.s_pid_kp = 0.01f;
mc_interface_set_configuration(&mcconf);
conf_general_store_mc_configuration(&mcconf);
```

詳細: `lispBM/lispif_vesc_extensions.c`, `datatypes.h` の `mc_configuration`

### ビルド時定義

以下は**ソース変更が必要**:

- ハードウェアピン定義 (`hwconf/*/hw_*.h`)
- タイマ設定
- コンパイル時最適化フラグ

---

## PWM周波数設定

### ソフトウェア上限

| モード | デフォルト上限 | 定義箇所 |
|--------|---------------|----------|
| **FOC制御ループ** | **30 kHz** | `HW_LIM_FOC_CTRL_LOOP_FREQ` (hwconf/hw.h) |
| **BLDCスイッチング** | **35 kHz** | `MCCONF_M_BLDC_F_SW_MAX` (motor/mcconf_default.h) |
| **BLDC最小周波数** | **3 kHz** | `MCCONF_M_BLDC_F_SW_MIN` |
| **DC** | **25 kHz** | `MCCONF_M_DC_F_SW` |

### キャリア周波数決定フロー（完全版）

#### 1. ビルド時（コンパイル）

```
[hwconf/*/hw_*_core.h] ボード固有のデフォルト値（オプション）
  ↓
  例: #define MCCONF_FOC_F_ZV 25000.0
  例: #define MCCONF_M_BLDC_F_SW_MAX 35000

[motor/mcconf_default.h] 全体デフォルト値
  ↓
  #define MCCONF_FOC_F_ZV 30000.0       // FOC制御ループ周波数
  #define MCCONF_M_BLDC_F_SW_MIN 3000   // BLDC最小周波数
  #define MCCONF_M_BLDC_F_SW_MAX 35000  // BLDC最大周波数
  #define MCCONF_M_DC_F_SW 25000        // DC周波数

[confgenerator.c] → confgenerator_set_defaults_mcconf()
  ↓
  mc_configuration構造体にデフォルト値を設定
  conf->foc_f_zv = MCCONF_FOC_F_ZV;
  conf->m_bldc_f_sw_max = MCCONF_M_BLDC_F_SW_MAX;
  conf->m_bldc_f_sw_min = MCCONF_M_BLDC_F_SW_MIN;
  conf->m_dc_f_sw = MCCONF_M_DC_F_SW;
```

#### 2. 起動時（ファームウェア初期化）

```
[main.c] → main()
  ↓
[conf_general.c] → conf_general_init()
  ↓
  conf_general_read_mc_configuration(&mcconf, false)
    ├─ EEPROMから設定読み込み（保存済みの場合）
    └─ 失敗時はconfgenerator_set_defaults_mcconf()でデフォルト値使用
  ↓
[comm/commands.c] → 設定値の検証・切り詰め
  ↓
  HW_LIM_FOC_CTRL_LOOP_FREQによるクリッピング:
  utils_truncate_number(&mcconf->foc_f_zv, HW_LIM_FOC_CTRL_LOOP_FREQ);
  // デフォルトでは3000.0 ≤ foc_f_zv ≤ 30000.0に制限
  ↓
[mc_interface.c] → mc_interface_init()
  ↓
  mc_interface_set_configuration(&mcconf)
    ↓
    [motor/mcpwm.c] → mcpwm_init(conf) または mcpwm_set_configuration(conf)
      ↓
      // BLDC/DC用の初期設定
      switching_frequency_now = conf->m_bldc_f_sw_max;
      timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;
      
      // タイマレジスタ初期化
      TIM1->ARR = timer_tmp.top;
      TIM8->ARR = timer_tmp.top;  // デュアルモーター時
```

#### 3. 通常運転中（動的更新）

##### 3-1. デューティ変更時の自動調整

```
[外部指令] 例: mcpwm_set_duty(0.5)
  ↓
[motor/mcpwm.c] → set_duty_cycle_hl(float dutyCycle)
  ↓
  set_duty_cycle_ll(float dutyCycle)
  ↓
  set_duty_cycle_hw(float dutyCycle) ← ★周波数決定の中心★
    │
    ├─ デューティ値のクリッピング
    │   utils_truncate_number(&dutyCycle, conf->l_min_duty, conf->l_max_duty);
    │
    ├─ モーター種別による周波数決定:
    │   
    │   if (conf->motor_type == MOTOR_TYPE_DC) {
    │       // DCモーター: 固定周波数
    │       switching_frequency_now = conf->m_dc_f_sw;
    │   
    │   } else {
    │       // BLDCモーター
    │       if (IS_DETECTING() || conf->pwm_mode == PWM_MODE_BIPOLAR) {
    │           // 検出モード or バイポーラPWM: 最大周波数を使用
    │           switching_frequency_now = conf->m_bldc_f_sw_max;
    │       } else {
    │           // 通常運転: デューティに応じた線形補間
    │           switching_frequency_now = 
    │               (float)conf->m_bldc_f_sw_min * (1.0 - fabsf(dutyCycle)) +
    │               conf->m_bldc_f_sw_max * fabsf(dutyCycle);
    │           
    │           // 例: duty=0%  → f_sw_min (3kHz)
    │           //     duty=50% → (f_sw_min + f_sw_max)/2 (19kHz)
    │           //     duty=100%→ f_sw_max (35kHz)
    │       }
    │   }
    │
    ├─ タイマ周期計算:
    │   timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;
    │   
    │   SYSTEM_CORE_CLOCKは通常168MHz (STM32F4系)
    │   例: 35kHz → top = 168000000 / 35000 = 4800
    │       25kHz → top = 168000000 / 25000 = 6720
    │        3kHz → top = 168000000 / 3000  = 56000
    │
    ├─ デューティカウント計算:
    │   if (BLDC && BIPOLAR && !DETECTING) {
    │       // バイポーラPWM用（中心揃え）
    │       timer_tmp.duty = (uint16_t)(
    │           ((float)timer_tmp.top / 2.0) * dutyCycle +
    │           ((float)timer_tmp.top / 2.0)
    │       );
    │   } else {
    │       // 通常PWM
    │       timer_tmp.duty = (uint16_t)((float)timer_tmp.top * dutyCycle);
    │   }
    │
    ├─ ADCサンプリング位置更新:
    │   update_adc_sample_pos(&timer_tmp);
    │     ↓
    │     // タイマ周期に合わせてADC Injectionトリガー位置を調整
    │     // 電流サンプリングタイミングを最適化
    │
    └─ タイマレジスタへ反映:
        set_next_timer_settings(&timer_tmp);
          ↓
          utils_sys_lock_cnt();
          next_timer_settings = timer_tmp;  // グローバルバッファに保存
          timer_struct_updated = true;      // 更新フラグセット
          utils_sys_unlock_cnt();
            ↓
            [割り込みハンドラ内] update_timer_attempt()
              ↓
              // 安全なタイミング（TIMx->CNTの特定範囲内）でレジスタ更新
              if (timer_struct_updated && (TIM1->CNT > 10) && (TIM1->CNT < (timer_struct.top - 10))) {
                  TIM1->ARR = timer_struct.top;      // 周期更新
                  TIM1->CCR1 = timer_struct.duty;    // デューティ更新
                  TIM1->CCR2 = timer_struct.duty;
                  TIM1->CCR3 = timer_struct.duty;
                  // TIM8も同様（デュアルモーター時）
                  timer_struct_updated = false;
              }
```

##### 3-2. 明示的な周波数設定

```
[外部指令] 例: set_switching_frequency(25000.0)
  ↓
[motor/mcpwm.c] → set_switching_frequency(float frequency)
    ↓
    switching_frequency_now = frequency;
    timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;
    update_adc_sample_pos(&timer_tmp);
    set_next_timer_settings(&timer_tmp);

[使用箇所]
  - mcpwm_set_detect() - 検出モード開始時
  - stop_pwm_hw() - PWM停止時
  - full_brake_hw() - フルブレーキ時
  いずれも conf->m_bldc_f_sw_max を使用
```

#### 4. 設定変更時（ランタイム更新）

##### 4-1. VESC Tool経由

```
[VESC Tool GUI] → Motor Settings → Advanced → Switching Frequency
  ↓
  USB/UART経由でコマンド送信
  ↓
[comm/commands.c] → commands_process_packet()
  ↓
  COMM_SET_MCCONF:
    ├─ confgenerator_deserialize_mcconf()で設定受信
    ├─ 設定値検証・クリッピング
    │   utils_truncate_number(&mcconf->foc_f_zv, HW_LIM_FOC_CTRL_LOOP_FREQ);
    └─ mc_interface_set_configuration(&mcconf)
          ↓
          mcpwm_set_configuration(&mcconf) // 新設定で再初期化
            ↓
            switching_frequency_now = conf->m_bldc_f_sw_max;
            // 以降、set_duty_cycle_hw()で新設定使用
```

##### 4-2. LispBM経由

```
[LispBM] (conf-set 'm-bldc-f-sw-max 30000.0)
  ↓
[lispBM/lispif_vesc_extensions.c] → ext_conf_set()
  ↓
  mc_configuration mcconf;
  mc_interface_get_configuration(&mcconf);
  mcconf.m_bldc_f_sw_max = 30000.0;
  mc_interface_set_configuration(&mcconf);
  ↓
  mcpwm_set_configuration(&mcconf)
    ↓
    switching_frequency_now = conf->m_bldc_f_sw_max;

[LispBM] (conf-store)
  ↓
  conf_general_store_mc_configuration(&mcconf);
    ↓
    EEPROMへ永続保存（次回起動時も有効）
```

#### 5. FOCモードの場合

FOCモードでは [`motor/mcpwm_foc.c`](motor/mcpwm_foc.c "motor/mcpwm_foc.c") で制御ループ周波数が使用されます:

```
[motor/mcpwm_foc.c] → mcpwm_foc_init()
  ↓
  制御ループタイマー設定:
  float f_zv = conf->foc_f_zv;  // FOC制御ループ周波数 (例: 30000.0 Hz)
  
  if (conf->foc_control_sample_mode == FOC_CONTROL_SAMPLE_MODE_V0_V7) {
      // V0/V7両方でサンプリング → 周波数2倍
      f_samp = f_zv * 2.0;
  } else {
      f_samp = f_zv;
  }
  
  // TIM1/TIM8のPWM周波数設定
  TIM1->ARR = SYSTEM_CORE_CLOCK / f_samp;
  
  // 制御ループ割り込みタイマー設定
  timer_freq_sample = f_samp;
```

#### 6. 主要変数の追跡

```
グローバル変数（motor/mcpwm.c）:
  static volatile float switching_frequency_now;  // 現在のPWM周波数
  static volatile mc_timer_struct timer_struct;   // タイマ設定バッファ
  static volatile mc_configuration *conf;         // 設定ポインタ

構造体メンバ（datatypes.h - mc_configuration）:
  float foc_f_zv;              // FOC制御ループ周波数
  float m_bldc_f_sw_min;       // BLDC最小スイッチング周波数
  float m_bldc_f_sw_max;       // BLDC最大スイッチング周波数
  float m_dc_f_sw;             // DCスイッチング周波数

タイマレジスタ（STM32ハードウェア）:
  TIM1->ARR                    // Auto-Reload Register (周期)
  TIM1->CCR1/2/3               // Capture/Compare Registers (デューティ)
```

### 周波数とタイマ周期の計算式

$$
\text{timer\_top} = \frac{\text{SYSTEM\_CORE\_CLOCK}}{\text{switching\_frequency\_now}}
$$

$$
\text{PWM\_period} = \frac{\text{timer\_top}}{\text{SYSTEM\_CORE\_CLOCK}} = \frac{1}{\text{switching\_frequency\_now}}
$$

**例** (`SYSTEM_CORE_CLOCK = 168 MHz` の場合):

| 周波数 | timer_top | PWM周期 |
|--------|-----------|---------|
| 35 kHz | 4800 | 28.6 μs |
| 30 kHz | 5600 | 33.3 μs |
| 25 kHz | 6720 | 40.0 μs |
| 3 kHz | 56000 | 333.3 μs |

### 制約事項

#### ハードウェア制約

1. **タイマ分解能**: 16-bit ARR → 最大65535カウント
   - 最低周波数: $168\text{MHz} / 65535 \approx 2.56\text{ kHz}$
   - 最高周波数: $168\text{MHz} / 1 = 168\text{ MHz}$ (実用外)

2. **デッドタイム**: [`MCPWM_DEAD_TIME_CYCLES`](motor/mcpwm.c ) の確保が必要
   ```c
   #define MCPWM_DEAD_TIME_CYCLES    80  // 約476ns @ 168MHz
   ```

3. **ADCサンプリング**: 電流サンプリングに十分な時間が必要
   - [`update_adc_sample_pos()`](motor/mcpwm.c ) で調整

4. **ゲートドライバ応答速度**: ハードウェア依存（通常数百ns）

#### ソフトウェア制約

1. **整数演算による丸め誤差**:
   ```c
   timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;
   // (int)キャストで小数点以下切り捨て
   ```

2. **ゼロ除算リスク**:
   `switching_frequency_now` が0になる可能性（通常は設定検証で防止）

3. **割り込みタイミング**: [`update_timer_attempt()`](motor/mcpwm.c ) の安全領域
   ```c
   if ((TIM1->CNT > 10) && (TIM1->CNT < (timer_struct.top - 10))) {
       // この範囲内でのみレジスタ更新
   }
   ```

### no-limits ビルド

**目的**: ハードウェア制限を解除して実験的な高周波数動作を試す

**実装例**: [`hwconf/trampa/75_300_MKIV/hw_75_300_mkiv_no_limits.h`](hwconf/trampa/75_300_MKIV/hw_75_300_mkiv_no_limits.h "hwconf/trampa/75_300_MKIV/hw_75_300_mkiv_no_limits.h")
```c
#define DISABLE_HW_LIMITS
#include "hw_75_300_mkiv_core.h"
```

**効果**:
- [`comm/commands.c`](comm/commands.c "comm/commands.c") の [`HW_LIM_FOC_CTRL_LOOP_FREQ`](hwconf/teamtriforceuk/a50s_v23/hw_a50s_v23_core.h ) による切り詰めが無効化される（条件付きコンパイル）
- ユーザー設定値がそのまま適用される

**警告**: 
- ハードウェア破損のリスクあり
- ゲートドライバの過熱
- 不正確な電流サンプリング
- 推奨されない（開発・テスト用のみ）

---

## ADC処理

### 主要実装箇所

1. **アプリケーション層**: `applications/app_adc.c`
   - スロットル入力読み取り
   - フィルタリング
   - ボタン検出

2. **ハードウェア定義**: 各 `hwconf/*/hw_*_core.h`
   - ADCチャネル定義 (`ADC_IND_CURR1`, [`ADC_IND_VIN_SENS`](hwconf/vesc/duet/hw_duet_core.h ) 等)
   - マクロ (`GET_CURRENT1()`, `ADC_VOLTS()`)

3. **ChibiOS HAL**: [`ChibiOS_3.0.5/os/hal`](ChibiOS_3.0.5/os/hal "ChibiOS_3.0.5/os/hal")
   - STM32 ADCドライバ
   - DMA連携

4. **LispBM統合**:
   ```lisp
   (get-adc 0)        ; ADC電圧読み取り
   (get-adc-decoded 0) ; デコード済み値
   ```

### 用途別チャネル

| チャネル | 用途 |
|---------|------|
| `ADC_IND_CURR1/2/3` | U/V/W相電流検出 |
| [`ADC_IND_SENS1/2/3`](hwconf/vesc/duet/hw_duet_core.h ) | バックEMF検出 |
| [`ADC_IND_VIN_SENS`](hwconf/vesc/duet/hw_duet_core.h ) | バッテリ電圧 |
| [`ADC_IND_TEMP_MOS/MOTOR`](hwconf/vesc/duet/hw_duet_core.h ) | 温度センサー |
| [`ADC_IND_EXT`](hwconf/vesc/duet/hw_duet_core.h ) | 外部入力 (スロットル等) |

---

## ビルド方法

### 前提条件

**Linux/Mac**:
```bash
sudo apt install git build-essential libgl-dev libxcb-xinerama0 wget
```

**Windows**:
1. [Chocolatey](https://chocolatey.org/install) インストール
2. Powershell (管理者権限**なし**) で:
```powershell
choco install make
```

### ビルド手順

```bash
# 1. リポジトリクローン
git clone http://github.com/vedderb/bldc.git
cd bldc

# 2. ツールチェーンインストール
make arm_sdk_install

# 3. ボード指定ビルド
make <board_name>
# 例: make 100_250
# 利用可能ボード一覧: make (引数なし)

# 4. ファームウェア生成
# build/BLDC_4_<board_name>.bin が生成される
```

### VESC Toolでアップロード

1. VESC ToolでESC接続
2. **Firmware** タブ → **Custom File** → `.bin` ファイル選択
3. **Upload** クリック

### ビルドオプション

```bash
# カスタムハードウェア定義
make HW_SOURCE=hwconf/your_custom/hw_your_board.c HW_HEADER=hwconf/your_custom/hw_your_board.h

# デバッグビルド
make DEBUG=1

# 最適化レベル変更
make OPTIMIZATION=-O3
```

---

## 対応ボード (抜粋)

- **Trampa**: VESC 4, VESC 6, 75/300, 100/250
- **Flipsky**: 4.12, 4.20, 6.6, 6.7
- **Stormcore**: 60D, 100D, 100S
- **VESC Express**: ESP32ベース ([VESC Express リポジトリ](https://github.com/vedderb/vesc_express))
- **TeamTriforceUK**: A50S, A200S
- **ENNOID**: BMS, Xavier
- 他多数 (100種類以上)

完全リスト: `make` 実行時に表示

---

## 開発環境

### Qt Creator (VESC Tool開発)

```bash
# Qt インストール
make qt_install

# プロジェクトを開く
qtcreator Project/Qt\ Creator/vesc.pro
```

### デバッグ

**SWD経由** (ST-Link / J-Link):
```bash
# OpenOCD
openocd -f pi_stm32.cfg

# GDB
arm-none-eabi-gdb build/BLDC_4_xxx.elf
(gdb) target remote :3333
(gdb) load
(gdb) continue
```

**Black Magic Probe統合**: `blackmagic/README.md`

**シリアルデバッグ**:
```bash
# VESC Tool Terminal タブ経由
# または
screen /dev/ttyACM0 115200
```

---

## テスト

### ユニットテスト

```bash
make all_ut        # 全テストビルド
make all_ut_run    # 全テスト実行
make all_ut_xml    # XML出力
```

### LispBMテスト

```bash
cd lispBM/lispBM/tests
./run_tests.sh        # 32bit
./run_tests_64.sh     # 64bit
```

テストレポート: `lispBM/lispBM/test_reports/`

---

## 主要ファイル

| ファイル | 説明 |
|---------|------|
| `main.c` | メインエントリポイント |
| `conf_general.c` | 設定管理 (フラッシュ読み書き) |
| `datatypes.h` | **構造体定義** (`mc_configuration`, `app_configuration` 等) |
| `terminal.c` | シリアル端末コマンド |
| `timeout.c` | タイムアウト・安全停止 |
| `bms.c` | バッテリーマネジメントシステム |
| `Makefile` | ビルドシステム |

---

## 主要データ構造

### `mc_configuration` (datatypes.h)

モーター制御設定の中心:
```c
typedef struct {
    // PWM設定
    float m_bldc_f_sw_min;       // BLDC最小スイッチング周波数
    float m_bldc_f_sw_max;       // BLDC最大スイッチング周波数
    float m_dc_f_sw;             // DCスイッチング周波数
    
    // 電流制限
    float l_current_max;         // 最大モーター電流
    float l_current_min;         // 最大回生電流
    float l_in_current_max;      // 最大バッテリ入力電流
    
    // 速度PID
    float s_pid_kp;              // P ゲイン
    float s_pid_ki;              // I ゲイン
    float s_pid_kd;              // D ゲイン
    
    // FOC設定
    float foc_f_zv;              // FOC制御ループ周波数
    float foc_current_kp;        // 電流制御 P
    float foc_current_ki;        // 電流制御 I
    
    // ... 他多数
} mc_configuration;
```

---

## ライセンス

**GPL v3** - LICENSE

例外条項 (安定版リリースのみ): exception.txt

**注意**: 
- 商用利用時は GPL v3 準拠が必要
- VESC Projectへの貢献推奨
- カスタムファームウェアは派生著作物として GPL が適用される

---

## 貢献・サポート

- **公式フォーラム**: [VESC Project Forum](https://vesc-project.com/forum)
- **Discord**: [VESC Discord](https://discord.gg/vesc)
- **バグ報告**: [GitHub Issues](https://github.com/vedderb/bldc/issues)
- **プルリクエスト**: `CONTRIBUTING` 参照

### 貢献ガイドライン

1. フォーク & ブランチ作成
2. コードスタイル準拠（インデント: タブ、命名規則: snake_case）
3. テスト追加
4. プルリクエスト作成（説明明記）

---

## 変更履歴

主要バージョン: `CHANGELOG.md`

- **7.00**: 最新機能
- **6.06**: LispBM統合強化
- **6.00**: FOC改善
- **5.03**: CAN拡張
- **4.00**: VESC Tool v2対応

---

## トラブルシューティング

### ビルドエラー

**エラー**: `arm-none-eabi-gcc: command not found`
```bash
# ツールチェーン再インストール
make arm_sdk_install
```

**エラー**: `hwconf/hw_xxx.h: No such file`
```bash
# ボード名確認
make
# 正しいボード名で再ビルド
make <correct_board_name>
```

### ファームウェアアップロード失敗

1. USB接続確認
2. ブートローダーモード確認（一部ボードはボタン長押し必要）
3. VESC Tool バージョン確認（最新版推奨）

### 動作異常

- DC Cal 再実行（VESC Tool → Motor Settings → Run DC Cal）
- 設定リセット（VESC Tool → Load Default Configuration）
- フォーラムで相談（ログ・設定ファイル添付）

---

## 参考リンク

- [VESC Project公式](https://vesc-project.com/)
- [VESC Tool](https://vesc-project.com/vesc_tool)
- [LispBM言語リファレンス](https://github.com/svenssonjoel/lispBM)
- [ハードウェア設計資料](https://vesc-project.com/Hardware)
- [ChibiOS ドキュメント](http://www.chibios.org/dokuwiki/doku.php)
- [FOC理論解説](https://vesc-project.com/node/197)

---

## 付録: よくある質問 (FAQ)

**Q: ビルドせずにPIDゲインを変更できますか？**  
A: はい。VESC Tool GUI または LispBM `(conf-set 's-pid-kp 値)` で可能。

**Q: PWM周波数の上限は？**  
A: ボード依存。FOC通常30kHz、BLDC 35kHz。ハードウェア制約に注意。

**Q: カスタムアプリケーションの作り方は？**  
A: `applications/app_custom_template.c` をコピーして修正。または LispBM スクリプトで実装。

**Q: 複数VESCのCAN同期は？**  
A: VESC Tool → App Settings → General → Multiple ESCs over CAN で設定。

**Q: バッテリ電圧カットオフ設定は？**  
A: Motor Settings → General → Battery Cutoff Start/End で設定。

---

**最終更新**: 2025年12月20日  
**対応ファームウェアバージョン**: 7.00系列  
**ドキュメント作成**: GitHub Copilot
 
---

## リポジトリ統計 (コード行数)

以下はワークスペース内のソース集計結果です（`*.c` と `*.h` を含む）。

- `.c` ファイル数: 493
- `.c` 総行数: 240,105
- `.h` ファイル数: 881
- `.h` 総行数: 277,995
- 合計ファイル数 (`.c` + `.h`): 1,374
- 合計行数 (`.c` + `.h`): 518,100

集計日: 2025年12月20日

注: 空行やコメント行を除外していない総行数です。コメント/空行を除外した実効行数が必要な場合は指示してください。
# VESC ファームウェア リポジトリ 説明書

## 概要

このリポジトリは **VESC (Vedder Electronic Speed Controller)** のオープンソースファームウェアプロジェクトです。DC/BLDC/FOCモーター制御用の完全な組み込みシステムを提供します。

**公式サイト**: [https://vesc-project.com/](https://vesc-project.com/)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Travis CI Status](https://travis-ci.com/vedderb/bldc.svg?branch=master)](https://travis-ci.com/vedderb/bldc)

---

## 主要機能

### モーター制御
- **BLDC (ブラシレスDC)** モーター制御
- **FOC (Field Oriented Control)** - 高性能ベクトル制御
- **DCモーター** 制御
- センサレス制御 / ホールセンサー対応
- エンコーダー対応（ABI、SinCos、Resolver等）

### リアルタイム制御
- **ChibiOS 3.0.5** RTOS ベース
- 高速PWM制御（最大30kHz制御ループ）
- デッドタイム補償
- 電流・電圧・温度保護

### 通信インターフェース
- UART / USB
- CAN bus (`libcanard` - DroneCAN対応)
- Bluetooth (BLE)
- I2C / SPI

---

## ディレクトリ構成

### コア制御

| ディレクトリ | 説明 |
|-------------|------|
| `motor/` | モーター制御ロジック（BLDC, FOC, DC） |
| `motor/mcpwm.c` | BLDC/DC PWM制御 |
| `motor/mcpwm_foc.c` | FOC制御実装 |
| `motor/foc_math.c` | FOC演算・PID制御 |
| `motor/mc_interface.c` | モーター制御統合API |

### ハードウェア設定

| ディレクトリ | 説明 |
|-------------|------|
| `hwconf/` | **各ESCボードの設定** (100種類以上対応) |
| `hwconf/hw.h` | 共通ハードウェア定義 |
| `hwconf/stormcore/100D/` | Stormcore 100D設定 |
| `hwconf/trampa/vesc6/` | VESC 6設定 |

### アプリケーション層

| ディレクトリ | 説明 |
|-------------|------|
| `applications/` | カスタムアプリケーション |
| `applications/app_adc.c` | ADC入力制御 (スロットル等) |
| `applications/app_balance.c` | バランス制御 (電動一輪車等) |
| `applications/app_pas.c` | ペダルアシスト (e-bike) |

### 通信

| ディレクトリ | 説明 |
|-------------|------|
| `comm/` | 通信プロトコル実装 |
| `comm/commands.c` | コマンド処理 (VESC Tool連携) |
| `comm/comm_can.c` | CAN通信 |
| `comm/comm_usb.c` | USB通信 |

### LispBM スクリプトエンジン

| ディレクトリ | 説明 |
|-------------|------|
| `lispBM/` | **組み込みLisp処理系** |
| `lispBM/README.md` | LispBM完全ドキュメント |
| `lispBM/lispif_vesc_extensions.c` | VESC拡張関数 |

**主な用途**:
- ランタイムでの制御ロジック変更
- カスタムアプリケーション開発
- 設定変更 (`conf-set`, `conf-store`)
- モーター制御API (`set-current`, `get-speed` 等)

### センサー・IMU

| ディレクトリ | 説明 |
|-------------|------|
| `imu/` | IMUセンサー統合 |
| `imu/Fusion/` | センサーフュージョンアルゴリズム |
| `imu/BMI160_driver/` | BMI160 IMUドライバ |
| `encoder/` | エンコーダードライバ群 |

### デバッグ・フラッシング

| ディレクトリ | 説明 |
|-------------|------|
| `blackmagic/` | **Black Magic Probe統合** |
| `blackmagic/README.md` | SWDプログラマ・デバッガ機能 |

対応ターゲット: STM32F0/F1/F2/F3/F4/F7, NRF51/52 等

### RTOS・HAL

| ディレクトリ | 説明 |
|-------------|------|
| `ChibiOS_3.0.5/` | **リアルタイムOS** |
| `ChibiOS_3.0.5/os/hal/` | ハードウェア抽象化層 |
| `ChibiOS_3.0.5/os/rt/` | リアルタイムカーネル |

---

## 制御パラメータ

### ランタイム変更可能

**ビルド不要**で以下のパラメータを変更可能:

| 分類 | 例 | LispBM経由 | VESC Tool経由 |
|-----|---|-----------|--------------|
| **速度PID** | Pゲイン (`s_pid_kp`) | `(conf-set 's-pid-kp 0.01)` | GUI設定画面 |
| **電流制限** | `l_current_max` | `(conf-set 'l-current-max 80.0)` | ✓ |
| **FOC設定** | `foc_f_zv` (制御周波数) | `(conf-set 'foc-f-zv 25000.0)` | ✓ |
| **BLDC PWM** | `m_bldc_f_sw_max` | `(conf-set 'm-bldc-f-sw-max 35000.0)` | ✓ |

**変更手順**:
1. LispBMで `(conf-set 'パラメータ名 値)` 実行
2. `(conf-store)` で永続化 (フラッシュ書き込み)

**C API経由**:
```c
mc_configuration mcconf;
conf_general_read_mc_configuration(&mcconf, false);
mcconf.s_pid_kp = 0.01f;
mc_interface_set_configuration(&mcconf);
conf_general_store_mc_configuration(&mcconf);
```

詳細: `lispBM/lispif_vesc_extensions.c`, `datatypes.h` の `mc_configuration`

### ビルド時定義

以下は**ソース変更が必要**:

- ハードウェアピン定義 (`hwconf/*/hw_*.h`)
- タイマ設定
- コンパイル時最適化フラグ

---

## PWM周波数設定

### ソフトウェア上限

| モード | デフォルト上限 | 定義箇所 |
|--------|---------------|----------|
| **FOC制御ループ** | **30 kHz** | `HW_LIM_FOC_CTRL_LOOP_FREQ` (hwconf/hw.h) |
| **BLDCスイッチング** | **35 kHz** | `MCCONF_M_BLDC_F_SW_MAX` (motor/mcconf_default.h) |
| **BLDC最小周波数** | **3 kHz** | `MCCONF_M_BLDC_F_SW_MIN` |
| **DC** | **25 kHz** | `MCCONF_M_DC_F_SW` |

### 実装詳細

**周波数決定フロー**:
```
起動時:
  mcpwm_init()
    → switching_frequency_now = conf->m_bldc_f_sw_max

通常運転時:
  mcpwm_set_duty()
    → set_duty_cycle_hl()
      → set_duty_cycle_ll()
        → set_duty_cycle_hw() ← switching_frequency_now を計算
          → update_adc_sample_pos()
          → set_next_timer_settings()
            → update_timer_attempt() ← TIM1/TIM8 ARR/CCR 更新

明示的変更:
  set_switching_frequency(float frequency)
    → switching_frequency_now = frequency
    → timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now
```

**計算式** (`motor/mcpwm.c` - `set_duty_cycle_hw()`):
```c
if (conf->motor_type == MOTOR_TYPE_DC) {
    switching_frequency_now = conf->m_dc_f_sw;
} else {
    if (IS_DETECTING() || conf->pwm_mode == PWM_MODE_BIPOLAR) {
        switching_frequency_now = conf->m_bldc_f_sw_max;
    } else {
        // デューティに応じた補間
        switching_frequency_now = (float)conf->m_bldc_f_sw_min * (1.0 - fabsf(dutyCycle)) +
                                  conf->m_bldc_f_sw_max * fabsf(dutyCycle);
    }
}

timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;
```

**周波数とタイマ周期の関係**:
$$
\text{timer\_top} = \frac{\text{SYSTEM\_CORE\_CLOCK}}{\text{switching\_frequency\_now}}
$$

通常 `SYSTEM_CORE_CLOCK = 168 MHz` (STM32F4系) の場合:
- 35 kHz → top ≈ 4800
- 25 kHz → top = 6720
- 3 kHz → top = 56000

**制約**:
- タイマ分解能 (16-bit ARR: 最大65535)
- デッドタイム確保 (`MCPWM_DEAD_TIME_CYCLES`)
- ADCサンプリングタイミング (`update_adc_sample_pos()`)
- ゲートドライバ応答速度

**no-limits ビルド**: `DISABLE_HW_LIMITS` 定義で制限解除可能
例: `hwconf/trampa/75_300_MKIV/hw_75_300_mkiv_no_limits.h`

### 設定フロー（初期化からランタイムまで）

```
1. コンパイル時:
   motor/mcconf_default.h → MCCONF_M_BLDC_F_SW_MAX = 35000

2. 起動時（デフォルト設定）:
   confgenerator_set_defaults_mcconf(&mcconf)
     → mcconf.m_bldc_f_sw_max = MCCONF_M_BLDC_F_SW_MAX

3. EEPROM読み込み:
   conf_general_read_mc_configuration(&mcconf)
     → フラッシュから設定読み込み（または上記デフォルト使用）

4. ランタイム適用:
   mc_interface_set_configuration(&mcconf)
     → mcpwm_init(&mcconf) または mcpwm_set_configuration(&mcconf)
       → switching_frequency_now = conf->m_bldc_f_sw_max

5. 運転中更新:
   set_duty_cycle_hw() または set_switching_frequency()
     → switching_frequency_now を再計算/設定
     → timer_tmp.top 更新 → TIMレジスタ反映
```

**重要**: `motor/mcpwm.c` には `switching_frequency_now` に対する明示的なランタイムクリッピング（範囲チェック）はありません。設定値の妥当性は `conf` レベルで保証される必要があります。

---

## ADC処理

### 主要実装箇所

1. **アプリケーション層**: `applications/app_adc.c`
   - スロットル入力読み取り
   - フィルタリング
   - ボタン検出

2. **ハードウェア定義**: 各 `hwconf/*/hw_*_core.h`
   - ADCチャネル定義 (`ADC_IND_CURR1`, `ADC_IND_VIN_SENS` 等)
   - マクロ (`GET_CURRENT1()`, `ADC_VOLTS()`)

3. **制御系**: `motor/mcpwm.c`, `motor/mcpwm_foc.c`
   - ADC Injectionトリガー設定 (`update_adc_sample_pos()`)
   - 割り込みハンドラ (`mcpwm_adc_inj_int_handler()`)
   - 電流サンプリング

4. **ChibiOS HAL**: `ChibiOS_3.0.5/os/hal/`
   - STM32 ADCドライバ
   - DMA連携

5. **LispBM統合**:
   ```lisp
   (get-adc 0)        ; ADC電圧読み取り
   (get-adc-decoded 0) ; デコード済み値
   ```

### 用途別チャネル

| チャネル | 用途 |
|---------|------|
| `ADC_IND_CURR1/2/3` | U/V/W相電流検出 |
| `ADC_IND_SENS1/2/3` | バックEMF検出 |
| `ADC_IND_VIN_SENS` | バッテリ電圧 |
| `ADC_IND_TEMP_MOS/MOTOR` | 温度センサー |
| `ADC_IND_EXT` | 外部入力 (スロットル等) |

### DCキャリブレーション

**目的**: 電流センサーのオフセット補正

**実装** (`motor/mcpwm.c`):
```c
void mcpwm_dccal(void) {
    // 1000サンプル平均でオフセット測定
    for (int i = 0; i < 1000; i++) {
        curr0_sum += GET_CURRENT1();
        curr1_sum += GET_CURRENT2();
        chThdSleepMilliseconds(1);
    }
    curr0_offset = curr0_sum / 1000;
    curr1_offset = curr1_sum / 1000;
}
```

起動時自動実行（全相OFF状態で測定）

---

## ビルド方法

### 前提条件

**Linux/Mac**:
```bash
sudo apt install git build-essential libgl-dev libxcb-xinerama0 wget
```

**Windows**:
1. [Chocolatey](https://chocolatey.org/install) インストール
2. Powershell (管理者権限**なし**) で:
```powershell
choco install make
```

### ビルド手順

```bash
# 1. リポジトリクローン
git clone http://github.com/vedderb/bldc.git
cd bldc

# 2. ツールチェーンインストール
make arm_sdk_install

# 3. ボード指定ビルド
make <board_name>
# 例: make 100_250
# 利用可能ボード一覧: make (引数なし)

# 4. ファームウェア生成
# build/BLDC_4_<board_name>.bin が生成される
```

### VESC Toolでアップロード

1. VESC ToolでESC接続
2. **Firmware** タブ → **Custom File** → `.bin` ファイル選択
3. **Upload** クリック

### ビルドオプション

```bash
# カスタムハードウェア定義
make HW_SOURCE=hwconf/your_custom/hw_your_board.c HW_HEADER=hwconf/your_custom/hw_your_board.h

# デバッグビルド
make DEBUG=1

# 最適化レベル変更
make OPTIMIZATION=-O3
```

---

## 対応ボード (抜粋)

- **Trampa**: VESC 4, VESC 6, 75/300, 100/250
- **Flipsky**: 4.12, 4.20, 6.6, 6.7
- **Stormcore**: 60D, 100D, 100S
- **VESC Express**: ESP32ベース ([VESC Express リポジトリ](https://github.com/vedderb/vesc_express))
- **TeamTriforceUK**: A50S, A200S
- **ENNOID**: BMS, Xavier
- 他多数 (100種類以上)

完全リスト: `make` 実行時に表示

---

## 開発環境

### Qt Creator (VESC Tool開発)

```bash
# Qt インストール
make qt_install

# プロジェクトを開く
qtcreator Project/Qt\ Creator/vesc.pro
```

### デバッグ

**SWD経由** (ST-Link / J-Link):
```bash
# OpenOCD
openocd -f pi_stm32.cfg

# GDB
arm-none-eabi-gdb build/BLDC_4_xxx.elf
(gdb) target remote :3333
(gdb) load
(gdb) continue
```

**Black Magic Probe統合**: `blackmagic/README.md`

**シリアルデバッグ**:
```bash
# VESC Tool Terminal タブ経由
# または
screen /dev/ttyACM0 115200
```

---

## テスト

### ユニットテスト

```bash
make all_ut        # 全テストビルド
make all_ut_run    # 全テスト実行
make all_ut_xml    # XML出力
```

### LispBMテスト

```bash
cd lispBM/lispBM/tests
./run_tests.sh        # 32bit
./run_tests_64.sh     # 64bit
```

テストレポート: `lispBM/lispBM/test_reports/`

---

## 主要ファイル

| ファイル | 説明 |
|---------|------|
| `main.c` | メインエントリポイント |
| `conf_general.c` | 設定管理 (フラッシュ読み書き) |
| `datatypes.h` | **構造体定義** (`mc_configuration`, `app_configuration` 等) |
| `terminal.c` | シリアル端末コマンド |
| `timeout.c` | タイムアウト・安全停止 |
| `bms.c` | バッテリーマネジメントシステム |
| `Makefile` | ビルドシステム |

---

## 主要データ構造

### `mc_configuration` (datatypes.h)

モーター制御設定の中心:
```c
typedef struct {
    // PWM設定
    float m_bldc_f_sw_min;       // BLDC最小スイッチング周波数
    float m_bldc_f_sw_max;       // BLDC最大スイッチング周波数
    float m_dc_f_sw;             // DCスイッチング周波数
    
    // 電流制限
    float l_current_max;         // 最大モーター電流
    float l_current_min;         // 最大回生電流
    float l_in_current_max;      // 最大バッテリ入力電流
    
    // 速度PID
    float s_pid_kp;              // P ゲイン
    float s_pid_ki;              // I ゲイン
    float s_pid_kd;              // D ゲイン
    
    // FOC設定
    float foc_f_zv;              // FOC制御ループ周波数
    float foc_current_kp;        // 電流制御 P
    float foc_current_ki;        // 電流制御 I
    
    // ... 他多数
} mc_configuration;
```

---

## ライセンス

**GPL v3** - LICENSE

例外条項 (安定版リリースのみ): exception.txt

**注意**: 
- 商用利用時は GPL v3 準拠が必要
- VESC Projectへの貢献推奨
- カスタムファームウェアは派生著作物として GPL が適用される

---

## 貢献・サポート

- **公式フォーラム**: [VESC Project Forum](https://vesc-project.com/forum)
- **Discord**: [VESC Discord](https://discord.gg/vesc)
- **バグ報告**: [GitHub Issues](https://github.com/vedderb/bldc/issues)
- **プルリクエスト**: `CONTRIBUTING` 参照

### 貢献ガイドライン

1. フォーク & ブランチ作成
2. コードスタイル準拠（インデント: タブ、命名規則: snake_case）
3. テスト追加
4. プルリクエスト作成（説明明記）

---

## 変更履歴

主要バージョン: `CHANGELOG.md`

- **7.00**: 最新機能
- **6.06**: LispBM統合強化
- **6.00**: FOC改善
- **5.03**: CAN拡張
- **4.00**: VESC Tool v2対応

---

## トラブルシューティング

### ビルドエラー

**エラー**: `arm-none-eabi-gcc: command not found`
```bash
# ツールチェーン再インストール
make arm_sdk_install
```

**エラー**: `hwconf/hw_xxx.h: No such file`
```bash
# ボード名確認
make
# 正しいボード名で再ビルド
make <correct_board_name>
```

### ファームウェアアップロード失敗

1. USB接続確認
2. ブートローダーモード確認（一部ボードはボタン長押し必要）
3. VESC Tool バージョン確認（最新版推奨）

### 動作異常

- DC Cal 再実行（VESC Tool → Motor Settings → Run DC Cal）
- 設定リセット（VESC Tool → Load Default Configuration）
- フォーラムで相談（ログ・設定ファイル添付）

---

## 参考リンク

- [VESC Project公式](https://vesc-project.com/)
- [VESC Tool](https://vesc-project.com/vesc_tool)
- [LispBM言語リファレンス](https://github.com/svenssonjoel/lispBM)
- [ハードウェア設計資料](https://vesc-project.com/Hardware)
- [ChibiOS ドキュメント](http://www.chibios.org/dokuwiki/doku.php)
- [FOC理論解説](https://vesc-project.com/node/197)

---

## 付録: よくある質問 (FAQ)

**Q: ビルドせずにPIDゲインを変更できますか？**  
A: はい。VESC Tool GUI または LispBM `(conf-set 's-pid-kp 値)` で可能。

**Q: PWM周波数の上限は？**  
A: ボード依存。FOC通常30kHz、BLDC 35kHz。ハードウェア制約に注意。

**Q: カスタムアプリケーションの作り方は？**  
A: `applications/app_custom_template.c` をコピーして修正。または LispBM スクリプトで実装。

**Q: 複数VESCのCAN同期は？**  
A: VESC Tool → App Settings → General → Multiple ESCs over CAN で設定。

**Q: バッテリ電圧カットオフ設定は？**  
A: Motor Settings → General → Battery Cutoff Start/End で設定。

---

### キャリア周波数を 2 kHz に下げた際の影響評価

**要約**
- 2 kHz は現行のソフト／ハード保護（多くのボードで最小 3 kHz）より低く、そのまま適用すると動作不可または性能劣化のリスクが高いです。

**技術的根拠（要点）**
- `timer_tmp.top = SYSTEM_CORE_CLOCK / (int)switching_frequency_now;` の実装により、`SYSTEM_CORE_CLOCK = 168MHz` の場合 2 kHz → top ≈ 84,000 (>65535)。STM32 の 16-bit ARR に収まらず不正動作になります。
- 起動時・設定受信時に `utils_truncate_number` で 3 kHz 未満に切り詰められる実装箇所が存在します（`HW_LIM_FOC_CTRL_LOOP_FREQ` など）。

**予想される影響**
- 動作不可: ARR オーバーフローにより PWM 周期が不正、モータが動作しない／異常動作の可能性。
- 制御性能低下: 電流リップルとトルクリップル増大、速度/トルク応答悪化。
- 検出・サンプリング不具合: ADC サンプリング位置や HFI/センサレス検出精度の劣化。
- 音・振動: 低周波スイッチングにより可聴ノイズと振動が増える。

**必要な実装変更（概略）**
1. `motor/mcpwm.c` の `timer_tmp.top` 算出を PSC（プリスケーラ）対応へ変更し、ARR<=65535 を満たすよう PSC を自動選定して書き込む。`timer_struct` に `psc` を追加、`update_timer_attempt()` 側で PSC レジスタ更新を行う必要があります。
2. `comm/commands.c` / `hwconf/*` の最小周波数制限（`HW_LIM_FOC_CTRL_LOOP_FREQ` や mcconf の下限）を 2 kHz を許可するよう修正（ただし安全確認後に行う）。
3. `update_adc_sample_pos()` と ADC トリガ調整ロジックを PSC 変動下でも正しく動作するよう見直す。
4. FOC/HFI パラメータの再チューニングと実機試験（温度・電流波形・騒音など）。

**推奨手順**
1. まず `motor/mcpwm.c` に PSC 自動算出を実装して ARR オーバーフロー問題を解消する。単純な ARR 計算のままでは 2 kHz サポートは不可。
2. PSC 実装後に `hwconf` と `comm` の下限チェックを緩和し、テスト用に 2 kHz を許可する。
3. 実機で ADC 波形、電流リップル、温度、ノイズを確認し、FOC/HFI の動作を評価する。

**結論**
- 単純に設定値を 2 kHz に下げるだけでは動作しない可能性が高い。安全に対応するにはタイマ PSC 処理の追加と ADC／FOC 関連の見直しが必須です。

**最終更新**: 2025年12月20日  
**対応ファームウェアバージョン**: 7.00系列  
**ドキュメント作成**: GitHub Copilot
