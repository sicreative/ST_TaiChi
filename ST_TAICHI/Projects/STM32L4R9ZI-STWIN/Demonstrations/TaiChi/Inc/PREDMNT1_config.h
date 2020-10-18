/**
  ******************************************************************************
  * @file    PREDMNT1_config.h
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   FP-IND-PREDMNT1 configuration
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
#ifndef __PREDMNT1_CONFIG_H
#define __PREDMNT1_CONFIG_H

/* Exported define ------------------------------------------------------------*/

/* Environmental Sensor Istance */
#define TEMPERATURE_INSTANCE_1  HTS221_0
#define HUMIDITY_INSTANCE       HTS221_0
#define TEMPERATURE_INSTANCE_2  LPS22HH_0
#define PRESSURE_INSTANCE       LPS22HH_0


/* Motion Sensor Istance */
#define ACCELERO_INSTANCE       ISM330DHCX_0
#define GYRO_INSTANCE           ISM330DHCX_0
#define MAGNETO_INSTANCE        IIS2MDC_0


/* Environmental Sensor API */
#define ENV_SENSOR_Init         BSP_ENV_SENSOR_Init
#define ENV_SENSOR_Enable       BSP_ENV_SENSOR_Enable
#define ENV_SENSOR_GetValue     BSP_ENV_SENSOR_GetValue


/* Motion Sensor API */
#define MOTION_SENSOR_Init                      BSP_MOTION_SENSOR_Init
#define MOTION_SENSOR_Enable                    BSP_MOTION_SENSOR_Enable

#define MOTION_SENSOR_AxesRaw_t                 BSP_MOTION_SENSOR_AxesRaw_t
#define MOTION_SENSOR_Axes_t                    BSP_MOTION_SENSOR_Axes_t

#define MOTION_SENSOR_GetAxes                   BSP_MOTION_SENSOR_GetAxes

#define MOTION_SENSOR_GetSensitivity            BSP_MOTION_SENSOR_GetSensitivity
#define MOTION_SENSOR_SetFullScale              BSP_MOTION_SENSOR_SetFullScale

#define MOTION_SENSOR_Write_Register            BSP_MOTION_SENSOR_Write_Register          

#define MOTION_SENSOR_SetOutputDataRate         BSP_MOTION_SENSOR_SetOutputDataRate
#define MOTION_SENSOR_Enable_HP_Filter          BSP_MOTION_SENSOR_Enable_HP_Filter
#define MOTION_SENSOR_Set_INT2_DRDY             BSP_MOTION_SENSOR_Set_INT2_DRDY
#define MOTION_SENSOR_DRDY_Set_Mode             BSP_MOTION_SENSOR_DRDY_Set_Mode

#define MOTION_SENSOR_FIFO_Set_Mode             BSP_MOTION_SENSOR_FIFO_Set_Mode
#define MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full   BSP_MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full
#define MOTION_SENSOR_FIFO_Get_Data_Word        BSP_MOTION_SENSOR_FIFO_Get_Data_Word
#define MOTION_SENSOR_FIFO_Set_Decimation       BSP_MOTION_SENSOR_FIFO_Set_Decimation
#define MOTION_SENSOR_FIFO_Set_BDR              BSP_MOTION_SENSOR_FIFO_Set_BDR
#define MOTION_SENSOR_FIFO_Set_Watermark_Level  BSP_MOTION_SENSOR_FIFO_Set_Watermark_Level
#define MOTION_SENSOR_FIFO_Set_Stop_On_Fth      BSP_MOTION_SENSOR_FIFO_Set_Stop_On_Fth 

#define ACCELERO_FIFO_XL_NO_DEC ISM330DHCX_NO_DECIMATION
#define ACCELERO_BYPASS_MODE    ISM330DHCX_BYPASS_MODE
#define ACCELERO_FIFO_MODE      ISM330DHCX_FIFO_MODE
#define ACCELERO_DRDY_PULSED    ISM330DHCX_DRDY_PULSED
#define ACCELERO_DRDY_LATCHED   ISM330DHCX_DRDY_LATCHED
#define ACCELERO_STREAM_MODE    ISM330DHCX_STREAM_MODE

/*************** Debug Defines ******************/
/* For enabling the printf on UART */
#define PREDMNT1_ENABLE_PRINTF
//#define PREDMNT1_PRINTF(...) printf(__VA_ARGS__)

/* For enabling connection and notification subscriptions debug */
#define PREDMNT1_DEBUG_CONNECTION

/* For enabling trasmission for notified services (except for quaternions) */
#define PREDMNT1_DEBUG_NOTIFY_TRAMISSION

/*************** Don't Change the following defines *************/

/* Package Version only numbers 0->9 */
#define PREDMNT1_VERSION_MAJOR '2'
#define PREDMNT1_VERSION_MINOR '2'
#define PREDMNT1_VERSION_PATCH '0'

/* Define the ALLMEMS1 Name MUST be 7 char long */
#define NAME_BLUEMS 'P','M','1','V',PREDMNT1_VERSION_MAJOR,PREDMNT1_VERSION_MINOR,PREDMNT1_VERSION_PATCH

/* Package Name */
#define PREDMNT1_PACKAGENAME "FP-IND-PREDMNT1"
#define CONFIG_NAME "Application - Predictive Maintenance"

/*****************
* Sensor Setting *
******************/
#define HPF_ODR_DIV_400         ISM330DHCX_HP_ODR_DIV_400 //!< ISM330DLCX HPF Configuration

/* Set defaul value for HPF Cut frequency */
#define SENSOR_HPF_CUT_VALUE                    HPF_ODR_DIV_400
/* Set defaul ODR value to 6600Hz for FFT demo */
#define SENSOR_ACC_ORD_VALUE                    6660
/* Set default decimation value for the FIFO with no decimation */
#define SENSOR_ACC_FIFO_DECIMATION_VALUE        1
/* Set defaul FIFO ODR value */
#define SENSOR_ACC_FIFO_ORD_VALUE               SENSOR_ACC_ORD_VALUE
/* Default value for Accelerometer full scale in g */
#define SENSOR_ACC_FS_DEFAULT                   4

/*  FIFO size limit */
#define FIFO_WATERMARK   ((MotionSP_Parameters.FftSize + 1)*3)

/*************************
* Serial control section *
**************************/
#ifdef PREDMNT1_ENABLE_PRINTF
    #include "usbd_cdc_interface.h"
    #define PREDMNT1_PRINTF(...) {\
      char TmpBufferToWrite[256];\
      int32_t TmpBytesToWrite;\
      TmpBytesToWrite = sprintf( TmpBufferToWrite, __VA_ARGS__);\
      CDC_Fill_Buffer(( uint8_t * )TmpBufferToWrite, TmpBytesToWrite);\
    }
#else /* PREDMNT1_ENABLE_PRINTF */
  #define PREDMNT1_PRINTF(...)
  //#define PREDMNT1_PRINTF(...) printf(__VA_ARGS__)
#endif /* PREDMNT1_ENABLE_PRINTF */

/* STM32 Unique ID */
#define STM32_UUID ((uint32_t *)0x1FFF7590)

/* STM32 MCU_ID */
#define STM32_MCU_ID ((uint32_t *)0xE0042000)
/* Control Section */

#endif /* __PREDMNT1_CONFIG_H */

/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
