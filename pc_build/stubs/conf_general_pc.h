/*
 * VESC Firmware PC Build - Configuration Header
 * Phase 4: HAL/Peripheral Stubs
 *
 * PC-specific configuration definitions
 */

#ifndef CONF_GENERAL_PC_H_
#define CONF_GENERAL_PC_H_

#include "datatypes.h"

// =============================================================================
// Firmware Version (PC Build Stub)
// =============================================================================

#define FW_VERSION_MAJOR    6
#define FW_VERSION_MINOR    0
#define FW_VERSION_PATCH    0

// =============================================================================
// PC Build Identification
// =============================================================================

#define PC_BUILD            1

// =============================================================================
// Default Configuration Values
// =============================================================================

// Controller ID range
#define APPCONF_CONTROLLER_ID_MIN   0
#define APPCONF_CONTROLLER_ID_MAX   254

// Default controller ID
#define APPCONF_CONTROLLER_ID       0

// =============================================================================
// Feature Flags for PC Build
// =============================================================================

// Disable hardware-specific features
#define DISABLE_HW_LIMITS

// Enable debug output
#define PC_DEBUG_OUTPUT

#endif /* CONF_GENERAL_PC_H_ */
