/**
 * @file mcconf_stub.h
 * @brief Motor configuration stub for PC build
 * 
 * Provides default motor configuration values for simulation and testing.
 */

#ifndef MCCONF_STUB_H_
#define MCCONF_STUB_H_

#include "datatypes.h"

/**
 * Set all mc_configuration fields to reasonable default values
 * suitable for simulation and testing.
 * 
 * @param conf Pointer to configuration structure to initialize
 */
void mcconf_set_defaults(mc_configuration *conf);

/**
 * Set motor parameters for a typical small BLDC motor
 * (e.g., 2808 size gimbal motor)
 * 
 * @param conf Pointer to configuration structure
 */
void mcconf_set_small_motor(mc_configuration *conf);

/**
 * Set motor parameters for a typical medium hub motor
 * (e.g., electric skateboard motor)
 * 
 * @param conf Pointer to configuration structure
 */
void mcconf_set_medium_motor(mc_configuration *conf);

/**
 * Set motor parameters for a typical large motor
 * (e.g., electric vehicle motor)
 * 
 * @param conf Pointer to configuration structure
 */
void mcconf_set_large_motor(mc_configuration *conf);

#endif /* MCCONF_STUB_H_ */
