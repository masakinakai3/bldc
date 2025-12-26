/*
 * VESC Firmware PC Build - app.h stub
 * 
 * This file provides a minimal app.h for PC build.
 * It replaces the real app.h which has complex dependencies.
 */

#ifndef APP_H_
#define APP_H_

#include "datatypes.h"

// =============================================================================
// Application Configuration API
// =============================================================================

/**
 * Get current application configuration
 * Implemented in app_stub.c
 */
const app_configuration* app_get_configuration(void);

/**
 * Set application configuration
 * Implemented in app_stub.c
 */
void app_set_configuration(app_configuration *conf);

/**
 * Disable output for specified time
 * Implemented in app_stub.c
 */
void app_disable_output(int time_ms);

/**
 * Check if output is disabled
 * Implemented in app_stub.c
 */
bool app_is_output_disabled(void);

/**
 * Calculate CRC of app configuration
 */
static inline unsigned short app_calc_crc(app_configuration* conf) {
    (void)conf;
    return 0;
}

// =============================================================================
// Application Stubs (not implemented for PC build)
// =============================================================================

// PPM application
static inline void app_ppm_start(void) {}
static inline void app_ppm_stop(void) {}
static inline float app_ppm_get_decoded_level(void) { return 0.0f; }
static inline void app_ppm_detach(bool detach) { (void)detach; }
static inline void app_ppm_override(float val) { (void)val; }
static inline void app_ppm_configure(ppm_config *conf) { (void)conf; }

// ADC application  
static inline void app_adc_start(bool use_rx_tx) { (void)use_rx_tx; }
static inline void app_adc_stop(void) {}
static inline void app_adc_configure(adc_config *conf) { (void)conf; }
static inline float app_adc_get_decoded_level(void) { return 0.0f; }
static inline float app_adc_get_voltage(void) { return 0.0f; }
static inline float app_adc_get_decoded_level2(void) { return 0.0f; }
static inline float app_adc_get_voltage2(void) { return 0.0f; }
static inline void app_adc_detach_adc(int detach) { (void)detach; }
static inline void app_adc_adc1_override(float val) { (void)val; }
static inline void app_adc_adc2_override(float val) { (void)val; }
static inline void app_adc_detach_buttons(bool state) { (void)state; }
static inline void app_adc_rev_override(bool state) { (void)state; }
static inline void app_adc_cc_override(bool state) { (void)state; }
static inline bool app_adc_range_ok(void) { return true; }

// UART application
typedef enum {
    UART_PORT_COMM_HEADER = 0,
    UART_PORT_BUILTIN,
    UART_PORT_EXTRA_HEADER
} UART_PORT;

static inline void app_uartcomm_initialize(void) {}
static inline void app_uartcomm_start(UART_PORT port) { (void)port; }
static inline void app_uartcomm_stop(UART_PORT port) { (void)port; }
static inline void app_uartcomm_configure(uint32_t baudrate, bool permanent, UART_PORT port) { 
    (void)baudrate; (void)permanent; (void)port; 
}
static inline void app_uartcomm_send_packet(unsigned char *data, unsigned int len, UART_PORT port) {
    (void)data; (void)len; (void)port;
}

// Nunchuk application
static inline void app_nunchuk_start(void) {}
static inline void app_nunchuk_stop(void) {}
static inline void app_nunchuk_configure(chuk_config *conf) { (void)conf; }
static inline float app_nunchuk_get_decoded_x(void) { return 0.0f; }
static inline float app_nunchuk_get_decoded_y(void) { return 0.0f; }
static inline bool app_nunchuk_get_bt_c(void) { return false; }
static inline bool app_nunchuk_get_bt_z(void) { return false; }
static inline bool app_nunchuk_get_is_rev(void) { return false; }
static inline float app_nunchuk_get_update_age(void) { return 0.0f; }
static inline void app_nunchuk_update_output(chuck_data *data) { (void)data; }

// PAS application
static inline void app_pas_start(bool is_primary) { (void)is_primary; }
static inline void app_pas_stop(void) {}
static inline bool app_pas_is_running(void) { return false; }
static inline void app_pas_configure(pas_config *conf) { (void)conf; }
static inline float app_pas_get_current_target_rel(void) { return 0.0f; }
static inline float app_pas_get_pedal_rpm(void) { return 0.0f; }
static inline void app_pas_set_current_sub_scaling(float scaling) { (void)scaling; }

// Custom application
static inline void app_custom_start(void) {}
static inline void app_custom_stop(void) {}
static inline void app_custom_configure(app_configuration *conf) { (void)conf; }

#endif /* APP_H_ */
