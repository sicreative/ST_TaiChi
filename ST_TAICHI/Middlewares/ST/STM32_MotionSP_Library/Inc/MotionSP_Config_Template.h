/**
  ******************************************************************************
  * @file           : MotionSP_Config_Template.h
  * @author         : System Research & Applications Team - Catania Lab
  * @version        : v2.2.0
  * @date           : 30-May-2019
  * @brief          : MotionSP configuration file for motion signal processing.
  *                   This file should be copied to the application folder and
  *                   renamed to MotionSP_Config.h.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#ifndef __MOTIONSP_CONFIG_H
#define __MOTIONSP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Refer to preprocessor defined symbols */
#include "stm32yyxx_hal.h"

/** @addtogroup MIDDLEWARES Middlewares
  * @{
  */

/** @addtogroup ST ST
  * @{
  */

/** @addtogroup STM32_MOTIONSP_LIB STM32 Motion Signal Processing Library
  * @{
  */

/** @addtogroup STM32_MOTIONSP_LIB_EXPORTED_DEFINES STM32 Motion Signal Processing Library Exported Defines
  * @{
  */

/* #define USE_SUBRANGE */                        //!< Uncomment this define for enabling subrange

#define NUM_AXES              3             //!< Number of sensor axes

#define FFT_SIZE_DEFAULT      FFT_SIZE_MAX  //!< Default value for FFT size
#define FFT_OVL_DEFAULT       FFT_OVL_MAX   //!< Default value for OVERLAPPING
#define TAU_DEFAULT           50            //!< Default value for Moving RMS Filtering in ms
#define WINDOW_DEFAULT        HANNING       //!< Default value for Windowing Method
#define TD_DEFAULT            TD_SPEED      //!< Default value for Time Domain Analysis
#define TACQ_DEFAULT          5000          //!< Defualt value for Total acquisition time in ms

#define CIRC_BUFFER_RATIO_NUM 12u           //!< Buffer Ratio numerator
#define CIRC_BUFFER_RATIO_DEN 10u           //!< Buffer Ratio denominator
#define FFT_SIZE_256          256u          //!< FFT will be performed on 256 samples
#define FFT_SIZE_512          512u          //!< FFT will be performed on 512 samples
#define FFT_SIZE_1024         1024u         //!< FFT will be performed on 1024 samples
#define FFT_SIZE_2048         2048u         //!< FFT will be performed on 2048 samples
#define FFT_SIZE_MAX          FFT_SIZE_2048 //!< Max FFT size
#define FFT_OVL_MIN           5             //!< Max FFT overlapping
#define FFT_OVL_MAX           70            //!< Max FFT overlapping
#define MAG_SIZE_MAX          (uint16_t)(FFT_SIZE_MAX/2)                      //!< Max MAG size
#define CIRC_BUFFER_SIZE_MAX  (uint16_t)((FFT_SIZE_MAX*CIRC_BUFFER_RATIO_NUM)/CIRC_BUFFER_RATIO_DEN) //!< Max circular buffer for storing input values for FFT

#define DC_SMOOTH             0.975f        //!< Smooth parameter used for DC filtering  
#define GAMMA                 0.5f          //!< GAMMA parameter used for Integration Algorithm               
  
#define G_CONST               9.80665f                //!< in m/s^2
#define G_CONV                (float)(G_CONST/1000.0) //!< CONSTANT for conversion from mm/s^2 to m/s^2

#ifdef USE_SUBRANGE
  #define SUBRANGE_DEFAULT      8           //!< Default value for FFT output subranges
  #define SUBRANGE_MAX          64          //!< Default value for MAX Subranges to analyze
#endif /* USE_SUBRANGE */

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

#endif /* __MOTIONSP_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
