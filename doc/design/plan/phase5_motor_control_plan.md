# Phase 5: モーター制御ロジック - 詳細計画書

## 1. 概要

### 1.1 目的

Phase 5では、FOC（Field Oriented Control）モーター制御のコアアルゴリズムをPC上で実行可能にし、以下を実現する：

- **アルゴリズム検証**: オブザーバ、PID制御、SVM等のロジックの単体テスト
- **シミュレーション**: 仮想モーターを用いた閉ループ制御のシミュレーション
- **パラメータ調整**: ゲイン調整やフィルタ設計のオフライン検証
- **回帰テスト**: 制御アルゴリズム変更時の影響評価

### 1.2 対象ファイル

| ファイル | 行数 | 概要 | PC移植難易度 |
|----------|------|------|-------------|
| [motor/foc_math.c](../../motor/foc_math.c) | 767 | FOC数学関数（オブザーバ、PLL、SVM、PID） | ★☆☆ 低 |
| [motor/foc_math.h](../../motor/foc_math.h) | 263 | FOC型定義（`motor_state_t`, `motor_all_state_t`等） | ★☆☆ 低 |
| [motor/virtual_motor.c](../../motor/virtual_motor.c) | 425 | 仮想モーターモデル | ★★☆ 中 |
| [motor/virtual_motor.h](../../motor/virtual_motor.h) | 32 | 仮想モーターAPI | ★☆☆ 低 |
| [motor/mcpwm_foc.c](../../motor/mcpwm_foc.c) | 5379 | FOCメイン制御（部分的に対象） | ★★★ 高 |
| [motor/mc_interface.c](../../motor/mc_interface.c) | ~2200 | モーターインターフェース（部分的に対象） | ★★★ 高 |

### 1.3 依存関係分析

```
foc_math.c
├── foc_math.h
├── utils_math.h         ✅ Phase 2で対応済み
└── <math.h>             ✅ 標準ライブラリ

virtual_motor.c
├── virtual_motor.h
├── terminal.h           ⚠️ スタブ必要
├── mc_interface.h       ⚠️ 部分スタブ必要
├── mcpwm_foc.h          ⚠️ 部分スタブ必要
├── utils_math.h         ✅ Phase 2で対応済み
├── commands.h           ⚠️ スタブ必要
└── encoder/encoder.h    ⚠️ スタブ必要

mcpwm_foc.c (選択的対象)
├── 多数のヘッダ依存
├── TIM1/TIM2/TIM8 レジスタ操作  ⚠️ Phase 4スタブ拡張必要
├── ADC/DMA 初期化              ⚠️ スタブ必要
├── ChibiOS API                 ✅ Phase 3で対応済み
└── virtual_motor 統合          ✅ 既存機能
```

---

## 2. 既存リソースの活用

### 2.1 仮想モーター（virtual_motor）の発見

> ✅ **重要な発見**: リポジトリに既存の仮想モーター実装 [motor/virtual_motor.c](../../motor/virtual_motor.c) が存在

#### 既存機能

```c
// virtual_motor.h より
void virtual_motor_init(volatile mc_configuration *conf);
void virtual_motor_set_configuration(volatile mc_configuration *conf);
void virtual_motor_int_handler(float v_alpha, float v_beta);
bool virtual_motor_is_connected(void);
float virtual_motor_get_angle_deg(void);
```

#### 仮想モーターの動作原理

1. **ADCトリガ切断**: `ADC_ExternalTrigConvEdge_None` に設定
2. **ソフトウェア割り込み**: `mcpwm_foc_adc_int_handler()` を直接呼び出し
3. **電気モデル**: d-q軸電流を微分方程式で計算
4. **機械モデル**: トルク・慣性・角速度を計算
5. **電流注入**: 計算結果を `ADC_Value[]` に書き込み

#### PC移植への活用

仮想モーターの設計思想をPC向けシミュレーションに応用：
- ADC/DMAハードウェアをバイパスする既存パターンを踏襲
- 制御ループ（`mcpwm_foc_adc_int_handler`）を直接呼び出すアーキテクチャ

### 2.2 foc_math.c のハードウェア非依存性

```c
// foc_math.c の依存関係（全てソフトウェア実装）
#include "foc_math.h"
#include "utils_math.h"    // Phase 2で移植済み
#include <math.h>          // 標準ライブラリ
```

**結論**: `foc_math.c` は修正なしでPCビルド可能

---

## 3. アーキテクチャ設計

### 3.1 階層構造

```
┌────────────────────────────────────────────────────────────────┐
│                     テストアプリケーション                       │
│  (test_foc_simulation.c, test_foc_math.c, test_virtual_motor.c) │
└────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌────────────────────────────────────────────────────────────────┐
│                  シミュレーションドライバ                        │
│  (pc_build/motor_sim/simulation_driver.c)                      │
│  - 時間ステップ管理                                             │
│  - 入力データ読み込み（CSV/バイナリ）                            │
│  - 出力データ記録                                               │
└────────────────────────────────────────────────────────────────┘
                               │
              ┌────────────────┼────────────────┐
              ▼                ▼                ▼
┌──────────────────┐ ┌──────────────────┐ ┌──────────────────┐
│  foc_math.c      │ │ virtual_motor.c  │ │ mcpwm_foc_pc.c   │
│  (オリジナル)     │ │ (PC適応版)       │ │ (選択的移植)     │
│  - Observer      │ │ - 電気モデル     │ │ - control_current│
│  - PLL           │ │ - 機械モデル     │ │ - timer_update   │
│  - SVM           │ │ - Clark/Park変換 │ │ - hfi_update     │
│  - PID制御       │ │                  │ │                  │
└──────────────────┘ └──────────────────┘ └──────────────────┘
                               │
                               ▼
┌────────────────────────────────────────────────────────────────┐
│                     PC用スタブ/HAL層                            │
│  (Phase 3, Phase 4 で構築済み)                                  │
│  - ChibiOS POSIX互換                                           │
│  - STM32ペリフェラルスタブ                                      │
│  - ハードウェア抽象化                                           │
└────────────────────────────────────────────────────────────────┘
```

### 3.2 mcpwm_foc.c の選択的移植戦略

`mcpwm_foc.c` (5379行) は全体の移植は現実的でないため、以下の関数を選択的に対象とする：

#### 移植対象（アルゴリズム中心）

| 関数 | 行番号 | 機能 | 理由 |
|------|--------|------|------|
| `control_current()` | L4506 | 電流制御ループ | FOCコアアルゴリズム |
| `timer_update()` | L3865 | 状態更新・フィルタ | 制御ループ補助 |
| `hfi_update()` | L4147 | 高周波注入 | センサレス制御 |
| `update_valpha_vbeta()` | L4995 | 電圧計算 | 逆変換 |
| `foc_precalc_values()` | foc_math.c | 事前計算 | パラメータ初期化 |

#### 移植対象外（ハードウェア依存）

| 関数/マクロ | 理由 |
|-------------|------|
| `mcpwm_foc_init()` | TIM/ADC/DMA初期化 |
| `mcpwm_foc_adc_int_handler()` | 割り込みハンドラ（一部抽出） |
| `TIMER_UPDATE_DUTY_*` マクロ | TIMレジスタ直接操作 |
| `stop_pwm_hw()`, `start_pwm_hw()`, `full_brake_hw()` | GPIOペリフェラル操作 |

---

## 4. 実装計画

### 4.1 サブフェーズ構成

| サブフェーズ | 期間 | 内容 |
|-------------|------|------|
| 5.1 | 3日 | foc_math.c 単体テスト |
| 5.2 | 5日 | virtual_motor PC適応 |
| 5.3 | 5日 | mcpwm_foc 選択的移植 |
| 5.4 | 4日 | シミュレーションドライバ |
| 5.5 | 3日 | 統合テスト・検証 |
| **合計** | **2-3週間** | |

---

### 4.2 サブフェーズ 5.1: foc_math.c 単体テスト

#### 目的
FOC数学関数の単体テスト環境構築

#### 対象関数

```c
// foc_math.h より
void foc_observer_update(float v_alpha, float v_beta, float i_alpha, float i_beta,
        float dt, observer_state *state, float *phase, motor_all_state_t *motor);
void foc_pll_run(float phase, float dt, float *phase_var,
        float *speed_var, mc_configuration *conf);
void foc_svm(float alpha, float beta, float max_mod, uint32_t PWMFullDutyCycle,
        uint32_t* tAout, uint32_t* tBout, uint32_t* tCout, uint32_t *svm_sector);
void foc_run_pid_control_pos(bool index_found, float dt, motor_all_state_t *motor);
void foc_run_pid_control_speed(bool index_found, float dt, motor_all_state_t *motor);
float foc_correct_encoder(float obs_angle, float enc_angle, float speed, float sl_erpm, motor_all_state_t *motor);
float foc_correct_hall(float angle, float dt, motor_all_state_t *motor, int hall_val);
void foc_run_fw(motor_all_state_t *motor, float dt);
void foc_hfi_adjust_angle(float ang_err, motor_all_state_t *motor, float dt);
void foc_precalc_values(motor_all_state_t *motor);
```

#### 必要なスタブ

```c
// pc_build/motor_sim/mcconf_stub.c
// mc_configuration 構造体のデフォルト値を提供
#include "datatypes.h"

void mcconf_set_defaults(mc_configuration *conf) {
    // モーターパラメータ
    conf->foc_motor_r = 0.05f;           // 50mΩ
    conf->foc_motor_l = 0.00005f;        // 50μH
    conf->foc_motor_flux_linkage = 0.01f; // 10mWb
    conf->foc_motor_ld_lq_diff = 0.0f;
    
    // 制御パラメータ
    conf->foc_f_zv = 30000.0f;           // 30kHz
    conf->foc_observer_gain = 50e6f;
    conf->foc_pll_kp = 2000.0f;
    conf->foc_pll_ki = 30000.0f;
    
    // PIDパラメータ
    conf->s_pid_kp = 0.004f;
    conf->s_pid_ki = 0.004f;
    conf->s_pid_kd = 0.0001f;
    conf->p_pid_kp = 0.03f;
    conf->p_pid_ki = 0.0f;
    conf->p_pid_kd = 0.0004f;
    
    // 制限値
    conf->l_current_max = 100.0f;
    conf->l_max_duty = 0.95f;
    conf->l_max_erpm = 100000.0f;
    
    // その他
    conf->foc_temp_comp = false;
    conf->foc_sat_comp_mode = SAT_COMP_DISABLED;
    conf->foc_observer_type = FOC_OBSERVER_ORTEGA_ORIGINAL;
    // ... 他のフィールドも初期化
}
```

#### テストケース設計

```c
// pc_build/tests/test_foc_math.c

/*===========================================================================*/
/* SVM (Space Vector Modulation) Tests                                       */
/*===========================================================================*/

static void test_foc_svm_sector_1(void) {
    // セクター1: α > 0, β > 0, |β| < α*√3
    uint32_t tA, tB, tC, sector;
    foc_svm(0.5f, 0.1f, 0.95f, 4200, &tA, &tB, &tC, &sector);
    
    TEST_ASSERT(sector == 1, "SVM sector 1 identification");
    TEST_ASSERT(tA >= tB && tB >= tC, "SVM duty cycle ordering for sector 1");
    TEST_ASSERT(tA <= 4200, "SVM duty cycle within PWM period");
}

static void test_foc_svm_all_sectors(void) {
    // 360度を6分割して各セクターをテスト
    float angles[] = {0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f};
    uint32_t expected_sectors[] = {1, 2, 3, 4, 5, 6};
    
    for (int i = 0; i < 6; i++) {
        float rad = angles[i] * M_PI / 180.0f;
        float alpha = 0.5f * cosf(rad);
        float beta = 0.5f * sinf(rad);
        
        uint32_t tA, tB, tC, sector;
        foc_svm(alpha, beta, 0.95f, 4200, &tA, &tB, &tC, &sector);
        
        // セクター番号の検証
        TEST_ASSERT_INT_EQ(sector, expected_sectors[i], "SVM sector identification");
    }
}

/*===========================================================================*/
/* Observer Tests                                                             */
/*===========================================================================*/

static void test_foc_observer_convergence(void) {
    // オブザーバが定常状態で正しい位相に収束することを検証
    motor_all_state_t motor;
    mc_configuration conf;
    observer_state state = {0};
    
    mcconf_set_defaults(&conf);
    motor.m_conf = &conf;
    motor.m_gamma_now = conf.foc_observer_gain;
    motor.m_motor_state.i_abs_filter = 10.0f;
    motor.m_motor_state.id = 0.0f;
    motor.m_motor_state.iq = 10.0f;
    
    state.lambda_est = conf.foc_motor_flux_linkage;
    
    float dt = 1.0f / 30000.0f;  // 30kHz
    float true_phase = 0.5f;     // rad
    float estimated_phase;
    
    // 定常状態の電圧/電流を設定（簡略化）
    float v_alpha = 10.0f * cosf(true_phase);
    float v_beta = 10.0f * sinf(true_phase);
    float i_alpha = 10.0f * cosf(true_phase - 0.1f);
    float i_beta = 10.0f * sinf(true_phase - 0.1f);
    
    // 1000ステップ実行
    for (int i = 0; i < 1000; i++) {
        foc_observer_update(v_alpha, v_beta, i_alpha, i_beta,
                           dt, &state, &estimated_phase, &motor);
    }
    
    // 収束確認（許容誤差 0.1 rad）
    float phase_error = fabsf(estimated_phase - true_phase);
    if (phase_error > M_PI) phase_error = 2*M_PI - phase_error;
    
    TEST_ASSERT(phase_error < 0.1f, "Observer converges to correct phase");
}

/*===========================================================================*/
/* PLL Tests                                                                  */
/*===========================================================================*/

static void test_foc_pll_tracking(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    float dt = 1.0f / 30000.0f;
    float true_speed = 1000.0f;  // rad/s electrical
    float phase_var = 0.0f;
    float speed_var = 0.0f;
    
    // 一定速度で回転する位相を追従
    for (int i = 0; i < 10000; i++) {
        float true_phase = fmodf(true_speed * i * dt, 2*M_PI);
        foc_pll_run(true_phase, dt, &phase_var, &speed_var, &conf);
    }
    
    // 速度推定値の確認（許容誤差 5%）
    float speed_error = fabsf(speed_var - true_speed) / true_speed;
    TEST_ASSERT(speed_error < 0.05f, "PLL speed estimation accuracy");
}
```

#### Makefile追記

```makefile
# Phase 5.1: FOC math sources
FOC_MATH_SRC = \
    $(ROOT)/motor/foc_math.c

FOC_MATH_OBJS = $(BUILDDIR)/motor/foc_math.o

# Phase 5.1 test
test_foc_math: dirs $(FOC_MATH_OBJS) $(UTIL_OBJS) $(CHIBIOS_OBJS) $(STUBS_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -I$(ROOT)/motor \
		tests/test_foc_math.c \
		$(FOC_MATH_OBJS) $(UTIL_OBJS) $(CHIBIOS_OBJS) $(STUBS_OBJS) \
		motor_sim/mcconf_stub.c \
		-o $(BUILDDIR)/test_foc_math $(LIBS)
	./$(BUILDDIR)/test_foc_math
```

---

### 4.3 サブフェーズ 5.2: virtual_motor PC適応

#### 目的
仮想モーターをPCで単独実行可能にする

#### 依存関係の解決

| 依存 | 対策 |
|------|------|
| `terminal.h` (`terminal_register_command_callback`) | 空関数スタブ |
| `mc_interface.h` (`mc_interface_get_input_voltage_filtered`) | 定数を返すスタブ |
| `mcpwm_foc.h` (`mcpwm_foc_set_current`, `mcpwm_foc_get_phase` 等) | シミュレーション用スタブ |
| `commands.h` (`commands_printf`) | `printf` にリダイレクト |
| `encoder/encoder.h` | 空関数スタブ |
| `ADC_Value[]`, `ADC_InitTypeDef` | PC用定義 |
| `FAC_CURRENT`, `GET_INPUT_VOLTAGE()` マクロ | PC用定義 |

#### 新規ファイル

```c
// pc_build/motor_sim/virtual_motor_pc.h
#ifndef VIRTUAL_MOTOR_PC_H_
#define VIRTUAL_MOTOR_PC_H_

#include "datatypes.h"

// PC用仮想モーター拡張API
typedef struct {
    float v_alpha_in;      // 入力: α軸電圧
    float v_beta_in;       // 入力: β軸電圧
    float i_alpha_out;     // 出力: α軸電流
    float i_beta_out;      // 出力: β軸電流
    float angle_rad;       // 出力: 電気角 [rad]
    float omega_e;         // 出力: 電気角速度 [rad/s]
    float torque;          // 出力: 電磁トルク [Nm]
} virtual_motor_io_t;

// 初期化（PCシミュレーション用）
void virtual_motor_pc_init(mc_configuration *conf);

// 1ステップ実行（メインループから呼び出し）
void virtual_motor_pc_step(virtual_motor_io_t *io, float dt);

// パラメータ設定
void virtual_motor_pc_set_load_torque(float torque_nm);
void virtual_motor_pc_set_inertia(float J_kgm2);

// 状態取得
float virtual_motor_pc_get_rpm(void);
float virtual_motor_pc_get_id(void);
float virtual_motor_pc_get_iq(void);

#endif /* VIRTUAL_MOTOR_PC_H_ */
```

```c
// pc_build/motor_sim/virtual_motor_pc.c
#include "virtual_motor_pc.h"
#include "utils_math.h"
#include <math.h>

// 仮想モーター内部状態（virtual_motor.c を参考に）
typedef struct {
    float Ts;           // サンプリング時間
    float J;            // 慣性モーメント
    int pole_pairs;     // 極対数
    float km;           // トルク定数係数
    float ld, lq;       // d-q軸インダクタンス
    float R;            // 抵抗
    float lambda;       // 鎖交磁束
    
    float id, iq;       // d-q軸電流
    float id_int;       // id積分項
    float we;           // 電気角速度
    float phi;          // 電気角
    float sin_phi, cos_phi;
    float ml;           // 負荷トルク
} vm_state_t;

static vm_state_t vm;
static mc_configuration *m_conf;

void virtual_motor_pc_init(mc_configuration *conf) {
    m_conf = conf;
    
    vm.pole_pairs = conf->si_motor_poles / 2;
    vm.km = 1.5f * vm.pole_pairs;
    vm.Ts = 1.0f / conf->foc_f_zv;
    vm.R = conf->foc_motor_r;
    vm.lambda = conf->foc_motor_flux_linkage;
    
    if (conf->foc_motor_ld_lq_diff > 0.0f) {
        vm.lq = conf->foc_motor_l + conf->foc_motor_ld_lq_diff / 2.0f;
        vm.ld = conf->foc_motor_l - conf->foc_motor_ld_lq_diff / 2.0f;
    } else {
        vm.lq = conf->foc_motor_l;
        vm.ld = conf->foc_motor_l;
    }
    
    vm.J = 0.0001f;  // デフォルト慣性 [kg·m²]
    vm.ml = 0.0f;    // 負荷トルクなし
    
    // 状態初期化
    vm.id = 0.0f;
    vm.iq = 0.0f;
    vm.id_int = 0.0f;
    vm.we = 0.0f;
    vm.phi = 0.0f;
    vm.sin_phi = 0.0f;
    vm.cos_phi = 1.0f;
}

void virtual_motor_pc_step(virtual_motor_io_t *io, float dt) {
    vm.Ts = dt;
    
    // Park変換（αβ → dq）
    float vd = vm.cos_phi * io->v_alpha_in + vm.sin_phi * io->v_beta_in;
    float vq = vm.cos_phi * io->v_beta_in - vm.sin_phi * io->v_alpha_in;
    
    // 電気モデル（d軸）
    vm.id_int += ((vd + vm.we * vm.pole_pairs * vm.lq * vm.iq 
                   - vm.R * vm.id) * dt) / vm.ld;
    vm.id = vm.id_int - vm.lambda / vm.ld;
    
    // 電気モデル（q軸）
    vm.iq += (vq - vm.we * vm.pole_pairs * (vm.ld * vm.id + vm.lambda)
              - vm.R * vm.iq) * dt / vm.lq;
    
    // 電流制限
    utils_truncate_number_abs(&vm.id, 200.0f);
    utils_truncate_number_abs(&vm.iq, 200.0f);
    
    // トルク計算
    float me = vm.km * (vm.lambda + (vm.ld - vm.lq) * vm.id) * vm.iq;
    
    // 機械モデル
    float tsj = dt / vm.J;
    vm.we += tsj * (me - vm.ml);
    vm.phi += vm.we * dt;
    
    // 角度正規化
    while (vm.phi > M_PI) vm.phi -= 2.0f * M_PI;
    while (vm.phi < -M_PI) vm.phi += 2.0f * M_PI;
    
    // sin/cos更新
    utils_fast_sincos_better(vm.phi, &vm.sin_phi, &vm.cos_phi);
    
    // 逆Park変換（dq → αβ）
    io->i_alpha_out = vm.cos_phi * vm.id - vm.sin_phi * vm.iq;
    io->i_beta_out = vm.sin_phi * vm.id + vm.cos_phi * vm.iq;
    io->angle_rad = vm.phi;
    io->omega_e = vm.we;
    io->torque = me;
}

void virtual_motor_pc_set_load_torque(float torque_nm) {
    vm.ml = torque_nm;
}

void virtual_motor_pc_set_inertia(float J_kgm2) {
    vm.J = J_kgm2;
}

float virtual_motor_pc_get_rpm(void) {
    return vm.we * 60.0f / (2.0f * M_PI * vm.pole_pairs);
}

float virtual_motor_pc_get_id(void) { return vm.id; }
float virtual_motor_pc_get_iq(void) { return vm.iq; }
```

---

### 4.4 サブフェーズ 5.3: mcpwm_foc 選択的移植

#### 目的
制御アルゴリズムコアをPC実行可能な形で抽出

#### 抽出対象関数

```c
// pc_build/motor_sim/foc_control_core.h

#ifndef FOC_CONTROL_CORE_H_
#define FOC_CONTROL_CORE_H_

#include "foc_math.h"
#include "datatypes.h"

/**
 * 電流制御ループ（mcpwm_foc.c:control_current より抽出）
 * @param motor モーター状態構造体
 * @param dt 制御周期 [s]
 */
void foc_control_current(motor_all_state_t *motor, float dt);

/**
 * 状態更新・フィルタ処理（mcpwm_foc.c:timer_update より抽出）
 * @param motor モーター状態構造体
 * @param dt 制御周期 [s]
 */
void foc_timer_update(motor_all_state_t *motor, float dt);

/**
 * HFI処理（mcpwm_foc.c:hfi_update より抽出）
 * @param motor モーター状態構造体
 * @param dt 制御周期 [s]
 */
void foc_hfi_update(motor_all_state_t *motor, float dt);

/**
 * モーター状態初期化
 * @param motor モーター状態構造体
 * @param conf モーター設定
 */
void foc_motor_state_init(motor_all_state_t *motor, mc_configuration *conf);

#endif /* FOC_CONTROL_CORE_H_ */
```

#### 抽出方法

1. **マクロ置換**: ハードウェア依存マクロをPC用関数に置換
2. **条件コンパイル**: `#ifdef USE_PC_BUILD` で分岐
3. **コールバック化**: PWM更新をコールバック関数経由に

```c
// pc_build/motor_sim/foc_control_core.c

#include "foc_control_core.h"
#include "utils_math.h"
#include <string.h>

// PWM更新コールバック（PC上ではログ出力または無視）
static void (*pwm_update_callback)(uint32_t duty1, uint32_t duty2, uint32_t duty3) = NULL;

void foc_set_pwm_callback(void (*cb)(uint32_t, uint32_t, uint32_t)) {
    pwm_update_callback = cb;
}

// control_current() から抽出・簡略化
void foc_control_current(motor_all_state_t *motor, float dt) {
    mc_configuration *conf = motor->m_conf;
    motor_state_t *state = &motor->m_motor_state;
    
    // 電流フィードバック処理
    float id_error = motor->m_id_set - state->id;
    float iq_error = motor->m_iq_set - state->iq;
    
    // PI制御（簡略化版）
    float kp = conf->foc_current_kp;
    float ki = conf->foc_current_ki;
    
    state->vd_int += id_error * ki * dt;
    state->vq_int += iq_error * ki * dt;
    
    // 積分項制限
    utils_truncate_number(&state->vd_int, -conf->l_max_duty, conf->l_max_duty);
    utils_truncate_number(&state->vq_int, -conf->l_max_duty, conf->l_max_duty);
    
    // d-q軸電圧指令
    state->vd = kp * id_error + state->vd_int;
    state->vq = kp * iq_error + state->vq_int;
    
    // デカップリング補償
    float we = motor->m_pll_speed;
    state->vd -= we * conf->foc_motor_l * state->iq;
    state->vq += we * (conf->foc_motor_l * state->id + conf->foc_motor_flux_linkage);
    
    // 電圧制限
    float v_max = state->v_bus * conf->l_max_duty;
    utils_truncate_number(&state->vd, -v_max, v_max);
    utils_truncate_number(&state->vq, -v_max, v_max);
    
    // 逆Park変換
    state->mod_d = state->vd / state->v_bus;
    state->mod_q = state->vq / state->v_bus;
    
    float mod_alpha = state->phase_cos * state->mod_d - state->phase_sin * state->mod_q;
    float mod_beta = state->phase_sin * state->mod_d + state->phase_cos * state->mod_q;
    
    // SVM
    uint32_t duty1, duty2, duty3;
    foc_svm(mod_alpha, mod_beta, conf->l_max_duty, 4200,
            &duty1, &duty2, &duty3, &state->svm_sector);
    
    // PWM更新（コールバック経由）
    if (pwm_update_callback) {
        pwm_update_callback(duty1, duty2, duty3);
    }
}

void foc_motor_state_init(motor_all_state_t *motor, mc_configuration *conf) {
    memset(motor, 0, sizeof(motor_all_state_t));
    motor->m_conf = conf;
    motor->m_state = MC_STATE_OFF;
    motor->m_control_mode = CONTROL_MODE_NONE;
    motor->m_motor_state.v_bus = 48.0f;  // デフォルト電源電圧
    foc_precalc_values(motor);
}
```

---

### 4.5 サブフェーズ 5.4: シミュレーションドライバ

#### 目的
テスト・シミュレーション用の統合ドライバを実装

#### データ入出力形式

```c
// pc_build/motor_sim/simulation_data.h

#ifndef SIMULATION_DATA_H_
#define SIMULATION_DATA_H_

#include <stdint.h>

// シミュレーション入力データ（1サンプル）
typedef struct {
    float timestamp_s;      // タイムスタンプ [s]
    float v_bus;            // バス電圧 [V]
    float speed_ref;        // 速度指令 [RPM] または電流指令 [A]
    float load_torque;      // 負荷トルク [Nm]
} sim_input_t;

// シミュレーション出力データ（1サンプル）
typedef struct {
    float timestamp_s;      // タイムスタンプ [s]
    float rpm;              // 回転数 [RPM]
    float id;               // d軸電流 [A]
    float iq;               // q軸電流 [A]
    float vd;               // d軸電圧 [V]
    float vq;               // q軸電圧 [V]
    float phase_est;        // 推定位相 [rad]
    float phase_true;       // 真の位相 [rad]
    float torque;           // トルク [Nm]
    uint32_t svm_sector;    // SVMセクター
} sim_output_t;

// CSVファイル読み書き
int sim_load_input_csv(const char *filename, sim_input_t **data, int *count);
int sim_save_output_csv(const char *filename, sim_output_t *data, int count);

// バイナリファイル読み書き（高速）
int sim_load_input_bin(const char *filename, sim_input_t **data, int *count);
int sim_save_output_bin(const char *filename, sim_output_t *data, int count);

#endif /* SIMULATION_DATA_H_ */
```

```c
// pc_build/motor_sim/simulation_driver.h

#ifndef SIMULATION_DRIVER_H_
#define SIMULATION_DRIVER_H_

#include "datatypes.h"
#include "foc_math.h"
#include "simulation_data.h"

// シミュレーション設定
typedef struct {
    mc_configuration motor_conf;    // モーター設定
    float sim_dt;                   // シミュレーション時間刻み [s]
    float sim_duration;             // シミュレーション時間 [s]
    float initial_rpm;              // 初期回転数 [RPM]
    float inertia;                  // 慣性モーメント [kg·m²]
    bool enable_hfi;                // HFI有効化
    bool enable_observer;           // オブザーバ有効化
    int control_mode;               // 制御モード
} sim_config_t;

// シミュレーション状態
typedef struct {
    motor_all_state_t motor;        // モーター状態
    float sim_time;                 // 現在時刻
    int step_count;                 // ステップ数
    sim_output_t *output_buffer;    // 出力バッファ
    int output_count;               // 出力サンプル数
} sim_state_t;

// API
void sim_init(sim_state_t *state, sim_config_t *config);
void sim_step(sim_state_t *state, sim_input_t *input);
void sim_run(sim_state_t *state, sim_config_t *config, 
             sim_input_t *inputs, int input_count);
void sim_cleanup(sim_state_t *state);

// ユーティリティ
void sim_config_set_defaults(sim_config_t *config);
void sim_print_summary(sim_state_t *state);

#endif /* SIMULATION_DRIVER_H_ */
```

#### 使用例

```c
// pc_build/tests/test_foc_simulation.c

#include "simulation_driver.h"
#include <stdio.h>

int main(int argc, char **argv) {
    sim_config_t config;
    sim_state_t state;
    
    // 設定初期化
    sim_config_set_defaults(&config);
    config.sim_duration = 1.0f;       // 1秒シミュレーション
    config.sim_dt = 1.0f / 30000.0f;  // 30kHz制御周期
    config.initial_rpm = 0.0f;
    config.control_mode = CONTROL_MODE_SPEED;
    
    // シミュレーション初期化
    sim_init(&state, &config);
    
    // 入力データ準備（定速度指令）
    int num_steps = (int)(config.sim_duration / config.sim_dt);
    sim_input_t *inputs = malloc(num_steps * sizeof(sim_input_t));
    
    for (int i = 0; i < num_steps; i++) {
        inputs[i].timestamp_s = i * config.sim_dt;
        inputs[i].v_bus = 48.0f;
        inputs[i].speed_ref = 3000.0f;  // 3000 RPM
        inputs[i].load_torque = 0.1f;   // 0.1 Nm
    }
    
    // シミュレーション実行
    sim_run(&state, &config, inputs, num_steps);
    
    // 結果保存
    sim_save_output_csv("simulation_result.csv", 
                        state.output_buffer, state.output_count);
    
    // サマリー表示
    sim_print_summary(&state);
    
    // クリーンアップ
    free(inputs);
    sim_cleanup(&state);
    
    return 0;
}
```

---

### 4.6 サブフェーズ 5.5: 統合テスト・検証

#### テストシナリオ

| シナリオ | 内容 | 検証項目 |
|----------|------|----------|
| 起動シーケンス | 停止状態→定速度指令 | 起動時間、オーバーシュート |
| 負荷変動 | 定速度中に負荷ステップ変化 | 速度偏差、回復時間 |
| 速度変更 | ランプ状速度指令 | 追従性、電流リップル |
| センサレス低速 | 0→500RPMの加速 | オブザーバ収束、位相誤差 |
| HFI動作 | 停止・微速でのトルク制御 | HFI注入波形、角度推定 |
| 過電流保護 | 大電流指令 | 電流制限動作 |

#### 既存テストとの比較

```c
// pc_build/tests/test_regression.c
// VESC Tool のシミュレーション結果と比較

static void test_compare_with_reference(void) {
    // 参照データ読み込み（VESC Tool出力）
    sim_output_t *reference;
    int ref_count;
    sim_load_output_csv("reference/speed_step_response.csv", 
                        &reference, &ref_count);
    
    // シミュレーション実行
    sim_state_t state;
    sim_config_t config;
    sim_config_set_defaults(&config);
    sim_init(&state, &config);
    
    // ... シミュレーション実行 ...
    
    // 比較
    float max_rpm_error = 0.0f;
    float max_phase_error = 0.0f;
    
    for (int i = 0; i < state.output_count && i < ref_count; i++) {
        float rpm_err = fabsf(state.output_buffer[i].rpm - reference[i].rpm);
        if (rpm_err > max_rpm_error) max_rpm_error = rpm_err;
        
        float phase_err = fabsf(state.output_buffer[i].phase_est - reference[i].phase_est);
        if (phase_err > max_phase_error) max_phase_error = phase_err;
    }
    
    printf("Max RPM error: %.2f\n", max_rpm_error);
    printf("Max phase error: %.4f rad\n", max_phase_error);
    
    TEST_ASSERT(max_rpm_error < 100.0f, "RPM error within tolerance");
    TEST_ASSERT(max_phase_error < 0.1f, "Phase error within tolerance");
}
```

---

## 5. ディレクトリ構成（Phase 5完了時）

```
pc_build/
├── Makefile                        # Phase 5追記
├── motor_sim/                      # 新規ディレクトリ
│   ├── foc_control_core.h          # 制御コア抽出
│   ├── foc_control_core.c
│   ├── virtual_motor_pc.h          # PC用仮想モーター
│   ├── virtual_motor_pc.c
│   ├── simulation_driver.h         # シミュレーションドライバ
│   ├── simulation_driver.c
│   ├── simulation_data.h           # データ入出力
│   ├── simulation_data.c
│   ├── mcconf_stub.h               # 設定スタブ
│   └── mcconf_stub.c
│
├── tests/
│   ├── test_foc_math.c             # foc_math単体テスト
│   ├── test_virtual_motor.c        # 仮想モーターテスト
│   ├── test_foc_simulation.c       # 閉ループシミュレーション
│   └── test_regression.c           # 回帰テスト
│
├── reference/                      # 参照データ
│   ├── speed_step_response.csv
│   ├── torque_step_response.csv
│   └── hfi_operation.csv
│
└── results/                        # 出力ディレクトリ
    └── .gitkeep
```

---

## 6. Makefile追記内容

```makefile
# =============================================================================
# Phase 5: Motor control simulation
# =============================================================================

# Motor control sources (PC build)
MOTOR_SIM_SRC = \
    motor_sim/virtual_motor_pc.c \
    motor_sim/foc_control_core.c \
    motor_sim/simulation_driver.c \
    motor_sim/simulation_data.c \
    motor_sim/mcconf_stub.c

MOTOR_SIM_OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(MOTOR_SIM_SRC))

# FOC math from original source
FOC_MATH_OBJS = $(BUILDDIR)/motor/foc_math.o

# Motor includes
MOTOR_INCLUDES = -I$(ROOT)/motor -Imotor_sim

# Phase 5 directories
$(BUILDDIR)/motor_sim $(BUILDDIR)/motor $(BUILDDIR)/tests:
	mkdir -p $@

# FOC math object
$(BUILDDIR)/motor/foc_math.o: $(ROOT)/motor/foc_math.c | $(BUILDDIR)/motor
	$(CC) $(CFLAGS) $(INCLUDES) $(MOTOR_INCLUDES) -c $< -o $@

# Motor sim objects
$(BUILDDIR)/motor_sim/%.o: motor_sim/%.c | $(BUILDDIR)/motor_sim
	$(CC) $(CFLAGS) $(INCLUDES) $(MOTOR_INCLUDES) -c $< -o $@

# =============================================================================
# Phase 5 test targets
# =============================================================================

# All Phase 5 dependencies
PHASE5_DEPS = $(FOC_MATH_OBJS) $(MOTOR_SIM_OBJS) $(UTIL_OBJS) \
              $(CHIBIOS_OBJS) $(STUBS_OBJS)

.PHONY: test_foc_math test_virtual_motor test_foc_simulation test_phase5

# 5.1: FOC math unit tests
test_foc_math: dirs $(PHASE5_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(MOTOR_INCLUDES) \
		tests/test_foc_math.c $(PHASE5_DEPS) \
		-o $(BUILDDIR)/test_foc_math $(LIBS)
	./$(BUILDDIR)/test_foc_math

# 5.2: Virtual motor tests
test_virtual_motor: dirs $(PHASE5_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(MOTOR_INCLUDES) \
		tests/test_virtual_motor.c $(PHASE5_DEPS) \
		-o $(BUILDDIR)/test_virtual_motor $(LIBS)
	./$(BUILDDIR)/test_virtual_motor

# 5.4: Full simulation test
test_foc_simulation: dirs $(PHASE5_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(MOTOR_INCLUDES) \
		tests/test_foc_simulation.c $(PHASE5_DEPS) \
		-o $(BUILDDIR)/test_foc_simulation $(LIBS)
	./$(BUILDDIR)/test_foc_simulation

# All Phase 5 tests
test_phase5: test_foc_math test_virtual_motor test_foc_simulation
	@echo "=== Phase 5 tests completed ==="
```

---

## 7. リスクと対策

| リスク | 影響度 | 発生確率 | 対策 |
|--------|--------|----------|------|
| mcpwm_foc.c の複雑な依存関係 | 高 | 高 | 選択的抽出、段階的移植 |
| 浮動小数点精度の差異 | 中 | 中 | 許容誤差の設定、比較テスト |
| 制御ループのタイミング依存 | 中 | 低 | dt パラメータ化 |
| HFI/センサレス制御の複雑さ | 高 | 中 | 基本機能から順次実装 |
| mc_configuration 構造体の巨大さ | 低 | 高 | デフォルト値関数で対応 |

---

## 8. 成果物

### 8.1 ソースファイル

| ファイル | 説明 |
|----------|------|
| `motor_sim/virtual_motor_pc.c/h` | PC用仮想モーター |
| `motor_sim/foc_control_core.c/h` | 制御コア抽出 |
| `motor_sim/simulation_driver.c/h` | シミュレーションドライバ |
| `motor_sim/simulation_data.c/h` | データ入出力 |
| `motor_sim/mcconf_stub.c/h` | 設定スタブ |

### 8.2 テストファイル

| ファイル | 説明 |
|----------|------|
| `tests/test_foc_math.c` | foc_math単体テスト |
| `tests/test_virtual_motor.c` | 仮想モーターテスト |
| `tests/test_foc_simulation.c` | 閉ループシミュレーション |
| `tests/test_regression.c` | 回帰テスト |

### 8.3 実行可能ファイル

| ターゲット | コマンド | 説明 |
|------------|----------|------|
| test_foc_math | `make test_foc_math` | FOC数学関数テスト |
| test_virtual_motor | `make test_virtual_motor` | 仮想モーターテスト |
| test_foc_simulation | `make test_foc_simulation` | シミュレーション実行 |
| test_phase5 | `make test_phase5` | 全Phase 5テスト |

---

## 9. 次フェーズへの接続

Phase 5完了後、Phase 6（統合テスト）では以下を実施：

1. **アプリケーション層の統合**
   - `app_ppm.c`, `app_adc.c` 等の入力処理
   - 制御ループとの結合テスト

2. **通信層の統合**
   - `commands.c` のPC実行
   - VESC Toolプロトコルのシミュレーション

3. **エンドツーエンドテスト**
   - 入力→制御→出力の完全パイプライン
   - 実機との比較検証

---

## 付録A: mc_configuration 主要フィールド

```c
// datatypes.h より抜粋（Phase 5で必要なフィールド）

typedef struct {
    // モーターパラメータ
    float foc_motor_r;              // 抵抗 [Ω]
    float foc_motor_l;              // インダクタンス [H]
    float foc_motor_flux_linkage;   // 鎖交磁束 [Wb]
    float foc_motor_ld_lq_diff;     // Ld-Lq差 [H]
    
    // FOC制御パラメータ
    float foc_f_zv;                 // PWM周波数 [Hz]
    float foc_observer_gain;        // オブザーバゲイン
    float foc_pll_kp;               // PLLゲイン
    float foc_pll_ki;
    float foc_current_kp;           // 電流PIゲイン
    float foc_current_ki;
    
    // 速度制御
    float s_pid_kp;
    float s_pid_ki;
    float s_pid_kd;
    
    // 位置制御
    float p_pid_kp;
    float p_pid_ki;
    float p_pid_kd;
    
    // 制限値
    float l_current_max;            // 最大電流 [A]
    float l_max_duty;               // 最大デューティ
    float l_max_erpm;               // 最大ERPM
    
    // モード設定
    mc_foc_sensor_mode foc_sensor_mode;
    mc_foc_control_sample_mode foc_control_sample_mode;
    foc_observer_type foc_observer_type;
    foc_sat_comp_mode foc_sat_comp_mode;
    bool foc_temp_comp;
    
    // 機械パラメータ
    int si_motor_poles;             // 極数
    
    // HFIパラメータ
    foc_hfi_samples foc_hfi_samples;
    float foc_hfi_voltage_start;
    float foc_hfi_voltage_run;
    float foc_hfi_voltage_max;
    
    // その他多数...
} mc_configuration;
```

---

## 付録B: 参考資料

- [motor/foc_math.c](../../motor/foc_math.c) - FOC数学関数
- [motor/virtual_motor.c](../../motor/virtual_motor.c) - 既存仮想モーター
- [motor/mcpwm_foc.c](../../motor/mcpwm_foc.c) - FOCメイン制御
- [datatypes.h](../../datatypes.h) - データ型定義
- [pc_build_strategy.md](./pc_build_strategy.md) - 全体戦略
