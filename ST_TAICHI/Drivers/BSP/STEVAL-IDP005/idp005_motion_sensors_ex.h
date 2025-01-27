/**
  ******************************************************************************
  * @file    idp005_motion_sensors_ex.h
  * @author  System Research & Applications Team - Catania Lab.
  * @version 2.0.0
  * @date    22 October 2018
  * @brief   This file provides a set of extended functions needed
  *          to manage the motion sensors
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
#ifndef IDP005_MOTION_SENSOR_EX_H
#define IDP005_MOTION_SENSOR_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "idp005_motion_sensors.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup STEVAL-IDP005V1
 * @{
 */

/** @addtogroup STEVAL-IDP005V1_MOTION_SENSOR_EX STEVAL-IDP005V1 MOTION SENSOR EX
 * @{
 */

/** @defgroup STEVAL-IDP005V1_MOTION_SENSOR_EX_Exported_Types STEVAL-IDP005V1 MOTION SENSOR EX Exported Types
 * @{
 */

typedef enum
{
  IDP005_MOTION_SENSOR_INT1_PIN = 0,
  IDP005_MOTION_SENSOR_INT2_PIN
} IDP005_MOTION_SENSOR_IntPin_t;

typedef struct
{
  unsigned int FreeFallStatus : 1;
  unsigned int TapStatus : 1;
  unsigned int DoubleTapStatus : 1;
  unsigned int WakeUpStatus : 1;
  unsigned int StepStatus : 1;
  unsigned int TiltStatus : 1;
  unsigned int D6DOrientationStatus : 1;
} IDP005_MOTION_SENSOR_Event_Status_t;

/**
 * @}
 */

/** @addtogroup STEVAL-IDP005V1_MOTION_SENSOR_EX_Exported_Functions STEVAL-IDP005V1 MOTION SENSOR EX Exported Functions
 * @{
 */

int32_t IDP005_MOTION_SENSOR_Enable_HP_Filter(uint32_t Instance, uint8_t CutOff);
int32_t IDP005_MOTION_SENSOR_Set_INT1_DRDY(uint32_t Instance,uint8_t Status);
int32_t IDP005_MOTION_SENSOR_Set_INT2_DRDY(uint32_t Instance,uint8_t Status);
int32_t IDP005_MOTION_SENSOR_DRDY_Set_Mode(uint32_t Instance,uint8_t Status);
int32_t IDP005_MOTION_SENSOR_Clear_DRDY(uint32_t Instance);

int32_t IDP005_MOTION_SENSOR_Enable_6D_Orientation(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_6D_Orientation(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Set_6D_Orientation_Threshold(uint32_t Instance, uint8_t Threshold);
int32_t IDP005_MOTION_SENSOR_Enable_Free_Fall_Detection(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_Free_Fall_Detection(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Set_Free_Fall_Threshold(uint32_t Instance, uint8_t Threshold);
int32_t IDP005_MOTION_SENSOR_Set_Free_Fall_Duration(uint32_t Instance, uint8_t Duration);
int32_t IDP005_MOTION_SENSOR_Enable_Single_Tap_Detection(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_Single_Tap_Detection(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Enable_Double_Tap_Detection(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_Double_Tap_Detection(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Set_Tap_Threshold(uint32_t Instance, uint8_t Threshold);
int32_t IDP005_MOTION_SENSOR_Set_Tap_Shock_Time(uint32_t Instance, uint8_t Time);
int32_t IDP005_MOTION_SENSOR_Set_Tap_Quiet_Time(uint32_t Instance, uint8_t Time);
int32_t IDP005_MOTION_SENSOR_Set_Tap_Duration_Time(uint32_t Instance, uint8_t Time);
int32_t IDP005_MOTION_SENSOR_Enable_Tilt_Detection(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_Tilt_Detection(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Enable_Wake_Up_Detection(uint32_t Instance, IDP005_MOTION_SENSOR_IntPin_t IntPin);
int32_t IDP005_MOTION_SENSOR_Disable_Wake_Up_Detection(uint32_t Instance);
int32_t IDP005_MOTION_SENSOR_Set_Wake_Up_Threshold(uint32_t Instance, uint8_t Threshold);
int32_t IDP005_MOTION_SENSOR_Set_Wake_Up_Duration(uint32_t Instance, uint8_t Duration);
int32_t IDP005_MOTION_SENSOR_Get_Event_Status(uint32_t Instance, IDP005_MOTION_SENSOR_Event_Status_t *Status);
int32_t IDP005_MOTION_SENSOR_Get_DRDY_Status(uint32_t Instance, uint32_t Function, uint8_t *Status);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_XL(uint32_t Instance, uint8_t *xl);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_XH(uint32_t Instance, uint8_t *xh);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_YL(uint32_t Instance, uint8_t *yl);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_YH(uint32_t Instance, uint8_t *yh);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_ZL(uint32_t Instance, uint8_t *zl);
int32_t IDP005_MOTION_SENSOR_Get_6D_Orientation_ZH(uint32_t Instance, uint8_t *zh);
int32_t IDP005_MOTION_SENSOR_Get_Step_Count(uint32_t Instance, uint16_t *StepCount);
int32_t IDP005_MOTION_SENSOR_Read_Register(uint32_t Instance, uint8_t Reg, uint8_t *Data);
int32_t IDP005_MOTION_SENSOR_Write_Register(uint32_t Instance, uint8_t Reg, uint8_t Data);
int32_t IDP005_MOTION_SENSOR_FIFO_Get_Num_Samples(uint32_t Instance, uint16_t *NumSamples);
int32_t IDP005_MOTION_SENSOR_FIFO_Get_Full_Status(uint32_t Instance, uint8_t *Status);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_Decimation(uint32_t Instance, uint32_t Function, uint8_t Decimation);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_ODR_Value(uint32_t Instance, float Odr);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_INT1_FIFO_Full(uint32_t Instance, uint8_t Status);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full(uint32_t Instance, uint8_t Status);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_Watermark_Level(uint32_t Instance, uint16_t Watermark);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_Stop_On_Fth(uint32_t Instance, uint8_t Status);
int32_t IDP005_MOTION_SENSOR_FIFO_Set_Mode(uint32_t Instance, uint8_t Mode);
int32_t IDP005_MOTION_SENSOR_FIFO_Get_Pattern(uint32_t Instance, uint16_t *Pattern);
int32_t IDP005_MOTION_SENSOR_FIFO_Get_Data_Word(uint32_t Instance,uint32_t Function,int16_t *Data);
int32_t IDP005_MOTION_SENSOR_FIFO_Get_Axis(uint32_t Instance, uint32_t Function, int32_t *Data);
int32_t IDP005_MOTION_SENSOR_Set_SelfTest(uint32_t Instance, uint32_t Function, uint8_t Status);

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

#endif /* IDP005_MOTION_SENSOR_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
