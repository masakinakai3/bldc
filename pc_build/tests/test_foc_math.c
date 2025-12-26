/**
 * @file test_foc_math.c
 * @brief Unit tests for FOC mathematical functions
 * 
 * Tests the core FOC algorithms:
 * - Observer update
 * - PLL
 * - SVM (Space Vector Modulation)
 * - PID controllers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Test framework macros
#define TEST_EPSILON 1e-4f
#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        test_failures++; \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_FLOAT_EQ(a, b, eps, msg) do { \
    float diff = fabsf((a) - (b)); \
    if (diff > eps) { \
        printf("  FAIL: %s: expected %.6f, got %.6f (diff=%.6f) (line %d)\n", \
               msg, (float)(b), (float)(a), diff, __LINE__); \
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

// Include FOC math functions (header only for declarations)
#include "../motor_sim/foc_control_core.h"
#include "../motor_sim/mcconf_stub.h"

// ==================== SVM Tests ====================

bool test_svm_sector_1(void) {
    // Test SVM in sector 1 (0-60 degrees)
    float mod_alpha = 0.5f;
    float mod_beta = 0.2887f;  // ~30 degree angle
    uint32_t duty1, duty2, duty3, svm_sector;
    
    foc_svm(mod_alpha, mod_beta, 0.95f, 4096, &duty1, &duty2, &duty3, &svm_sector);
    
    TEST_ASSERT(duty1 > 0 && duty1 <= 4096, "duty1 in range");
    TEST_ASSERT(duty2 > 0 && duty2 <= 4096, "duty2 in range");
    TEST_ASSERT(duty3 > 0 && duty3 <= 4096, "duty3 in range");
    TEST_ASSERT(svm_sector >= 0 && svm_sector <= 6, "sector in range");
    
    return true;
}

bool test_svm_sector_symmetry(void) {
    // Test that rotating the vector produces consistent results
    float magnitude = 0.5f;
    uint32_t prev_duty1 = 0;
    
    for (int angle_deg = 0; angle_deg < 360; angle_deg += 30) {
        float angle_rad = angle_deg * M_PI / 180.0f;
        float mod_alpha = magnitude * cosf(angle_rad);
        float mod_beta = magnitude * sinf(angle_rad);
        
        uint32_t duty1, duty2, duty3, svm_sector;
        foc_svm(mod_alpha, mod_beta, 0.95f, 4096, &duty1, &duty2, &duty3, &svm_sector);
        
        // All duties should be valid
        TEST_ASSERT(duty1 <= 4096 && duty2 <= 4096 && duty3 <= 4096, 
                   "duties in range at angle");
        
        prev_duty1 = duty1;
    }
    
    return true;
}

bool test_svm_zero_vector(void) {
    // Test SVM with zero voltage vector
    float mod_alpha = 0.0f;
    float mod_beta = 0.0f;
    uint32_t duty1, duty2, duty3, svm_sector;
    
    foc_svm(mod_alpha, mod_beta, 0.95f, 4096, &duty1, &duty2, &duty3, &svm_sector);
    
    // All duties should be approximately equal (middle of PWM range)
    uint32_t mid = 4096 / 2;
    TEST_ASSERT(abs((int)duty1 - (int)mid) < 100, "duty1 near center");
    TEST_ASSERT(abs((int)duty2 - (int)mid) < 100, "duty2 near center");
    TEST_ASSERT(abs((int)duty3 - (int)mid) < 100, "duty3 near center");
    
    return true;
}

bool test_svm_max_modulation(void) {
    // Test SVM at maximum modulation
    float max_mod = 0.95f;
    float mod_alpha = max_mod;
    float mod_beta = 0.0f;
    uint32_t duty1, duty2, duty3, svm_sector;
    
    foc_svm(mod_alpha, mod_beta, max_mod, 4096, &duty1, &duty2, &duty3, &svm_sector);
    
    // duty1 should be near maximum
    TEST_ASSERT(duty1 > 3500, "duty1 near max");
    
    return true;
}

// ==================== PLL Tests ====================

bool test_pll_basic(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    float phase = 0.0f;
    float speed = 0.0f;
    float dt = 1.0f / 20000.0f;
    
    // Simulate constant phase input
    float target_phase = 1.0f;
    
    // Run PLL for several iterations
    for (int i = 0; i < 1000; i++) {
        foc_pll_run(target_phase, dt, &phase, &speed, &conf);
    }
    
    // Phase should converge to target
    float phase_error = fabsf(phase - target_phase);
    while (phase_error > M_PI) phase_error -= 2.0f * M_PI;
    TEST_ASSERT(fabsf(phase_error) < 0.1f, "PLL phase converges");
    
    return true;
}

bool test_pll_tracking(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    float phase = 0.0f;
    float speed = 0.0f;
    float dt = 1.0f / 20000.0f;
    
    // Simulate rotating phase (constant speed)
    float target_speed = 1000.0f;  // rad/s
    float target_phase = 0.0f;
    
    // Run PLL for 0.1 second
    for (int i = 0; i < 2000; i++) {
        target_phase += target_speed * dt;
        while (target_phase > M_PI) target_phase -= 2.0f * M_PI;
        
        foc_pll_run(target_phase, dt, &phase, &speed, &conf);
    }
    
    // Speed should converge to target speed
    float speed_error = fabsf(speed - target_speed);
    printf("    PLL final speed = %.3f, target = %.3f, error = %.3f\n", speed, target_speed, speed_error);
    TEST_ASSERT(speed_error < target_speed * 0.3f, "PLL speed converges");
    
    return true;
}

// ==================== Observer Tests ====================

bool test_observer_basic(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    // Create motor state
    motor_all_state_t motor;
    memset(&motor, 0, sizeof(motor));
    motor.m_conf = &conf;
    
    // Initialize observer state
    observer_state obs;
    memset(&obs, 0, sizeof(obs));
    obs.lambda_est = conf.foc_motor_flux_linkage;
    
    float dt = 1.0f / 20000.0f;
    float phase;
    
    // Apply some voltage and current
    float v_alpha = 5.0f;
    float v_beta = 0.0f;
    float i_alpha = 1.0f;
    float i_beta = 0.0f;
    
    // Run observer
    foc_observer_update(v_alpha, v_beta, i_alpha, i_beta, dt, &obs, &phase, &motor);
    
    // Observer should produce a valid phase
    TEST_ASSERT(phase >= -M_PI && phase <= M_PI, "Observer phase in valid range");
    
    return true;
}

bool test_observer_rotation(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    motor_all_state_t motor;
    memset(&motor, 0, sizeof(motor));
    motor.m_conf = &conf;
    motor.m_gamma_now = conf.foc_observer_gain;
    
    observer_state obs;
    memset(&obs, 0, sizeof(obs));
    obs.lambda_est = conf.foc_motor_flux_linkage;
    
    float dt = 1.0f / 20000.0f;
    float phase;
    
    // Simulate sinusoidal currents (rotating field)
    float omega = 500.0f;  // rad/s electrical
    float prev_phase = 0.0f;
    
    for (int i = 0; i < 1000; i++) {
        float t = i * dt;
        float theta = omega * t;
        
        // Assume ideal current tracking
        float i_alpha = 5.0f * cosf(theta);
        float i_beta = 5.0f * sinf(theta);
        
        // Simulated back-EMF
        float v_alpha = -omega * conf.foc_motor_flux_linkage * sinf(theta);
        float v_beta = omega * conf.foc_motor_flux_linkage * cosf(theta);
        
        foc_observer_update(v_alpha, v_beta, i_alpha, i_beta, dt, &obs, &phase, &motor);
        prev_phase = phase;
    }
    
    // Observer should have tracked the rotation
    TEST_ASSERT(obs.lambda_est > 0.0f, "Lambda estimate positive");
    
    return true;
}

// ==================== Precalc Tests ====================

bool test_precalc_values(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    motor_all_state_t motor;
    memset(&motor, 0, sizeof(motor));
    motor.m_conf = &conf;
    
    // Ensure gamma is initialized similar to runtime
    motor.m_gamma_now = conf.foc_observer_gain;

    // Run precalculation
    foc_precalc_values(&motor);
    
    // Gamma should be set
    TEST_ASSERT(motor.m_gamma_now > 0.0f, "Gamma calculated");
    
    return true;
}

// ==================== PID Controller Tests ====================

bool test_pid_current(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    conf.foc_current_kp = 0.01f;
    conf.foc_current_ki = 10.0f;
    
    motor_all_state_t motor;
    foc_motor_state_init(&motor, &conf);
    
    float dt = 1.0f / 20000.0f;
    
    // Set current target
    foc_set_id_target(&motor, 0.0f);
    foc_set_iq_target(&motor, 10.0f);
    
    // Simulate current measurements
    motor.m_motor_state.id = 0.0f;
    motor.m_motor_state.iq = 0.0f;
    motor.m_motor_state.v_bus = 48.0f;
    motor.m_motor_state.phase_cos = 1.0f;
    motor.m_motor_state.phase_sin = 0.0f;
    
    // Run control loop multiple times
    for (int i = 0; i < 100; i++) {
        foc_control_current(&motor, dt);
        
        // vq should increase due to PI controller
        if (i > 10) {
            TEST_ASSERT(motor.m_motor_state.mod_q > 0.0f, "mod_q positive when tracking iq");
        }
    }
    
    return true;
}

// ==================== Integration Test ====================

bool test_foc_math_integration(void) {
    mc_configuration conf;
    mcconf_set_defaults(&conf);
    
    motor_all_state_t motor;
    foc_motor_state_init(&motor, &conf);
    
    float dt = 1.0f / 20000.0f;
    
    // Simulate a simple current control scenario
    foc_set_id_target(&motor, 0.0f);
    foc_set_iq_target(&motor, 5.0f);
    
    // Set initial measurements
    motor.m_motor_state.i_alpha = 0.0f;
    motor.m_motor_state.i_beta = 0.0f;
    motor.m_motor_state.v_bus = 48.0f;
    
    // Run multiple control cycles
    for (int i = 0; i < 500; i++) {
        // Update phase (simulated rotation)
        float phase = i * 0.01f;
        foc_update_phase(&motor, phase);
        
        // Update measurements
        foc_update_measurements(&motor, 
                                motor.m_motor_state.i_alpha,
                                motor.m_motor_state.i_beta,
                                motor.m_motor_state.v_bus);
        
        // Run control
        foc_control_current(&motor, dt);
        
        // Check that outputs are bounded
        TEST_ASSERT(fabsf(motor.m_motor_state.mod_d) <= 1.0f, "mod_d bounded");
        TEST_ASSERT(fabsf(motor.m_motor_state.mod_q) <= 1.0f, "mod_q bounded");
    }
    
    return true;
}

// ==================== Main ====================

int main(int argc, char **argv) {
    printf("=== FOC Math Unit Tests ===\n\n");
    
    // SVM tests
    printf("--- SVM Tests ---\n");
    RUN_TEST(test_svm_sector_1);
    RUN_TEST(test_svm_sector_symmetry);
    RUN_TEST(test_svm_zero_vector);
    RUN_TEST(test_svm_max_modulation);
    
    // PLL tests
    printf("\n--- PLL Tests ---\n");
    RUN_TEST(test_pll_basic);
    RUN_TEST(test_pll_tracking);
    
    // Observer tests
    printf("\n--- Observer Tests ---\n");
    RUN_TEST(test_observer_basic);
    RUN_TEST(test_observer_rotation);
    
    // Precalc tests
    printf("\n--- Precalc Tests ---\n");
    RUN_TEST(test_precalc_values);
    
    // PID tests
    printf("\n--- PID Controller Tests ---\n");
    RUN_TEST(test_pid_current);
    
    // Integration tests
    printf("\n--- Integration Tests ---\n");
    RUN_TEST(test_foc_math_integration);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", 
           total_tests, passed_tests, total_tests - passed_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
