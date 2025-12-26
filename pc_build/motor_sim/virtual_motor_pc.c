/**
 * @file virtual_motor_pc.c
 * @brief PC-compatible virtual motor model implementation
 * 
 * Implementation based on motor/virtual_motor.c, adapted for PC simulation.
 * Implements a PMSM motor model with d-q axis dynamics.
 */

#include "virtual_motor_pc.h"
#include "utils_math.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Internal state
static virtual_motor_state_t vm;
static mc_configuration *m_conf = NULL;

// Forward declarations
static inline void run_electrical_model(float v_alpha, float v_beta, float dt);
static inline void run_mechanical_model(float dt);
static inline void update_transformations(void);

void virtual_motor_pc_init(mc_configuration *conf) {
    m_conf = conf;
    
    // Initialize parameters from configuration
    vm.pole_pairs = conf->si_motor_poles / 2;
    if (vm.pole_pairs < 1) vm.pole_pairs = 1;
    
    vm.km = 1.5f * (float)vm.pole_pairs;
    vm.Ts = 1.0f / conf->foc_f_zv;
    vm.R = conf->foc_motor_r;
    vm.lambda = conf->foc_motor_flux_linkage;
    
    // Handle inductance with possible saliency
    if (conf->foc_motor_ld_lq_diff > 0.0f) {
        vm.lq = conf->foc_motor_l + conf->foc_motor_ld_lq_diff / 2.0f;
        vm.ld = conf->foc_motor_l - conf->foc_motor_ld_lq_diff / 2.0f;
    } else {
        vm.lq = conf->foc_motor_l;
        vm.ld = conf->foc_motor_l;
    }
    
    // Ensure minimum inductance values
    if (vm.ld < 1e-9f) vm.ld = 1e-9f;
    if (vm.lq < 1e-9f) vm.lq = 1e-9f;
    
    // Default mechanical parameters
    vm.J = 0.0001f;  // Default inertia [kg·m²]
    vm.ml = 0.0f;    // No load torque
    
    // Reset state
    virtual_motor_pc_reset();
}

void virtual_motor_pc_set_configuration(mc_configuration *conf) {
    if (conf != NULL) {
        m_conf = conf;
        
        // Update parameters
        vm.pole_pairs = conf->si_motor_poles / 2;
        if (vm.pole_pairs < 1) vm.pole_pairs = 1;
        
        vm.km = 1.5f * (float)vm.pole_pairs;
        vm.R = conf->foc_motor_r;
        vm.lambda = conf->foc_motor_flux_linkage;
        
        if (conf->foc_motor_ld_lq_diff > 0.0f) {
            vm.lq = conf->foc_motor_l + conf->foc_motor_ld_lq_diff / 2.0f;
            vm.ld = conf->foc_motor_l - conf->foc_motor_ld_lq_diff / 2.0f;
        } else {
            vm.lq = conf->foc_motor_l;
            vm.ld = conf->foc_motor_l;
        }
        
        if (vm.ld < 1e-9f) vm.ld = 1e-9f;
        if (vm.lq < 1e-9f) vm.lq = 1e-9f;
    }
}

void virtual_motor_pc_step(virtual_motor_io_t *io, float dt) {
    if (io == NULL || dt <= 0.0f) return;
    
    vm.Ts = dt;
    
    // Run motor model
    run_electrical_model(io->v_alpha_in, io->v_beta_in, dt);
    run_mechanical_model(dt);
    update_transformations();
    
    // Inverse Park transform (d-q to α-β)
    io->i_alpha_out = vm.cos_phi * vm.id - vm.sin_phi * vm.iq;
    io->i_beta_out = vm.sin_phi * vm.id + vm.cos_phi * vm.iq;
    io->angle_rad = vm.phi;
    io->omega_e = vm.we;
    
    // Calculate electromagnetic torque
    io->torque = vm.km * (vm.lambda + (vm.ld - vm.lq) * vm.id) * vm.iq;
}

static inline void run_electrical_model(float v_alpha, float v_beta, float dt) {
    // Park transform (α-β to d-q)
    float vd = vm.cos_phi * v_alpha + vm.sin_phi * v_beta;
    float vq = vm.cos_phi * v_beta - vm.sin_phi * v_alpha;
    
    // d-axis current dynamics
    // di_d/dt = (v_d - R*i_d + w_e*L_q*i_q) / L_d
    // Using integral formulation for flux linkage:
    // psi_d = L_d * i_d + lambda_pm
    // d(psi_d)/dt = v_d - R*i_d + w_e*psi_q
    // where psi_q = L_q * i_q
    
    float we_pp = vm.we * (float)vm.pole_pairs;  // Electrical angular velocity
    
    vm.id_int += ((vd + we_pp * vm.lq * vm.iq - vm.R * vm.id) * dt) / vm.ld;
    vm.id = vm.id_int - vm.lambda / vm.ld;
    
    // q-axis current dynamics
    // di_q/dt = (v_q - R*i_q - w_e*(L_d*i_d + lambda_pm)) / L_q
    vm.iq += (vq - we_pp * (vm.ld * vm.id + vm.lambda) - vm.R * vm.iq) * dt / vm.lq;
    
    // Current limiting (prevent numerical instability)
    const float max_current = 500.0f;
    utils_truncate_number_abs(&vm.id, max_current);
    utils_truncate_number_abs(&vm.iq, max_current);
}

static inline void run_mechanical_model(float dt) {
    // Calculate electromagnetic torque
    // T_e = (3/2) * p * (lambda_pm * i_q + (L_d - L_q) * i_d * i_q)
    float me = vm.km * (vm.lambda + (vm.ld - vm.lq) * vm.id) * vm.iq;
    
    // Mechanical dynamics
    // J * dw/dt = T_e - T_load
    if (vm.J > 1e-12f) {
        float tsj = dt / vm.J;
        vm.we += tsj * (me - vm.ml);
    }
    
    // Update angle
    vm.phi += vm.we * dt;
    
    // Normalize angle to [-π, π]
    while (vm.phi > M_PI) {
        vm.phi -= 2.0f * M_PI;
    }
    while (vm.phi < -M_PI) {
        vm.phi += 2.0f * M_PI;
    }
}

static inline void update_transformations(void) {
    // Update sin/cos for transforms
    utils_fast_sincos_better(vm.phi, &vm.sin_phi, &vm.cos_phi);
}

void virtual_motor_pc_set_load_torque(float torque_nm) {
    vm.ml = torque_nm;
}

void virtual_motor_pc_set_inertia(float J_kgm2) {
    if (J_kgm2 > 1e-12f) {
        vm.J = J_kgm2;
    }
}

void virtual_motor_pc_set_angle(float angle_rad) {
    vm.phi = angle_rad;
    
    // Normalize
    while (vm.phi > M_PI) vm.phi -= 2.0f * M_PI;
    while (vm.phi < -M_PI) vm.phi += 2.0f * M_PI;
    
    utils_fast_sincos_better(vm.phi, &vm.sin_phi, &vm.cos_phi);
}

void virtual_motor_pc_set_speed(float omega_rad_s) {
    vm.we = omega_rad_s;
}

float virtual_motor_pc_get_rpm(void) {
    // Mechanical RPM = electrical_rad_s * 60 / (2*pi * pole_pairs)
    return vm.we * 60.0f / (2.0f * M_PI * (float)vm.pole_pairs);
}

float virtual_motor_pc_get_erpm(void) {
    // Electrical RPM = electrical_rad_s * 60 / (2*pi)
    return vm.we * 60.0f / (2.0f * M_PI);
}

float virtual_motor_pc_get_id(void) {
    return vm.id;
}

float virtual_motor_pc_get_iq(void) {
    return vm.iq;
}

float virtual_motor_pc_get_angle_rad(void) {
    return vm.phi;
}

float virtual_motor_pc_get_angle_deg(void) {
    return vm.phi * 180.0f / M_PI;
}

float virtual_motor_pc_get_torque(void) {
    return vm.km * (vm.lambda + (vm.ld - vm.lq) * vm.id) * vm.iq;
}

float virtual_motor_pc_get_omega_e(void) {
    return vm.we;
}

const virtual_motor_state_t* virtual_motor_pc_get_state(void) {
    return &vm;
}

void virtual_motor_pc_reset(void) {
    vm.id = 0.0f;
    vm.iq = 0.0f;
    vm.id_int = 0.0f;
    vm.we = 0.0f;
    vm.phi = 0.0f;
    vm.sin_phi = 0.0f;
    vm.cos_phi = 1.0f;
}
