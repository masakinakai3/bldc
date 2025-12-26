/*
 * VESC Firmware PC Build - conf_general.h stub
 * 
 * This file provides a minimal conf_general.h for PC build.
 * It replaces the real conf_general.h which requires hardware headers.
 */

#ifndef CONF_GENERAL_H_
#define CONF_GENERAL_H_

#include "datatypes.h"

// =============================================================================
// Firmware Version
// =============================================================================

#define FW_VERSION_MAJOR    6
#define FW_VERSION_MINOR    0  
#define FW_VERSION_PATCH    0

// Combined version for comparison
#define FW_VERSION          ((FW_VERSION_MAJOR << 16) | (FW_VERSION_MINOR << 8) | FW_VERSION_PATCH)

// =============================================================================
// Build Configuration
// =============================================================================

// Hardware header (stub for PC build)
#define HW_HEADER           "hw.h"
#define HW_NAME             "PC_STUB"

// =============================================================================
// Default Motor Configuration 
// =============================================================================

// Motor configuration defaults
#define MCCONF_DEFAULT_MOTOR_TYPE           MOTOR_TYPE_FOC
#define MCCONF_FOC_ENCODER_INVERTED         false
#define MCCONF_FOC_ENCODER_OFFSET           0.0f
#define MCCONF_FOC_ENCODER_RATIO            1.0f

// =============================================================================
// Application Configuration Defaults
// =============================================================================

#define APPCONF_CONTROLLER_ID               0
#define APPCONF_TIMEOUT_MSEC                1000
#define APPCONF_TIMEOUT_BRAKE_CURRENT       0.0f
#define APPCONF_CAN_STATUS_RATE_1           50
#define APPCONF_CAN_STATUS_RATE_2           5
#define APPCONF_CAN_BAUD_RATE               CAN_BAUD_500K
#define APPCONF_APP_TO_USE                  APP_NONE

// =============================================================================
// System Configuration
// =============================================================================

// Maximum number of motor configurations
#define MCCONF_NUM                          1

// Configuration signature
#define MCCONF_SIGNATURE                    0xDEADBEEF
#define APPCONF_SIGNATURE                   0xCAFEBABE

// =============================================================================
// Include Hardware Configuration
// =============================================================================

#include "hw.h"

#endif /* CONF_GENERAL_H_ */
