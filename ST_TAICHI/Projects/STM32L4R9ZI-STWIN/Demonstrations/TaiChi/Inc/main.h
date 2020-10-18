 /**
  ******************************************************************************
  * @file    main.h 
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Header for main.c module
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
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "console.h" 

#include "PREDMNT1_config.h"
#include "MotionSP_Manager.h"

/** @addtogroup Projects
  * @{
  */

/** @addtogroup DEMONSTRATIONS Demonstrations
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE Predictive Maintenance BLE
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE_MAIN Predictive Maintenance main
  * @{
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_TYPES Predictive Maintenance Main Exported Types
  * @{
  */

/**
  * @brief MCU identifiers
  */
typedef struct
{
  uint16_t McuDevId;    //!< Device identifier
  uint16_t McuRevId;    //!< Revision identifier
  uint32_t u_id_31_0;   //!< 31:0 unique ID bits
  uint32_t u_id_63_32;  //!< 63:32 unique ID bits
  uint32_t u_id_95_64;  //!< 95:64 unique ID bits
  uint16_t FlashSize;   //!< The size of the device Flash memory expressed in Kbytes
  uint8_t Package;      //!< Package data
} sMcuId_t;

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_MACRO Predictive Maintenance Main Exported Macro
  * @{
  */

/* Exported macro ------------------------------------------------------------*/
#define MCR_BLUEMS_F2I_1D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (int32_t)((in-out_int)*10);};
#define MCR_BLUEMS_F2I_2D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (int32_t)((in-out_int)*100);};
#define MCR_BLUEMS_F2I_3D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (abs)((int32_t)((in-out_int)*1000));};

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_FUNCTIONS_PROTOTYPES Predictive Maintenance Main Exported Functions Prototypes
  * @{
  */

/* Exported functions ------------------------------------------------------- */
extern void Error_Handler(void);

extern void Get_McuId(sMcuId_t *pMcuId);
extern unsigned char SaveVibrationParamToMemory(void);

uint8_t getBlueNRG2_Version(uint8_t *hwVersion, uint16_t *fwVersion);

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_VARIABLES Predictive Maintenance Main Exported Variables
  * @{
  */

/* Exported Variables ------------------------------------------------------- */
extern uint8_t BufferToWrite[256];
extern int32_t BytesToWrite;

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_DEFINES Predictive Maintenance Main Exported Defines
  * @{
  */

/* Exported defines --------------------------------------------------------- */

/* Update frequency for Acc/Gyro/Mag sensor [Hz] */
#define FREQ_ACC_GYRO_MAG               20U
/* Update period for Acc/Gyro/Mag [ms] */
#define ALGO_PERIOD_ACC_GYRO_MAG        (1000U / FREQ_ACC_GYRO_MAG) 
/* 10kHz/20  For Acc/Gyro/Mag@20Hz */
#define DEFAULT_uhCCR4_Val              (10000U / FREQ_ACC_GYRO_MAG)

/* Update frequency for environmental sensor [Hz] */
#define ALGO_FREQ_ENV   2U
/* Compute the prescaler value to have TIM4 counter clock equal to 2 KHz */
#define TIM_CLOCK_ENV   2000U
/* Update period for environmental sensor [ms] */
#define ALGO_PERIOD_ENV (1000U / ALGO_FREQ_ENV)

/* Update frequency for mic audio level [Hz] */
#define ALGO_FREQ_AUDIO_LEVEL   20U
/* Compute the prescaler value to have TIM5 counter clock equal to 10 KHz */
#define TIM_CLOCK_AUDIO_LEVEL   10000U
/* Update period for mic audio level [ms] */
#define ALGO_PERIOD_AUDIO_LEVEL (1000U / ALGO_FREQ_AUDIO_LEVEL)

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

/**
  * @}
  */


#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
