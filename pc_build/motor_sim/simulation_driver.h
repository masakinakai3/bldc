/**
 * @file simulation_driver.h
 * @brief Time step management and main simulation driver
 * 
 * Manages time synchronization between FOC control loop and motor model,
 * handles multi-rate stepping (control typically faster than simulation output).
 */

#ifndef SIMULATION_DRIVER_H_
#define SIMULATION_DRIVER_H_

#include "virtual_motor_pc.h"
#include "foc_control_core.h"
#include "mcconf_stub.h"

// Simulation time management
typedef struct {
    float control_dt;       // Control loop time step (typically 1/20kHz = 50us)
    float model_dt;         // Motor model time step (can be same or smaller)
    float output_dt;        // Output recording interval
    float current_time;     // Current simulation time
    float end_time;         // Simulation end time
    uint32_t control_steps; // Number of control steps executed
    uint32_t output_steps;  // Number of output records
    float real_time_start;  // Wall clock start for timing
} sim_time_t;

// Simulation state
typedef enum {
    SIM_STATE_IDLE = 0,
    SIM_STATE_RUNNING,
    SIM_STATE_PAUSED,
    SIM_STATE_COMPLETED,
    SIM_STATE_ERROR
} sim_state_e;

// Control mode for simulation
typedef enum {
    SIM_CTRL_NONE = 0,
    SIM_CTRL_CURRENT,       // Direct current control (id, iq)
    SIM_CTRL_SPEED,         // Speed control loop
    SIM_CTRL_POSITION,      // Position control loop
    SIM_CTRL_DUTY,          // Duty cycle control
    SIM_CTRL_OPENLOOP       // Open-loop V/f control
} sim_control_mode_e;

// Simulation reference commands
typedef struct {
    sim_control_mode_e mode;
    float id_ref;           // D-axis current reference [A]
    float iq_ref;           // Q-axis current reference [A]
    float speed_ref;        // Speed reference [ERPM]
    float position_ref;     // Position reference [deg]
    float duty_ref;         // Duty cycle reference [0-1]
    float ol_voltage;       // Open-loop voltage [V]
    float ol_frequency;     // Open-loop frequency [Hz]
    float ramp_rate;        // Reference ramp rate (units/s)
} sim_command_t;

// Simulation disturbance/fault injection
typedef struct {
    bool enable;
    float load_torque;      // External load torque [Nm]
    float load_time;        // Time to apply load [s]
    float load_duration;    // Duration of load [s]
    float phase_loss_time;  // Time for phase loss fault
    int phase_loss_phase;   // Which phase (0=A, 1=B, 2=C)
    float sensor_noise;     // Current sensor noise std dev [A]
} sim_disturbance_t;

// Data recording configuration
typedef struct {
    bool enable;
    const char *filename;
    uint32_t max_records;
    bool record_currents;
    bool record_voltages;
    bool record_position;
    bool record_speed;
    bool record_torque;
    bool record_power;
} sim_recording_t;

// Main simulation context
typedef struct {
    // State
    sim_state_e state;
    sim_time_t time;
    
    // Configuration
    mc_configuration mc_conf;
    
    // Virtual motor I/O
    virtual_motor_io_t vm_io;
    
    // FOC control state
    motor_all_state_t foc_state;
    
    // Commands
    sim_command_t command;
    sim_disturbance_t disturbance;
    
    // Recording
    sim_recording_t recording;
    
    // Statistics
    float min_id, max_id;
    float min_iq, max_iq;
    float min_speed, max_speed;
    float total_energy_in;
    float total_energy_out;
} sim_context_t;

// ==== API Functions ====

/**
 * @brief Initialize simulation context with default parameters
 */
void sim_init(sim_context_t *ctx);

/**
 * @brief Configure controller parameters
 */
void sim_set_controller_params(sim_context_t *ctx, mc_configuration *conf);

/**
 * @brief Set timing configuration
 */
void sim_set_timing(sim_context_t *ctx, 
                    float control_dt, 
                    float model_dt, 
                    float output_dt,
                    float duration);

/**
 * @brief Set control reference
 */
void sim_set_reference(sim_context_t *ctx, sim_command_t *cmd);

/**
 * @brief Set disturbance/fault parameters
 */
void sim_set_disturbance(sim_context_t *ctx, sim_disturbance_t *dist);

/**
 * @brief Configure data recording
 */
void sim_set_recording(sim_context_t *ctx, sim_recording_t *rec);

/**
 * @brief Set load torque on motor
 */
void sim_set_load_torque(sim_context_t *ctx, float torque);

/**
 * @brief Set motor inertia
 */
void sim_set_inertia(sim_context_t *ctx, float inertia);

/**
 * @brief Start simulation
 */
int sim_start(sim_context_t *ctx);

/**
 * @brief Run one control step
 * @return 0 on success, -1 on error, 1 on completion
 */
int sim_step(sim_context_t *ctx);

/**
 * @brief Run simulation to completion
 */
int sim_run(sim_context_t *ctx);

/**
 * @brief Pause simulation
 */
void sim_pause(sim_context_t *ctx);

/**
 * @brief Resume simulation
 */
void sim_resume(sim_context_t *ctx);

/**
 * @brief Stop and reset simulation
 */
void sim_stop(sim_context_t *ctx);

/**
 * @brief Get current simulation results summary
 */
void sim_get_summary(sim_context_t *ctx, 
                     float *avg_speed,
                     float *avg_torque,
                     float *efficiency);

/**
 * @brief Get current state snapshot
 */
void sim_get_state(sim_context_t *ctx,
                   float *id, float *iq,
                   float *speed, float *position,
                   float *torque, float *power);

#endif // SIMULATION_DRIVER_H_
