/**
 * @file hal_stub.c
 * @brief ChibiOS HAL stub implementation for PC build
 */

#include "hal.h"
#include <stdio.h>
#include <string.h>

/*===========================================================================*/
/* Driver instances                                                          */
/*===========================================================================*/

/* ADC drivers */
ADCDriver ADCD1 = { .state = HAL_STOP };
ADCDriver ADCD2 = { .state = HAL_STOP };
ADCDriver ADCD3 = { .state = HAL_STOP };

/* PWM drivers */
PWMDriver PWMD1 = { .state = HAL_STOP };
PWMDriver PWMD2 = { .state = HAL_STOP };
PWMDriver PWMD3 = { .state = HAL_STOP };
PWMDriver PWMD4 = { .state = HAL_STOP };
PWMDriver PWMD5 = { .state = HAL_STOP };
PWMDriver PWMD8 = { .state = HAL_STOP };

/* ICU drivers */
ICUDriver ICUD1 = { .state = HAL_STOP };
ICUDriver ICUD2 = { .state = HAL_STOP };
ICUDriver ICUD3 = { .state = HAL_STOP };
ICUDriver ICUD4 = { .state = HAL_STOP };
ICUDriver ICUD5 = { .state = HAL_STOP };
ICUDriver ICUD8 = { .state = HAL_STOP };

/* Serial drivers */
SerialDriver SD1 = { .state = HAL_STOP };
SerialDriver SD2 = { .state = HAL_STOP };
SerialDriver SD3 = { .state = HAL_STOP };
SerialDriver SD4 = { .state = HAL_STOP };
SerialDriver SD5 = { .state = HAL_STOP };
SerialDriver SD6 = { .state = HAL_STOP };

/* UART drivers */
UARTDriver UARTD1 = { .state = HAL_STOP };
UARTDriver UARTD2 = { .state = HAL_STOP };
UARTDriver UARTD3 = { .state = HAL_STOP };

/* SPI drivers */
SPIDriver SPID1 = { .state = HAL_STOP };
SPIDriver SPID2 = { .state = HAL_STOP };
SPIDriver SPID3 = { .state = HAL_STOP };

/* I2C drivers */
I2CDriver I2CD1 = { .state = HAL_STOP };
I2CDriver I2CD2 = { .state = HAL_STOP };

/* CAN drivers */
CANDriver CAND1 = { .state = HAL_STOP };
CANDriver CAND2 = { .state = HAL_STOP };

/* Watchdog driver */
WDGDriver WDGD1 = { .state = HAL_STOP };

/* EXT driver */
EXTDriver EXTD1 = { .state = HAL_STOP };

/* GPT drivers */
GPTDriver GPTD1 = { .state = HAL_STOP };
GPTDriver GPTD2 = { .state = HAL_STOP };
GPTDriver GPTD3 = { .state = HAL_STOP };
GPTDriver GPTD4 = { .state = HAL_STOP };
GPTDriver GPTD5 = { .state = HAL_STOP };
GPTDriver GPTD8 = { .state = HAL_STOP };

/*===========================================================================*/
/* HAL initialization                                                        */
/*===========================================================================*/

void halInit(void) {
    /* PC build - nothing to initialize */
}

void halStart(void) {
    /* PC build - nothing to start */
}
