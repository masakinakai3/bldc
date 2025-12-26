/**
 * @file foc_control_core.c
 * @brief Extracted FOC control core functions implementation
 * 
 * Implementation based on mcpwm_foc.c control functions,
 * adapted for PC simulation without hardware dependencies.
 */

#include "foc_control_core.h"
#include "utils_math.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// PWM callback
static foc_pwm_callback_t pwm_callback = NULL;

// Constants
#define ONE_BY_SQRT3        (0.57735026919f)
#define TWO_BY_SQRT3        (1.15470053838f)
#define SQRT3_BY_2          (0.86602540378f)

void foc_set_pwm_callback(foc_pwm_callback_t cb) {
    pwm_callback = cb;
}

void foc_motor_state_init(motor_all_state_t *motor, mc_configuration *conf) {
    if (motor == NULL || conf == NULL) return;
    
    memset(motor, 0, sizeof(motor_all_state_t));
    motor->m_conf = conf;
    motor->m_state = MC_STATE_OFF;
    motor->m_control_mode = CONTROL_MODE_NONE;
    
    // Set default bus voltage
    motor->m_motor_state.v_bus = 48.0f;
    
    // Initialize observer state
    motor->m_observer_state.lambda_est = conf->foc_motor_flux_linkage;
    
    // Set gamma (observer gain)
    motor->m_gamma_now = conf->foc_observer_gain;
    
    // Pre-calculate values
    foc_precalc_values(motor);
}

void foc_motor_state_reset(motor_all_state_t *motor) {
    if (motor == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    
    // Reset state variables but keep configuration
    motor->m_state = MC_STATE_OFF;
    motor->m_control_mode = CONTROL_MODE_NONE;
    
    // Reset motor state
    memset(&motor->m_motor_state, 0, sizeof(motor_state_t));
    motor->m_motor_state.v_bus = 48.0f;
    motor->m_motor_state.phase_cos = 1.0f;
    
    // Reset observer
    memset(&motor->m_observer_state, 0, sizeof(observer_state));
    motor->m_observer_state.lambda_est = conf->foc_motor_flux_linkage;
    
    // Reset PLL
    motor->m_pll_phase = 0.0f;
    motor->m_pll_speed = 0.0f;
    
    // Reset speed estimates
    motor->m_speed_est_fast = 0.0f;
    motor->m_speed_est_fast_corrected = 0.0f;
    motor->m_speed_est_faster = 0.0f;
    
    // Reset PID states
    motor->m_speed_i_term = 0.0f;
    motor->m_speed_prev_error = 0.0f;
    motor->m_pos_i_term = 0.0f;
    motor->m_pos_prev_error = 0.0f;
    
    // Reset integrators
    motor->m_motor_state.vd_int = 0.0f;
    motor->m_motor_state.vq_int = 0.0f;
    
    // Reset targets
    motor->m_id_set = 0.0f;
    motor->m_iq_set = 0.0f;
    motor->m_duty_cycle_set = 0.0f;
    motor->m_speed_pid_set_rpm = 0.0f;
    motor->m_pos_pid_set = 0.0f;
}

void foc_update_measurements(motor_all_state_t *motor, 
                             float i_alpha, float i_beta, float v_bus) {
    if (motor == NULL) return;
    
    motor_state_t *state = &motor->m_motor_state;
    
    state->i_alpha = i_alpha;
    state->i_beta = i_beta;
    state->v_bus = v_bus;
    
    // Calculate current magnitude
    state->i_abs = sqrtf(i_alpha * i_alpha + i_beta * i_beta);
    
    // Low-pass filter for i_abs
    float filter_const = motor->m_conf->foc_current_filter_const;
    UTILS_LP_FAST(state->i_abs_filter, state->i_abs, filter_const);
    
    // Park transform to get d-q currents
    state->id = state->phase_cos * i_alpha + state->phase_sin * i_beta;
    state->iq = state->phase_cos * i_beta - state->phase_sin * i_alpha;
    
    // Low-pass filter for id, iq
    UTILS_LP_FAST(state->id_filter, state->id, filter_const);
    UTILS_LP_FAST(state->iq_filter, state->iq, filter_const);
}

void foc_control_current(motor_all_state_t *motor, float dt) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    motor_state_t *state = &motor->m_motor_state;
    
    float id_target = motor->m_id_set;
    float iq_target = motor->m_iq_set;
    
    // Current errors
    float id_error = id_target - state->id;
    float iq_error = iq_target - state->iq;
    
    // PI controller gains
    float kp = conf->foc_current_kp;
    float ki = conf->foc_current_ki;
    
    // Update integrators
    state->vd_int += id_error * ki * dt;
    state->vq_int += iq_error * ki * dt;
    
    // Anti-windup: limit integrators
    float max_duty = conf->l_max_duty;
    utils_truncate_number(&state->vd_int, -max_duty, max_duty);
    utils_truncate_number(&state->vq_int, -max_duty, max_duty);
    
    // Calculate voltage commands (normalized to bus voltage)
    state->mod_d = kp * id_error + state->vd_int;
    state->mod_q = kp * iq_error + state->vq_int;
    
    // Decoupling compensation
    if (conf->foc_cc_decoupling != FOC_CC_DECOUPLING_DISABLED) {
        float we = motor->m_pll_speed;  // rad/s electrical
        float mod_d_comp = 0.0f;
        float mod_q_comp = 0.0f;
        
        if (conf->foc_cc_decoupling == FOC_CC_DECOUPLING_CROSS ||
            conf->foc_cc_decoupling == FOC_CC_DECOUPLING_CROSS_BEMF) {
            // Cross-coupling compensation
            mod_d_comp = -we * conf->foc_motor_l * state->iq / state->v_bus;
            mod_q_comp = we * conf->foc_motor_l * state->id / state->v_bus;
        }
        
        if (conf->foc_cc_decoupling == FOC_CC_DECOUPLING_BEMF ||
            conf->foc_cc_decoupling == FOC_CC_DECOUPLING_CROSS_BEMF) {
            // Back-EMF compensation
            mod_q_comp += we * conf->foc_motor_flux_linkage / state->v_bus;
        }
        
        state->mod_d += mod_d_comp;
        state->mod_q += mod_q_comp;
    }
    
    // Limit modulation
    utils_truncate_number(&state->mod_d, -max_duty, max_duty);
    utils_truncate_number(&state->mod_q, -max_duty, max_duty);
    
    // Calculate actual voltage commands
    state->vd = state->mod_d * state->v_bus;
    state->vq = state->mod_q * state->v_bus;
    
    // Inverse Park transform
    float mod_alpha = state->phase_cos * state->mod_d - state->phase_sin * state->mod_q;
    float mod_beta = state->phase_sin * state->mod_d + state->phase_cos * state->mod_q;
    
    // Store raw modulation
    state->mod_alpha_raw = mod_alpha;
    state->mod_beta_raw = mod_beta;
    
    // Calculate αβ voltages
    state->v_alpha = mod_alpha * state->v_bus;
    state->v_beta = mod_beta * state->v_bus;
    
    // Run SVM to get duty cycles
    uint32_t duty1, duty2, duty3;
    foc_svm(mod_alpha, mod_beta, max_duty, FOC_PWM_PERIOD_DEFAULT,
            &duty1, &duty2, &duty3, &state->svm_sector);
    
    // Call PWM callback if set
    if (pwm_callback != NULL) {
        pwm_callback(duty1, duty2, duty3);
    }
    
    // Calculate duty cycle
    state->duty_now = sqrtf(state->mod_d * state->mod_d + 
                            state->mod_q * state->mod_q);
}

void foc_control_speed(motor_all_state_t *motor, float dt, float speed_ref) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    
    // Get current speed (ERPM)
    float speed_now = motor->m_pll_speed * 60.0f / (2.0f * M_PI);
    
    // Speed error
    float error = speed_ref - speed_now;
    
    // PID control
    float p_term = conf->s_pid_kp * error;
    motor->m_speed_i_term += conf->s_pid_ki * error * dt;
    float d_term = conf->s_pid_kd * (error - motor->m_speed_prev_error) / dt;
    
    motor->m_speed_prev_error = error;
    
    // Anti-windup
    float max_current = conf->l_current_max;
    utils_truncate_number(&motor->m_speed_i_term, -max_current, max_current);
    
    // Calculate q-axis current command
    float iq_cmd = p_term + motor->m_speed_i_term + d_term;
    utils_truncate_number(&iq_cmd, -max_current, max_current);
    
    // Set current targets
    motor->m_id_set = 0.0f;  // MTPA could modify this
    motor->m_iq_set = iq_cmd;
    
    motor->m_speed_pid_set_rpm = speed_ref;
}

void foc_control_position(motor_all_state_t *motor, float dt, float pos_ref) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    
    // Get current position (degrees)
    float pos_now = motor->m_pos_pid_now;
    
    // Position error
    float error = pos_ref - pos_now;
    
    // Normalize error to [-180, 180]
    while (error > 180.0f) error -= 360.0f;
    while (error < -180.0f) error += 360.0f;
    
    // PID control
    float p_term = conf->p_pid_kp * error;
    motor->m_pos_i_term += conf->p_pid_ki * error * dt;
    float d_term = conf->p_pid_kd * (error - motor->m_pos_prev_error) / dt;
    
    motor->m_pos_prev_error = error;
    
    // Anti-windup
    float max_rpm = conf->l_max_erpm;
    utils_truncate_number(&motor->m_pos_i_term, -max_rpm, max_rpm);
    
    // Calculate speed command
    float speed_cmd = p_term + motor->m_pos_i_term + d_term;
    utils_truncate_number(&speed_cmd, -max_rpm, max_rpm);
    
    // Run speed control
    foc_control_speed(motor, dt, speed_cmd);
    
    motor->m_pos_pid_set = pos_ref;
}

void foc_run_observer(motor_all_state_t *motor, float dt) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    motor_state_t *state = &motor->m_motor_state;
    
    // Run observer update
    float phase;
    foc_observer_update(
        state->v_alpha, state->v_beta,
        state->i_alpha, state->i_beta,
        dt, &motor->m_observer_state, &phase, motor);
    
    motor->m_phase_now_observer = phase;
    
    // Run PLL
    foc_pll_run(phase, dt, &motor->m_pll_phase, &motor->m_pll_speed, motor->m_conf);
    
    // Update speed estimates
    motor->m_speed_est_fast = motor->m_pll_speed;
    motor->m_speed_est_fast_corrected = motor->m_pll_speed;
}

void foc_update_phase(motor_all_state_t *motor, float phase) {
    if (motor == NULL) return;
    
    motor_state_t *state = &motor->m_motor_state;
    
    state->phase = phase;
    utils_fast_sincos_better(phase, &state->phase_sin, &state->phase_cos);
    
    // Update position (convert rad to degrees and track rotations)
    float new_pos = phase * 180.0f / M_PI;
    motor->m_pos_pid_now = new_pos;
}

void foc_set_id_target(motor_all_state_t *motor, float id_target) {
    if (motor != NULL) {
        motor->m_id_set = id_target;
    }
}

void foc_set_iq_target(motor_all_state_t *motor, float iq_target) {
    if (motor != NULL) {
        motor->m_iq_set = iq_target;
    }
}

void foc_set_duty_target(motor_all_state_t *motor, float duty) {
    if (motor != NULL) {
        motor->m_duty_cycle_set = duty;
    }
}

float foc_get_erpm(motor_all_state_t *motor) {
    if (motor == NULL) return 0.0f;
    return motor->m_pll_speed * 60.0f / (2.0f * M_PI);
}

float foc_get_position(motor_all_state_t *motor) {
    if (motor == NULL) return 0.0f;
    return motor->m_pos_pid_now;
}

void foc_timer_update(motor_all_state_t *motor, float dt) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    motor_state_t *state = &motor->m_motor_state;
    
    // Filter duty cycle
    float filter_const = conf->foc_current_filter_const;
    UTILS_LP_FAST(motor->m_duty_abs_filtered, fabsf(state->duty_now), filter_const);
    UTILS_LP_FAST(motor->m_duty_filtered, state->duty_now, filter_const);
    
    // Update modulation filter
    UTILS_LP_FAST(state->mod_q_filter, state->mod_q, filter_const);
    UTILS_LP_FAST(state->v_mag_filter, 
                  sqrtf(state->vd * state->vd + state->vq * state->vq), 
                  filter_const);
    
    // Update tachometer based on phase change
    float phase_diff = state->phase - motor->m_phase_before_speed_est;
    
    // Normalize to [-π, π]
    while (phase_diff > M_PI) phase_diff -= 2.0f * M_PI;
    while (phase_diff < -M_PI) phase_diff += 2.0f * M_PI;
    
    // Count electrical revolutions for tachometer
    int tacho_step = (int)(phase_diff / (M_PI / 3.0f));
    motor->m_tachometer += tacho_step;
    motor->m_tachometer_abs += abs(tacho_step);
    
    motor->m_phase_before_speed_est = state->phase;
    
    // Update speed estimate from phase derivative
    if (dt > 0.0f) {
        float speed_rad_s = phase_diff / dt;
        float speed_filter = 0.02f;
        UTILS_LP_FAST(motor->m_speed_est_faster, speed_rad_s, speed_filter);
    }
    
    // Min RPM hysteresis timer (for sensorless handoff)
    float abs_rpm = fabsf(foc_get_erpm(motor));
    if (abs_rpm < conf->foc_sl_erpm) {
        motor->m_min_rpm_timer += dt;
    } else {
        motor->m_min_rpm_timer = 0.0f;
    }
    
    // HFI hysteresis timer
    if (abs_rpm < conf->foc_sl_erpm_hfi) {
        motor->m_min_rpm_hyst_timer += dt;
    } else {
        motor->m_min_rpm_hyst_timer = 0.0f;
    }
}

void foc_hfi_update(motor_all_state_t *motor, float dt) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    hfi_state_t *hfi = &motor->m_hfi;
    
    // Check if HFI should be active
    float abs_rpm = fabsf(foc_get_erpm(motor));
    bool should_use_hfi = (abs_rpm < conf->foc_sl_erpm_hfi) &&
                          (conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI ||
                           conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V2 ||
                           conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V3 ||
                           conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V4 ||
                           conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V5 ||
                           conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_START);
    
    if (!should_use_hfi) {
        // Reset HFI state when not in use
        hfi->ready = false;
        hfi->ind = 0;
        hfi->double_integrator = 0.0f;
        return;
    }
    
    // Simple HFI angle tracking (simplified from full implementation)
    // This simulates the HFI response for testing purposes
    
    // Increment sample counter
    hfi->ind++;
    if (hfi->ind >= hfi->samples) {
        hfi->ind = 0;
        hfi->ready = true;
    }
    
    // HFI angle error tracking
    // In real implementation, this would process ADC samples
    // Here we simulate convergence to the true angle
    if (hfi->ready) {
        float angle_err = motor->m_motor_state.phase - hfi->angle;
        
        // Normalize error
        while (angle_err > M_PI) angle_err -= 2.0f * M_PI;
        while (angle_err < -M_PI) angle_err += 2.0f * M_PI;
        
        // Integrator for angle tracking
        float hfi_gain = conf->foc_hfi_gain;
        hfi->double_integrator += angle_err * hfi_gain * dt;
        hfi->angle += hfi->double_integrator * dt + angle_err * hfi_gain * 0.1f * dt;
        
        // Normalize angle
        while (hfi->angle > M_PI) hfi->angle -= 2.0f * M_PI;
        while (hfi->angle < -M_PI) hfi->angle += 2.0f * M_PI;
        
        // Use HFI angle for observer when at low speed
        if (motor->m_min_rpm_hyst_timer > 0.1f) {
            foc_hfi_adjust_angle(angle_err, motor, dt);
        }
    }
}

bool foc_is_hfi_active(motor_all_state_t *motor) {
    if (motor == NULL || motor->m_conf == NULL) return false;
    
    mc_configuration *conf = motor->m_conf;
    float abs_rpm = fabsf(foc_get_erpm(motor));
    
    bool hfi_mode = (conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI ||
                     conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V2 ||
                     conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V3 ||
                     conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V4 ||
                     conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_V5 ||
                     conf->foc_sensor_mode == FOC_SENSOR_MODE_HFI_START);
    
    return hfi_mode && (abs_rpm < conf->foc_sl_erpm_hfi) && motor->m_hfi.ready;
}

void foc_set_hfi_voltage(motor_all_state_t *motor, float voltage) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    // Clamp voltage
    if (voltage < motor->m_conf->foc_hfi_voltage_start) {
        voltage = motor->m_conf->foc_hfi_voltage_start;
    }
    if (voltage > motor->m_conf->foc_hfi_voltage_max) {
        voltage = motor->m_conf->foc_hfi_voltage_max;
    }
    
    // This would set the injection voltage in real implementation
    // For simulation, we store it for reference
    (void)voltage;
}

void foc_run_field_weakening(motor_all_state_t *motor, float dt) {
    if (motor == NULL || motor->m_conf == NULL) return;
    
    mc_configuration *conf = motor->m_conf;
    motor_state_t *state = &motor->m_motor_state;
    
    // Check if field weakening is enabled
    if (conf->foc_fw_current_max < 0.001f) {
        motor->m_i_fw_set = 0.0f;
        return;
    }
    
    // Calculate duty cycle
    float duty = sqrtf(state->mod_d * state->mod_d + state->mod_q * state->mod_q);
    
    // Field weakening activation threshold
    float fw_duty_start = conf->foc_fw_duty_start;
    
    if (duty > fw_duty_start) {
        // Calculate required field weakening current
        float duty_excess = duty - fw_duty_start;
        float fw_current = duty_excess * conf->foc_fw_current_max / (1.0f - fw_duty_start);
        
        // Ramp the field weakening current
        float ramp_rate = conf->foc_fw_current_max / conf->foc_fw_ramp_time;
        float target_fw = fminf(fw_current, conf->foc_fw_current_max);
        
        if (motor->m_i_fw_set < target_fw) {
            motor->m_i_fw_set += ramp_rate * dt;
            if (motor->m_i_fw_set > target_fw) {
                motor->m_i_fw_set = target_fw;
            }
        } else {
            motor->m_i_fw_set -= ramp_rate * dt;
            if (motor->m_i_fw_set < target_fw) {
                motor->m_i_fw_set = target_fw;
            }
        }
        
        // Apply negative d-axis current for field weakening
        motor->m_id_set = -motor->m_i_fw_set;
    } else {
        // Ramp down field weakening
        float ramp_rate = conf->foc_fw_current_max / conf->foc_fw_ramp_time;
        if (motor->m_i_fw_set > 0.0f) {
            motor->m_i_fw_set -= ramp_rate * dt;
            if (motor->m_i_fw_set < 0.0f) {
                motor->m_i_fw_set = 0.0f;
            }
            motor->m_id_set = -motor->m_i_fw_set;
        }
    }
}
