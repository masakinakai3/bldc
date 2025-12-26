/**
 * @file mcconf_stub.c
 * @brief Motor configuration stub implementation for PC build
 */

#include "mcconf_stub.h"
#include <string.h>

void mcconf_set_defaults(mc_configuration *conf) {
    memset(conf, 0, sizeof(mc_configuration));
    
    // =========================================================================
    // Limits
    // =========================================================================
    conf->l_current_max = 60.0f;
    conf->l_current_min = -60.0f;
    conf->l_in_current_max = 60.0f;
    conf->l_in_current_min = -20.0f;
    conf->l_in_current_map_start = 0.8f;
    conf->l_in_current_map_filter = 0.1f;
    conf->l_abs_current_max = 150.0f;
    conf->l_min_erpm = -100000.0f;
    conf->l_max_erpm = 100000.0f;
    conf->l_erpm_start = 0.8f;
    conf->l_max_erpm_fbrake = 300.0f;
    conf->l_max_erpm_fbrake_cc = 1500.0f;
    conf->l_min_vin = 8.0f;
    conf->l_max_vin = 57.0f;
    conf->l_battery_cut_start = 10.0f;
    conf->l_battery_cut_end = 8.0f;
    conf->l_battery_regen_cut_start = 4.2f;
    conf->l_battery_regen_cut_end = 4.4f;
    conf->l_slow_abs_current = true;
    conf->l_temp_fet_start = 80.0f;
    conf->l_temp_fet_end = 100.0f;
    conf->l_temp_motor_start = 80.0f;
    conf->l_temp_motor_end = 100.0f;
    conf->l_temp_accel_dec = 15.0f;
    conf->l_min_duty = 0.005f;
    conf->l_max_duty = 0.95f;
    conf->l_watt_max = 25000.0f;
    conf->l_watt_min = -25000.0f;
    conf->l_current_max_scale = 1.0f;
    conf->l_current_min_scale = 1.0f;
    conf->l_duty_start = 1.0f;
    
    // Override limits (runtime computed)
    conf->lo_current_max = conf->l_current_max;
    conf->lo_current_min = conf->l_current_min;
    conf->lo_in_current_max = conf->l_in_current_max;
    conf->lo_in_current_min = conf->l_in_current_min;
    
    // =========================================================================
    // Motor type
    // =========================================================================
    conf->motor_type = MOTOR_TYPE_FOC;
    conf->pwm_mode = PWM_MODE_SYNCHRONOUS;
    conf->comm_mode = COMM_MODE_INTEGRATE;
    conf->sensor_mode = SENSOR_MODE_SENSORLESS;
    
    // =========================================================================
    // FOC parameters
    // =========================================================================
    conf->foc_current_kp = 0.02f;
    conf->foc_current_ki = 10.0f;
    conf->foc_f_zv = 30000.0f;
    conf->foc_dt_us = 0.08f;
    conf->foc_encoder_offset = 0.0f;
    conf->foc_encoder_inverted = false;
    conf->foc_encoder_ratio = 1.0f;
    conf->foc_motor_l = 0.000015f;        // 15 µH
    conf->foc_motor_ld_lq_diff = 0.0f;
    conf->foc_motor_r = 0.015f;           // 15 mΩ
    conf->foc_motor_flux_linkage = 0.008f; // 8 mWb
    conf->foc_observer_gain = 9e7f;
    conf->foc_observer_gain_slow = 0.3f;
    conf->foc_observer_offset = 0.0f;
    conf->foc_pll_kp = 4000.0f;
    conf->foc_pll_ki = 60000.0f;
    conf->foc_duty_dowmramp_kp = 10.0f;
    conf->foc_duty_dowmramp_ki = 200.0f;
    conf->foc_start_curr_dec = 1.0f;
    conf->foc_start_curr_dec_rpm = 100000.0f;
    conf->foc_openloop_rpm = 1500.0f;
    conf->foc_openloop_rpm_low = 0.0f;
    conf->foc_sl_openloop_hyst = 0.1f;
    conf->foc_sl_openloop_time = 0.1f;
    conf->foc_sl_openloop_time_lock = 0.0f;
    conf->foc_sl_openloop_time_ramp = 0.1f;
    conf->foc_sl_openloop_boost_q = 0.0f;
    conf->foc_sl_openloop_max_q = 0.0f;
    conf->foc_sensor_mode = FOC_SENSOR_MODE_SENSORLESS;
    
    // Hall table
    for (int i = 0; i < 8; i++) {
        conf->foc_hall_table[i] = 255;
    }
    
    conf->foc_hall_interp_erpm = 500.0f;
    conf->foc_sl_erpm_start = 2500.0f;
    conf->foc_sl_erpm = 2500.0f;
    conf->foc_control_sample_mode = FOC_CONTROL_SAMPLE_MODE_V0_V7;
    conf->foc_current_sample_mode = FOC_CURRENT_SAMPLE_MODE_LONGEST_ZERO;
    conf->foc_sat_comp_mode = SAT_COMP_DISABLED;
    conf->foc_sat_comp = 0.0f;
    conf->foc_temp_comp = false;
    conf->foc_temp_comp_base_temp = 25.0f;
    conf->foc_current_filter_const = 0.1f;
    conf->foc_cc_decoupling = FOC_CC_DECOUPLING_DISABLED;
    conf->foc_observer_type = FOC_OBSERVER_ORTEGA_ORIGINAL;
    
    // HFI parameters
    conf->foc_hfi_voltage_start = 1.0f;
    conf->foc_hfi_voltage_run = 4.0f;
    conf->foc_hfi_voltage_max = 6.0f;
    conf->foc_hfi_gain = 0.5f;
    conf->foc_hfi_max_err = 0.05f;
    conf->foc_hfi_hyst = 0.3f;
    conf->foc_hfi_samples = HFI_SAMPLES_16;
    conf->foc_sl_erpm_hfi = 2000.0f;
    conf->foc_hfi_reset_erpm = 0.0f;
    conf->foc_hfi_start_samples = 65;
    conf->foc_hfi_obs_ovr_sec = 0.0f;
    conf->foc_offsets_cal_mode = 0;
    
    // Phase filter
    conf->foc_phase_filter_enable = true;
    conf->foc_phase_filter_disable_fault = false;
    conf->foc_phase_filter_max_erpm = 10000.0f;
    conf->foc_mtpa_mode = MTPA_MODE_OFF;
    
    // Field weakening
    conf->foc_fw_current_max = 0.0f;
    conf->foc_fw_duty_start = 0.9f;
    conf->foc_fw_ramp_time = 0.2f;
    conf->foc_fw_q_current_factor = 0.02f;
    conf->foc_speed_soure = FOC_SPEED_SRC_CORRECTED;
    conf->foc_short_ls_on_zero_duty = false;
    conf->foc_overmod_factor = 0.0f;
    
    // =========================================================================
    // Speed PID
    // =========================================================================
    conf->sp_pid_loop_rate = PID_RATE_5000_HZ;
    conf->s_pid_kp = 0.004f;
    conf->s_pid_ki = 0.004f;
    conf->s_pid_kd = 0.0001f;
    conf->s_pid_kd_filter = 0.2f;
    conf->s_pid_min_erpm = 300.0f;
    conf->s_pid_allow_braking = true;
    conf->s_pid_ramp_erpms_s = 25000.0f;
    conf->s_pid_speed_source = S_PID_SPEED_SRC_PLL;
    
    // =========================================================================
    // Position PID
    // =========================================================================
    conf->p_pid_kp = 0.03f;
    conf->p_pid_ki = 0.0f;
    conf->p_pid_kd = 0.0004f;
    conf->p_pid_kd_proc = 0.00035f;
    conf->p_pid_kd_filter = 0.2f;
    conf->p_pid_ang_div = 1.0f;
    conf->p_pid_gain_dec_angle = 0.0f;
    conf->p_pid_offset = 0.0f;
    
    // =========================================================================
    // Current controller
    // =========================================================================
    conf->cc_startup_boost_duty = 0.01f;
    conf->cc_min_current = 0.05f;
    conf->cc_gain = 0.0046f;
    conf->cc_ramp_step_max = 0.04f;
    
    // =========================================================================
    // Misc
    // =========================================================================
    conf->m_fault_stop_time_ms = 3000;
    conf->m_duty_ramp_step = 0.00004f;
    conf->m_current_backoff_gain = 0.5f;
    conf->m_encoder_counts = 8192;
    conf->m_sensor_port_mode = SENSOR_PORT_MODE_HALL;
    conf->m_invert_direction = false;
    conf->m_drv8301_oc_mode = DRV8301_OC_LIMIT;
    conf->m_drv8301_oc_adj = 16;
    conf->m_bldc_f_sw_min = 3000.0f;
    conf->m_bldc_f_sw_max = 35000.0f;
    conf->m_dc_f_sw = 25000.0f;
    conf->m_ntc_motor_beta = 3380.0f;
    conf->m_out_aux_mode = OUT_AUX_MODE_OFF;
    conf->m_motor_temp_sens_type = TEMP_SENSOR_NTC_10K_25C;
    conf->m_ptc_motor_coeff = 0.61f;
    conf->m_hall_extra_samples = 1;
    conf->m_batt_filter_const = 10;
    
    // =========================================================================
    // Setup info
    // =========================================================================
    conf->si_motor_poles = 14;
    conf->si_gear_ratio = 1.0f;
    conf->si_wheel_diameter = 0.083f;
    conf->si_battery_type = BATTERY_TYPE_LIION_3_0__4_2;
    conf->si_battery_cells = 12;
    conf->si_battery_ah = 6.0f;
    conf->si_motor_nl_current = 1.0f;
    
    // BMS config
    conf->bms.type = BMS_TYPE_NONE;
    conf->bms.limit_mode = 0;
    conf->bms.t_limit_start = 45.0f;
    conf->bms.t_limit_end = 65.0f;
    conf->bms.soc_limit_start = 0.05f;
    conf->bms.soc_limit_end = 0.0f;
}

void mcconf_set_small_motor(mc_configuration *conf) {
    mcconf_set_defaults(conf);
    
    // Small gimbal motor parameters
    conf->l_current_max = 10.0f;
    conf->l_current_min = -10.0f;
    conf->l_abs_current_max = 20.0f;
    
    conf->foc_motor_l = 0.0001f;          // 100 µH
    conf->foc_motor_r = 0.2f;             // 200 mΩ
    conf->foc_motor_flux_linkage = 0.003f; // 3 mWb
    conf->foc_observer_gain = 1e7f;
    
    conf->si_motor_poles = 14;
}

void mcconf_set_medium_motor(mc_configuration *conf) {
    mcconf_set_defaults(conf);
    
    // Medium hub motor (skateboard) parameters
    conf->l_current_max = 50.0f;
    conf->l_current_min = -40.0f;
    conf->l_abs_current_max = 100.0f;
    
    conf->foc_motor_l = 0.000020f;        // 20 µH
    conf->foc_motor_r = 0.020f;           // 20 mΩ
    conf->foc_motor_flux_linkage = 0.015f; // 15 mWb
    conf->foc_observer_gain = 5e7f;
    
    conf->si_motor_poles = 28;
}

void mcconf_set_large_motor(mc_configuration *conf) {
    mcconf_set_defaults(conf);
    
    // Large motor (EV) parameters
    conf->l_current_max = 200.0f;
    conf->l_current_min = -150.0f;
    conf->l_abs_current_max = 400.0f;
    
    conf->foc_motor_l = 0.000008f;        // 8 µH
    conf->foc_motor_r = 0.005f;           // 5 mΩ
    conf->foc_motor_flux_linkage = 0.030f; // 30 mWb
    conf->foc_observer_gain = 2e8f;
    
    conf->si_motor_poles = 10;
}
