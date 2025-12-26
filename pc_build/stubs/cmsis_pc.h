/**
 * @file cmsis_pc.h
 * @brief CMSIS core intrinsics stubs for PC build
 * 
 * Provides no-op or minimal implementations of ARM Cortex-M intrinsics
 * to allow compilation on x86/x64 platforms.
 */

#ifndef _CMSIS_PC_H_
#define _CMSIS_PC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Compiler barrier / Memory barrier                                         */
/*===========================================================================*/

#if defined(__GNUC__)
#define __DMB()     __asm__ volatile ("" ::: "memory")
#define __DSB()     __asm__ volatile ("" ::: "memory")
#define __ISB()     __asm__ volatile ("" ::: "memory")
#else
#define __DMB()     ((void)0)
#define __DSB()     ((void)0)
#define __ISB()     ((void)0)
#endif

/*===========================================================================*/
/* No-op intrinsics                                                          */
/*===========================================================================*/

#ifndef __NOP
#define __NOP()     ((void)0)
#endif

#ifndef __WFI
#define __WFI()     ((void)0)
#endif

#ifndef __WFE
#define __WFE()     ((void)0)
#endif

#ifndef __SEV
#define __SEV()     ((void)0)
#endif

/*===========================================================================*/
/* Interrupt enable/disable stubs                                            */
/*===========================================================================*/

static volatile uint32_t _pc_irq_disabled = 0;

static inline void __disable_irq(void) {
    _pc_irq_disabled = 1;
}

static inline void __enable_irq(void) {
    _pc_irq_disabled = 0;
}

static inline uint32_t __get_PRIMASK(void) {
    return _pc_irq_disabled;
}

static inline void __set_PRIMASK(uint32_t value) {
    _pc_irq_disabled = value & 1;
}

static inline uint32_t __get_BASEPRI(void) {
    return 0;
}

static inline void __set_BASEPRI(uint32_t value) {
    (void)value;
}

static inline uint32_t __get_FAULTMASK(void) {
    return 0;
}

static inline void __set_FAULTMASK(uint32_t value) {
    (void)value;
}

/*===========================================================================*/
/* Stack pointer / Control register stubs                                    */
/*===========================================================================*/

static inline uint32_t __get_MSP(void) {
    return 0x20010000; /* Simulated stack address */
}

static inline void __set_MSP(uint32_t topOfMainStack) {
    (void)topOfMainStack;
}

#ifndef __get_PSP
static inline uint32_t __get_PSP(void) {
    return 0x20008000; /* Simulated process stack */
}
#endif

static inline void __set_PSP(uint32_t topOfProcStack) {
    (void)topOfProcStack;
}

static inline uint32_t __get_CONTROL(void) {
    return 0;
}

static inline void __set_CONTROL(uint32_t control) {
    (void)control;
}

/*===========================================================================*/
/* Bit manipulation intrinsics                                               */
/*===========================================================================*/

static inline uint32_t __CLZ(uint32_t value) {
    if (value == 0) return 32;
#if defined(__GNUC__)
    return (uint32_t)__builtin_clz(value);
#else
    uint32_t count = 0;
    if ((value & 0xFFFF0000U) == 0) { count += 16; value <<= 16; }
    if ((value & 0xFF000000U) == 0) { count += 8;  value <<= 8;  }
    if ((value & 0xF0000000U) == 0) { count += 4;  value <<= 4;  }
    if ((value & 0xC0000000U) == 0) { count += 2;  value <<= 2;  }
    if ((value & 0x80000000U) == 0) { count += 1; }
    return count;
#endif
}

static inline uint32_t __RBIT(uint32_t value) {
    uint32_t result = 0;
    for (int i = 0; i < 32; i++) {
        result <<= 1;
        result |= (value & 1);
        value >>= 1;
    }
    return result;
}

static inline uint32_t __REV(uint32_t value) {
    return ((value & 0x000000FFU) << 24) |
           ((value & 0x0000FF00U) << 8)  |
           ((value & 0x00FF0000U) >> 8)  |
           ((value & 0xFF000000U) >> 24);
}

static inline uint16_t __REV16(uint16_t value) {
    return (uint16_t)(((value & 0x00FFU) << 8) | ((value & 0xFF00U) >> 8));
}

static inline int16_t __REVSH(int16_t value) {
    return (int16_t)__REV16((uint16_t)value);
}

static inline uint32_t __ROR(uint32_t value, uint32_t shift) {
    shift &= 0x1F;
    if (shift == 0) return value;
    return (value >> shift) | (value << (32 - shift));
}

/*===========================================================================*/
/* Saturation intrinsics                                                     */
/*===========================================================================*/

static inline int32_t __SSAT(int32_t val, uint32_t sat) {
    if (sat >= 32) return val;
    int32_t max = (1 << (sat - 1)) - 1;
    int32_t min = -(1 << (sat - 1));
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

static inline uint32_t __USAT(int32_t val, uint32_t sat) {
    if (sat >= 32) return (uint32_t)val;
    if (val < 0) return 0;
    uint32_t max = (1U << sat) - 1;
    if ((uint32_t)val > max) return max;
    return (uint32_t)val;
}

/*===========================================================================*/
/* FPU-related stubs (for Cortex-M4F)                                        */
/*===========================================================================*/

static inline uint32_t __get_FPSCR(void) {
    return 0;
}

static inline void __set_FPSCR(uint32_t fpscr) {
    (void)fpscr;
}

/*===========================================================================*/
/* Exclusive access stubs (for atomic operations)                            */
/*===========================================================================*/

static inline uint32_t __LDREXW(volatile uint32_t *addr) {
    return *addr;
}

static inline uint32_t __STREXW(uint32_t value, volatile uint32_t *addr) {
    *addr = value;
    return 0; /* Success */
}

static inline uint8_t __LDREXB(volatile uint8_t *addr) {
    return *addr;
}

static inline uint32_t __STREXB(uint8_t value, volatile uint8_t *addr) {
    *addr = value;
    return 0;
}

static inline uint16_t __LDREXH(volatile uint16_t *addr) {
    return *addr;
}

static inline uint32_t __STREXH(uint16_t value, volatile uint16_t *addr) {
    *addr = value;
    return 0;
}

static inline void __CLREX(void) {
    /* No-op on PC */
}

/*===========================================================================*/
/* NVIC stubs                                                                */
/*===========================================================================*/

#define NVIC_SetPriority(IRQn, priority)    ((void)(IRQn), (void)(priority))
#define NVIC_GetPriority(IRQn)              (0U)
#define NVIC_EnableIRQ(IRQn)                ((void)(IRQn))
#define NVIC_DisableIRQ(IRQn)               ((void)(IRQn))
#define NVIC_GetPendingIRQ(IRQn)            (0U)
#define NVIC_SetPendingIRQ(IRQn)            ((void)(IRQn))
#define NVIC_ClearPendingIRQ(IRQn)          ((void)(IRQn))
#define NVIC_GetActive(IRQn)                (0U)
#define NVIC_SystemReset()                  ((void)0)

/*===========================================================================*/
/* SysTick stub                                                              */
/*===========================================================================*/

#define SysTick_Config(ticks)               (0U)

#ifdef __cplusplus
}
#endif

#endif /* _CMSIS_PC_H_ */
