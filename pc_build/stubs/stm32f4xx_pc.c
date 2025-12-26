/**
 * @file stm32f4xx_pc.c
 * @brief STM32F4 peripheral instance definitions for PC build
 */

#include "stm32f4xx_pc.h"

/*===========================================================================*/
/* GPIO instances                                                            */
/*===========================================================================*/

GPIO_TypeDef GPIOA_Instance = {0};
GPIO_TypeDef GPIOB_Instance = {0};
GPIO_TypeDef GPIOC_Instance = {0};
GPIO_TypeDef GPIOD_Instance = {0};
GPIO_TypeDef GPIOE_Instance = {0};
GPIO_TypeDef GPIOF_Instance = {0};
GPIO_TypeDef GPIOG_Instance = {0};
GPIO_TypeDef GPIOH_Instance = {0};
GPIO_TypeDef GPIOI_Instance = {0};

/*===========================================================================*/
/* Timer instances                                                           */
/*===========================================================================*/

TIM_TypeDef TIM1_Instance = {0};
TIM_TypeDef TIM2_Instance = {0};
TIM_TypeDef TIM3_Instance = {0};
TIM_TypeDef TIM4_Instance = {0};
TIM_TypeDef TIM5_Instance = {0};
TIM_TypeDef TIM6_Instance = {0};
TIM_TypeDef TIM7_Instance = {0};
TIM_TypeDef TIM8_Instance = {0};
TIM_TypeDef TIM9_Instance = {0};
TIM_TypeDef TIM10_Instance = {0};
TIM_TypeDef TIM11_Instance = {0};
TIM_TypeDef TIM12_Instance = {0};
TIM_TypeDef TIM13_Instance = {0};
TIM_TypeDef TIM14_Instance = {0};

/*===========================================================================*/
/* ADC instances                                                             */
/*===========================================================================*/

ADC_TypeDef ADC1_Instance = {0};
ADC_TypeDef ADC2_Instance = {0};
ADC_TypeDef ADC3_Instance = {0};
ADC_Common_TypeDef ADC_Common_Instance = {0};

/*===========================================================================*/
/* DMA instances                                                             */
/*===========================================================================*/

DMA_TypeDef DMA1_Instance = {0};
DMA_TypeDef DMA2_Instance = {0};

DMA_Stream_TypeDef DMA1_Stream0_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream1_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream2_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream3_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream4_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream5_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream6_Instance = {0};
DMA_Stream_TypeDef DMA1_Stream7_Instance = {0};

DMA_Stream_TypeDef DMA2_Stream0_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream1_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream2_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream3_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream4_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream5_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream6_Instance = {0};
DMA_Stream_TypeDef DMA2_Stream7_Instance = {0};

/*===========================================================================*/
/* RCC instance                                                              */
/*===========================================================================*/

RCC_TypeDef RCC_Instance = {0};

/*===========================================================================*/
/* SPI instances                                                             */
/*===========================================================================*/

SPI_TypeDef SPI1_Instance = {0};
SPI_TypeDef SPI2_Instance = {0};
SPI_TypeDef SPI3_Instance = {0};

/*===========================================================================*/
/* USART instances                                                           */
/*===========================================================================*/

USART_TypeDef USART1_Instance = {0};
USART_TypeDef USART2_Instance = {0};
USART_TypeDef USART3_Instance = {0};
USART_TypeDef UART4_Instance = {0};
USART_TypeDef UART5_Instance = {0};
USART_TypeDef USART6_Instance = {0};

/*===========================================================================*/
/* I2C instances                                                             */
/*===========================================================================*/

I2C_TypeDef I2C1_Instance = {0};
I2C_TypeDef I2C2_Instance = {0};
I2C_TypeDef I2C3_Instance = {0};

/*===========================================================================*/
/* CAN instances                                                             */
/*===========================================================================*/

CAN_TypeDef CAN1_Instance = {0};
CAN_TypeDef CAN2_Instance = {0};

/*===========================================================================*/
/* Other instances                                                           */
/*===========================================================================*/

IWDG_TypeDef IWDG_Instance = {0};
FLASH_TypeDef FLASH_Instance = {0};
