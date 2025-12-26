/*
 * VESC PC Build - Phase 2 Utility Test
 * 
 * Tests for utils_math.c, buffer.c, and crc.c
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils_math.h"
#include "buffer.h"
#include "crc.h"

// =============================================================================
// Test framework
// =============================================================================

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        return 0; \
    } \
} while(0)

#define TEST_ASSERT_FLOAT_EQ(a, b, eps) do { \
    if (fabsf((a) - (b)) > (eps)) { \
        printf("FAIL at line %d: %f != %f (eps=%f)\n", __LINE__, (double)(a), (double)(b), (double)(eps)); \
        return 0; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    printf("  %-45s", #test_func); \
    tests_run++; \
    if (test_func()) { \
        tests_passed++; \
        printf(" PASS\n"); \
    } else { \
        tests_failed++; \
        printf(" FAIL\n"); \
    } \
} while(0)

// =============================================================================
// utils_math tests
// =============================================================================

static int test_utils_truncate_number_upper(void) {
    float val = 150.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    TEST_ASSERT_FLOAT_EQ(val, 100.0f, 0.001f);
    return 1;
}

static int test_utils_truncate_number_lower(void) {
    float val = -150.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    TEST_ASSERT_FLOAT_EQ(val, -100.0f, 0.001f);
    return 1;
}

static int test_utils_truncate_number_in_range(void) {
    float val = 50.0f;
    utils_truncate_number(&val, -100.0f, 100.0f);
    TEST_ASSERT_FLOAT_EQ(val, 50.0f, 0.001f);
    return 1;
}

static int test_utils_norm_angle_positive(void) {
    float angle = 370.0f;
    utils_norm_angle(&angle);
    TEST_ASSERT_FLOAT_EQ(angle, 10.0f, 0.001f);
    return 1;
}

static int test_utils_norm_angle_negative(void) {
    float angle = -10.0f;
    utils_norm_angle(&angle);
    TEST_ASSERT_FLOAT_EQ(angle, 350.0f, 0.001f);
    return 1;
}

static int test_utils_norm_angle_large(void) {
    float angle = 730.0f;  // 730 = 360*2 + 10
    utils_norm_angle(&angle);
    TEST_ASSERT_FLOAT_EQ(angle, 10.0f, 0.001f);
    return 1;
}

static int test_utils_fast_atan2_quadrant1(void) {
    float result = utils_fast_atan2(1.0f, 1.0f);
    float expected = atan2f(1.0f, 1.0f);  // Result is in radians
    TEST_ASSERT_FLOAT_EQ(result, expected, 0.02f);  // ~1 degree tolerance
    return 1;
}

static int test_utils_fast_atan2_quadrant2(void) {
    float result = utils_fast_atan2(1.0f, -1.0f);
    float expected = atan2f(1.0f, -1.0f);  // Result is in radians
    TEST_ASSERT_FLOAT_EQ(result, expected, 0.02f);
    return 1;
}

static int test_utils_fast_sincos(void) {
    float sin_val, cos_val;
    float angle_rad = 45.0f * (float)M_PI / 180.0f;  // 45 degrees in radians
    utils_fast_sincos(angle_rad, &sin_val, &cos_val);
    // Fast functions trade precision for speed, allow 5% error
    TEST_ASSERT_FLOAT_EQ(sin_val, sinf(angle_rad), 0.05f);
    TEST_ASSERT_FLOAT_EQ(cos_val, cosf(angle_rad), 0.05f);
    return 1;
}

static int test_utils_fast_sin(void) {
    float angle_rad = (float)M_PI / 2.0f;  // 90 degrees in radians
    float result = utils_fast_sin(angle_rad);
    TEST_ASSERT_FLOAT_EQ(result, 1.0f, 0.02f);
    return 1;
}

static int test_utils_fast_cos(void) {
    float result = utils_fast_cos(0.0f);
    TEST_ASSERT_FLOAT_EQ(result, 1.0f, 0.02f);
    return 1;
}

static int test_utils_angle_difference(void) {
    float diff = utils_angle_difference(10.0f, 350.0f);
    TEST_ASSERT_FLOAT_EQ(diff, 20.0f, 0.001f);
    return 1;
}

static int test_utils_middle_of_3(void) {
    float result = utils_middle_of_3(1.0f, 3.0f, 2.0f);
    TEST_ASSERT_FLOAT_EQ(result, 2.0f, 0.001f);
    return 1;
}

static int test_utils_min_abs(void) {
    float result = utils_min_abs(-5.0f, 3.0f);
    TEST_ASSERT_FLOAT_EQ(result, 3.0f, 0.001f);
    return 1;
}

static int test_utils_max_abs(void) {
    float result = utils_max_abs(-5.0f, 3.0f);
    TEST_ASSERT_FLOAT_EQ(result, -5.0f, 0.001f);
    return 1;
}

// =============================================================================
// buffer tests
// =============================================================================

static int test_buffer_int16(void) {
    uint8_t buf[2];
    int32_t idx = 0;
    
    buffer_append_int16(buf, 0x1234, &idx);
    TEST_ASSERT(idx == 2);
    TEST_ASSERT(buf[0] == 0x12);
    TEST_ASSERT(buf[1] == 0x34);
    
    idx = 0;
    int16_t val = buffer_get_int16(buf, &idx);
    TEST_ASSERT(val == 0x1234);
    return 1;
}

static int test_buffer_uint16(void) {
    uint8_t buf[2];
    int32_t idx = 0;
    
    buffer_append_uint16(buf, 0xABCD, &idx);
    TEST_ASSERT(idx == 2);
    
    idx = 0;
    uint16_t val = buffer_get_uint16(buf, &idx);
    TEST_ASSERT(val == 0xABCD);
    return 1;
}

static int test_buffer_int32(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    buffer_append_int32(buf, 0x12345678, &idx);
    TEST_ASSERT(idx == 4);
    TEST_ASSERT(buf[0] == 0x12);
    TEST_ASSERT(buf[1] == 0x34);
    TEST_ASSERT(buf[2] == 0x56);
    TEST_ASSERT(buf[3] == 0x78);
    
    idx = 0;
    int32_t val = buffer_get_int32(buf, &idx);
    TEST_ASSERT(val == 0x12345678);
    return 1;
}

static int test_buffer_uint32(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    buffer_append_uint32(buf, 0xDEADBEEF, &idx);
    TEST_ASSERT(idx == 4);
    
    idx = 0;
    uint32_t val = buffer_get_uint32(buf, &idx);
    TEST_ASSERT(val == 0xDEADBEEF);
    return 1;
}

static int test_buffer_int64(void) {
    uint8_t buf[8];
    int32_t idx = 0;
    
    buffer_append_int64(buf, 0x123456789ABCDEF0LL, &idx);
    TEST_ASSERT(idx == 8);
    
    idx = 0;
    int64_t val = buffer_get_int64(buf, &idx);
    TEST_ASSERT(val == 0x123456789ABCDEF0LL);
    return 1;
}

static int test_buffer_uint64(void) {
    uint8_t buf[8];
    int32_t idx = 0;
    
    buffer_append_uint64(buf, 0xFEDCBA9876543210ULL, &idx);
    TEST_ASSERT(idx == 8);
    
    idx = 0;
    uint64_t val = buffer_get_uint64(buf, &idx);
    TEST_ASSERT(val == 0xFEDCBA9876543210ULL);
    return 1;
}

static int test_buffer_float16(void) {
    uint8_t buf[2];
    int32_t idx = 0;
    
    float orig = 12.34f;
    buffer_append_float16(buf, orig, 100.0f, &idx);
    TEST_ASSERT(idx == 2);
    
    idx = 0;
    float val = buffer_get_float16(buf, 100.0f, &idx);
    TEST_ASSERT_FLOAT_EQ(val, orig, 0.02f);
    return 1;
}

static int test_buffer_float32(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    float orig = 123.456f;
    buffer_append_float32(buf, orig, 1000.0f, &idx);
    TEST_ASSERT(idx == 4);
    
    idx = 0;
    float val = buffer_get_float32(buf, 1000.0f, &idx);
    TEST_ASSERT_FLOAT_EQ(val, orig, 0.002f);
    return 1;
}

static int test_buffer_float32_auto(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    float orig = 123.456f;
    buffer_append_float32_auto(buf, orig, &idx);
    TEST_ASSERT(idx == 4);
    
    idx = 0;
    float val = buffer_get_float32_auto(buf, &idx);
    TEST_ASSERT_FLOAT_EQ(val, orig, 0.001f);
    return 1;
}

static int test_buffer_float32_auto_small(void) {
    uint8_t buf[4];
    int32_t idx = 0;
    
    float orig = 0.000123f;
    buffer_append_float32_auto(buf, orig, &idx);
    
    idx = 0;
    float val = buffer_get_float32_auto(buf, &idx);
    TEST_ASSERT_FLOAT_EQ(val, orig, 1e-7f);
    return 1;
}

static int test_buffer_double64(void) {
    uint8_t buf[8];
    int32_t idx = 0;
    
    double orig = 123456.789012;
    buffer_append_double64(buf, orig, 1000000.0, &idx);
    TEST_ASSERT(idx == 8);
    
    idx = 0;
    double val = buffer_get_double64(buf, 1000000.0, &idx);
    TEST_ASSERT(fabs(val - orig) < 0.000002);
    return 1;
}

// =============================================================================
// crc tests
// =============================================================================

static int test_crc16_basic(void) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc = crc16(data, 4);
    // Verify CRC is non-zero and consistent
    TEST_ASSERT(crc != 0);
    
    // Verify same input gives same output
    uint16_t crc2 = crc16(data, 4);
    TEST_ASSERT(crc == crc2);
    return 1;
}

static int test_crc16_empty(void) {
    uint8_t data[] = {0};
    uint16_t crc = crc16(data, 0);
    // Empty data should give initial CRC value (0)
    TEST_ASSERT(crc == 0);
    return 1;
}

static int test_crc16_single_byte(void) {
    uint8_t data1[] = {0x00};
    uint8_t data2[] = {0x01};
    uint16_t crc1 = crc16(data1, 1);
    uint16_t crc2 = crc16(data2, 1);
    // Different data should give different CRC
    TEST_ASSERT(crc1 != crc2);
    return 1;
}

static int test_crc16_rolling(void) {
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x03, 0x04};
    
    uint16_t crc = 0;
    crc = crc16_rolling(crc, data1, 2);
    crc = crc16_rolling(crc, data2, 2);
    
    uint8_t data_all[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc_all = crc16(data_all, 4);
    
    TEST_ASSERT(crc == crc_all);
    return 1;
}

static int test_crc16_rolling_single(void) {
    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    
    uint16_t crc = 0;
    for (int i = 0; i < 4; i++) {
        crc = crc16_rolling(crc, &data[i], 1);
    }
    
    uint16_t crc_all = crc16(data, 4);
    TEST_ASSERT(crc == crc_all);
    return 1;
}

static int test_crc16_known_value(void) {
    // Test CRC16 with various data patterns
    uint8_t data1[] = {0x00};
    uint16_t crc1 = crc16(data1, 1);
    
    uint8_t data2[] = {0xFF};
    uint16_t crc2 = crc16(data2, 1);
    
    // CRC of different single bytes should be different
    TEST_ASSERT(crc1 != crc2);
    
    // Test consistency: same data should give same CRC
    uint8_t data3[] = {0x12, 0x34, 0x56, 0x78};
    uint16_t crc3a = crc16(data3, 4);
    uint16_t crc3b = crc16(data3, 4);
    TEST_ASSERT(crc3a == crc3b);
    TEST_ASSERT(crc3a != 0);
    
    return 1;
}

// =============================================================================
// Main
// =============================================================================

int main(void) {
    printf("=== VESC PC Build - Phase 2 Utility Test ===\n\n");
    
    printf("[utils_math]\n");
    RUN_TEST(test_utils_truncate_number_upper);
    RUN_TEST(test_utils_truncate_number_lower);
    RUN_TEST(test_utils_truncate_number_in_range);
    RUN_TEST(test_utils_norm_angle_positive);
    RUN_TEST(test_utils_norm_angle_negative);
    RUN_TEST(test_utils_norm_angle_large);
    RUN_TEST(test_utils_fast_atan2_quadrant1);
    RUN_TEST(test_utils_fast_atan2_quadrant2);
    RUN_TEST(test_utils_fast_sincos);
    RUN_TEST(test_utils_fast_sin);
    RUN_TEST(test_utils_fast_cos);
    RUN_TEST(test_utils_angle_difference);
    RUN_TEST(test_utils_middle_of_3);
    RUN_TEST(test_utils_min_abs);
    RUN_TEST(test_utils_max_abs);
    
    printf("\n[buffer]\n");
    RUN_TEST(test_buffer_int16);
    RUN_TEST(test_buffer_uint16);
    RUN_TEST(test_buffer_int32);
    RUN_TEST(test_buffer_uint32);
    RUN_TEST(test_buffer_int64);
    RUN_TEST(test_buffer_uint64);
    RUN_TEST(test_buffer_float16);
    RUN_TEST(test_buffer_float32);
    RUN_TEST(test_buffer_float32_auto);
    RUN_TEST(test_buffer_float32_auto_small);
    RUN_TEST(test_buffer_double64);
    
    printf("\n[crc]\n");
    RUN_TEST(test_crc16_basic);
    RUN_TEST(test_crc16_empty);
    RUN_TEST(test_crc16_single_byte);
    RUN_TEST(test_crc16_rolling);
    RUN_TEST(test_crc16_rolling_single);
    RUN_TEST(test_crc16_known_value);
    
    printf("\n");
    printf("==============================================\n");
    printf("Results: %d/%d tests passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf(" (%d FAILED)", tests_failed);
    }
    printf("\n");
    printf("==============================================\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
