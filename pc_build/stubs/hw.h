/*
 * VESC Firmware PC Build - Hardware Header Stub
 * 
 * This file provides a minimal hw.h stub for PC build.
 * It is included via the stubs directory which takes priority in include path.
 */

#ifndef HW_H_
#define HW_H_

#include "hw_stub.h"

// =============================================================================
// Hardware Name and Configuration
// =============================================================================

#ifndef HW_NAME
#define HW_NAME     "PC_STUB"
#endif

#ifndef FW_NAME
#define FW_NAME     "pc_build"
#endif

// =============================================================================
// Motor Configuration
// =============================================================================

// Single motor by default (no dual motor support)
// #define HW_HAS_DUAL_MOTORS

// No hardware watchdog
// #define HW_HAS_NO_WATCHDOG

// =============================================================================
// Phase Shunt Configuration
// =============================================================================

#define HW_HAS_3_SHUNTS

// =============================================================================
// Default Pin/Port Definitions (stubs)
// =============================================================================

// LED pins (stub - no actual GPIO)
#define LED_GREEN_GPIO      NULL
#define LED_GREEN_PIN       0
#define LED_RED_GPIO        NULL
#define LED_RED_PIN         1

// =============================================================================
// ADC Configuration (stub values)
// =============================================================================

#define HW_ADC_CHANNELS     16
#define HW_ADC_INJ_CHANNELS 3

// ADC channel indices
#define ADC_IND_SENS1       0
#define ADC_IND_SENS2       1
#define ADC_IND_SENS3       2
#define ADC_IND_CURR1       3
#define ADC_IND_CURR2       4
#define ADC_IND_CURR3       5
#define ADC_IND_VIN_SENS    6
#define ADC_IND_TEMP_MOS    7
#define ADC_IND_TEMP_MOTOR  8

// =============================================================================
// Voltage and Current Scaling (stub values)
// =============================================================================

#define V_REG               3.3f
#define VIN_R1              39000.0f
#define VIN_R2              2200.0f

#define CURRENT_AMP_GAIN    20.0f
#define CURRENT_SHUNT_RES   0.0005f

// =============================================================================
// Motor Hardware Limits (for simulation/testing)
// =============================================================================

#define HW_LIM_CURRENT          -100.0f, 100.0f
#define HW_LIM_CURRENT_IN       -100.0f, 100.0f
#define HW_LIM_CURRENT_ABS      0.0f, 150.0f
#define HW_LIM_VIN              6.0f, 60.0f
#define HW_LIM_ERPM             -100000.0f, 100000.0f
#define HW_LIM_DUTY_MIN         0.005f, 0.1f
#define HW_LIM_DUTY_MAX         0.8f, 0.99f
#define HW_LIM_TEMP_FET         -40.0f, 110.0f

// =============================================================================
// Function Stubs
// =============================================================================

// Initialize hardware (stub)
static inline void hw_init_gpio(void) {}
static inline void hw_setup_adc_channels(void) {}
static inline float hw_read_temp(void) { return 25.0f; }

// Gate driver control (stub)
#define ENABLE_GATE()       do {} while(0)
#define DISABLE_GATE()      do {} while(0)

// Current sense (stub)
#define GET_CURRENT1()      0.0f
#define GET_CURRENT2()      0.0f
#define GET_CURRENT3()      0.0f

// Phase control (stub)
#define PHASE_A_TOP_ON()    do {} while(0)
#define PHASE_A_TOP_OFF()   do {} while(0)
#define PHASE_A_BOT_ON()    do {} while(0)
#define PHASE_A_BOT_OFF()   do {} while(0)
#define PHASE_B_TOP_ON()    do {} while(0)
#define PHASE_B_TOP_OFF()   do {} while(0)
#define PHASE_B_BOT_ON()    do {} while(0)
#define PHASE_B_BOT_OFF()   do {} while(0)
#define PHASE_C_TOP_ON()    do {} while(0)
#define PHASE_C_TOP_OFF()   do {} while(0)
#define PHASE_C_BOT_ON()    do {} while(0)
#define PHASE_C_BOT_OFF()   do {} while(0)

#endif /* HW_H_ */
