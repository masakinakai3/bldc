/*
 * VESC Firmware PC Build - Application Stub Header
 * Phase 4: HAL/Peripheral Stubs
 *
 * Provides stub implementations for application layer functions
 */

#ifndef APP_STUB_H_
#define APP_STUB_H_

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Test/Simulation API
// =============================================================================

/**
 * Set controller ID for testing
 * @param id Controller ID to set
 */
void app_stub_set_controller_id(uint8_t id);

/**
 * Reset application stub to default state
 */
void app_stub_reset(void);

#endif /* APP_STUB_H_ */
