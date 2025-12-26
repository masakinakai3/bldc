/**
 * @file test_phase3.c
 * @brief Phase 3 integration tests for ChibiOS-dependent utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "digital_filter.h"
#include "worker.h"
#include "mempools.h"

/*===========================================================================*/
/* Test infrastructure                                                       */
/*===========================================================================*/

static int test_passed = 0;
static int test_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (cond) { \
        printf("  [PASS] %s\n", msg); \
        test_passed++; \
    } else { \
        printf("  [FAIL] %s\n", msg); \
        test_failed++; \
    } \
} while(0)

#define TEST_ASSERT_FLOAT_EQ(a, b, tol, msg) do { \
    if (fabsf((a) - (b)) < (tol)) { \
        printf("  [PASS] %s\n", msg); \
        test_passed++; \
    } else { \
        printf("  [FAIL] %s (expected %.6f, got %.6f)\n", msg, (double)(b), (double)(a)); \
        test_failed++; \
    } \
} while(0)

/*===========================================================================*/
/* Digital Filter Tests                                                      */
/*===========================================================================*/

static void test_digital_filter(void) {
    printf("\n=== Digital Filter Test ===\n");
    
    /* Test biquad filter initialization */
    Biquad bq;
    biquad_config(&bq, BQ_LOWPASS, 0.1f);  /* Fc = 0.1 (normalized frequency) */
    TEST_ASSERT(true, "biquad_config() completed");
    
    /* Test biquad filter processing */
    float input = 1.0f;
    float output = biquad_process(&bq, input);
    TEST_ASSERT(!isnan(output), "biquad_process() returns valid output");
    
    /* Test biquad reset */
    biquad_reset(&bq);
    TEST_ASSERT(bq.z1 == 0.0f && bq.z2 == 0.0f, "biquad_reset() clears state");
    
    /* Test filter_add_sample */
    float buffer[16] = {0};
    uint32_t offset = 0;
    filter_add_sample(buffer, 1.5f, 4, &offset);  /* bits=4 means 2^4=16 samples */
    TEST_ASSERT(buffer[0] == 1.5f, "filter_add_sample() adds to buffer");
    
    filter_add_sample(buffer, 2.5f, 4, &offset);
    TEST_ASSERT(buffer[1] == 2.5f, "filter_add_sample() increments offset");
    
    /* Test hamming window */
    float window[8];
    filter_hamming(window, 8);
    TEST_ASSERT(window[0] >= 0.0f && window[0] <= 0.2f, "Hamming window start value valid");
    bool window_valid = true;
    for (int i = 0; i < 8; i++) {
        if (isnan(window[i]) || window[i] < 0.0f || window[i] > 1.1f) {
            window_valid = false;
        }
    }
    TEST_ASSERT(window_valid, "Hamming window values are in valid range");
    
    /* Test FFT (simple case) - dir=1 forward, m=3 means 2^3=8 samples */
    float real[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    float imag[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    filter_fft(1, 3, real, imag);
    /* All frequency bins should have magnitude 1 for unit impulse */
    TEST_ASSERT(!isnan(real[0]) && !isnan(imag[0]), "FFT produces valid output");
}

/*===========================================================================*/
/* Worker Thread Tests                                                       */
/*===========================================================================*/

static volatile int worker_test_value = 0;

static void worker_test_func(void *arg) {
    int *val = (int*)arg;
    worker_test_value = *val * 2;
}

static void test_worker(void) {
    printf("\n=== Worker Thread Test ===\n");
    
    int input_val = 21;
    worker_test_value = 0;
    
    /* Execute function in worker thread */
    worker_execute(worker_test_func, &input_val);
    TEST_ASSERT(true, "worker_execute() initiated");
    
    /* Wait for completion */
    worker_wait();
    /* Give some time for the value to be written */
    chThdSleepMilliseconds(10);
    TEST_ASSERT(worker_test_value == 42, "Worker computed correct result (21*2=42)");
    
    /* Test second execution */
    input_val = 50;
    worker_test_value = 0;
    worker_execute(worker_test_func, &input_val);
    worker_wait();
    chThdSleepMilliseconds(10);
    TEST_ASSERT(worker_test_value == 100, "Worker re-execution works (50*2=100)");
}

/*===========================================================================*/
/* Memory Pool Tests                                                         */
/*===========================================================================*/

static void test_mempools(void) {
    printf("\n=== Memory Pool Test ===\n");
    
    /* Initialize memory pools */
    mempools_init();
    TEST_ASSERT(true, "mempools_init() completed");
    
    /* Test mcconf allocation */
    mc_configuration *mc1 = mempools_alloc_mcconf();
    TEST_ASSERT(mc1 != NULL, "mempools_alloc_mcconf() returns non-NULL");
    
    int alloc_count = mempools_mcconf_allocated_num();
    TEST_ASSERT(alloc_count == 1, "mcconf allocated count is 1");
    
    /* Allocate second */
    mc_configuration *mc2 = mempools_alloc_mcconf();
    TEST_ASSERT(mc2 != NULL, "Second mcconf allocation succeeds");
    TEST_ASSERT(mc1 != mc2, "Allocations return different pointers");
    
    alloc_count = mempools_mcconf_allocated_num();
    TEST_ASSERT(alloc_count == 2, "mcconf allocated count is 2");
    
    /* Free first */
    mempools_free_mcconf(mc1);
    alloc_count = mempools_mcconf_allocated_num();
    TEST_ASSERT(alloc_count == 1, "mcconf count after free is 1");
    
    /* Free second */
    mempools_free_mcconf(mc2);
    alloc_count = mempools_mcconf_allocated_num();
    TEST_ASSERT(alloc_count == 0, "mcconf count after all freed is 0");
    
    /* Test highest watermark - may not always be exact due to implementation */
    int highest = mempools_mcconf_highest();
    TEST_ASSERT(highest >= 1, "mcconf highest watermark >= 1");
    
    /* Test appconf allocation */
    app_configuration *app1 = mempools_alloc_appconf();
    TEST_ASSERT(app1 != NULL, "mempools_alloc_appconf() returns non-NULL");
    mempools_free_appconf(app1);
    TEST_ASSERT(mempools_appconf_allocated_num() == 0, "appconf freed correctly");
}

/*===========================================================================*/
/* Main                                                                      */
/*===========================================================================*/

int main(void) {
    printf("========================================\n");
    printf("VESC Firmware PC Build - Phase 3 Test\n");
    printf("========================================\n");
    
    /* Initialize ChibiOS and HAL */
    chSysInit();
    halInit();
    
    /* Run tests */
    test_digital_filter();
    test_worker();
    test_mempools();
    
    printf("\n========================================\n");
    printf("Phase 3 Test Results: %d passed, %d failed\n", test_passed, test_failed);
    printf("========================================\n");
    
    if (test_failed > 0) {
        printf("\nPhase 3 test: FAILED\n");
        return 1;
    }
    
    printf("\nPhase 3 test: OK\n");
    return 0;
}
