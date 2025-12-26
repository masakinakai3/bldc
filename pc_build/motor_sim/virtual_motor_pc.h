/**
 * @file virtual_motor_pc.h
 * @brief PC-compatible virtual motor model for FOC simulation
 * 
 * This module provides a software implementation of a PMSM motor model
 * that can be used for closed-loop FOC simulation on PC without hardware.
 * Based on the original virtual_motor.c implementation.
 */

#ifndef VIRTUAL_MOTOR_PC_H_
#define VIRTUAL_MOTOR_PC_H_

#include "datatypes.h"

/**
 * Virtual motor I/O structure
 * Used for step-by-step simulation
 */
typedef struct {
    // Inputs
    float v_alpha_in;      ///< Input: α-axis voltage [V]
    float v_beta_in;       ///< Input: β-axis voltage [V]
    
    // Outputs
    float i_alpha_out;     ///< Output: α-axis current [A]
    float i_beta_out;      ///< Output: β-axis current [A]
    float angle_rad;       ///< Output: electrical angle [rad]
    float omega_e;         ///< Output: electrical angular velocity [rad/s]
    float torque;          ///< Output: electromagnetic torque [Nm]
} virtual_motor_io_t;

/**
 * Virtual motor state structure (for external access if needed)
 */
typedef struct {
    // Parameters
    float Ts;              ///< Sampling time [s]
    float J;               ///< Rotor inertia [kg·m²]
    int pole_pairs;        ///< Number of pole pairs
    float km;              ///< Torque constant factor (1.5 * pole_pairs)
    float ld;              ///< d-axis inductance [H]
    float lq;              ///< q-axis inductance [H]
    float R;               ///< Stator resistance [Ω]
    float lambda;          ///< Flux linkage [Wb]
    
    // State variables
    float id;              ///< d-axis current [A]
    float iq;              ///< q-axis current [A]
    float id_int;          ///< d-axis current integral term
    float we;              ///< Electrical angular velocity [rad/s]
    float phi;             ///< Electrical angle [rad]
    float sin_phi;         ///< sin(phi)
    float cos_phi;         ///< cos(phi)
    float ml;              ///< Load torque [Nm]
} virtual_motor_state_t;

/**
 * Initialize the PC virtual motor with motor configuration
 * 
 * @param conf Pointer to motor configuration
 */
void virtual_motor_pc_init(mc_configuration *conf);

/**
 * Update motor configuration (runtime reconfiguration)
 * 
 * @param conf Pointer to new motor configuration
 */
void virtual_motor_pc_set_configuration(mc_configuration *conf);

/**
 * Execute one simulation step
 * 
 * @param io Pointer to I/O structure (input voltages, output currents/angle)
 * @param dt Time step [s]
 */
void virtual_motor_pc_step(virtual_motor_io_t *io, float dt);

/**
 * Set the load torque applied to the motor
 * 
 * @param torque_nm Load torque [Nm] (positive = resisting rotation)
 */
void virtual_motor_pc_set_load_torque(float torque_nm);

/**
 * Set the rotor inertia
 * 
 * @param J_kgm2 Rotor inertia [kg·m²]
 */
void virtual_motor_pc_set_inertia(float J_kgm2);

/**
 * Set initial rotor angle
 * 
 * @param angle_rad Initial electrical angle [rad]
 */
void virtual_motor_pc_set_angle(float angle_rad);

/**
 * Set initial angular velocity
 * 
 * @param omega_rad_s Initial electrical angular velocity [rad/s]
 */
void virtual_motor_pc_set_speed(float omega_rad_s);

/**
 * Get current mechanical RPM
 * 
 * @return Mechanical RPM
 */
float virtual_motor_pc_get_rpm(void);

/**
 * Get electrical RPM (ERPM)
 * 
 * @return Electrical RPM
 */
float virtual_motor_pc_get_erpm(void);

/**
 * Get d-axis current
 * 
 * @return d-axis current [A]
 */
float virtual_motor_pc_get_id(void);

/**
 * Get q-axis current
 * 
 * @return q-axis current [A]
 */
float virtual_motor_pc_get_iq(void);

/**
 * Get electrical angle in radians
 * 
 * @return Electrical angle [rad]
 */
float virtual_motor_pc_get_angle_rad(void);

/**
 * Get electrical angle in degrees
 * 
 * @return Electrical angle [degrees]
 */
float virtual_motor_pc_get_angle_deg(void);

/**
 * Get electromagnetic torque
 * 
 * @return Electromagnetic torque [Nm]
 */
float virtual_motor_pc_get_torque(void);

/**
 * Get electrical angular velocity
 * 
 * @return Electrical angular velocity [rad/s]
 */
float virtual_motor_pc_get_omega_e(void);

/**
 * Get read-only access to internal state
 * 
 * @return Pointer to internal state structure
 */
const virtual_motor_state_t* virtual_motor_pc_get_state(void);

/**
 * Reset motor state to initial conditions
 */
void virtual_motor_pc_reset(void);

#endif /* VIRTUAL_MOTOR_PC_H_ */
