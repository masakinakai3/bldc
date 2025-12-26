/**
 * @file hal.h
 * @brief ChibiOS HAL stubs for PC build
 */

#ifndef _HAL_H_
#define _HAL_H_

#include "ch.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* HAL driver types                                                          */
/*===========================================================================*/

/* Base driver state */
typedef enum {
    HAL_UNINIT = 0,
    HAL_STOP,
    HAL_READY,
    HAL_ACTIVE,
    HAL_COMPLETE,
    HAL_ERROR
} halstate_t;

/*===========================================================================*/
/* PAL (Port Abstraction Layer) stubs                                        */
/*===========================================================================*/

typedef uint32_t ioportmask_t;
typedef uint32_t iopadid_t;
typedef void*    ioportid_t;
typedef uint32_t iomode_t;

#define PAL_LOW             0U
#define PAL_HIGH            1U

#define PAL_MODE_RESET                  0U
#define PAL_MODE_UNCONNECTED            1U
#define PAL_MODE_INPUT                  2U
#define PAL_MODE_INPUT_PULLUP           3U
#define PAL_MODE_INPUT_PULLDOWN         4U
#define PAL_MODE_INPUT_ANALOG           5U
#define PAL_MODE_OUTPUT_PUSHPULL        6U
#define PAL_MODE_OUTPUT_OPENDRAIN       7U
#define PAL_MODE_ALTERNATE(n)           (16U | (n))
#define PAL_STM32_OSPEED_HIGHEST        3U

#define palSetPad(port, pad)            ((void)0)
#define palClearPad(port, pad)          ((void)0)
#define palTogglePad(port, pad)         ((void)0)
#define palReadPad(port, pad)           (0U)
#define palWritePad(port, pad, bit)     ((void)0)
#define palSetPadMode(port, pad, mode)  ((void)0)
#define palReadPort(port)               (0U)
#define palWritePort(port, bits)        ((void)0)
#define palSetPort(port, bits)          ((void)0)
#define palClearPort(port, bits)        ((void)0)
#define palTogglePort(port, bits)       ((void)0)
#define palSetGroupMode(port, mask, offset, mode) ((void)0)
#define palEnablePadEvent(port, pad, mode) ((void)0)
#define palDisablePadEvent(port, pad)   ((void)0)

/*===========================================================================*/
/* ADC stubs                                                                 */
/*===========================================================================*/

typedef struct ADCDriver ADCDriver;
typedef uint16_t adcsample_t;
typedef struct ADCConversionGroup ADCConversionGroup;

struct ADCConversionGroup {
    bool                    circular;
    uint16_t                num_channels;
    void                    (*end_cb)(ADCDriver *adcp);
    void                    (*error_cb)(ADCDriver *adcp, uint32_t err);
    uint32_t                cr1;
    uint32_t                cr2;
    uint32_t                smpr1;
    uint32_t                smpr2;
    uint32_t                sqr1;
    uint32_t                sqr2;
    uint32_t                sqr3;
};

struct ADCDriver {
    halstate_t              state;
    const ADCConversionGroup *grpp;
    adcsample_t            *samples;
    size_t                  depth;
};

extern ADCDriver ADCD1;
extern ADCDriver ADCD2;
extern ADCDriver ADCD3;

#define adcStart(adcp, config)          ((adcp)->state = HAL_READY)
#define adcStop(adcp)                   ((adcp)->state = HAL_STOP)
#define adcStartConversion(adcp, grpp, samples, depth) ((void)0)
#define adcStopConversion(adcp)         ((void)0)
#define adcStartConversionI(adcp, grpp, samples, depth) ((void)0)
#define adcStopConversionI(adcp)        ((void)0)
#define adcConvert(adcp, grpp, samples, depth) MSG_OK

/*===========================================================================*/
/* PWM stubs                                                                 */
/*===========================================================================*/

typedef struct PWMDriver PWMDriver;
typedef uint32_t pwmcnt_t;
typedef uint16_t pwmchannel_t;

typedef struct {
    pwmcnt_t                frequency;
    pwmcnt_t                period;
    void                    (*callback)(PWMDriver *pwmp);
    struct {
        uint32_t            mode;
        void                (*callback)(PWMDriver *pwmp);
    } channels[4];
    uint32_t                cr2;
    uint32_t                dier;
} PWMConfig;

struct PWMDriver {
    halstate_t              state;
    const PWMConfig        *config;
    pwmcnt_t                period;
};

extern PWMDriver PWMD1;
extern PWMDriver PWMD2;
extern PWMDriver PWMD3;
extern PWMDriver PWMD4;
extern PWMDriver PWMD5;
extern PWMDriver PWMD8;

#define pwmStart(pwmp, config)          ((pwmp)->state = HAL_READY, (pwmp)->config = (config))
#define pwmStop(pwmp)                   ((pwmp)->state = HAL_STOP)
#define pwmChangePeriod(pwmp, period)   ((pwmp)->period = (period))
#define pwmEnableChannel(pwmp, ch, width) ((void)0)
#define pwmDisableChannel(pwmp, ch)     ((void)0)
#define pwmEnablePeriodicNotification(pwmp) ((void)0)
#define pwmDisablePeriodicNotification(pwmp) ((void)0)
#define pwmEnableChannelNotification(pwmp, ch) ((void)0)
#define pwmDisableChannelNotification(pwmp, ch) ((void)0)

#define PWM_OUTPUT_DISABLED             0U
#define PWM_OUTPUT_ACTIVE_HIGH          1U
#define PWM_OUTPUT_ACTIVE_LOW           2U

/*===========================================================================*/
/* ICU (Input Capture) stubs                                                 */
/*===========================================================================*/

typedef struct ICUDriver ICUDriver;
typedef uint32_t icucnt_t;

typedef struct {
    uint32_t                mode;
    uint32_t                frequency;
    void                    (*width_cb)(ICUDriver *icup);
    void                    (*period_cb)(ICUDriver *icup);
    void                    (*overflow_cb)(ICUDriver *icup);
    uint32_t                channel;
    uint32_t                dier;
} ICUConfig;

struct ICUDriver {
    halstate_t              state;
    const ICUConfig        *config;
};

extern ICUDriver ICUD1;
extern ICUDriver ICUD2;
extern ICUDriver ICUD3;
extern ICUDriver ICUD4;
extern ICUDriver ICUD5;
extern ICUDriver ICUD8;

#define icuStart(icup, config)          ((icup)->state = HAL_READY)
#define icuStop(icup)                   ((icup)->state = HAL_STOP)
#define icuStartCapture(icup)           ((void)0)
#define icuStopCapture(icup)            ((void)0)
#define icuEnableNotifications(icup)    ((void)0)
#define icuDisableNotifications(icup)   ((void)0)
#define icuGetWidthX(icup)              (0U)
#define icuGetPeriodX(icup)             (0U)

#define ICU_INPUT_ACTIVE_HIGH           0U
#define ICU_INPUT_ACTIVE_LOW            1U

/*===========================================================================*/
/* Serial / UART stubs                                                       */
/*===========================================================================*/

typedef struct SerialDriver SerialDriver;
typedef struct UARTDriver UARTDriver;

struct SerialDriver {
    halstate_t              state;
};

struct UARTDriver {
    halstate_t              state;
};

extern SerialDriver SD1;
extern SerialDriver SD2;
extern SerialDriver SD3;
extern SerialDriver SD4;
extern SerialDriver SD5;
extern SerialDriver SD6;

extern UARTDriver UARTD1;
extern UARTDriver UARTD2;
extern UARTDriver UARTD3;

#define sdStart(sdp, config)            ((sdp)->state = HAL_READY)
#define sdStop(sdp)                     ((sdp)->state = HAL_STOP)
#define sdWrite(sdp, buf, n)            ((size_t)(n))
#define sdWriteTimeout(sdp, buf, n, t)  ((size_t)(n))
#define sdAsynchronousWrite(sdp, buf, n) ((size_t)(n))
#define sdRead(sdp, buf, n)             ((size_t)0)
#define sdReadTimeout(sdp, buf, n, t)   ((size_t)0)
#define sdAsynchronousRead(sdp, buf, n) ((size_t)0)
#define sdGetTimeout(sdp, t)            ((msg_t)0)
#define sdPutTimeout(sdp, b, t)         (MSG_OK)
#define sdPut(sdp, b)                   (MSG_OK)
#define sdGet(sdp)                      ((msg_t)0)

/*===========================================================================*/
/* SPI stubs                                                                 */
/*===========================================================================*/

typedef struct SPIDriver SPIDriver;

struct SPIDriver {
    halstate_t              state;
};

extern SPIDriver SPID1;
extern SPIDriver SPID2;
extern SPIDriver SPID3;

#define spiStart(spip, config)          ((spip)->state = HAL_READY)
#define spiStop(spip)                   ((spip)->state = HAL_STOP)
#define spiSelect(spip)                 ((void)0)
#define spiUnselect(spip)               ((void)0)
#define spiExchange(spip, n, txbuf, rxbuf) ((void)0)
#define spiSend(spip, n, txbuf)         ((void)0)
#define spiReceive(spip, n, rxbuf)      ((void)0)

/*===========================================================================*/
/* I2C stubs                                                                 */
/*===========================================================================*/

typedef struct I2CDriver I2CDriver;

struct I2CDriver {
    halstate_t              state;
};

extern I2CDriver I2CD1;
extern I2CDriver I2CD2;

#define i2cStart(i2cp, config)          ((i2cp)->state = HAL_READY)
#define i2cStop(i2cp)                   ((i2cp)->state = HAL_STOP)
#define i2cMasterTransmitTimeout(i2cp, addr, txbuf, txbytes, rxbuf, rxbytes, timeout) MSG_OK
#define i2cMasterReceiveTimeout(i2cp, addr, rxbuf, rxbytes, timeout) MSG_OK
#define i2cGetErrors(i2cp)              (0U)

/*===========================================================================*/
/* CAN stubs                                                                 */
/*===========================================================================*/

typedef struct CANDriver CANDriver;
typedef uint32_t canmbx_t;

typedef struct {
    uint32_t                EID;
    uint32_t                IDE;
    uint32_t                RTR;
    uint8_t                 DLC;
    uint8_t                 data8[8];
} CANTxFrame;

typedef struct {
    uint32_t                EID;
    uint32_t                IDE;
    uint32_t                RTR;
    uint8_t                 DLC;
    uint8_t                 FMI;
    uint16_t                TIME;
    uint8_t                 data8[8];
} CANRxFrame;

struct CANDriver {
    halstate_t              state;
};

extern CANDriver CAND1;
extern CANDriver CAND2;

#define canStart(canp, config)          ((canp)->state = HAL_READY)
#define canStop(canp)                   ((canp)->state = HAL_STOP)
#define canTransmit(canp, mailbox, ctfp, timeout) MSG_OK
#define canReceive(canp, mailbox, crfp, timeout) MSG_TIMEOUT
#define canTryReceiveI(canp, mailbox, crfp) false

#define CAN_ANY_MAILBOX                 0U

/*===========================================================================*/
/* WDG stubs                                                                 */
/*===========================================================================*/

typedef struct WDGDriver WDGDriver;

struct WDGDriver {
    halstate_t              state;
};

extern WDGDriver WDGD1;

#define wdgStart(wdgp, config)          ((wdgp)->state = HAL_READY)
#define wdgStop(wdgp)                   ((wdgp)->state = HAL_STOP)
#define wdgReset(wdgp)                  ((void)0)

/*===========================================================================*/
/* EXT (External interrupts) stubs                                           */
/*===========================================================================*/

typedef struct EXTDriver EXTDriver;

struct EXTDriver {
    halstate_t              state;
};

extern EXTDriver EXTD1;

#define extStart(extp, config)          ((extp)->state = HAL_READY)
#define extStop(extp)                   ((extp)->state = HAL_STOP)
#define extChannelEnable(extp, ch)      ((void)0)
#define extChannelDisable(extp, ch)     ((void)0)

/*===========================================================================*/
/* GPT (General Purpose Timer) stubs                                         */
/*===========================================================================*/

typedef struct GPTDriver GPTDriver;

typedef struct {
    uint32_t                frequency;
    void                    (*callback)(GPTDriver *gptp);
    uint32_t                cr2;
    uint32_t                dier;
} GPTConfig;

struct GPTDriver {
    halstate_t              state;
    const GPTConfig        *config;
};

extern GPTDriver GPTD1;
extern GPTDriver GPTD2;
extern GPTDriver GPTD3;
extern GPTDriver GPTD4;
extern GPTDriver GPTD5;
extern GPTDriver GPTD8;

#define gptStart(gptp, config)          ((gptp)->state = HAL_READY, (gptp)->config = (config))
#define gptStop(gptp)                   ((gptp)->state = HAL_STOP)
#define gptStartContinuous(gptp, interval) ((void)0)
#define gptStartOneShot(gptp, interval) ((void)0)
#define gptStopTimer(gptp)              ((void)0)
#define gptChangeInterval(gptp, interval) ((void)0)
#define gptGetCounterX(gptp)            (0U)
#define gptPolledDelay(gptp, interval)  ((void)0)

/*===========================================================================*/
/* DMA stubs                                                                 */
/*===========================================================================*/

typedef void (*stm32_dmaisr_t)(void *p, uint32_t flags);

#define dmaStreamAlloc(stream, priority, func, param)   ((void*)1)
#define dmaStreamFree(dmastp)           ((void)0)
#define dmaStreamSetPeripheral(dmastp, addr) ((void)0)
#define dmaStreamSetMemory0(dmastp, addr) ((void)0)
#define dmaStreamSetMemory1(dmastp, addr) ((void)0)
#define dmaStreamSetTransactionSize(dmastp, n) ((void)0)
#define dmaStreamSetMode(dmastp, mode)  ((void)0)
#define dmaStreamSetFIFO(dmastp, mode)  ((void)0)
#define dmaStreamEnable(dmastp)         ((void)0)
#define dmaStreamDisable(dmastp)        ((void)0)
#define dmaStreamClearInterrupt(dmastp) ((void)0)
#define dmaStreamGetTransactionSize(dmastp) (0U)

#define STM32_DMA_CR_PL(n)              (0U)
#define STM32_DMA_CR_MSIZE_BYTE         (0U)
#define STM32_DMA_CR_MSIZE_HWORD        (0U)
#define STM32_DMA_CR_MSIZE_WORD         (0U)
#define STM32_DMA_CR_PSIZE_BYTE         (0U)
#define STM32_DMA_CR_PSIZE_HWORD        (0U)
#define STM32_DMA_CR_PSIZE_WORD         (0U)
#define STM32_DMA_CR_MINC               (0U)
#define STM32_DMA_CR_PINC               (0U)
#define STM32_DMA_CR_CIRC               (0U)
#define STM32_DMA_CR_DIR_M2M            (0U)
#define STM32_DMA_CR_DIR_M2P            (0U)
#define STM32_DMA_CR_DIR_P2M            (0U)
#define STM32_DMA_CR_TCIE               (0U)
#define STM32_DMA_CR_HTIE               (0U)
#define STM32_DMA_CR_TEIE               (0U)
#define STM32_DMA_CR_DMEIE              (0U)
#define STM32_DMA_CR_DBM                (0U)
#define STM32_DMA_CR_CHSEL(n)           (0U)
#define STM32_DMA_FCR_DMDIS             (0U)
#define STM32_DMA_FCR_FTH_FULL          (0U)
#define STM32_DMA_ISR_TCIF              (0x20U)

/*===========================================================================*/
/* HAL functions                                                             */
/*===========================================================================*/

void halInit(void);
void halStart(void);

/*===========================================================================*/
/* Utility macros                                                            */
/*===========================================================================*/

#define OSAL_IRQ_PROLOGUE()             do {} while(0)
#define OSAL_IRQ_EPILOGUE()             do {} while(0)
#define OSAL_IRQ_HANDLER(name)          void name(void)

/* Port / Line macros */
#define LINE_LED                        0U
#define PAL_LINE(port, pad)             (((uint32_t)(uintptr_t)(port) << 16) | (pad))
#define PAL_PORT(line)                  ((ioportid_t)(uintptr_t)((line) >> 16))
#define PAL_PAD(line)                   ((iopadid_t)((line) & 0xFFFF))

#define palSetLine(line)                palSetPad(PAL_PORT(line), PAL_PAD(line))
#define palClearLine(line)              palClearPad(PAL_PORT(line), PAL_PAD(line))
#define palToggleLine(line)             palTogglePad(PAL_PORT(line), PAL_PAD(line))
#define palReadLine(line)               palReadPad(PAL_PORT(line), PAL_PAD(line))
#define palWriteLine(line, bit)         palWritePad(PAL_PORT(line), PAL_PAD(line), (bit))
#define palSetLineMode(line, mode)      palSetPadMode(PAL_PORT(line), PAL_PAD(line), (mode))

#ifdef __cplusplus
}
#endif

#endif /* _HAL_H_ */
