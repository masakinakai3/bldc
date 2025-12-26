/*
 * VESC Firmware PC Build - Hardware Stub Header
 * Phase 4: HAL/Peripheral Stubs
 *
 * Provides stub definitions for hardware-dependent code
 */

#ifndef HW_STUB_H_
#define HW_STUB_H_

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Hall Sensor Stubs
// =============================================================================

// Motor 1 hall sensors
#define READ_HALL1()    hw_stub_read_hall1()
#define READ_HALL2()    hw_stub_read_hall2()
#define READ_HALL3()    hw_stub_read_hall3()

// Motor 2 hall sensors (dual motor support)
#define READ_HALL1_2()  hw_stub_read_hall1_2()
#define READ_HALL2_2()  hw_stub_read_hall2_2()
#define READ_HALL3_2()  hw_stub_read_hall3_2()

// Hall sensor stub functions
int hw_stub_read_hall1(void);
int hw_stub_read_hall2(void);
int hw_stub_read_hall3(void);
int hw_stub_read_hall1_2(void);
int hw_stub_read_hall2_2(void);
int hw_stub_read_hall3_2(void);

// Simulation: Set hall sensor values for testing
void hw_stub_set_hall_values(int h1, int h2, int h3);
void hw_stub_set_hall_values_2(int h1, int h2, int h3);

// =============================================================================
// Hardware Configuration
// =============================================================================

// Hardware name for PC build
#define HW_NAME     "PC_STUB"
#define HW_SOURCE   "hw_stub.h"

// Dual motor support (disabled by default for PC build)
// #define HW_HAS_DUAL_MOTORS

// =============================================================================
// Stack Alignment Type (ChibiOS compatible)
// =============================================================================

typedef uint32_t stkalign_t;

// =============================================================================
// ARM Intrinsics Stubs (for utils_sys.c compatibility)
// =============================================================================

// PSP register - not available on PC, return dummy value
#ifndef __get_PSP
#define __get_PSP() ((uint32_t)0)
#endif

// Port interrupt context is defined in ch.h for PC build

// =============================================================================
// GPIO Stubs (minimal for Phase 4)
// =============================================================================

// GPIO TypeDef stub (if not already defined)
#ifndef GPIO_TypeDef
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;
#endif

// =============================================================================
// Additional Hardware Stubs
// =============================================================================

// Watchdog (disabled for PC)
#define HW_HAS_NO_WATCHDOG

// CAN (optional, disabled by default)
// #define HW_HAS_NO_CAN

// Hardware initialization stub
void hw_stub_init(void);

#endif /* HW_STUB_H_ */
