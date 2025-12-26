/**
 * @file simulation_driver.c
 * @brief Time step management and main simulation driver implementation
 */

#include "simulation_driver.h"
#include "simulation_data.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Default timing parameters
#define DEFAULT_CONTROL_DT      (1.0f / 20000.0f)  // 20kHz control loop
#define DEFAULT_MODEL_DT        (1.0f / 100000.0f) // 100kHz motor model
#define DEFAULT_OUTPUT_DT       (1.0f / 1000.0f)   // 1kHz output recording
#define DEFAULT_DURATION        1.0f               // 1 second default

// PWM handling for closed-loop connection
static sim_context_t *g_active_context = NULL;

static void pwm_callback(uint32_t duty1, uint32_t duty2, uint32_t duty3) {
    if (g_active_context == NULL) return;
    
    // Convert PWM duty cycles to αβ voltages using SVM inverse
    float v_bus = g_active_context->foc_state.m_motor_state.v_bus;
    float duty_max = (float)FOC_PWM_PERIOD_DEFAULT;
    
    // Normalize duties to 0-1 range
    float d1 = (float)duty1 / duty_max;
    float d2 = (float)duty2 / duty_max;
    float d3 = (float)duty3 / duty_max;
    
    // Convert to phase voltages (center-aligned PWM)
    float va = (d1 - 0.5f) * v_bus;
    float vb = (d2 - 0.5f) * v_bus;
    float vc = (d3 - 0.5f) * v_bus;
    
    // Clarke transform to αβ
    float v_alpha = va;
    float v_beta = (va + 2.0f * vb) / 1.73205080757f;  // 1/sqrt(3) * (va + 2*vb)
    
    // Store for motor model
    g_active_context->vm_io.v_alpha_in = v_alpha;
    g_active_context->vm_io.v_beta_in = v_beta;
}

void sim_init(sim_context_t *ctx) {
    if (ctx == NULL) return;
    
    memset(ctx, 0, sizeof(sim_context_t));
    
    ctx->state = SIM_STATE_IDLE;
    
    // Set default timing
    ctx->time.control_dt = DEFAULT_CONTROL_DT;
    ctx->time.model_dt = DEFAULT_MODEL_DT;
    ctx->time.output_dt = DEFAULT_OUTPUT_DT;
    ctx->time.end_time = DEFAULT_DURATION;
    
    // Initialize motor configuration defaults
    mcconf_set_defaults(&ctx->mc_conf);
    
    // Initialize virtual motor with configuration
    virtual_motor_pc_init(&ctx->mc_conf);
    
    // Initialize FOC state
    foc_motor_state_init(&ctx->foc_state, &ctx->mc_conf);
    
    // Set up PWM callback for closed-loop
    foc_set_pwm_callback(pwm_callback);
    
    // Initialize command (default: current control, zero current)
    ctx->command.mode = SIM_CTRL_CURRENT;
    ctx->command.id_ref = 0.0f;
    ctx->command.iq_ref = 0.0f;
    
    // Initialize I/O
    memset(&ctx->vm_io, 0, sizeof(virtual_motor_io_t));
    
    // Initialize statistics
    ctx->min_id = 1e6f;
    ctx->max_id = -1e6f;
    ctx->min_iq = 1e6f;
    ctx->max_iq = -1e6f;
    ctx->min_speed = 1e6f;
    ctx->max_speed = -1e6f;
}

void sim_set_controller_params(sim_context_t *ctx, mc_configuration *conf) {
    if (ctx == NULL || conf == NULL) return;
    
    memcpy(&ctx->mc_conf, conf, sizeof(mc_configuration));
    foc_motor_state_init(&ctx->foc_state, &ctx->mc_conf);
    virtual_motor_pc_set_configuration(&ctx->mc_conf);
}

void sim_set_timing(sim_context_t *ctx, 
                    float control_dt, 
                    float model_dt, 
                    float output_dt,
                    float duration) {
    if (ctx == NULL) return;
    
    ctx->time.control_dt = control_dt;
    ctx->time.model_dt = model_dt;
    ctx->time.output_dt = output_dt;
    ctx->time.end_time = duration;
}

void sim_set_reference(sim_context_t *ctx, sim_command_t *cmd) {
    if (ctx == NULL || cmd == NULL) return;
    memcpy(&ctx->command, cmd, sizeof(sim_command_t));
}

void sim_set_disturbance(sim_context_t *ctx, sim_disturbance_t *dist) {
    if (ctx == NULL || dist == NULL) return;
    memcpy(&ctx->disturbance, dist, sizeof(sim_disturbance_t));
}

void sim_set_recording(sim_context_t *ctx, sim_recording_t *rec) {
    if (ctx == NULL || rec == NULL) return;
    memcpy(&ctx->recording, rec, sizeof(sim_recording_t));
}

void sim_set_load_torque(sim_context_t *ctx, float torque) {
    if (ctx == NULL) return;
    virtual_motor_pc_set_load_torque(torque);
}

void sim_set_inertia(sim_context_t *ctx, float inertia) {
    if (ctx == NULL) return;
    virtual_motor_pc_set_inertia(inertia);
}

int sim_start(sim_context_t *ctx) {
    if (ctx == NULL) return -1;
    
    // Reset time
    ctx->time.current_time = 0.0f;
    ctx->time.control_steps = 0;
    ctx->time.output_steps = 0;
    
    // Reset states
    foc_motor_state_reset(&ctx->foc_state);
    virtual_motor_pc_reset();
    
    // Reset I/O
    memset(&ctx->vm_io, 0, sizeof(virtual_motor_io_t));
    
    // Reset statistics
    ctx->min_id = 1e6f;
    ctx->max_id = -1e6f;
    ctx->min_iq = 1e6f;
    ctx->max_iq = -1e6f;
    ctx->min_speed = 1e6f;
    ctx->max_speed = -1e6f;
    ctx->total_energy_in = 0.0f;
    ctx->total_energy_out = 0.0f;
    
    // Open recording file if enabled
    if (ctx->recording.enable && ctx->recording.filename != NULL) {
        if (sim_data_open_write(ctx->recording.filename) < 0) {
            ctx->state = SIM_STATE_ERROR;
            return -1;
        }
    }
    
    // Set active context for callback
    g_active_context = ctx;
    
    ctx->state = SIM_STATE_RUNNING;
    return 0;
}

static void apply_disturbance(sim_context_t *ctx) {
    if (!ctx->disturbance.enable) return;
    
    float t = ctx->time.current_time;
    
    // Load torque step
    if (t >= ctx->disturbance.load_time && 
        t < (ctx->disturbance.load_time + ctx->disturbance.load_duration)) {
        virtual_motor_pc_set_load_torque(ctx->disturbance.load_torque);
    } else {
        virtual_motor_pc_set_load_torque(0.0f);
    }
}

static void update_statistics(sim_context_t *ctx) {
    float id = ctx->foc_state.m_motor_state.id;
    float iq = ctx->foc_state.m_motor_state.iq;
    float speed = foc_get_erpm(&ctx->foc_state);
    
    if (id < ctx->min_id) ctx->min_id = id;
    if (id > ctx->max_id) ctx->max_id = id;
    if (iq < ctx->min_iq) ctx->min_iq = iq;
    if (iq > ctx->max_iq) ctx->max_iq = iq;
    if (speed < ctx->min_speed) ctx->min_speed = speed;
    if (speed > ctx->max_speed) ctx->max_speed = speed;
    
    // Energy calculations
    float p_in = fabsf(ctx->foc_state.m_motor_state.id * ctx->foc_state.m_motor_state.vd +
                       ctx->foc_state.m_motor_state.iq * ctx->foc_state.m_motor_state.vq);
    float p_out = fabsf(virtual_motor_pc_get_torque() * virtual_motor_pc_get_omega_e() / 
                        (float)ctx->mc_conf.si_motor_poles * 2.0f);
    
    ctx->total_energy_in += p_in * ctx->time.control_dt;
    ctx->total_energy_out += p_out * ctx->time.control_dt;
}

int sim_step(sim_context_t *ctx) {
    if (ctx == NULL) return -1;
    if (ctx->state != SIM_STATE_RUNNING) return -1;
    
    float t = ctx->time.current_time;
    float dt_ctrl = ctx->time.control_dt;
    float dt_model = ctx->time.model_dt;
    
    // Check completion
    if (t >= ctx->time.end_time) {
        ctx->state = SIM_STATE_COMPLETED;
        if (ctx->recording.enable) {
            sim_data_close();
        }
        g_active_context = NULL;
        return 1;
    }
    
    // Get motor model outputs (currents in αβ)
    float i_alpha = ctx->vm_io.i_alpha_out;
    float i_beta = ctx->vm_io.i_beta_out;
    
    // Get bus voltage and update measurements
    float v_bus = ctx->foc_state.m_motor_state.v_bus;
    foc_update_measurements(&ctx->foc_state, i_alpha, i_beta, v_bus);
    
    // Run observer for angle estimation
    foc_run_observer(&ctx->foc_state, dt_ctrl);
    
    // Get electrical angle from motor model for simulation
    float theta_e = virtual_motor_pc_get_angle_rad();
    foc_update_phase(&ctx->foc_state, theta_e);
    
    // Apply control based on mode
    switch (ctx->command.mode) {
        case SIM_CTRL_CURRENT:
            foc_set_id_target(&ctx->foc_state, ctx->command.id_ref);
            foc_set_iq_target(&ctx->foc_state, ctx->command.iq_ref);
            foc_control_current(&ctx->foc_state, dt_ctrl);
            break;
            
        case SIM_CTRL_SPEED:
            foc_control_speed(&ctx->foc_state, dt_ctrl, ctx->command.speed_ref);
            foc_control_current(&ctx->foc_state, dt_ctrl);
            break;
            
        case SIM_CTRL_POSITION:
            foc_control_position(&ctx->foc_state, dt_ctrl, ctx->command.position_ref);
            foc_control_current(&ctx->foc_state, dt_ctrl);
            break;
            
        case SIM_CTRL_DUTY:
            foc_set_duty_target(&ctx->foc_state, ctx->command.duty_ref);
            break;
            
        case SIM_CTRL_OPENLOOP:
        case SIM_CTRL_NONE:
        default:
            break;
    }
    
    // Apply disturbances
    apply_disturbance(ctx);
    
    // Step motor model (possibly multiple sub-steps)
    int model_substeps = (int)(dt_ctrl / dt_model + 0.5f);
    if (model_substeps < 1) model_substeps = 1;
    
    float sub_dt = dt_ctrl / (float)model_substeps;
    for (int i = 0; i < model_substeps; i++) {
        virtual_motor_pc_step(&ctx->vm_io, sub_dt);
    }
    
    // Update statistics
    update_statistics(ctx);
    
    // Record data at output rate
    float output_interval = ctx->time.output_dt;
    uint32_t expected_outputs = (uint32_t)(t / output_interval);
    if (ctx->recording.enable && ctx->time.output_steps <= expected_outputs) {
        sim_data_record_t rec;
        rec.time = t;
        rec.id = ctx->foc_state.m_motor_state.id;
        rec.iq = ctx->foc_state.m_motor_state.iq;
        rec.vd = ctx->foc_state.m_motor_state.vd;
        rec.vq = ctx->foc_state.m_motor_state.vq;
        rec.theta_e = virtual_motor_pc_get_angle_rad();
        rec.omega_e = virtual_motor_pc_get_omega_e();
        rec.omega_m = virtual_motor_pc_get_omega_e() / (float)ctx->mc_conf.si_motor_poles * 2.0f;
        rec.torque = virtual_motor_pc_get_torque();
        
        sim_data_write_record(&rec);
        ctx->time.output_steps++;
    }
    
    // Advance time
    ctx->time.current_time += dt_ctrl;
    ctx->time.control_steps++;
    
    return 0;
}

int sim_run(sim_context_t *ctx) {
    if (ctx == NULL) return -1;
    
    int result = sim_start(ctx);
    if (result < 0) return result;
    
    while (ctx->state == SIM_STATE_RUNNING) {
        result = sim_step(ctx);
        if (result != 0) break;
    }
    
    return (ctx->state == SIM_STATE_COMPLETED) ? 0 : -1;
}

void sim_pause(sim_context_t *ctx) {
    if (ctx != NULL && ctx->state == SIM_STATE_RUNNING) {
        ctx->state = SIM_STATE_PAUSED;
    }
}

void sim_resume(sim_context_t *ctx) {
    if (ctx != NULL && ctx->state == SIM_STATE_PAUSED) {
        ctx->state = SIM_STATE_RUNNING;
    }
}

void sim_stop(sim_context_t *ctx) {
    if (ctx == NULL) return;
    
    if (ctx->recording.enable) {
        sim_data_close();
    }
    
    ctx->state = SIM_STATE_IDLE;
    g_active_context = NULL;
}

void sim_get_summary(sim_context_t *ctx, 
                     float *avg_speed,
                     float *avg_torque,
                     float *efficiency) {
    if (ctx == NULL) return;
    
    // Calculate averages from statistics
    if (avg_speed != NULL) {
        *avg_speed = (ctx->min_speed + ctx->max_speed) / 2.0f;
    }
    
    if (avg_torque != NULL) {
        *avg_torque = (ctx->min_iq + ctx->max_iq) / 2.0f * 
                      ctx->mc_conf.foc_motor_flux_linkage * 1.5f * 
                      (float)(ctx->mc_conf.si_motor_poles / 2);
    }
    
    if (efficiency != NULL) {
        if (ctx->total_energy_in > 0.0001f) {
            *efficiency = ctx->total_energy_out / ctx->total_energy_in * 100.0f;
        } else {
            *efficiency = 0.0f;
        }
    }
}

void sim_get_state(sim_context_t *ctx,
                   float *id, float *iq,
                   float *speed, float *position,
                   float *torque, float *power) {
    if (ctx == NULL) return;
    
    if (id != NULL) *id = ctx->foc_state.m_motor_state.id;
    if (iq != NULL) *iq = ctx->foc_state.m_motor_state.iq;
    if (speed != NULL) *speed = foc_get_erpm(&ctx->foc_state);
    if (position != NULL) *position = foc_get_position(&ctx->foc_state);
    if (torque != NULL) *torque = virtual_motor_pc_get_torque();
    if (power != NULL) {
        float omega_m = virtual_motor_pc_get_omega_e() / (float)ctx->mc_conf.si_motor_poles * 2.0f;
        *power = virtual_motor_pc_get_torque() * omega_m;
    }
}
