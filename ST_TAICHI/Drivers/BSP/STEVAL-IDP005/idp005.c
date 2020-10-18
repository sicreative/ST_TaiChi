/**
  ******************************************************************************
  * @file    idp0051.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version 2.0.0
  * @date    22 October 2018
  * @brief   This file provides the IO driver for the IDP005
  *          board.
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

/* Includes ------------------------------------------------------------------*/
#include "idp005.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STEVAL-IDP005V1
  * @{
  */ 

/** @addtogroup STEVAL-IDP005V1_Private_Defines STEVAL-IDP005V1 Private Defines
  * @{
  */ 

/**
  * @brief STEVAL BSP Driver version number
  */
#define __STEVAL_IDP005V1_BOARD_BSP_VERSION_MAJOR   (0x00) //!< [31:24] major version
#define __STEVAL_IDP005V1_BOARD_BSP_VERSION_MINOR   (0x00) //!< [23:16] minor version
#define __STEVAL_IDP005V1_BOARD_BSP_VERSION_REVISON (0x01) //!< [15:8]  revision version
#define __STEVAL_IDP005V1_BOARD_BSP_VERSION_RC      (0x00) //!< [7:0]   release candidate
#define __STEVAL_IDP005V1_BOARD_BSP_VERSION         ((__STEVAL_IDP005V1_BOARD_BSP_VERSION_MAJOR << 24)\
                                                    |(__STEVAL_IDP005V1_BOARD_BSP_VERSION_MINOR << 16)\
                                                    |(__STEVAL_IDP005V1_BOARD_BSP_VERSION_REVISON << 8 )\
                                                    |(__STEVAL_IDP005V1_BOARD_BSP_VERSION_RC))

/**
  * @}
  */ 

/** @addtogroup STEVAL-IDP005V1_Private_Variables STEVAL-IDP005V1 Private variables
  * @{
  */

GPIO_TypeDef* GPIO_PORT[LEDn] = {USER_LED_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {USER_LED_PIN};

/**
  * @}
  */ 
    
/** @addtogroup STEVAL-IDP005V1_DATA_COMM STEVAL-IDP005V1 Data Communication
 * @{
 */

/** @addtogroup STEVAL-IDP005V1_DATA_COMM_Public_Functions STEVAL-IDP005V1 Data Communication Public Functions
  * @{
  */

/** @brief  Enable/Diag GPIO Port driving 
  * @param  status Pin Status
  */
void BSP_L6362A_ENDIAG(uint8_t status)
{
  if(status==0)
    HAL_GPIO_WritePin(L6362A_EN_DIAG_GPIO_PORT, L6362A_EN_DIAG_PIN, GPIO_PIN_RESET); 
  else if(status==1)
    HAL_GPIO_WritePin(L6362A_EN_DIAG_GPIO_PORT, L6362A_EN_DIAG_PIN, GPIO_PIN_SET);
}


/**
 * @}
 */

/**
 * @}
 */
 
 
/** @defgroup STEVAL-IDP005V1_Private_Functions STEVAL-IDP005V1 Private Functions
  * @{
  */ 

/**
  * @brief  This method returns the STM32F4xx NUCLEO BSP Driver revision
  * @retval version: 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t BSP_GetVersion(void)
{
  return __STEVAL_IDP005V1_BOARD_BSP_VERSION;
}

/**
  * @brief Provides a tick value in millisecond.
  * @retval tick value
  */
int32_t BSP_GetTick(void)
{
  return HAL_GetTick();
}

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured. 
  *   This parameter can be one of following parameters:
  *     @arg USER_LED
  */
void BSP_LED_Init(BoardLed_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);
  
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET); 

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = GPIO_PIN[Led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  
  HAL_GPIO_Init(GPIO_PORT[Led], &GPIO_InitStruct);
}

/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init. 
  *   This parameter can be one of the following values:
  *     @arg  USER_LED
  * @note Led DeInit does not disable the GPIO clock nor disable the Mfx 
  */
void BSP_LED_DeInit(BoardLed_TypeDef Led)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Turn off LED */
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET);
  /* DeInit the GPIO_LED pin */
  gpio_init_structure.Pin = GPIO_PIN[Led];
  HAL_GPIO_DeInit(GPIO_PORT[Led], gpio_init_structure.Pin);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg USER_LED
  */
void BSP_LED_On(BoardLed_TypeDef Led)
{
  HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_PIN, GPIO_PIN_SET); 
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg USER_LED
  */
void BSP_LED_Off(BoardLed_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET); 
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *     @arg USER_LED  
  */
void BSP_LED_Toggle(BoardLed_TypeDef Led)
{
  HAL_GPIO_TogglePin(GPIO_PORT[Led], GPIO_PIN[Led]);
}


/**
 * @}
 */

/** @addtogroup STEVAL-IDP005V1_IO STEVAL-IDP005V1 IO
  * @{
  */ 


/**
  * @brief  Configures the I/Os for the HTS221 relative humidity and temperature sensor.
  * @retval COMPONENT_OK in case of success
  * @retval COMPONENT_ERROR in case of failure
*/
void L6362A_IO_Init(uint8_t IO_Link_TX)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  L6362A_EN_DIAG_GPIO_CLK_DISABLE();
  L6362A_OL_GPIO_CLK_DISABLE();
  
  /* Configure GPIO pin : L6362A*/
  if (IO_Link_TX == 1)
  {
    L6362A_EN_DIAG_GPIO_CLK_ENABLE();
    L6362A_OL_GPIO_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = L6362A_EN_DIAG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(L6362A_EN_DIAG_GPIO_PORT, &GPIO_InitStruct); 
    
    GPIO_InitStruct.Pin = L6362A_OL_PIN;
    HAL_GPIO_Init(L6362A_OL_GPIO_PORT, &GPIO_InitStruct);  
    
    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(L6362A_EXTI_IRQn, L6362A_EXTI_IRQ_PP, L6362A_EXTI_IRQ_SP);
    HAL_NVIC_EnableIRQ(L6362A_EXTI_IRQn);
    while(HAL_GPIO_ReadPin(L6362A_EN_DIAG_GPIO_PORT,L6362A_EN_DIAG_PIN)!=1);
  }
  else if (IO_Link_TX == 0)
  {  
    HAL_NVIC_DisableIRQ(L6362A_EXTI_IRQn);
    
    L6362A_EN_DIAG_GPIO_CLK_ENABLE();
    L6362A_OL_GPIO_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = L6362A_EN_DIAG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(L6362A_EN_DIAG_GPIO_PORT, &GPIO_InitStruct);
    while(HAL_GPIO_ReadPin(L6362A_EN_DIAG_GPIO_PORT,L6362A_EN_DIAG_PIN)!=0);
  }  
}

void L6362A_IO_DeInit( void )
{
  HAL_GPIO_DeInit(L6362A_EN_DIAG_GPIO_PORT, L6362A_EN_DIAG_PIN);
  HAL_GPIO_DeInit(L6362A_OL_GPIO_PORT, L6362A_OL_PIN);
}


/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
