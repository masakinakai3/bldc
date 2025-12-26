/*
 * VESC Firmware PC Build - Application Stub Implementation
 * Phase 4: HAL/Peripheral Stubs
 */

#include "datatypes.h"
#include "app_stub.h"
#include <string.h>
#include <stdbool.h>

// =============================================================================
// Private Variables
// =============================================================================

static app_configuration app_conf;
static bool initialized = false;
static bool output_disabled = false;
static int output_disable_time = 0;

// =============================================================================
// Private Functions
// =============================================================================

static void init_default_conf(void) {
    if (!initialized) {
        memset(&app_conf, 0, sizeof(app_conf));
        
        // Set default values
        app_conf.controller_id = 0;
        app_conf.timeout_msec = 1000;
        app_conf.timeout_brake_current = 0.0f;
        app_conf.can_status_rate_1 = 50;
        app_conf.can_status_rate_2 = 5;
        app_conf.can_baud_rate = CAN_BAUD_500K;
        app_conf.app_to_use = APP_NONE;
        
        initialized = true;
    }
}

// =============================================================================
// Public API Implementation
// =============================================================================

const app_configuration* app_get_configuration(void) {
    init_default_conf();
    return &app_conf;
}

void app_set_configuration(app_configuration *conf) {
    if (conf) {
        memcpy(&app_conf, conf, sizeof(app_conf));
        initialized = true;
    }
}

void app_disable_output(int time_ms) {
    output_disabled = true;
    output_disable_time = time_ms;
}

bool app_is_output_disabled(void) {
    return output_disabled;
}

// =============================================================================
// Test/Simulation API
// =============================================================================

void app_stub_set_controller_id(uint8_t id) {
    init_default_conf();
    app_conf.controller_id = id;
}

void app_stub_reset(void) {
    initialized = false;
    output_disabled = false;
    output_disable_time = 0;
    init_default_conf();
}
