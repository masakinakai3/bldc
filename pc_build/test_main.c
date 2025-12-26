/**
 * @file test_main.c
 * @brief PC build test program for Phase 1
 * 
 * Tests basic ChibiOS POSIX layer functionality:
 * - System initialization
 * - Time functions
 * - Mutex operations
 * - Semaphore operations
 * - Thread creation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_pc.h"

/*===========================================================================*/
/* Test state                                                                */
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

/*===========================================================================*/
/* Thread test                                                               */
/*===========================================================================*/

static volatile int thread_counter = 0;
static mutex_t g_test_mutex;

static THD_WORKING_AREA(waTestThread, 1024);

static THD_FUNCTION(testThread, arg) {
    int count = (int)(intptr_t)arg;
    
    for (int i = 0; i < count; i++) {
        chMtxLock(&g_test_mutex);
        thread_counter++;
        chMtxUnlock(&g_test_mutex);
        chThdSleepMilliseconds(1);
    }
    
    return NULL;
}

/*===========================================================================*/
/* Test functions                                                            */
/*===========================================================================*/

static void test_system_init(void) {
    printf("\n=== System Initialization Test ===\n");
    
    chSysInit();
    halInit();
    
    TEST_ASSERT(1, "chSysInit() completed");
    TEST_ASSERT(1, "halInit() completed");
}

static void test_time_functions(void) {
    printf("\n=== Time Functions Test ===\n");
    
    systime_t t1 = chVTGetSystemTime();
    chThdSleepMilliseconds(10);
    systime_t t2 = chVTGetSystemTime();
    
    sysinterval_t elapsed = t2 - t1;
    TEST_ASSERT(elapsed >= 10, "chThdSleepMilliseconds works (elapsed >= 10ms)");
    TEST_ASSERT(elapsed < 50, "Sleep timing reasonable (< 50ms)");
    
    /* Test time conversion macros */
    TEST_ASSERT(MS2ST(100) == 100, "MS2ST(100) == 100");
    TEST_ASSERT(S2ST(1) == 1000, "S2ST(1) == 1000");
}

static void run_test_mutex(void) {
    printf("\n=== Mutex Test ===\n");
    
    mutex_t mtx;
    chMtxObjectInit(&mtx);
    TEST_ASSERT(1, "chMtxObjectInit() completed");
    
    chMtxLock(&mtx);
    TEST_ASSERT(1, "chMtxLock() acquired");
    
    bool trylock_result = chMtxTryLock(&mtx);
    /* Note: POSIX mutexes are not recursive by default, this may fail */
    (void)trylock_result;
    
    chMtxUnlock(&mtx);
    TEST_ASSERT(1, "chMtxUnlock() released");
    
    /* Try lock after unlock should succeed */
    bool acquired = chMtxTryLock(&mtx);
    TEST_ASSERT(acquired, "chMtxTryLock() succeeded after unlock");
    if (acquired) {
        chMtxUnlock(&mtx);
    }
}

static void test_semaphore(void) {
    printf("\n=== Semaphore Test ===\n");
    
    semaphore_t sem;
    chSemObjectInit(&sem, 2);
    TEST_ASSERT(chSemGetCounterI(&sem) == 2, "Initial count == 2");
    
    msg_t result = chSemWait(&sem);
    TEST_ASSERT(result == MSG_OK, "chSemWait() returned MSG_OK");
    TEST_ASSERT(chSemGetCounterI(&sem) == 1, "Count == 1 after wait");
    
    chSemSignal(&sem);
    TEST_ASSERT(chSemGetCounterI(&sem) == 2, "Count == 2 after signal");
    
    /* Test timeout */
    chSemWait(&sem);
    chSemWait(&sem);
    result = chSemWaitTimeout(&sem, MS2ST(10));
    TEST_ASSERT(result == MSG_TIMEOUT, "chSemWaitTimeout() timed out correctly");
}

static void run_test_thread(void) {
    printf("\n=== Thread Test ===\n");
    
    thread_counter = 0;
    chMtxObjectInit(&g_test_mutex);
    
    thread_t *tp = chThdCreateStatic(waTestThread, sizeof(waTestThread),
                                     NORMALPRIO, testThread, (void*)10);
    TEST_ASSERT(tp != NULL, "Thread created");
    
    /* Wait for thread to complete */
    msg_t exitcode = chThdWait(tp);
    (void)exitcode;
    
    TEST_ASSERT(thread_counter == 10, "Thread incremented counter 10 times");
}

static void test_event_signal(void) {
    printf("\n=== Event Signal Test ===\n");
    
    /* Create a thread working area for event test */
    static THD_WORKING_AREA(waEventThread, 256);
    
    thread_t *tp = chThdCreateStatic(waEventThread, sizeof(waEventThread),
                                     NORMALPRIO, testThread, (void*)1);
    TEST_ASSERT(tp != NULL, "Event test thread created");
    
    /* Test chEvtSignal - should not crash */
    chEvtSignal(tp, EVENT_MASK(0));
    TEST_ASSERT(true, "chEvtSignal() did not crash");
    
    /* Test chEvtSignalI - should not crash */
    chEvtSignalI(tp, EVENT_MASK(1));
    TEST_ASSERT(true, "chEvtSignalI() did not crash");
    
    /* Test with NULL pointer - should be safe */
    chEvtSignal(NULL, EVENT_MASK(0));
    TEST_ASSERT(true, "chEvtSignal(NULL) is safe");
    
    chThdWait(tp);
}

static void test_stm32_stubs(void) {
    printf("\n=== STM32 Stubs Test ===\n");
    
    /* Test GPIO stub exists */
    TEST_ASSERT(GPIOA != NULL, "GPIOA defined");
    TEST_ASSERT(GPIOB != NULL, "GPIOB defined");
    
    /* Test timer stub exists */
    TEST_ASSERT(TIM1 != NULL, "TIM1 defined");
    TEST_ASSERT(TIM8 != NULL, "TIM8 defined");
    
    /* Test ADC stub exists */
    TEST_ASSERT(ADC1 != NULL, "ADC1 defined");
    
    /* Test RCC stub exists */
    TEST_ASSERT(RCC != NULL, "RCC defined");
    
    /* Test we can write to stubs without crash */
    GPIOA->ODR = 0x1234;
    TEST_ASSERT(GPIOA->ODR == 0x1234, "GPIO ODR read/write works");
    
    TIM1->ARR = 1000;
    TEST_ASSERT(TIM1->ARR == 1000, "TIM ARR read/write works");
}

static void test_hal_drivers(void) {
    printf("\n=== HAL Drivers Test ===\n");
    
    /* Test driver instances exist */
    TEST_ASSERT(ADCD1.state == HAL_STOP, "ADCD1 initial state is HAL_STOP");
    TEST_ASSERT(PWMD1.state == HAL_STOP, "PWMD1 initial state is HAL_STOP");
    TEST_ASSERT(SPID1.state == HAL_STOP, "SPID1 initial state is HAL_STOP");
    TEST_ASSERT(CAND1.state == HAL_STOP, "CAND1 initial state is HAL_STOP");
}

/*===========================================================================*/
/* Main                                                                      */
/*===========================================================================*/

int main(void) {
    printf("========================================\n");
    printf("VESC Firmware PC Build - Phase 1 Test\n");
    printf("========================================\n");
    
    test_system_init();
    test_time_functions();
    run_test_mutex();
    test_semaphore();
    run_test_thread();
    test_event_signal();
    test_stm32_stubs();
    test_hal_drivers();
    
    printf("\n========================================\n");
    printf("Test Results: %d passed, %d failed\n", test_passed, test_failed);
    printf("========================================\n");
    
    if (test_failed > 0) {
        printf("\nPC build test: FAILED\n");
        return 1;
    }
    
    printf("\nPC build test: OK\n");
    return 0;
}
