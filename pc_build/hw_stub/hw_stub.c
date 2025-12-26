/*
 * VESC Firmware PC Build - Hardware Stub Implementation
 * Phase 4: HAL/Peripheral Stubs
 */

#include "hw_stub.h"

// =============================================================================
// Hall Sensor Simulation
// =============================================================================

// Motor 1 hall sensor values
static int hall1 = 0;
static int hall2 = 0;
static int hall3 = 0;

// Motor 2 hall sensor values
static int hall1_2 = 0;
static int hall2_2 = 0;
static int hall3_2 = 0;

// Motor 1 hall sensor read functions
int hw_stub_read_hall1(void) {
    return hall1;
}

int hw_stub_read_hall2(void) {
    return hall2;
}

int hw_stub_read_hall3(void) {
    return hall3;
}

// Motor 2 hall sensor read functions
int hw_stub_read_hall1_2(void) {
    return hall1_2;
}

int hw_stub_read_hall2_2(void) {
    return hall2_2;
}

int hw_stub_read_hall3_2(void) {
    return hall3_2;
}

// Set hall sensor values for simulation/testing
void hw_stub_set_hall_values(int h1, int h2, int h3) {
    hall1 = h1;
    hall2 = h2;
    hall3 = h3;
}

void hw_stub_set_hall_values_2(int h1, int h2, int h3) {
    hall1_2 = h1;
    hall2_2 = h2;
    hall3_2 = h3;
}

// =============================================================================
// Hardware Initialization
// =============================================================================

void hw_stub_init(void) {
    // Reset hall sensor values
    hall1 = hall2 = hall3 = 0;
    hall1_2 = hall2_2 = hall3_2 = 0;
}
