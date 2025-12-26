/**
 * @file stm32f4xx_pc.h
 * @brief STM32F4 peripheral definitions for PC build
 * 
 * This header provides PC-compatible definitions by wrapping the existing
 * CMSIS headers and providing stub peripheral instances.
 */

#ifndef _STM32F4XX_PC_H_
#define _STM32F4XX_PC_H_

#include <stdint.h>

/* CMSIS type macros for PC (remove ARM dependency) */
#ifndef __IO
#define __IO volatile
#endif
#ifndef __I
#define __I  volatile const
#endif
#ifndef __O
#define __O  volatile
#endif

/* ARM-specific macros - make them PC compatible */
#ifndef __INLINE
#define __INLINE static inline
#endif
#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif

/* Prevent ARM intrinsics from being included */
#define __CORE_CM4_H_GENERIC
#define __CORE_CMFUNC_H
#define __CORE_CMINSTR_H

/* Include CMSIS PC stubs first */
#include "cmsis_pc.h"

/*===========================================================================*/
/* GPIO TypeDef                                                              */
/*===========================================================================*/

typedef struct {
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;

/*===========================================================================*/
/* Timer TypeDef                                                             */
/*===========================================================================*/

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
    __IO uint32_t RCR;
    __IO uint32_t CCR1;
    __IO uint32_t CCR2;
    __IO uint32_t CCR3;
    __IO uint32_t CCR4;
    __IO uint32_t BDTR;
    __IO uint32_t DCR;
    __IO uint32_t DMAR;
} TIM_TypeDef;

/*===========================================================================*/
/* ADC TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    __IO uint32_t JOFR1;
    __IO uint32_t JOFR2;
    __IO uint32_t JOFR3;
    __IO uint32_t JOFR4;
    __IO uint32_t HTR;
    __IO uint32_t LTR;
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    __IO uint32_t JSQR;
    __IO uint32_t JDR1;
    __IO uint32_t JDR2;
    __IO uint32_t JDR3;
    __IO uint32_t JDR4;
    __IO uint32_t DR;
} ADC_TypeDef;

typedef struct {
    __IO uint32_t CSR;
    __IO uint32_t CCR;
    __IO uint32_t CDR;
} ADC_Common_TypeDef;

/*===========================================================================*/
/* DMA TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t NDTR;
    __IO uint32_t PAR;
    __IO uint32_t M0AR;
    __IO uint32_t M1AR;
    __IO uint32_t FCR;
} DMA_Stream_TypeDef;

typedef struct {
    __IO uint32_t LISR;
    __IO uint32_t HISR;
    __IO uint32_t LIFCR;
    __IO uint32_t HIFCR;
} DMA_TypeDef;

/*===========================================================================*/
/* RCC TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t PLLCFGR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t AHB1RSTR;
    __IO uint32_t AHB2RSTR;
    __IO uint32_t AHB3RSTR;
    uint32_t      RESERVED0;
    __IO uint32_t APB1RSTR;
    __IO uint32_t APB2RSTR;
    uint32_t      RESERVED1[2];
    __IO uint32_t AHB1ENR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t AHB3ENR;
    uint32_t      RESERVED2;
    __IO uint32_t APB1ENR;
    __IO uint32_t APB2ENR;
    uint32_t      RESERVED3[2];
    __IO uint32_t AHB1LPENR;
    __IO uint32_t AHB2LPENR;
    __IO uint32_t AHB3LPENR;
    uint32_t      RESERVED4;
    __IO uint32_t APB1LPENR;
    __IO uint32_t APB2LPENR;
    uint32_t      RESERVED5[2];
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
    uint32_t      RESERVED6[2];
    __IO uint32_t SSCGR;
    __IO uint32_t PLLI2SCFGR;
} RCC_TypeDef;

/*===========================================================================*/
/* SPI TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t CRCPR;
    __IO uint32_t RXCRCR;
    __IO uint32_t TXCRCR;
    __IO uint32_t I2SCFGR;
    __IO uint32_t I2SPR;
} SPI_TypeDef;

/*===========================================================================*/
/* USART TypeDef                                                             */
/*===========================================================================*/

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t BRR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t CR3;
    __IO uint32_t GTPR;
} USART_TypeDef;

/*===========================================================================*/
/* I2C TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t OAR1;
    __IO uint32_t OAR2;
    __IO uint32_t DR;
    __IO uint32_t SR1;
    __IO uint32_t SR2;
    __IO uint32_t CCR;
    __IO uint32_t TRISE;
    __IO uint32_t FLTR;
} I2C_TypeDef;

/*===========================================================================*/
/* CAN TypeDef                                                               */
/*===========================================================================*/

typedef struct {
    __IO uint32_t TIR;
    __IO uint32_t TDTR;
    __IO uint32_t TDLR;
    __IO uint32_t TDHR;
} CAN_TxMailBox_TypeDef;

typedef struct {
    __IO uint32_t RIR;
    __IO uint32_t RDTR;
    __IO uint32_t RDLR;
    __IO uint32_t RDHR;
} CAN_FIFOMailBox_TypeDef;

typedef struct {
    __IO uint32_t FR1;
    __IO uint32_t FR2;
} CAN_FilterRegister_TypeDef;

typedef struct {
    __IO uint32_t              MCR;
    __IO uint32_t              MSR;
    __IO uint32_t              TSR;
    __IO uint32_t              RF0R;
    __IO uint32_t              RF1R;
    __IO uint32_t              IER;
    __IO uint32_t              ESR;
    __IO uint32_t              BTR;
    uint32_t                   RESERVED0[88];
    CAN_TxMailBox_TypeDef      sTxMailBox[3];
    CAN_FIFOMailBox_TypeDef    sFIFOMailBox[2];
    uint32_t                   RESERVED1[12];
    __IO uint32_t              FMR;
    __IO uint32_t              FM1R;
    uint32_t                   RESERVED2;
    __IO uint32_t              FS1R;
    uint32_t                   RESERVED3;
    __IO uint32_t              FFA1R;
    uint32_t                   RESERVED4;
    __IO uint32_t              FA1R;
    uint32_t                   RESERVED5[8];
    CAN_FilterRegister_TypeDef sFilterRegister[28];
} CAN_TypeDef;

/*===========================================================================*/
/* IWDG TypeDef                                                              */
/*===========================================================================*/

typedef struct {
    __IO uint32_t KR;
    __IO uint32_t PR;
    __IO uint32_t RLR;
    __IO uint32_t SR;
} IWDG_TypeDef;

/*===========================================================================*/
/* FLASH TypeDef                                                             */
/*===========================================================================*/

typedef struct {
    __IO uint32_t ACR;
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t OPTCR;
    __IO uint32_t OPTCR1;
} FLASH_TypeDef;

/*===========================================================================*/
/* Peripheral instances (extern declarations)                                */
/*===========================================================================*/

extern GPIO_TypeDef GPIOA_Instance, GPIOB_Instance, GPIOC_Instance;
extern GPIO_TypeDef GPIOD_Instance, GPIOE_Instance, GPIOF_Instance;
extern GPIO_TypeDef GPIOG_Instance, GPIOH_Instance, GPIOI_Instance;

extern TIM_TypeDef TIM1_Instance, TIM2_Instance, TIM3_Instance;
extern TIM_TypeDef TIM4_Instance, TIM5_Instance, TIM6_Instance;
extern TIM_TypeDef TIM7_Instance, TIM8_Instance, TIM9_Instance;
extern TIM_TypeDef TIM10_Instance, TIM11_Instance, TIM12_Instance;
extern TIM_TypeDef TIM13_Instance, TIM14_Instance;

extern ADC_TypeDef ADC1_Instance, ADC2_Instance, ADC3_Instance;
extern ADC_Common_TypeDef ADC_Common_Instance;

extern DMA_TypeDef DMA1_Instance, DMA2_Instance;
extern DMA_Stream_TypeDef DMA1_Stream0_Instance, DMA1_Stream1_Instance;
extern DMA_Stream_TypeDef DMA1_Stream2_Instance, DMA1_Stream3_Instance;
extern DMA_Stream_TypeDef DMA1_Stream4_Instance, DMA1_Stream5_Instance;
extern DMA_Stream_TypeDef DMA1_Stream6_Instance, DMA1_Stream7_Instance;
extern DMA_Stream_TypeDef DMA2_Stream0_Instance, DMA2_Stream1_Instance;
extern DMA_Stream_TypeDef DMA2_Stream2_Instance, DMA2_Stream3_Instance;
extern DMA_Stream_TypeDef DMA2_Stream4_Instance, DMA2_Stream5_Instance;
extern DMA_Stream_TypeDef DMA2_Stream6_Instance, DMA2_Stream7_Instance;

extern RCC_TypeDef RCC_Instance;

extern SPI_TypeDef SPI1_Instance, SPI2_Instance, SPI3_Instance;

extern USART_TypeDef USART1_Instance, USART2_Instance, USART3_Instance;
extern USART_TypeDef UART4_Instance, UART5_Instance, USART6_Instance;

extern I2C_TypeDef I2C1_Instance, I2C2_Instance, I2C3_Instance;

extern CAN_TypeDef CAN1_Instance, CAN2_Instance;

extern IWDG_TypeDef IWDG_Instance;

extern FLASH_TypeDef FLASH_Instance;

/*===========================================================================*/
/* Peripheral pointer macros                                                 */
/*===========================================================================*/

#define GPIOA       (&GPIOA_Instance)
#define GPIOB       (&GPIOB_Instance)
#define GPIOC       (&GPIOC_Instance)
#define GPIOD       (&GPIOD_Instance)
#define GPIOE       (&GPIOE_Instance)
#define GPIOF       (&GPIOF_Instance)
#define GPIOG       (&GPIOG_Instance)
#define GPIOH       (&GPIOH_Instance)
#define GPIOI       (&GPIOI_Instance)

#define TIM1        (&TIM1_Instance)
#define TIM2        (&TIM2_Instance)
#define TIM3        (&TIM3_Instance)
#define TIM4        (&TIM4_Instance)
#define TIM5        (&TIM5_Instance)
#define TIM6        (&TIM6_Instance)
#define TIM7        (&TIM7_Instance)
#define TIM8        (&TIM8_Instance)
#define TIM9        (&TIM9_Instance)
#define TIM10       (&TIM10_Instance)
#define TIM11       (&TIM11_Instance)
#define TIM12       (&TIM12_Instance)
#define TIM13       (&TIM13_Instance)
#define TIM14       (&TIM14_Instance)

#define ADC1        (&ADC1_Instance)
#define ADC2        (&ADC2_Instance)
#define ADC3        (&ADC3_Instance)
#define ADC         (&ADC_Common_Instance)

#define DMA1        (&DMA1_Instance)
#define DMA2        (&DMA2_Instance)
#define DMA1_Stream0 (&DMA1_Stream0_Instance)
#define DMA1_Stream1 (&DMA1_Stream1_Instance)
#define DMA1_Stream2 (&DMA1_Stream2_Instance)
#define DMA1_Stream3 (&DMA1_Stream3_Instance)
#define DMA1_Stream4 (&DMA1_Stream4_Instance)
#define DMA1_Stream5 (&DMA1_Stream5_Instance)
#define DMA1_Stream6 (&DMA1_Stream6_Instance)
#define DMA1_Stream7 (&DMA1_Stream7_Instance)
#define DMA2_Stream0 (&DMA2_Stream0_Instance)
#define DMA2_Stream1 (&DMA2_Stream1_Instance)
#define DMA2_Stream2 (&DMA2_Stream2_Instance)
#define DMA2_Stream3 (&DMA2_Stream3_Instance)
#define DMA2_Stream4 (&DMA2_Stream4_Instance)
#define DMA2_Stream5 (&DMA2_Stream5_Instance)
#define DMA2_Stream6 (&DMA2_Stream6_Instance)
#define DMA2_Stream7 (&DMA2_Stream7_Instance)

#define RCC         (&RCC_Instance)

#define SPI1        (&SPI1_Instance)
#define SPI2        (&SPI2_Instance)
#define SPI3        (&SPI3_Instance)

#define USART1      (&USART1_Instance)
#define USART2      (&USART2_Instance)
#define USART3      (&USART3_Instance)
#define UART4       (&UART4_Instance)
#define UART5       (&UART5_Instance)
#define USART6      (&USART6_Instance)

#define I2C1        (&I2C1_Instance)
#define I2C2        (&I2C2_Instance)
#define I2C3        (&I2C3_Instance)

#define CAN1        (&CAN1_Instance)
#define CAN2        (&CAN2_Instance)

#define IWDG        (&IWDG_Instance)

#define FLASH       (&FLASH_Instance)

/*===========================================================================*/
/* System constants                                                          */
/*===========================================================================*/

#define SYSTEM_CORE_CLOCK   168000000U
#define HSE_VALUE           8000000U
#define HSI_VALUE           16000000U

#endif /* _STM32F4XX_PC_H_ */
