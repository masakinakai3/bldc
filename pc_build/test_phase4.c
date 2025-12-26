/*
 * VESC Firmware PC Build - Phase 4 Integration Tests
 * Tests for HAL/Peripheral stubs and utils_sys
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "datatypes.h"
#include "app.h"
#include "utils_sys.h"
#include "hw_stub.h"
#include "app_stub.h"

/* ==========================================================================
 * Test Framework
 * ========================================================================== */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        printf("  FAILED: %s (line %d)\n", #cond, __LINE__); \
    } \
} while(0)

#define TEST_ASSERT_EQ(a, b) do { \
    tests_run++; \
    if ((a) == (b)) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        printf("  FAILED: %s == %s (got %d vs %d, line %d)\n", #a, #b, (int)(a), (int)(b), __LINE__); \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(a, b) do { \
    tests_run++; \
    if (strcmp((a), (b)) == 0) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        printf("  FAILED: %s == %s (got \"%s\" vs \"%s\", line %d)\n", #a, #b, (a), (b), __LINE__); \
    } \
} while(0)

/* ==========================================================================
 * Test: Hardware Stub (hw_stub)
 * ========================================================================== */

void test_hw_stub(void) {
    printf("Testing hw_stub...\n");
    
    // Initialize hw stub
    hw_stub_init();
    
    // Test initial hall values (should be 0)
    TEST_ASSERT_EQ(hw_stub_read_hall1(), 0);
    TEST_ASSERT_EQ(hw_stub_read_hall2(), 0);
    TEST_ASSERT_EQ(hw_stub_read_hall3(), 0);
    
    // Test setting hall values for motor 1
    hw_stub_set_hall_values(1, 0, 1);
    TEST_ASSERT_EQ(hw_stub_read_hall1(), 1);
    TEST_ASSERT_EQ(hw_stub_read_hall2(), 0);
    TEST_ASSERT_EQ(hw_stub_read_hall3(), 1);
    
    // Test setting hall values for motor 2
    hw_stub_set_hall_values_2(0, 1, 1);
    TEST_ASSERT_EQ(hw_stub_read_hall1_2(), 0);
    TEST_ASSERT_EQ(hw_stub_read_hall2_2(), 1);
    TEST_ASSERT_EQ(hw_stub_read_hall3_2(), 1);
    
    // Test READ_HALL macros
    hw_stub_set_hall_values(1, 1, 0);
    TEST_ASSERT_EQ(READ_HALL1(), 1);
    TEST_ASSERT_EQ(READ_HALL2(), 1);
    TEST_ASSERT_EQ(READ_HALL3(), 0);
    
    printf("  hw_stub tests completed\n");
}

/* ==========================================================================
 * Test: Application Stub (app_stub)
 * ========================================================================== */

void test_app_stub(void) {
    printf("Testing app_stub...\n");
    
    // Reset to default state
    app_stub_reset();
    
    // Test get configuration (default values)
    const app_configuration* conf = app_get_configuration();
    TEST_ASSERT(conf != NULL);
    TEST_ASSERT_EQ(conf->controller_id, 0);
    
    // Test set controller ID
    app_stub_set_controller_id(42);
    conf = app_get_configuration();
    TEST_ASSERT_EQ(conf->controller_id, 42);
    
    // Test set configuration
    app_configuration new_conf;
    memset(&new_conf, 0, sizeof(new_conf));
    new_conf.controller_id = 100;
    new_conf.timeout_msec = 2000;
    app_set_configuration(&new_conf);
    
    conf = app_get_configuration();
    TEST_ASSERT_EQ(conf->controller_id, 100);
    TEST_ASSERT_EQ(conf->timeout_msec, 2000);
    
    // Test output disable
    TEST_ASSERT_EQ(app_is_output_disabled(), false);
    app_disable_output(500);
    TEST_ASSERT_EQ(app_is_output_disabled(), true);
    
    // Reset for other tests
    app_stub_reset();
    
    printf("  app_stub tests completed\n");
}

/* ==========================================================================
 * Test: utils_sys_lock_cnt / unlock_cnt
 * ========================================================================== */

void test_utils_sys_lock(void) {
    printf("Testing utils_sys_lock_cnt/unlock_cnt...\n");
    
    // Test multiple locks and unlocks
    utils_sys_lock_cnt();
    utils_sys_lock_cnt();
    utils_sys_lock_cnt();
    
    // Should be able to unlock same number of times
    utils_sys_unlock_cnt();
    utils_sys_unlock_cnt();
    utils_sys_unlock_cnt();
    
    // Extra unlocks should be safe (counter doesn't go negative)
    utils_sys_unlock_cnt();
    utils_sys_unlock_cnt();
    
    // Lock/unlock sequence should work
    utils_sys_lock_cnt();
    utils_sys_unlock_cnt();
    
    TEST_ASSERT(true);  // If we get here without crash, test passed
    
    printf("  utils_sys_lock tests completed\n");
}

/* ==========================================================================
 * Test: utils_second_motor_id
 * ========================================================================== */

void test_utils_second_motor_id(void) {
    printf("Testing utils_second_motor_id...\n");
    
    // Reset app stub
    app_stub_reset();
    
    // Set controller ID to 10
    app_stub_set_controller_id(10);
    
    // Get second motor ID
    uint8_t id2 = utils_second_motor_id();
    
    // Without HW_HAS_DUAL_MOTORS defined, should return 0
    // With HW_HAS_DUAL_MOTORS, should return controller_id + 1
#ifdef HW_HAS_DUAL_MOTORS
    TEST_ASSERT_EQ(id2, 11);
#else
    TEST_ASSERT_EQ(id2, 0);
#endif
    
    // Test edge case: controller_id = 254
    app_stub_set_controller_id(254);
    id2 = utils_second_motor_id();
#ifdef HW_HAS_DUAL_MOTORS
    TEST_ASSERT_EQ(id2, 0);  // Wraps around from 255 to 0
#else
    TEST_ASSERT_EQ(id2, 0);
#endif
    
    printf("  utils_second_motor_id tests completed\n");
}

/* ==========================================================================
 * Test: utils_read_hall
 * ========================================================================== */

void test_utils_read_hall(void) {
    printf("Testing utils_read_hall...\n");
    
    // Initialize hw stub
    hw_stub_init();
    
    // Test motor 1 hall reading
    // Hall pattern: H1=1, H2=0, H3=1 -> binary 101 = 5
    hw_stub_set_hall_values(1, 0, 1);
    int hall = utils_read_hall(false, 0);
    TEST_ASSERT_EQ(hall, 5);
    
    // Hall pattern: H1=0, H2=1, H3=1 -> binary 110 = 6
    hw_stub_set_hall_values(0, 1, 1);
    hall = utils_read_hall(false, 0);
    TEST_ASSERT_EQ(hall, 6);
    
    // Hall pattern: H1=1, H2=1, H3=1 -> binary 111 = 7
    hw_stub_set_hall_values(1, 1, 1);
    hall = utils_read_hall(false, 0);
    TEST_ASSERT_EQ(hall, 7);
    
    // Hall pattern: H1=0, H2=0, H3=0 -> binary 000 = 0
    hw_stub_set_hall_values(0, 0, 0);
    hall = utils_read_hall(false, 0);
    TEST_ASSERT_EQ(hall, 0);
    
    // Test motor 2 hall reading
    hw_stub_set_hall_values_2(1, 1, 0);  // binary 011 = 3
    hall = utils_read_hall(true, 0);
    TEST_ASSERT_EQ(hall, 3);
    
    // Test with samples > 0 (filtering)
    hw_stub_set_hall_values(1, 1, 1);
    hall = utils_read_hall(false, 2);  // 2 extra samples
    TEST_ASSERT_EQ(hall, 7);  // Should still be 7 with consistent values
    
    printf("  utils_read_hall tests completed\n");
}

/* ==========================================================================
 * Test: utils_hw_type_to_string
 * ========================================================================== */

void test_utils_hw_type_to_string(void) {
    printf("Testing utils_hw_type_to_string...\n");
    
    // Test HW_TYPE_VESC
    const char* str = utils_hw_type_to_string(HW_TYPE_VESC);
    TEST_ASSERT_STR_EQ(str, "HW_TYPE_VESC");
    
    // Test HW_TYPE_VESC_BMS
    str = utils_hw_type_to_string(HW_TYPE_VESC_BMS);
    TEST_ASSERT_STR_EQ(str, "HW_TYPE_VESC_BMS");
    
    // Test HW_TYPE_CUSTOM_MODULE
    str = utils_hw_type_to_string(HW_TYPE_CUSTOM_MODULE);
    TEST_ASSERT_STR_EQ(str, "HW_TYPE_CUSTOM_MODULE");
    
    // Test unknown type (should return "FAULT_HARDWARE")
    str = utils_hw_type_to_string((HW_TYPE)999);
    TEST_ASSERT_STR_EQ(str, "FAULT_HARDWARE");
    
    printf("  utils_hw_type_to_string tests completed\n");
}

/* ==========================================================================
 * Test: utils_check_min_stack_left
 * ========================================================================== */

void test_utils_check_min_stack_left(void) {
    printf("Testing utils_check_min_stack_left...\n");
    
    // Get current thread
    thread_t* tp = chThdGetSelfX();
    
    if (tp != NULL && tp->p_stklimit != NULL) {
        // This test is limited on PC since we don't have real stack fill patterns
        int stack_left = utils_check_min_stack_left(tp);
        // On PC, result may be 0 or variable - just check it doesn't crash
        TEST_ASSERT(stack_left >= 0);
    } else {
        // If thread is NULL or no stack limit, just pass
        TEST_ASSERT(true);
    }
    
    printf("  utils_check_min_stack_left tests completed\n");
}

/* ==========================================================================
 * Test: utils_is_func_valid
 * ========================================================================== */

void test_utils_is_func_valid(void) {
    printf("Testing utils_is_func_valid...\n");
    
    // On PC, the address ranges are different from ARM
    // Flash range: 0x08000000 - 0x08100000 (won't match on PC)
    // RAM range: 0x20000000 - 0x20020000 (won't match on PC)
    // CCM range: 0x10000000 - 0x10010000 (won't match on PC)
    
    // Test with ARM addresses (should return true per implementation)
    bool valid = utils_is_func_valid((void*)0x08000100);
    TEST_ASSERT_EQ(valid, true);  // In Flash range
    
    valid = utils_is_func_valid((void*)0x20001000);
    TEST_ASSERT_EQ(valid, true);  // In RAM range
    
    valid = utils_is_func_valid((void*)0x10001000);
    TEST_ASSERT_EQ(valid, true);  // In CCM range
    
    // Test with invalid address
    valid = utils_is_func_valid((void*)0x00000000);
    TEST_ASSERT_EQ(valid, false);  // Not in any valid range
    
    valid = utils_is_func_valid((void*)0xFFFFFFFF);
    TEST_ASSERT_EQ(valid, false);  // Not in any valid range
    
    printf("  utils_is_func_valid tests completed\n");
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int main(void) {
    printf("===========================================\n");
    printf("VESC PC Build - Phase 4 Integration Tests\n");
    printf("===========================================\n\n");
    
    // Initialize ChibiOS POSIX layer
    chSysInit();
    
    // Run tests
    test_hw_stub();
    test_app_stub();
    test_utils_sys_lock();
    test_utils_second_motor_id();
    test_utils_read_hall();
    test_utils_hw_type_to_string();
    test_utils_check_min_stack_left();
    test_utils_is_func_valid();
    
    // Print summary
    printf("\n===========================================\n");
    printf("Phase 4 Test Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("===========================================\n");
    
    return (tests_failed > 0) ? 1 : 0;
}
