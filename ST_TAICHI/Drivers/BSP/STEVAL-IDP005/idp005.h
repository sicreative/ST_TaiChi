/**
  ******************************************************************************
  * @file    idp005.h
  * @author  System Research & Applications Team - Catania Lab.
  * @version 2.0.0
  * @date    22 October 2018
  * @brief   This file contains definitions for the idp005v1.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IDP005_H
#define __IDP005_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STEVAL-IDP005V1
  * @{
  */

/** @addtogroup STEVAL-IDP005V1_Exported_Types STEVAL-IDP005V1 Exported Types
  * @{
  */
typedef enum
{
  USER_LED = 0
} BoardLed_TypeDef;

typedef enum
{
  USER_BUTTON = 0,
} BoardButton_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} BoardButtonMode_TypeDef;


/**
  * @}
  */ 

/** @addtogroup STEVAL-IDP005V1_Exported_Constants STEVAL-IDP005V1 Exported Constants
  * @{
  */

/** @addtogroup STEVAL-IDP005V1_SENSOR_ID STEVAL-IDP005V1 Sensor ID
  * @{
  */

#define IDP005V1_HTS221_WHO_AM_I                (uint8_t)0xBC
#define IDP005V1_LPS22HB_WHO_AM_I               (uint8_t)0xB1
#define IDP005V1_ISM330DLC_ACC_GYRO_WHO_AM_I    (uint8_t)0x6A

#define IDP005V1_M95M01_DF_INST                 (uint8_t)0x00
#define IDP005V1_M95M01_DF_S                    (uint8_t)0x00
#define IDP005V1_M95M01_DF_HOLD                 (uint8_t)0x01
#define IDP005V1_M95M01_DF_W                    (uint8_t)0x02

/**
  * @}
  */ 

/** @addtogroup STEVAL-IDP005V1_LED STEVAL-IDP005V1 LED
  * @{
  */
  
#define LEDn                                    1

#define USER_LED_PIN                            GPIO_PIN_5
#define USER_LED_GPIO_PORT                      GPIOD
#define USER_LED_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOD_CLK_ENABLE()
#define USER_LED_GPIO_CLK_DISABLE()             __HAL_RCC_GPIOD_CLK_DISABLE()  

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)         USER_LED_GPIO_CLK_ENABLE()
#define LEDx_GPIO_CLK_DISABLE(__INDEX__)        USER_LED_GPIO_CLK_DISABLE()

#define USER_LED_ON_OFF_DELAY                   1000

/**
  * @}
  */

/**
  * @}
  */

  
/** @addtogroup STEVAL-IDP005V1_DATA_COMM STEVAL-IDP005V1 Data Communication
 * @{
 */

/** @addtogroup STEVAL-IDP005V1_DATA_COMM_Public_Functions_Prototypes STEVAL-IDP005V1 Data Communication Public function prototypes
  * @{
  */

void BSP_L6362A_ENDIAG(uint8_t status);

/**
  * @}
  */

/**
  * @}
  */  
  
/** @addtogroup STEVAL-IDP005V1_IO STEVAL-IDP005V1 IO
  * @{
  */ 

/** @addtogroup STEVAL-IDP005V1_IO_SPI_BUS STEVAL-IDP005V1 IO SPI BUS
  * @{
  */
    
    
#ifdef HAL_SPI_MODULE_ENABLED
/*##################### SD ###################################*/
/* Chip Select macro definition */
#define MOTION_CS_LOW()       HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET)
#define MOTION_CS_HIGH()      HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET)

#define ISM330DLC_SPI_IRQn                         SPI1_IRQn
#define ISM330DLC_SPI_IRQ_PP                       1
#define ISM330DLC_SPI_IRQ_SP                       0
#define ISM330DLC_SPI_IRQHandler                   SPI1_IRQHandler
#define ISM330DLC_SPI_MAX_CLOCK                    10000000 /* in MHz */

#define ISM330DLC_SPI_BUS_CLOCK                    HAL_RCC_GetPCLK2Freq()
#define ISM330DLC_SPI_RX_DMA                       DMA2_Stream2
#define ISM330DLC_SPI_RX_DMA_CH                    DMA_CHANNEL_3
#define ISM330DLC_SPI_DMA_CLK_ENABLE()             __HAL_RCC_DMA2_CLK_ENABLE();
#define ISM330DLC_SPI_RX_DMA_Stream_IRQn           DMA2_Stream2_IRQn
#define ISM330DLC_SPI_RX_DMA_Stream_IRQ_PP         1
#define ISM330DLC_SPI_RX_DMA_Stream_IRQ_SP         0
#define ISM330DLC_SPI_RX_DMA_Stream_IRQHandler     DMA2_Stream2_IRQHandler
#define ISM330DLC_SPI_TX_DMA                       DMA2_Stream3
#define ISM330DLC_SPI_TX_DMA_CH                    DMA_CHANNEL_3
#define ISM330DLC_SPI_TX_DMA_Stream_IRQn           DMA2_Stream3_IRQn
#define ISM330DLC_SPI_TX_DMA_Stream_IRQ_PP         1
#define ISM330DLC_SPI_TX_DMA_Stream_IRQ_SP         0
#define ISM330DLC_SPI_TX_DMA_Stream_IRQHandler     DMA2_Stream3_IRQHandler

#define SPIx_TIMEOUT_MAX                        1000 /*<! Maximum timeout value for BUS waiting loops */
#define SPIx_BUFFERSIZE                         264

#define M95M01_DF_SPI_MAX_CLOCK                 10000000 /* in MHz */
#define M95M01_DF_SPI_BUS_CLOCK                 HAL_RCC_GetPCLK2Freq()
#define M95M01_DF_SPI                           SPI4
#define M95M01_DF_SPI_CLK_ENABLE()              __HAL_RCC_SPI4_CLK_ENABLE()
#define M95M01_DF_SPI_CLK_DISABLE()             __HAL_RCC_SPI4_CLK_DISABLE()
#define M95M01_DF_SPI_MISO_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOE_CLK_ENABLE()
#define M95M01_DF_SPI_MOSI_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOE_CLK_ENABLE()
#define M95M01_DF_SPI_SCK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOE_CLK_ENABLE()
#define M95M01_DF_SPI_MISO_GPIO_PORT            GPIOE
#define M95M01_DF_SPI_MOSI_GPIO_PORT            GPIOE
#define M95M01_DF_SPI_SCK_GPIO_PORT             GPIOE
#define M95M01_DF_SPI_MISO_PIN                  GPIO_PIN_5
#define M95M01_DF_SPI_MOSI_PIN                  GPIO_PIN_6
#define M95M01_DF_SPI_SCK_PIN                   GPIO_PIN_2
#define M95M01_DF_SPI_MISO_AF                   GPIO_AF5_SPI4
#define M95M01_DF_SPI_MOSI_AF                   GPIO_AF5_SPI4
#define M95M01_DF_SPI_SCK_AF                    GPIO_AF5_SPI4
#define M95M01_DF_SPI_FORCE_RESET()             __HAL_RCC_SPI4_FORCE_RESET()
#define M95M01_DF_SPI_RELEASE_RESET()           __HAL_RCC_SPI4_RELEASE_RESET()
#define M95M01_DF_SPI_IRQn                      SPI4_IRQn
#define M95M01_DF_SPI_IRQ_PP                    7
#define M95M01_DF_SPI_IRQ_SP                    0
#define M95M01_DF_SPI_IRQHandler                SPI4_IRQHandler

#define M95M01_DF_SPI_RX_DMA                    DMA2_Stream0
#define M95M01_DF_SPI_RX_DMA_CH                 DMA_CHANNEL_4
#define M95M01_DF_SPI_DMA_CLK_ENABLE()          __HAL_RCC_DMA2_CLK_ENABLE();
#define M95M01_DF_SPI_RX_DMA_Stream_IRQn         DMA2_Stream0_IRQn
#define M95M01_DF_SPI_RX_DMA_Stream_IRQ_PP      7
#define M95M01_DF_SPI_RX_DMA_Stream_IRQ_SP      0
#define M95M01_DF_SPI_RX_DMA_Stream_IRQHandler  DMA2_Stream0_IRQHandler
#define M95M01_DF_SPI_TX_DMA                    DMA2_Stream1
#define M95M01_DF_SPI_TX_DMA_CH                 DMA_CHANNEL_4
#define M95M01_DF_SPI_TX_DMA_Stream_IRQn        DMA2_Stream1_IRQn
#define M95M01_DF_SPI_TX_DMA_Stream_IRQ_PP      7
#define M95M01_DF_SPI_TX_DMA_Stream_IRQ_SP      0
#define M95M01_DF_SPI_TX_DMA_Stream_IRQHandler  DMA2_Stream1_IRQHandler

#define M95M01_DF_S_GPIO_PORT                   GPIOI
#define M95M01_DF_S_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define M95M01_DF_S_GPIO_CLK_DISABLE()          __GPIOI_CLK_DISABLE()
#define M95M01_DF_S_PIN                         GPIO_PIN_4

#endif // HAL_SPI_MODULE_ENABLED


/**
  * @}
  */





/** @addtogroup STEVAL-IDP005V1_MEMORY STEVAL-IDP005V1 Memory
  * @{
  */

#define M95M01_DF_HOLD_GPIO_PORT                GPIOI
#define M95M01_DF_HOLD_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOI_CLK_ENABLE()
#define M95M01_DF_HOLD_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOI_CLK_DISABLE()
#define M95M01_DF_HOLD_PIN                      GPIO_PIN_5
#define M95M01_DF_W_GPIO_PORT                   GPIOI
#define M95M01_DF_W_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define M95M01_DF_W_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()
#define M95M01_DF_W_PIN                         GPIO_PIN_6

/**
  * @}
  */

/** @addtogroup STEVAL-IDP005V1_Humidity_and_Temperture STEVAL-IDP005V1 Humidity and Temperture
  * @{
  */

#define HTS221_DRDY_GPIO_PORT                   GPIOG
#define HTS221_DRDY_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOG_CLK_ENABLE()
#define HTS221_DRDY_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOG_CLK_DISABLE()
#define HTS221_DRDY_PIN                         GPIO_PIN_11
#define ENV_DRDY_EXTI_IRQn                      EXTI15_10_IRQn
#define ENV_DRDY_EXTI_IRQ_PP                    5
#define ENV_DRDY_EXTI_IRQ_SP                    1
#define ENV_DRDY_EXTI_IRQHandler                EXTI15_10_IRQHandler

/**
  * @}
  */

/** @addtogroup STEVAL-IDP005V1_Accelero STEVAL-IDP005V1 Accelerometer
  * @{
  */

#define ISM330DLC_INT1_GPIO_PORT                   GPIOG
#define ISM330DLC_INT1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOG_CLK_ENABLE()
#define ISM330DLC_INT1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOG_CLK_DISABLE()
#define ISM330DLC_INT1_GPIO_PIN                    GPIO_PIN_3
#define ISM330DLC_INT1_EXTI_IRQn                   EXTI3_IRQn
#define ISM330DLC_INT1_EXTI_IRQ_PP                 1
#define ISM330DLC_INT1_EXTI_IRQ_SP                 0
#define ISM330DLC_INT1_EXTI_IRQHandler             EXTI3_IRQHandler
#define ISM330DLC_INT2_GPIO_PORT                   GPIOG
#define ISM330DLC_INT2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOG_CLK_ENABLE()
#define ISM330DLC_INT2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOG_CLK_DISABLE()
#define ISM330DLC_INT2_GPIO_PIN                    GPIO_PIN_4
#define ISM330DLC_INT2_EXTI_IRQn                   EXTI4_IRQn
#define ISM330DLC_INT2_EXTI_IRQ_PP                 1
#define ISM330DLC_INT2_EXTI_IRQ_SP                 0
#define ISM330DLC_INT2_EXTI_IRQHandler             EXTI4_IRQHandler

#define ISM330DLC_SW_PIN                           GPIO_PIN_4
#define ISM330DLC_SW_IRQn                          EXTI4_IRQn
#define ISM330DLC_SW_IRQ_PP                        1
#define ISM330DLC_SW_IRQ_SP                        1
#define ISM330DLC_SW_IRQHandler                    EXTI4_IRQHandler

/**
  * @}
  */

/** @addtogroup STEVAL-IDP005V1_Pressure STEVAL-IDP005V1 Pressure
  * @{
  */

#define LPS22HB_DRDY_GPIO_PORT                  GPIOG
#define LPS22HB_DRDY_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOG_CLK_ENABLE()
#define LPS22HB_DRDY_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOG_CLK_DISABLE()
#define LPS22HB_DRDY_PIN                        GPIO_PIN_12
#define ENV_DRDY_EXTI_LPS_IRQn                  EXTI15_10_IRQn
#define ENV_DRDY_EXTI_LPS_IRQ_PP                1
#define ENV_DRDY_EXTI_LPS_IRQ_SP                0
#define ENV_DRDY_EXTI_LPS_IRQHandler            EXTI15_10_IRQHandler

/**
  * @}
  */

/** @addtogroup STEVAL-IDP005V1_IOLINK STEVAL-IDP005V1 IO-Link
  * @{
  */

#define L6362A_EN_DIAG_GPIO_PORT                GPIOE
#define L6362A_EN_DIAG_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOE_CLK_ENABLE()
#define L6362A_EN_DIAG_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOE_CLK_DISABLE()
#define L6362A_EN_DIAG_PIN                      GPIO_PIN_7
#define L6362A_OL_GPIO_PORT                     GPIOE
#define L6362A_OL_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOE_CLK_ENABLE()
#define L6362A_OL_GPIO_CLK_DISABLE()            __HAL_RCC_GPIOE_CLK_DISABLE()
#define L6362A_OL_PIN                           GPIO_PIN_8
#define L6362A_EXTI_IRQn                        EXTI9_5_IRQn
#define L6362A_EXTI_IRQ_PP                      6
#define L6362A_EXTI_IRQ_SP                      0
#define L6362A_EXTI_IRQHandler                  EXTI9_5_IRQHandler

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup STEVAL-IDP005V1_Exported_Functions STEVAL-IDP005V1 Exported Functions
  * @{
  */

uint32_t BSP_GetVersion(void);
void BSP_LED_Init(BoardLed_TypeDef Led);
void BSP_LED_DeInit(BoardLed_TypeDef Led);
void BSP_LED_On(BoardLed_TypeDef Led);
void BSP_LED_Off(BoardLed_TypeDef Led);
void BSP_LED_Toggle(BoardLed_TypeDef Led);
int32_t BSP_GetTick(void);

void L6362A_IO_Init(uint8_t IO_Link);
void L6362A_IO_DeInit(void);

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __IDP005_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
