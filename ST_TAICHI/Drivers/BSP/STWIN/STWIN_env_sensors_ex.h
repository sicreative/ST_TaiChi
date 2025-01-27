/**
******************************************************************************
* @file    STWIN_env_sensors_ex.h
* @author  SRA - Central Labs
* @version v1.3.0
* @date    17-Jan-2020
* @brief   This file provides a set of extended functions needed to manage the environmental sensors
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2019 STMicroelectronics. 
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the 
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STWIN_ENV_SENSOR_EX_H
#define STWIN_ENV_SENSOR_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "STWIN_env_sensors.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup STWIN STWIN
 * @{
 */

/** @addtogroup ENV_SENSOR_EX ENV_SENSOR_EX
 * @{
 */

/** @addtogroup ENV_SENSOR_EX_Exported_Functions ENV_SENSOR_EX Exported Functions
 * @{
 */

int32_t BSP_ENV_SENSOR_Get_DRDY_Status(uint32_t Instance, uint32_t Function, uint8_t *Status);
int32_t BSP_ENV_SENSOR_Read_Register(uint32_t Instance, uint8_t Reg, uint8_t *Data);
int32_t BSP_ENV_SENSOR_Write_Register(uint32_t Instance, uint8_t Reg, uint8_t Data);
int32_t BSP_ENV_SENSOR_Set_High_Temperature_Threshold(uint32_t Instance, float Value);
int32_t BSP_ENV_SENSOR_Set_Low_Temperature_Threshold(uint32_t Instance, float Value);
int32_t BSP_ENV_SENSOR_Get_Temperature_Limit_Status(uint32_t Instance, uint8_t *HighLimit, uint8_t *LowLimit, uint8_t *ThermLimit);
int32_t BSP_ENV_SENSOR_Set_Event_Pin(uint32_t Instance, uint8_t Enable);
int32_t BSP_ENV_SENSOR_FIFO_Get_Data(uint32_t Instance, float *Press, float *Temp);
int32_t BSP_ENV_SENSOR_FIFO_Get_Fth_Status(uint32_t Instance, uint8_t *Status);
int32_t BSP_ENV_SENSOR_FIFO_Get_Full_Status(uint32_t Instance, uint8_t *Status);
int32_t BSP_ENV_SENSOR_FIFO_Get_Num_Samples(uint32_t Instance, uint8_t *NumSamples);
int32_t BSP_ENV_SENSOR_FIFO_Get_Ovr_Status(uint32_t Instance, uint8_t *Status);
int32_t BSP_ENV_SENSOR_FIFO_Reset_Interrupt(uint32_t Instance, uint8_t Interrupt);
int32_t BSP_ENV_SENSOR_FIFO_Set_Interrupt(uint32_t Instance, uint8_t Interrupt);
int32_t BSP_ENV_SENSOR_FIFO_Set_Mode(uint32_t Instance, uint8_t Mode);
int32_t BSP_ENV_SENSOR_FIFO_Set_Watermark_Level(uint32_t Instance, uint8_t Watermark);
int32_t BSP_ENV_SENSOR_FIFO_Stop_On_Watermark(uint32_t Instance, uint8_t Stop);
int32_t BSP_ENV_SENSOR_Set_One_Shot(uint32_t Instance);
int32_t BSP_ENV_SENSOR_Get_One_Shot_Status(uint32_t Instance, uint8_t *Status);


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

#endif /* STWIN_ENV_SENSOR_EX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
