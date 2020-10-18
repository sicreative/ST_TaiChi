/**
  ******************************************************************************
  * @file    sensor_service.h 
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Sensors services APIs
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
#ifndef _SENSOR_SERVICE_H_
#define _SENSOR_SERVICE_H_

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "TargetFeatures.h"

#include "bluenrg1_gatt_server.h"
#include "bluenrg1_gap.h"
#include "string.h"
#include "bluenrg1_gap_aci.h"
#include "bluenrg1_gatt_aci.h"
#include "hci_const.h"
#include "bluenrg1_hal_aci.h"  
#include "hci.h"
#include "bluenrg1_hci_le.h"
   
//#include "sm.h"
//#include "gatt_db.h"
//   
//#include "hci_tl.h"
//#include "hci_le.h"


#include <stdlib.h>
#include "main.h"



/* Exported functions ------------------------------------------------------- */
extern tBleStatus Add_HW_ServW2ST_Service(void);
extern tBleStatus AccGyroMag_Update(MOTION_SENSOR_Axes_t *Acc,MOTION_SENSOR_Axes_t *Gyro,MOTION_SENSOR_Axes_t *Mag);
extern tBleStatus Environmental_Update(int32_t Press,uint16_t Hum,int16_t Temp2,int16_t Temp1);
extern tBleStatus AudioLevel_Update(uint16_t *Mic);
extern tBleStatus BatteryInfo_Update(uint32_t BatteryLevel, uint32_t Voltage, stbc02_State_TypeDef BC_State);

extern tBleStatus Add_SW_ServW2ST_Service(void);
extern tBleStatus FFT_Amplitude_Update(uint8_t *TotalBuff, uint16_t ActualMagSize, uint8_t *SendingFFT, uint16_t *CountSendData);
extern tBleStatus TimeDomain_Update(sAcceleroParam_t *sTimeDomain);
extern tBleStatus FFT_AlarmSpeedRMS_Status_Update(sTimeDomainAlarm_t *pTdAlarm, sAcceleroParam_t *sTimeDomainVal);
extern tBleStatus FFT_AlarmAccStatus_Update(sTimeDomainAlarm_t *pTdAlarm, sAcceleroParam_t *sTimeDomainVal);
extern tBleStatus FFT_AlarmSubrangeStatus_Update(sAxesMagResults_t *AccAxesMagResults,sFreqDomainAlarm_t *THR_Fft_Alarms, uint16_t SubrangeNum, uint16_t ActualMagSize);

extern tBleStatus Add_ConsoleW2ST_Service(void);
extern tBleStatus Stderr_Update(uint8_t *data,uint8_t length);
extern tBleStatus Term_Update(uint8_t *data,uint8_t length);

extern tBleStatus Add_ConfigW2ST_Service(void);
extern tBleStatus Config_Notify(uint32_t Feature,uint8_t Command,uint8_t val);

extern void		  setBeacon(void);
extern void       setConnectable(void);



extern tBleStatus Add_TaiChi_ServW2ST_Service(void);
extern tBleStatus TaiChi_Update(uint8_t buff[]);



/* Exported variables --------------------------------------------------------*/

/* For enabling the capability to handle BlueNRG Congestion */
#define ACC_BLUENRG_CONGESTION

#ifdef ACC_BLUENRG_CONGESTION
/* For defining how many events skip when there is a congestion */
#define ACC_BLUENRG_CONGESTION_SKIP 30
#endif /* ACC_BLUENRG_CONGESTION */

/*************** Don't Change the following defines *************/

/* Define the Max dimesion of the Bluetooth characteristics for each packet */
#define W2ST_MAX_CHAR_LEN 20

/* Define the symbol used for defining each termination string
used in Console service */
#define W2ST_CONSOLE_END_STRING "\0"

/* @brief  Scale factor. It is used to scale acceleration from mg to g */ 
#define FROM_MG_TO_G    0.001

/* Feature mask for Temperature1 */
#define FEATURE_MASK_TEMP1 0x00040000

/* Feature mask for Temperature2 */
#define FEATURE_MASK_TEMP2 0x00010000

/* Feature mask for Pressure */
#define FEATURE_MASK_PRESS 0x00100000

/* Feature mask for Humidity */
#define FEATURE_MASK_HUM   0x00080000

/* Feature mask for Accelerometer */
#define FEATURE_MASK_ACC   0x00800000

/* Feature mask for Gyroscope */
#define FEATURE_MASK_GRYO  0x00400000

/* Feature mask for Magnetometer */
#define FEATURE_MASK_MAG   0x00200000

/* Feature mask for Microphone */
#define FEATURE_MASK_MIC   0x04000000

/* BLE Characteristic connection control */

/* Environmental Data */
#define W2ST_CONNECT_ENV           (1   )

/* Acceleration/Gyroscope/Magneto */
#define W2ST_CONNECT_ACC_GYRO_MAG  (1<<2)

/* Mic */
#define W2ST_CONNECT_AUDIO_LEVEL   (1<<3)

/* FFT Amplitude */
#define W2ST_CONNECT_FFT_AMPLITUDE (1<<4)

/* Time Domain */
#define W2ST_CONNECT_TIME_DOMAIN   (1<<5)

/* FFT Alarm Speed RMS Staus */
#define W2ST_CONNECT_FFT_ALARM_SPEED_RMS_STATUS (1<<6)

/* FFT Alarm Acc Status */
#define W2ST_CONNECT_FFT_ALARM_ACC_STATUS       (1<<7)

/* FFT Alarm Subrange Status */
#define W2ST_CONNECT_FFT_ALARM_SUBRANGE_STATUS  (1<<8)

/* Battery Info Feature */
#define W2ST_CONNECT_BATTERY_INFO       (1<<9)

/* Standard Terminal */
#define W2ST_CONNECT_STD_TERM           (1<<10)

/* Standard Error */
#define W2ST_CONNECT_STD_ERR            (1<<11)

/* TaiChi*/
#define W2ST_CONNECT_TAICHI				(1<<12)
// Max num of items every bluetooth event can be send
#define W2ST_TAICHI_MAX_BUFF_LEN		4
// Max num of items waiting to send
#define BUFF_TAICHIRESULT_SIZE 			10


#define W2ST_CHECK_CONNECTION(BleChar) ((ConnectionBleStatus&(BleChar)) ? 1 : 0)
#define W2ST_ON_CONNECTION(BleChar)    (ConnectionBleStatus|=(BleChar))
#define W2ST_OFF_CONNECTION(BleChar)   (ConnectionBleStatus&=(~BleChar))

#ifdef __cplusplus
}
#endif

#endif /* _SENSOR_SERVICE_H_ */

/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
