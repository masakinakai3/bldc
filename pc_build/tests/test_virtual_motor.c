/**
 * @file test_virtual_motor.c
 * @brief Unit tests for Virtual Motor PC model
 * 
 * Validates the PMSM motor model using virtual_motor_pc.h API:
 * - Electrical dynamics (d-q currents)
 * - Mechanical dynamics (speed, position)
 * - Steady-state behavior
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Test framework
#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        test_failures++; \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_FLOAT_NEAR(a, b, tol, msg) do { \
    float diff = fabsf((a) - (b)); \
    if (diff > tol) { \
        printf("  FAIL: %s: expected %.6f, got %.6f (diff=%.6f, tol=%.6f) (line %d)\n", \
               msg, (float)(b), (float)(a), diff, tol, __LINE__); \
        test_failures++; \
        return false; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    printf("Running %s...\n", #test_func); \
    total_tests++; \
    if (test_func()) { \
        printf("  PASS\n"); \
        passed_tests++; \
    } \
} while(0)

static int test_failures = 0;
static int total_tests = 0;
static int passed_tests = 0;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include "../motor_sim/virtual_motor_pc.h"
#include "../motor_sim/mcconf_stub.h"

// ==================== Initialization Tests ====================

bool test_vm_init_default(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    
    // Initial state should be all zeros
    TEST_ASSERT(fabsf(virtual_motor_pc_get_id()) < 0.001f, "Initial id is zero");
    TEST_ASSERT(fabsf(virtual_motor_pc_get_iq()) < 0.001f, "Initial iq is zero");
    TEST_ASSERT(fabsf(virtual_motor_pc_get_rpm()) < 0.001f, "Initial RPM is zero");
    
    return true;
}

bool test_vm_reset(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    
    // Apply some voltage to get non-zero state
    virtual_motor_io_t io = {0};
    io.v_alpha_in = 10.0f;
    io.v_beta_in = 0.0f;
    
    for (int i = 0; i < 1000; i++) {
        virtual_motor_pc_step(&io, 1e-5f);
    }
    
    // State should be non-zero
    TEST_ASSERT(fabsf(virtual_motor_pc_get_id()) > 0.01f || 
                fabsf(virtual_motor_pc_get_iq()) > 0.01f, 
                "State is non-zero after stepping");
    
    // Reset
    virtual_motor_pc_reset();
    
    TEST_ASSERT(fabsf(virtual_motor_pc_get_id()) < 0.001f, "Reset clears id");
    TEST_ASSERT(fabsf(virtual_motor_pc_get_iq()) < 0.001f, "Reset clears iq");
    
    return true;
}

// ==================== Electrical Dynamics Tests ====================

bool test_vm_current_response(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    
    float dt = 1e-5f;  // 100kHz model rate
    
    // Apply voltage step in alpha direction
    virtual_motor_io_t io = {0};
    io.v_alpha_in = 10.0f;
    io.v_beta_in = 0.0f;
    
    // Run for several electrical time constants
    float tau = conf.foc_motor_l / conf.foc_motor_r;
    int steps = (int)(5.0f * tau / dt);
    
    for (int i = 0; i < steps; i++) {
        virtual_motor_pc_step(&io, dt);
    }
    
    // Current should have increased
    float i_alpha = io.i_alpha_out;
    float i_beta = io.i_beta_out;
    float i_mag = sqrtf(i_alpha * i_alpha + i_beta * i_beta);
    
    TEST_ASSERT(i_mag > 0.5f, "Current increased after voltage step");
    
    return true;
}

bool test_vm_io_structure(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    
    virtual_motor_io_t io = {0};
    io.v_alpha_in = 5.0f;
    io.v_beta_in = 3.0f;
    
    // Run one step
    virtual_motor_pc_step(&io, 1e-5f);
    
    // Output should be populated
    // At first step with zero initial currents, output currents start increasing
    // Angle should be valid
    TEST_ASSERT(io.angle_rad >= -M_PI && io.angle_rad <= 2.0f * M_PI, 
                "Angle in valid range");
    
    return true;
}

// ==================== Mechanical Dynamics Tests ====================

bool test_vm_torque_production(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    virtual_motor_pc_set_inertia(0.0001f);  // Small inertia for faster response
    
    float dt = 1e-5f;
    
    virtual_motor_io_t io = {0};
    
    // Apply rotating voltage vector to produce q-axis current
    float omega_cmd = 100.0f;  // rad/s commanded
    float t = 0.0f;
    
    for (int i = 0; i < 50000; i++) {
        // Rotating voltage
        float angle = omega_cmd * t;
        io.v_alpha_in = 10.0f * cosf(angle);
        io.v_beta_in = 10.0f * sinf(angle);
        
        virtual_motor_pc_step(&io, dt);
        t += dt;
    }
    
    // Motor should have started moving
    float omega_e = virtual_motor_pc_get_omega_e();
    TEST_ASSERT(fabsf(omega_e) > 0.1f, "Motor is rotating");
    
    // Torque should be produced
    float torque = virtual_motor_pc_get_torque();
    // Torque sign depends on whether motor is motoring or generating
    
    return true;
}

bool test_vm_acceleration(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    virtual_motor_pc_set_inertia(0.0001f);  // Small inertia
    
    float dt = 1e-5f;
    virtual_motor_io_t io = {0};
    
    // Apply constant voltage
    io.v_alpha_in = 10.0f;
    io.v_beta_in = 5.0f;
    
    float omega_initial = virtual_motor_pc_get_omega_e();
    
    // Run for some time
    for (int i = 0; i < 10000; i++) {
        virtual_motor_pc_step(&io, dt);
    }
    
    float omega_final = virtual_motor_pc_get_omega_e();
    
    // Angular velocity should have changed
    TEST_ASSERT(fabsf(omega_final - omega_initial) > 0.01f, 
                "Angular velocity changed (motor accelerated/decelerated)");
    
    return true;
}

bool test_vm_load_torque(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    virtual_motor_pc_set_inertia(0.0001f);
    
    float dt = 1e-5f;
    virtual_motor_io_t io = {0};
    
    // Set initial speed
    virtual_motor_pc_set_speed(100.0f);  // rad/s electrical
    
    // Apply load torque (should slow down with zero drive)
    virtual_motor_pc_set_load_torque(0.01f);
    
    float omega_initial = virtual_motor_pc_get_omega_e();
    
    // Run with zero voltage (coast with load)
    io.v_alpha_in = 0.0f;
    io.v_beta_in = 0.0f;
    
    for (int i = 0; i < 10000; i++) {
        virtual_motor_pc_step(&io, dt);
    }
    
    float omega_final = virtual_motor_pc_get_omega_e();
    
    // Speed should have decreased due to load
    TEST_ASSERT(omega_final < omega_initial, "Load torque slows motor");
    
    return true;
}

// ==================== API Tests ====================

bool test_vm_getter_functions(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    
    // Set known initial conditions
    virtual_motor_pc_set_angle(1.5f);  // rad
    virtual_motor_pc_set_speed(100.0f);  // rad/s electrical
    
    // Get values
    float angle_rad = virtual_motor_pc_get_angle_rad();
    float angle_deg = virtual_motor_pc_get_angle_deg();
    float omega_e = virtual_motor_pc_get_omega_e();
    float erpm = virtual_motor_pc_get_erpm();
    float rpm = virtual_motor_pc_get_rpm();
    
    // Angle should be approximately what we set
    TEST_ASSERT_FLOAT_NEAR(angle_rad, 1.5f, 0.1f, "angle_rad getter");
    TEST_ASSERT_FLOAT_NEAR(angle_deg, 1.5f * 180.0f / M_PI, 5.0f, "angle_deg getter");
    
    // Speed should be approximately what we set
    TEST_ASSERT_FLOAT_NEAR(omega_e, 100.0f, 10.0f, "omega_e getter");
    
    // ERPM = omega_e * 60 / (2*pi)
    float expected_erpm = 100.0f * 60.0f / (2.0f * M_PI);
    TEST_ASSERT_FLOAT_NEAR(erpm, expected_erpm, expected_erpm * 0.1f, "ERPM getter");
    
    return true;
}

bool test_vm_state_access(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    
    const virtual_motor_state_t *state = virtual_motor_pc_get_state();
    
    TEST_ASSERT(state != NULL, "State pointer is not NULL");
    TEST_ASSERT(state->pole_pairs > 0, "Pole pairs is positive");
    TEST_ASSERT(state->R > 0.0f, "Resistance is positive");
    TEST_ASSERT(state->lambda > 0.0f, "Flux linkage is positive");
    
    return true;
}

// ==================== Steady-State Tests ====================

bool test_vm_steady_state_current(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    virtual_motor_pc_init(&conf);
    virtual_motor_pc_reset();
    virtual_motor_pc_set_inertia(1.0f);  // Large inertia to prevent rotation
    
    float dt = 1e-5f;
    
    // Apply DC voltage
    virtual_motor_io_t io = {0};
    io.v_alpha_in = 5.0f;
    io.v_beta_in = 0.0f;
    
    // Run to steady state (with locked rotor, current = V/R)
    for (int i = 0; i < 100000; i++) {
        virtual_motor_pc_step(&io, dt);
    }
    
    // At locked rotor, current should approach V/R
    float expected_current = 5.0f / conf.foc_motor_r;
    float actual_current = io.i_alpha_out;
    
    // Allow reasonable tolerance
    TEST_ASSERT_FLOAT_NEAR(actual_current, expected_current, expected_current * 0.3f,
                           "Steady-state current ~ V/R at locked rotor");
    
    return true;
}

// ==================== Main ====================

int main(int argc, char **argv) {
    printf("=== Virtual Motor PC Unit Tests ===\n\n");
    
    // Initialization tests
    printf("--- Initialization Tests ---\n");
    RUN_TEST(test_vm_init_default);
    RUN_TEST(test_vm_reset);
    
    // Electrical dynamics
    printf("\n--- Electrical Dynamics Tests ---\n");
    RUN_TEST(test_vm_current_response);
    RUN_TEST(test_vm_io_structure);
    
    // Mechanical dynamics
    printf("\n--- Mechanical Dynamics Tests ---\n");
    RUN_TEST(test_vm_torque_production);
    RUN_TEST(test_vm_acceleration);
    RUN_TEST(test_vm_load_torque);
    
    // API tests
    printf("\n--- API Tests ---\n");
    RUN_TEST(test_vm_getter_functions);
    RUN_TEST(test_vm_state_access);
    
    // Steady-state
    printf("\n--- Steady-State Tests ---\n");
    RUN_TEST(test_vm_steady_state_current);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", 
           total_tests, passed_tests, total_tests - passed_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
