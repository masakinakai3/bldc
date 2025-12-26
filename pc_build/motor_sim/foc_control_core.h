/**
 * @file foc_control_core.h
 * @brief Extracted FOC control core functions for PC simulation
 * 
 * This module provides FOC control algorithms extracted from mcpwm_foc.c,
 * adapted for PC execution and simulation.
 */

#ifndef FOC_CONTROL_CORE_H_
#define FOC_CONTROL_CORE_H_

#include "foc_math.h"
#include "datatypes.h"

/**
 * PWM update callback type
 * Called when new PWM duty cycles are calculated
 */
typedef void (*foc_pwm_callback_t)(uint32_t duty1, uint32_t duty2, uint32_t duty3);

/**
 * Set PWM update callback
 * 
 * @param cb Callback function (can be NULL to disable)
 */
void foc_set_pwm_callback(foc_pwm_callback_t cb);

/**
 * Initialize motor state structure
 * 
 * @param motor Pointer to motor state structure
 * @param conf Pointer to motor configuration
 */
void foc_motor_state_init(motor_all_state_t *motor, mc_configuration *conf);

/**
 * Reset motor state (keep configuration)
 * 
 * @param motor Pointer to motor state structure
 */
void foc_motor_state_reset(motor_all_state_t *motor);

/**
 * Update motor state with new measurements
 * 
 * @param motor Pointer to motor state structure
 * @param i_alpha Measured α-axis current [A]
 * @param i_beta Measured β-axis current [A]
 * @param v_bus DC bus voltage [V]
 */
void foc_update_measurements(motor_all_state_t *motor, 
                             float i_alpha, float i_beta, float v_bus);

/**
 * Run current control loop
 * Executes PI current controllers and calculates voltage commands
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 */
void foc_control_current(motor_all_state_t *motor, float dt);

/**
 * Run speed control loop
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 * @param speed_ref Speed reference [ERPM]
 */
void foc_control_speed(motor_all_state_t *motor, float dt, float speed_ref);

/**
 * Run position control loop
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 * @param pos_ref Position reference [degrees]
 */
void foc_control_position(motor_all_state_t *motor, float dt, float pos_ref);

/**
 * Run observer and PLL
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 */
void foc_run_observer(motor_all_state_t *motor, float dt);

/**
 * Update phase angle from observer/encoder
 * 
 * @param motor Pointer to motor state structure
 * @param phase New phase angle [rad]
 */
void foc_update_phase(motor_all_state_t *motor, float phase);

/**
 * Set d-axis current target
 * 
 * @param motor Pointer to motor state structure
 * @param id_target Target d-axis current [A]
 */
void foc_set_id_target(motor_all_state_t *motor, float id_target);

/**
 * Set q-axis current target
 * 
 * @param motor Pointer to motor state structure
 * @param iq_target Target q-axis current [A]
 */
void foc_set_iq_target(motor_all_state_t *motor, float iq_target);

/**
 * Set duty cycle target
 * 
 * @param motor Pointer to motor state structure
 * @param duty Target duty cycle [-1.0 to 1.0]
 */
void foc_set_duty_target(motor_all_state_t *motor, float duty);

/**
 * Get estimated speed in ERPM
 * 
 * @param motor Pointer to motor state structure
 * @return Estimated speed [ERPM]
 */
float foc_get_erpm(motor_all_state_t *motor);

/**
 * Get estimated position in degrees
 * 
 * @param motor Pointer to motor state structure
 * @return Estimated position [degrees]
 */
float foc_get_position(motor_all_state_t *motor);

/**
 * Timer update - runs filters and state updates at control rate
 * Extracted from mcpwm_foc.c timer_update()
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 */
void foc_timer_update(motor_all_state_t *motor, float dt);

/**
 * HFI (High Frequency Injection) update
 * Processes HFI samples and updates angle estimate
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 */
void foc_hfi_update(motor_all_state_t *motor, float dt);

/**
 * Check if HFI is active
 * 
 * @param motor Pointer to motor state structure
 * @return true if HFI is active
 */
bool foc_is_hfi_active(motor_all_state_t *motor);

/**
 * Set HFI injection voltage
 * 
 * @param motor Pointer to motor state structure
 * @param voltage HFI injection voltage [V]
 */
void foc_set_hfi_voltage(motor_all_state_t *motor, float voltage);

/**
 * Run field weakening control
 * 
 * @param motor Pointer to motor state structure
 * @param dt Control period [s]
 */
void foc_run_field_weakening(motor_all_state_t *motor, float dt);

/**
 * PWM period for SVM calculations (default 4200 for 30kHz at 168MHz)
 */
#define FOC_PWM_PERIOD_DEFAULT  4200

#endif /* FOC_CONTROL_CORE_H_ */
