/**
  ******************************************************************************
  * @file    TargetFeatures.h 
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Specification of the HW Features for each target platform
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 STMicroelectronics</center></h2>
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
#ifndef _TARGET_FEATURES_H_
#define _TARGET_FEATURES_H_

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
   
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
//#include "stm32l4xx_hal_def.h"
//#include "stm32f4xx_UART.h"
//#include "stm32l4xx_periph_conf.h"

#include "STWIN.h"
#include "STWIN_audio.h"
#include "STWIN_motion_sensors.h"
#include "STWIN_motion_sensors_ex.h"
#include "STWIN_env_sensors.h"
#include "STWIN_bc.h"
   
#include "PREDMNT1_config.h"
#include "MetaDataManager.h"
   
/** @addtogroup Projects
  * @{
  */

/** @addtogroup DEMONSTRATIONS Demonstrations
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE Predictive Maintenance BLE
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE_TARGET_PLATFORM Predictive Maintenance Target Platform
  * @{
  */

/** @defgroup PREDCTIVE_MAINTENANCE_TARGET_PLATFORM_EXPORTED_DEFINES Predictive Maintenance Target Platform Exported Defines
  * @{
  */
   
/* Exported defines ------------------------------------------------------- */
#define MAX_TEMP_SENSORS 2

/* BlueNRG Board Type */
#define IDB04A1 0
#define IDB05A1 1
   
/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_TARGET_PLATFORM_EXPORTED_TYPES Predictive Maintenance Target Platform Exported Types
  * @{
  */

/* Exported types ------------------------------------------------------- */
   
/**
 * @brief  Target type data definition
 */
typedef enum
{
  TARGET_NUCLEO,
  TARGET_IDP005,
  TARGET_STWIN,
  TARGETS_NUMBER
} TargetType_t;

/**
 * @brief  Target's Features data structure definition
 */
typedef struct
{
  TargetType_t BoardType;
  
  uint8_t TempSensorsIsInit[MAX_TEMP_SENSORS];
  uint8_t PressSensorIsInit;
  uint8_t HumSensorIsInit;

  uint8_t AccSensorIsInit;
  uint8_t GyroSensorIsInit;
  uint8_t MagSensorIsInit;  
  
  int32_t NumTempSensors;
  int32_t NumMicSensors;

  uint8_t LedStatus;
  uint8_t bnrg_expansion_board;
} TargetFeatures_t;

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_TARGET_PLATFORM_EXPORTED_VARIABLES Predictive Maintenance Target Platform Exported Variables
  * @{
  */

/* Exported variables ------------------------------------------------------- */
extern TargetFeatures_t TargetBoardFeatures;

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_TARGET_PLATFORM_EXPORTED_FUNCTIONS_PROTOTYPES Predictive Maintenance Target Platform Exported Functions Prototypes
  * @{
  */

/* Exported functions ------------------------------------------------------- */
extern void InitTargetPlatform(TargetType_t BoardType);

extern void InitMics(uint32_t AudioFreq, uint32_t AudioVolume);
extern void DeInitMics(void);

extern void LedOnTargetPlatform(void);
extern void LedOffTargetPlatform(void);
extern void LedToggleTargetPlatform(void);

/**
  * @}
  */

/**
  * @}
  */

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

#endif /* _TARGET_FEATURES_H_ */

/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/

