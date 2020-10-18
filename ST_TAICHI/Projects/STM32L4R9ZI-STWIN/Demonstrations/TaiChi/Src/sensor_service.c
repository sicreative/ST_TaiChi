/**
  ******************************************************************************
  * @file    sensor_service.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Add 4 bluetooth services using vendor specific profiles.
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
#include <stdio.h>
#include "TargetFeatures.h"
#include "main.h"
#include "sensor_service.h"
//#include "MetaDataManager.h"
//#include "console.h"
#include "bluenrg1_l2cap_aci.h"
#include "uuid_ble_service.h"
#include "OTA.h"

/** @addtogroup Projects
  * @{
  */

/** @addtogroup DEMONSTRATIONS Demonstrations
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE Predictive Maintenance BLE
  * @{
  */

/** @addtogroup PREDCTIVE_MAINTENANCE_SENSOR_SERVICE Predictive Maintenance sensor service
  * @{
  */

/* Exported variables ---------------------------------------------------------*/
int connected = FALSE;
uint8_t set_connectable = FALSE;

// Indicate the iBeacon is advertising
uint8_t isBeacon = TRUE;


volatile uint32_t FeatureMask;

/* Imported Variables -------------------------------------------------------------*/
extern uint32_t ConnectionBleStatus;
  
extern uint32_t FirstConnectionConfig;

extern TIM_HandleTypeDef    TimCCHandle;
extern TIM_HandleTypeDef    TimEnvHandle;
extern TIM_HandleTypeDef    TimAudioDataHandle;

extern volatile uint32_t PredictiveMaintenance;

extern volatile uint32_t FFT_Amplitude;
extern volatile uint32_t FFT_Alarm;

extern sAccelerometer_Parameter_t Accelerometer_Parameters;
extern uint8_t IsFirstTime;

extern volatile float RMS_Ch[];

extern float DBNOISE_Value_Old_Ch[];

extern uint8_t bdaddr[6];
extern uint8_t NodeName[8];

extern uint32_t uhCCR4_Val;




/* Private variables ------------------------------------------------------------*/
#ifdef DISABLE_FOTA
static uint32_t FirstCommandSent= 1;
#endif /* DISABLE_FOTA */

static uint16_t HWServW2STHandle;
static uint16_t EnvironmentalCharHandle;
static uint16_t AccGyroMagCharHandle;
static uint16_t AudioLevelCharHandle;
static uint16_t BatteryFeaturesCharHandle;

static uint16_t SWServW2STHandle;
static uint16_t FFTAmplitudeCharHandle;
static uint16_t TimeDomainCharHandle;
static uint16_t FFTAlarmSpeedRMS_StatusCharHandle;
static uint16_t FFTAlarmAccStatusCharHandle;
static uint16_t FFTAlarmSubrangeStatusCharHandle;

static uint16_t ConfigServW2STHandle;
static uint16_t ConfigCharHandle;

static uint16_t ConsoleW2STHandle;
static uint16_t TermCharHandle;
static uint16_t StdErrCharHandle;


static uint16_t TaiChiServW2STHandle;
static uint16_t TaiChiCharHandle;


static uint8_t LastStderrBuffer[W2ST_MAX_CHAR_LEN];
static uint8_t LastStderrLen;
static uint8_t LastTermBuffer[W2ST_MAX_CHAR_LEN];
static uint8_t LastTermLen;

static uint8_t VibrParam[80];

static uint8_t  EnvironmentalCharSize = 2; /* Size for Environmental BLE characteristic */

static uint32_t SizeOfUpdateBlueFW=0;

static uint16_t connection_handle = 0;

volatile uint8_t BLE_Buffer_Full=0;

Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

/* Private functions ------------------------------------------------------------*/
//static void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
//static void GAP_DisconnectionComplete_CB(void);
static uint32_t DebugConsoleCommandParsing(uint8_t * att_data, uint8_t data_length);
static uint32_t ConfigCommandParsing(uint8_t * att_data, uint8_t data_length);
static void Read_Request_CB(uint16_t handle);
static void Attribute_Modified_Request_CB(uint16_t Connection_Handle, uint16_t attr_handle, uint16_t Offset, uint8_t data_length, uint8_t *att_data);

static void Force_UUID_Rescan(void);
static uint8_t VibrationParametersCommandParsing(uint8_t CommandLenght);

/************************************/
/* Hardware Characteristics Service */
/************************************/
static void Environmental_AttributeModified_CB(uint8_t *att_data);
static void AccGyroMag_AttributeModified_CB(uint8_t *att_data);
static void AudioLevel_AttributeModified_CB(uint8_t *att_data);
static void BatteryFeatures_AttributeModified_CB(uint8_t *att_data);

/***********************************
* Software Characteristics Service *
************************************/
static void FFTAmplitude_AttributeModified_CB(uint8_t *att_data);
static void TimeDomain_AttributeModified_CB(uint8_t *att_data);
static void FFTAlarmSpeedRMS_AttributeModified_CB(uint8_t *att_data);
static void FFTAlarmAccStatus_AttributeModified_CB(uint8_t *att_data);
static void FFTAlarmSubrangeStatus_AttributeModified_CB(uint8_t *att_data);

/* Private define ------------------------------------------------------------*/
static void TaiChi_AttributeModified_CB(uint8_t *att_data);

#ifdef ACC_BLUENRG_CONGESTION
#define ACI_GATT_UPDATE_CHAR_VALUE safe_aci_gatt_update_char_value
static int32_t breath;


/* @brief  Update the value of a characteristic avoiding (for a short time) to
 *         send the next updates if an error in the previous sending has
 *         occurred.
 * @param  servHandle The handle of the service
 * @param  charHandle The handle of the characteristic
 * @param  charValOffset The offset of the characteristic
 * @param  charValueLen The length of the characteristic
 * @param  charValue The pointer to the characteristic
 * @retval tBleStatus Status
 */
tBleStatus safe_aci_gatt_update_char_value(uint16_t servHandle, 
				      uint16_t charHandle,
				      uint8_t charValOffset,
				      uint8_t charValueLen,   
				      uint8_t *charValue)
{
  tBleStatus ret = BLE_STATUS_INSUFFICIENT_RESOURCES;
  
  if (breath > 0) {
    breath--;
  } else {
    ret = aci_gatt_update_char_value(servHandle,charHandle,charValOffset,charValueLen,charValue);
    
    if (ret != BLE_STATUS_SUCCESS){
      breath = ACC_BLUENRG_CONGESTION_SKIP;
    }
  }
  
  return (ret);
}

#else /* ACC_BLUENRG_CONGESTION */
#define ACI_GATT_UPDATE_CHAR_VALUE aci_gatt_update_char_value
#endif /* ACC_BLUENRG_CONGESTION */


/**
 * @brief  Add the Config service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_ConfigW2ST_Service(void)
{
  tBleStatus ret;

  uint8_t uuid[16];

  COPY_CONFIG_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128,  &service_uuid, PRIMARY_SERVICE, 1+3,&ConfigServW2STHandle);


  if (ret != BLE_STATUS_SUCCESS)
    goto fail;

  COPY_CONFIG_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(ConfigServW2STHandle, UUID_TYPE_128, &char_uuid, W2ST_MAX_CHAR_LEN,
                           CHAR_PROP_NOTIFY| CHAR_PROP_WRITE_WITHOUT_RESP,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_ATTRIBUTE_WRITE | GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 1, &ConfigCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
 
  return BLE_STATUS_SUCCESS;

fail:
  //PREDMNT1_PRINTF("Error while adding Configuration service.\n");
  return BLE_STATUS_ERROR;
}


/**
 * @brief  Add the Console service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_ConsoleW2ST_Service(void)
{
  tBleStatus ret;

  uint8_t uuid[16];

  COPY_CONSOLE_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128,  &service_uuid, PRIMARY_SERVICE, 1+3*2,&ConsoleW2STHandle);

  

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_TERM_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(ConsoleW2STHandle, UUID_TYPE_128, &char_uuid, W2ST_MAX_CHAR_LEN,
                           CHAR_PROP_NOTIFY| CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_READ ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_ATTRIBUTE_WRITE | GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 1, &TermCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_STDERR_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(ConsoleW2STHandle, UUID_TYPE_128, &char_uuid, W2ST_MAX_CHAR_LEN,
                           CHAR_PROP_NOTIFY | CHAR_PROP_READ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 1, &StdErrCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
     goto fail;
  }







  return BLE_STATUS_SUCCESS;

fail:
  //PREDMNT1_PRINTF("Error while adding Console service.\n");
  return BLE_STATUS_ERROR;
}

/**
 * @brief  Update Stderr characteristic value
 * @param  uint8_t *data string to write
 * @param  uint8_t lenght lengt of string to write
 * @retval tBleStatus      Status
 */
tBleStatus Stderr_Update(uint8_t *data,uint8_t length)
{
  tBleStatus ret;
  uint8_t Offset;
  uint8_t DataToSend;

  /* Split the code in packages*/
  for(Offset =0; Offset<length; Offset +=W2ST_MAX_CHAR_LEN){
    DataToSend = (length-Offset);
    DataToSend = (DataToSend>W2ST_MAX_CHAR_LEN) ?  W2ST_MAX_CHAR_LEN : DataToSend;

    /* keep a copy */
    memcpy(LastStderrBuffer,data+Offset,DataToSend);
    LastStderrLen = DataToSend;

    ret = aci_gatt_update_char_value(ConsoleW2STHandle, StdErrCharHandle, 0, DataToSend , data+Offset);
    if (ret != BLE_STATUS_SUCCESS) {
      return BLE_STATUS_ERROR;
    }
    HAL_Delay(10);
  }

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update Terminal characteristic value
 * @param  uint8_t *data string to write
 * @param  uint8_t lenght lengt of string to write
 * @retval tBleStatus      Status
 */
tBleStatus Term_Update(uint8_t *data,uint8_t length)
{
  tBleStatus ret;
  uint8_t Offset;
  uint8_t DataToSend;

  /* Split the code in packages */
  for(Offset =0; Offset<length; Offset +=W2ST_MAX_CHAR_LEN){
    DataToSend = (length-Offset);
    DataToSend = (DataToSend>W2ST_MAX_CHAR_LEN) ?  W2ST_MAX_CHAR_LEN : DataToSend;

    /* keep a copy */
    memcpy(LastTermBuffer,data+Offset,DataToSend);
    LastTermLen = DataToSend;

    ret = aci_gatt_update_char_value(ConsoleW2STHandle, TermCharHandle, 0, DataToSend , data+Offset);
    if (ret != BLE_STATUS_SUCCESS) {
        PREDMNT1_PRINTF("Error Updating Stdout Char\r\n");
      return BLE_STATUS_ERROR;
    }
    HAL_Delay(20);
  }

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update Stderr characteristic value after a read request
 * @param None
 * @retval tBleStatus      Status
 */
static tBleStatus Stderr_Update_AfterRead(void)
{
  tBleStatus ret;

  ret = aci_gatt_update_char_value(ConsoleW2STHandle, StdErrCharHandle, 0, LastStderrLen , LastStderrBuffer);
  if (ret != BLE_STATUS_SUCCESS) {
    return BLE_STATUS_ERROR;
  }

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update Terminal characteristic value after a read request
 * @param None
 * @retval tBleStatus      Status
 */
static tBleStatus Term_Update_AfterRead(void)
{
  tBleStatus ret;

  ret = aci_gatt_update_char_value(ConsoleW2STHandle, TermCharHandle, 0, LastTermLen , LastTermBuffer);
  if (ret != BLE_STATUS_SUCCESS) {
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating Stdout Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Stdout Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }

  return BLE_STATUS_SUCCESS;
}

/* @brief  Send a notification for answering to a configuration command for Accelerometer events
 * @param  uint32_t Feature Feature calibrated
 * @param  uint8_t Command Replay to this Command
 * @param  uint8_t data result to send back
 * @retval tBleStatus Status
 */
tBleStatus Config_Notify(uint32_t Feature,uint8_t Command,uint8_t data)
{
  tBleStatus ret;
  uint8_t buff[2+4+1+1];

  STORE_LE_16(buff  ,(HAL_GetTick()>>3));
  STORE_BE_32(buff+2,Feature);
  buff[6] = Command;
  buff[7] = data;

  ret = aci_gatt_update_char_value (ConfigServW2STHandle, ConfigCharHandle, 0, 8,buff);
  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating Configuration Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Configuration Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Add the HW Features service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_HW_ServW2ST_Service(void)
{
  tBleStatus ret;

  uint8_t uuid[16];
  
  /* Environmental */
  uint8_t max_attr_records= 4;
  
  COPY_HW_SENS_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE,
                             1+3*max_attr_records,
                             &HWServW2STHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  /* Fill the Environmental BLE Characteristc */
  COPY_ENVIRONMENTAL_W2ST_CHAR_UUID(uuid);
  if(TargetBoardFeatures.NumTempSensors==2) {
    uuid[14] |= 0x05; /* Two Temperature values*/
    EnvironmentalCharSize+=2*2;
  } else if(TargetBoardFeatures.NumTempSensors==1) {
    uuid[14] |= 0x04; /* One Temperature value*/
    EnvironmentalCharSize+=2;
  }

  /* Humidity Value */
  if(TargetBoardFeatures.HumSensorIsInit)
  {
   uuid[14] |= 0x08;
   EnvironmentalCharSize+=2;
  }

  /* Pressure value*/
  if(TargetBoardFeatures.PressSensorIsInit)
  {
    uuid[14] |= 0x10;
    EnvironmentalCharSize+=4;
  }

  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid, EnvironmentalCharSize,
                           CHAR_PROP_NOTIFY|CHAR_PROP_READ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &EnvironmentalCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_ACC_GYRO_MAG_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+3*3*2,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &AccGyroMagCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  COPY_MIC_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid,2+AUDIO_IN_CHANNELS,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &AudioLevelCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  COPY_BATTERY_INFO_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+2+2+2+1,
                           CHAR_PROP_NOTIFY | CHAR_PROP_READ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &BatteryFeaturesCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  return BLE_STATUS_SUCCESS;

fail:
  //PREDMNT1_PRINTF("Error while adding HW's Characteristcs service.\n");
  return BLE_STATUS_ERROR;
}



/**
 * @brief  Add the TaiChi Feature service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_TaiChi_ServW2ST_Service(void)
{
  tBleStatus ret;
  int32_t NumberOfRecords=4;

  uint8_t uuid[16];

  COPY_TAICHI_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128,  &service_uuid, PRIMARY_SERVICE,
                          1+3*NumberOfRecords,
                          &TaiChiServW2STHandle);




  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }





  COPY_TAICHI_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(TaiChiServW2STHandle, UUID_TYPE_128, &char_uuid, 2+2+6*W2ST_TAICHI_MAX_BUFF_LEN,
                           CHAR_PROP_NOTIFY ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 1, &TaiChiCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }





  return BLE_STATUS_SUCCESS;

fail:
  //PREDMNT1_PRINTF("Error while adding TaiChi's service. %2x\n",ret);
  return BLE_STATUS_ERROR;
}


/**
 * @brief  Add the SW Feature service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_SW_ServW2ST_Service(void)
{
  tBleStatus ret;
  int32_t NumberOfRecords=5;

  uint8_t uuid[16];

  COPY_SW_SENS_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128,  &service_uuid, PRIMARY_SERVICE,
                          1+3*NumberOfRecords,
                          &SWServW2STHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  



  COPY_FFT_AMPLITUDE_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, &char_uuid, 20,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 1, &FFTAmplitudeCharHandle);
  
  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
 
  COPY_TIME_DOMAIN_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+18,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &TimeDomainCharHandle);
  
  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
  
  COPY_FFT_ALARM_SPEED_STATUS_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+13,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &FFTAlarmSpeedRMS_StatusCharHandle);
  
  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }
    
  COPY_FFT_ALARM_ACC_STATUS_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+13,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &FFTAlarmAccStatusCharHandle);
  
  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_FFT_ALARM_SUBRANGE_STATUS_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, &char_uuid, 2+13,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &FFTAlarmSubrangeStatusCharHandle);
  
  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  return BLE_STATUS_SUCCESS;

fail:
  //PREDMNT1_PRINTF("Error while adding SW's Characteristcs service.\n");
  return BLE_STATUS_ERROR;
}


/**
 * @brief  Update acceleration/Gryoscope and Magneto characteristics value
 * @param  MOTION_SENSOR_Axes_t Acc Structure containing acceleration value in mg
 * @param  MOTION_SENSOR_Axes_t Gyro Structure containing Gyroscope value
 * @param  MOTION_SENSOR_Axes_t Mag Structure containing magneto value
 * @retval tBleStatus      Status
 */
tBleStatus AccGyroMag_Update(MOTION_SENSOR_Axes_t *Acc,MOTION_SENSOR_Axes_t *Gyro,MOTION_SENSOR_Axes_t *Mag)
{  
  tBleStatus ret;

  uint8_t buff[2+3*3*2];

  STORE_LE_16(buff   ,(HAL_GetTick()>>3));
  
  STORE_LE_16(buff+2 ,Acc->x);
  STORE_LE_16(buff+4 ,Acc->y);
  STORE_LE_16(buff+6 ,Acc->z);
  
  Gyro->x/=100;
  Gyro->y/=100;
  Gyro->z/=100;

  STORE_LE_16(buff+8 ,Gyro->x);
  STORE_LE_16(buff+10,Gyro->y);
  STORE_LE_16(buff+12,Gyro->z);
  
  STORE_LE_16(buff+14,Mag->x);
  STORE_LE_16(buff+16,Mag->y);
  STORE_LE_16(buff+18,Mag->z);
  
  ret = ACI_GATT_UPDATE_CHAR_VALUE(HWServW2STHandle, AccGyroMagCharHandle, 0, 2+3*3*2, buff);
	
  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating Acc/Gyro/Mag Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Acc/Gyro/Mag Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;	
}


/**
 * @brief  Update Environmental characteristic value
 * @param  int32_t Press Pressure in mbar
 * @param  uint16_t Hum humidity RH (Relative Humidity) in thenths of %
 * @param  int16_t Temp2 Temperature in tenths of degree second sensor
 * @param  int16_t Temp1 Temperature in tenths of degree first sensor
 * @retval tBleStatus   Status
 */
tBleStatus Environmental_Update(int32_t Press,uint16_t Hum,int16_t Temp2,int16_t Temp1)
{
  tBleStatus ret;
  uint8_t BuffPos;
  
  uint8_t buff[2+4/*Press*/+2/*Hum*/+2/*Temp2*/+2/*Temp1*/];

  STORE_LE_16(buff  ,(HAL_GetTick()>>3));
  BuffPos=2;

  if(TargetBoardFeatures.PressSensorIsInit)
  {
    STORE_LE_32(buff+BuffPos,Press);
    BuffPos+=4;
  }

  if(TargetBoardFeatures.HumSensorIsInit)
  {
    STORE_LE_16(buff+BuffPos,Hum);
    BuffPos+=2;
  }

  if(TargetBoardFeatures.NumTempSensors==2) {
    if(TargetBoardFeatures.TempSensorsIsInit[0]){
      STORE_LE_16(buff+BuffPos,Temp2);
      BuffPos+=2;
    }
    
    if(TargetBoardFeatures.TempSensorsIsInit[1]){
      STORE_LE_16(buff+BuffPos,Temp1);
      BuffPos+=2;
    }
  } else if(TargetBoardFeatures.NumTempSensors==1) {
    if(TargetBoardFeatures.TempSensorsIsInit[1]){
      STORE_LE_16(buff+BuffPos,Temp1);
      BuffPos+=2;
    }
  }

  ret = aci_gatt_update_char_value(HWServW2STHandle, EnvironmentalCharHandle, 0, EnvironmentalCharSize,buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating Environmental Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Environmental Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }
  
  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update Microphones characteristic values
 * @param  uint16_t *Mic SNR dB Microphones array
 * @retval tBleStatus   Status
 */
tBleStatus AudioLevel_Update(uint16_t *Mic)
{  
  tBleStatus ret;
  uint16_t Counter;
  
  uint8_t buff[2+1*AUDIO_IN_CHANNELS]; /* BlueCoin has 4 Mics */

  STORE_LE_16(buff  ,(HAL_GetTick()>>3));
  for(Counter=0;Counter<AUDIO_IN_CHANNELS;Counter++) {
    buff[2+Counter]= Mic[Counter]&0xFF;
  }
    
  ret = ACI_GATT_UPDATE_CHAR_VALUE(HWServW2STHandle, AudioLevelCharHandle, 0, 2+AUDIO_IN_CHANNELS,buff);
  
  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite = sprintf((char *)BufferToWrite, "Error Updating Mic Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    }
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;	
}




/**
 * @brief  Update TaiChi value
 * @param  taiChiResult*[]
 * @retval tBleStatus   Status
 */
tBleStatus TaiChi_Update(uint8_t buff[])
{



	tBleStatus ret = aci_gatt_update_char_value(TaiChiServW2STHandle, TaiChiCharHandle, 0, 4+W2ST_TAICHI_MAX_BUFF_LEN*6,buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite = sprintf((char *)BufferToWrite, "Error Updating TaiChi Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating TaiChi Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;
}



/**
 * @brief  Update Battery Info value
 * @param  None
 * @retval tBleStatus   Status
 */
tBleStatus BatteryInfo_Update(uint32_t BatteryLevel, uint32_t Voltage, stbc02_State_TypeDef BC_State)
{
  tBleStatus ret;

  uint8_t buff[2+2+2+2+1];

  STORE_LE_16(buff  ,(HAL_GetTick()>>3));
  STORE_LE_16(buff+2,BatteryLevel*10);
  STORE_LE_16(buff+4,Voltage);
  STORE_LE_16(buff+6,-32768);

  /* if it's < 15% Low Battery*/
  if(BatteryLevel < 15) {
    /* Low Battery */
    buff[8] = 0x00; 
  } else {
    /* Unknown */
    static uint32_t Status= 0x04;
    
    /* Charging */
    if(BC_State.Id == ChargingPhase)
      Status = 0x03;
    
    /* Plugget Not Charging */
    if(BC_State.Id == EndOfCharge)
      Status = 0x02;
    
    /* Discharging */
    if(BC_State.Id == ValidInput)
      Status = 0x01;
    
    buff[8] = Status;
  }

  ret = aci_gatt_update_char_value(HWServW2STHandle, BatteryFeaturesCharHandle, 0, 2+2+2+2+1,buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite = sprintf((char *)BufferToWrite, "Error Updating Battery Info Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Battery Info Char\r\n");
    }
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;
}


/*
 * @brief  Update FFT Amplitude characteristic value
 * @param  uint8_t *TotalBuff
 * @param  uint16_t ActualMagSize Number of samples
 * @param  uint8_t *SendingFFT
 * @param  uint16_t *CountSendData
 * @retval tBleStatus   Status
 */
tBleStatus FFT_Amplitude_Update(uint8_t *TotalBuff, uint16_t ActualMagSize, uint8_t *SendingFFT, uint16_t *CountSendData)
{
  tBleStatus ret;
  
  uint16_t TotalSize;
  
  uint16_t index;
  uint16_t indexStart;
  uint16_t indexStop;
  
  uint8_t Buff[20];
  
  uint8_t  NumByteSent;

  TotalSize= 2 /* nSample */ + 1 /* nComponents */ + 4 /*  Frequency Steps */ + ((3 * ActualMagSize) * 4) /* Samples */;

  indexStart= 20 * (*CountSendData);
  indexStop=  20 * ((*CountSendData) + 1);

  NumByteSent= 20;
  
  if(indexStop > TotalSize)
  {
    indexStop= TotalSize;
    NumByteSent= TotalSize % NumByteSent;
  }
  
  for(index=indexStart; index<indexStop; index++)
  {
    Buff[index - indexStart]= TotalBuff[index];
  }
  
  if(!BLE_Buffer_Full)
  {
    ret = aci_gatt_update_char_value(SWServW2STHandle, FFTAmplitudeCharHandle, 0, NumByteSent,Buff);
    
    if (ret != BLE_STATUS_SUCCESS)
    {
      /* When the requested operation failed for a temporary lack of resources, repeats sending after 200 ms*/
      if(ret == BLE_STATUS_INSUFFICIENT_RESOURCES)
      {
        BLE_Buffer_Full = 1;
      }
      else
      {
        return BLE_STATUS_ERROR;
      }
    }
    else
    {
      (*CountSendData)++;
      
      if(indexStop == TotalSize)
      {
        *SendingFFT= 0;
        *CountSendData= 0;
      }
    }
  }

  return BLE_STATUS_SUCCESS;
}

/**
  * @brief Each time BLE FW stack raises the error code @ref ble_status_insufficient_resources (0x64),
           the @ref aci_gatt_tx_pool_available_event event is generated as soon as the available buffer size 
           is greater than maximum ATT MTU (on stack versions below v2.1 this event is generated when at least 2 packets
           with MTU of 23 bytes are available).
  * @param Connection_Handle Connection handle related to the request
  * @param Available_Buffers Not used.
  * @retval None
*/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle, uint16_t Available_Buffers)
{
  BLE_Buffer_Full=0;
}

/*
 * @brief  Update Time Domain characteristic value
 * @param  sAcceleroParam_t *sTimeDomain
 * @retval tBleStatus   Status
 */
tBleStatus TimeDomain_Update(sAcceleroParam_t *sTimeDomain)
{
  tBleStatus ret;
  uint16_t Temp;
  
  uint8_t Buff[2 + 18];
  uint8_t BuffPos;
  
  float TempFloat;
  uint8_t *TempBuff = (uint8_t *) & TempFloat;
  
  STORE_LE_16(Buff  ,(HAL_GetTick()>>3));

  Temp= (uint16_t)(sTimeDomain->AccPeak.AXIS_X * 100);
  STORE_LE_16(Buff+2 ,Temp);
  
  Temp= (uint16_t)(sTimeDomain->AccPeak.AXIS_Y * 100);
  STORE_LE_16(Buff+4 ,Temp);
  
  Temp= (uint16_t)(sTimeDomain->AccPeak.AXIS_Z * 100);
  STORE_LE_16(Buff+6 ,Temp);
  
  BuffPos= 8;
  
  TempFloat = sTimeDomain->SpeedRms.AXIS_X * 1000;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
 
  TempFloat = sTimeDomain->SpeedRms.AXIS_Y * 1000;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
  
  TempFloat = sTimeDomain->SpeedRms.AXIS_Z * 1000;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
  
  ret = aci_gatt_update_char_value(SWServW2STHandle, TimeDomainCharHandle, 0, 20,Buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating Time Domain Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating Time Domain Char = \r\n");
    }
    return BLE_STATUS_ERROR;
  }
  
  return BLE_STATUS_SUCCESS;
}

/*
 * @brief  Update FFT Alarm Speed RMS status value
 * @param  sTimeDomainAlarm_t *pTdAlarm
 * @retval tBleStatus   Status
 */
tBleStatus FFT_AlarmSpeedRMS_Status_Update(sTimeDomainAlarm_t *pTdAlarm, sAcceleroParam_t *sTimeDomainVal)
{
  tBleStatus ret;
  
  float TempFloat;
  uint8_t *TempBuff = (uint8_t *) & TempFloat;
  
  uint8_t Buff[2 + 13];
  uint8_t BuffPos;
  uint8_t Temp;
  
  STORE_LE_16(Buff  ,(HAL_GetTick()>>3));
  
  Temp= 0x00;
  Temp= (pTdAlarm->RMS_STATUS_AXIS_X << 4) | (pTdAlarm->RMS_STATUS_AXIS_Y  << 2) | (pTdAlarm->RMS_STATUS_AXIS_Z);
  Buff[2]= Temp;
  
  BuffPos= 3;
  
  TempFloat = sTimeDomainVal->SpeedRms.AXIS_X;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
 
  TempFloat = sTimeDomainVal->SpeedRms.AXIS_Y;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
  
  TempFloat = sTimeDomainVal->SpeedRms.AXIS_Z;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
    
  
  ret = aci_gatt_update_char_value(SWServW2STHandle, FFTAlarmSpeedRMS_StatusCharHandle, 0, 2+13, Buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating FFT Alarm Speed Status Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating FFT Alarm Speed Status Char = \r\n");
    }
    return BLE_STATUS_ERROR;
  }
  
  return BLE_STATUS_SUCCESS;
}

/*
 * @brief  Update FFT Alarm Acc status value
 * @param  sTimeDomainAlarm_t *pTdAlarm
 * @retval tBleStatus   Status
 */
tBleStatus FFT_AlarmAccStatus_Update(sTimeDomainAlarm_t *pTdAlarm, sAcceleroParam_t *sTimeDomainVal)
{
  tBleStatus ret;
  
  float TempFloat;
  uint8_t *TempBuff = (uint8_t *) & TempFloat;
  
  uint8_t Buff[2 + 13];
  uint8_t BuffPos;
  uint8_t Temp;
  
  STORE_LE_16(Buff  ,(HAL_GetTick()>>3));
  
  Temp= 0x00;
  Temp= (pTdAlarm->PK_STATUS_AXIS_X << 4) | (pTdAlarm->PK_STATUS_AXIS_Y  << 2) | (pTdAlarm->PK_STATUS_AXIS_Z);
  Buff[2]= Temp;
  
  BuffPos= 3;
  
  TempFloat = sTimeDomainVal->AccPeak.AXIS_X;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
 
  TempFloat = sTimeDomainVal->AccPeak.AXIS_Y;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
  
  TempFloat = sTimeDomainVal->AccPeak.AXIS_Z;
  Buff[BuffPos]= TempBuff[0];
  BuffPos++;
  Buff[BuffPos]= TempBuff[1];
  BuffPos++;
  Buff[BuffPos]= TempBuff[2];
  BuffPos++;
  Buff[BuffPos]= TempBuff[3];
  BuffPos++;
    
  
  ret = aci_gatt_update_char_value(SWServW2STHandle, FFTAlarmAccStatusCharHandle, 0, 2+13, Buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating FFT Alarm Acc Status Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating FFT Alarm Acc Status Char = \r\n");
    }
    return BLE_STATUS_ERROR;
  }
  
  return BLE_STATUS_SUCCESS;
}

/*
 * @brief  Update FFT Alarm Subrange Status
 * @param  sTimeDomainAlarm_t *pTdAlarm
 * @retval tBleStatus   Status
 */
tBleStatus FFT_AlarmSubrangeStatus_Update(sAxesMagResults_t *AccAxesMagResults,sFreqDomainAlarm_t *THR_Fft_Alarms, uint16_t SubrangeNum, uint16_t ActualMagSize)
{
  tBleStatus ret;
  float BinFreqStep;
  float SendValue;
  
  Alarm_Type_t TempAlarm_X = GOOD;
  Alarm_Type_t TempAlarm_Y = GOOD;
  Alarm_Type_t TempAlarm_Z = GOOD;
  
  uint8_t Buff[2 + 13];
  uint8_t Temp;
  
  for(int i=0; i<SubrangeNum; i++)
  {
    TempAlarm_X = ALARM_MAX(TempAlarm_X, THR_Fft_Alarms->STATUS_AXIS_X[i]);
    TempAlarm_Y = ALARM_MAX(TempAlarm_X, THR_Fft_Alarms->STATUS_AXIS_Y[i]);
    TempAlarm_Z = ALARM_MAX(TempAlarm_X, THR_Fft_Alarms->STATUS_AXIS_Z[i]);
  }

  STORE_LE_16(Buff  ,(HAL_GetTick()>>3));

  Temp= 0x00;
  Temp= (TempAlarm_X << 4) | (TempAlarm_Y  << 2) | (TempAlarm_Z);
  Buff[2]= Temp;
  
  BinFreqStep = (AcceleroODR.Frequency/2) / ActualMagSize;
  
  /* X */
  SendValue= (float)(AccAxesMagResults->X_Index*BinFreqStep);
  STORE_LE_16(Buff + 3, ((uint16_t)(SendValue * 10)));
  SendValue= AccAxesMagResults->X_Value;
  STORE_LE_16(Buff + 5, ((uint16_t)(SendValue * 100)));
  
  /* Y */
  SendValue= (float)(AccAxesMagResults->Y_Index*BinFreqStep);
  STORE_LE_16(Buff + 7, ((uint16_t)(SendValue * 10)));
  SendValue= AccAxesMagResults->Y_Value;
  STORE_LE_16(Buff + 9, ((uint16_t)(SendValue * 100)));
  
  /* Z */
  SendValue= (float)(AccAxesMagResults->Z_Index*BinFreqStep);
  STORE_LE_16(Buff + 11, ((uint16_t)(SendValue * 10)));
  SendValue= AccAxesMagResults->Z_Value;
  STORE_LE_16(Buff + 13, ((uint16_t)(SendValue * 100)));
  
  ret = aci_gatt_update_char_value(SWServW2STHandle, FFTAlarmSubrangeStatusCharHandle, 0, 2+13, Buff);

  if (ret != BLE_STATUS_SUCCESS){
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Error Updating FFT Alarm Subrange Status Char\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Error Updating FFT Alarm Subrange Status Char = \r\n");
    }
    return BLE_STATUS_ERROR;
  }
  
  return BLE_STATUS_SUCCESS;
}


/**
 * @brief  Puts the device in connectable mode.
 * @param  None
 * @retval None
 */
void setBeacon(void)
{

/*Set iBeacon avertising packet based on Apple Proximity beacon specification 2.1 */
	  uint8_t beacon[31] =
	  {0x02,0x01,0x06,0x1A,0xFF,0x4C,0x00,0x02,0x15,
	   0x00,0x02,0x00,0x00,0x00,0x0d,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b,
	   0x00,0x00,0x00,0x00,0xc6,0x00};

	  /* switch off connectable */
	    aci_gap_set_non_discoverable();

	    /* switch off scanable */
	    hci_le_set_scan_enable(0x0,0x0);



/*Set advertising parameters for non connectable avertising */
	  tBleStatus ret = hci_le_set_advertising_parameters(0xA0,0xA0,ADV_NONCONN_IND,0x01,0x01,NULL,0x07,0x00);

  if(ret){
       PREDMNT1_PRINTF("\r\n set advertising error failed %d\r\n",ret);
       goto fail;
    }





  /* set beacon advertising data */
  ret = hci_le_set_advertising_data(0x1E,beacon);

  if(ret){
         PREDMNT1_PRINTF("\r\n set advertising data error failed %d\r\n",ret);
         goto fail;
      }



  /* enable beacon advertise */
  ret = hci_le_set_advertise_enable(0x01);





    if(ret){
           PREDMNT1_PRINTF("\r\n set advertising enable error failed %d\r\n",ret);
           goto fail;
        }

    fail:
	 return;


}

/**
 * @brief  Puts the device in connectable mode.
 * @param  None 
 * @retval None
 */
void setConnectable(void)
{  




  uint8_t local_name[8] = {AD_TYPE_COMPLETE_LOCAL_NAME,NodeName[1],NodeName[2],NodeName[3],NodeName[4],NodeName[5],NodeName[6],NodeName[7]};
  uint8_t manuf_data[26] = {
    2,0x0A,0x00 /* 0 dBm */, // Trasmission Power
    //8,0x09,NAME_BLUEMS, // Complete Name
    8,0x09,NodeName[1],NodeName[2],NodeName[3],NodeName[4],NodeName[5],NodeName[6],NodeName[7], // Complete Name
    13,0xFF,0x01/*SKD version */,
    0x92,  /* Changed  default Sensor.tile DeviceID 0x02 to 0x92 for TaiChi Device */
    0x00, /* AudioSync+AudioData */
    0xE0, /* ACC + Gyro + Mag + Environmental + Battery Info */
    0x00, /*  FFT Amplitude */
    0x00, /*  */
    0x00, /* BLE MAC start */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, /* BLE MAC stop */
  };
  
  /* BLE MAC */
  manuf_data[20] = bdaddr[5];
  manuf_data[21] = bdaddr[4];
  manuf_data[22] = bdaddr[3];
  manuf_data[23] = bdaddr[2];
  manuf_data[24] = bdaddr[1];
  manuf_data[25] = bdaddr[0];

  /* Mic */
  manuf_data[16] |= 0x04;

  if(TargetBoardFeatures.NumTempSensors==2) {
    /* Two Temperature values*/
    manuf_data[17] |= 0x05;
  } else if(TargetBoardFeatures.NumTempSensors==1) {
    /* One Temperature value*/
    manuf_data[17] |= 0x04; 
  }

  /* Humidity Value */
  if(TargetBoardFeatures.HumSensorIsInit)
  {
    manuf_data[17] |= 0x08;
  }

  /* Pressure value*/
  if(TargetBoardFeatures.PressSensorIsInit)
  {
    manuf_data[17] |= 0x10;
  }
  
  /* Battery Info */
  manuf_data[17] |= 0x02;
  
  /*  FFT Amplitude */
  manuf_data[19] |= 0x05;

  /*  Time Domain */
  manuf_data[19] |= 0x06;   
  
  /*  FFT Alarm */
  manuf_data[19] |= 0x07; 
  
  /* disable beacon */
  tBleStatus ret = hci_le_set_advertise_enable(0x00);


  /* disable scan response */
  hci_le_set_scan_response_data(0,NULL);


  aci_gap_set_discoverable(ADV_IND, 0, 0,
                           RANDOM_ADDR,
                           NO_WHITE_LIST_USE,
                           sizeof(local_name), local_name, 0, NULL, 0, 0);

  /* Send Advertising data */
  aci_gap_update_adv_data(26, manuf_data);


}

/**
 * @brief  This function is called when there is a Bluetooth Read request
 * @param  uint16_t handle Handle of the attribute
 * @retval None
 */
static void Read_Request_CB(uint16_t handle)
{
  if(handle == EnvironmentalCharHandle + 1){
    /* Read Request for Pressure,Humidity, and Temperatures*/
    float SensorValue;
    int32_t PressToSend=0;
    uint16_t HumToSend=0;
    int16_t Temp2ToSend=0,Temp1ToSend=0;
    int32_t decPart, intPart;

    ENV_SENSOR_GetValue(PRESSURE_INSTANCE,ENV_PRESSURE, &SensorValue);
    MCR_BLUEMS_F2I_2D(SensorValue, intPart, decPart);
    PressToSend=intPart*100+decPart;

    ENV_SENSOR_GetValue(HUMIDITY_INSTANCE,ENV_HUMIDITY,&SensorValue);
    MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
    HumToSend = intPart*10+decPart;

    if(TargetBoardFeatures.NumTempSensors==2) {
      ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_1,ENV_TEMPERATURE,&SensorValue);
      MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
      Temp1ToSend = intPart*10+decPart; 

      ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_2,ENV_TEMPERATURE,&SensorValue);
      MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
      Temp2ToSend = intPart*10+decPart; 
    } else if(TargetBoardFeatures.NumTempSensors==1) {
      ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_1,ENV_TEMPERATURE,&SensorValue);
      MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
      Temp1ToSend = intPart*10+decPart; 
    }
    Environmental_Update(PressToSend,HumToSend,Temp2ToSend,Temp1ToSend);
  } else if (handle == StdErrCharHandle + 1) {
    /* Send again the last packet for StdError */
    Stderr_Update_AfterRead();
  } else if (handle == TermCharHandle + 1) {
    /* Send again the last packet for Terminal */
    Term_Update_AfterRead();
  }
 
  //EXIT:
  if(connection_handle != 0)
    aci_gatt_allow_read(connection_handle);
}

/**
 * @brief  This function is called when there is a change on the gatt attribute
 * With this function it's possible to understand if one application 
 * is subscribed or not to the one service
 * @param uint16_t att_handle Handle of the attribute
 * @param uint8_t *att_data attribute data
 * @param uint8_t data_length length of the data
 * @retval None
 */
static void Attribute_Modified_Request_CB(uint16_t Connection_Handle, uint16_t attr_handle, uint16_t Offset, uint8_t data_length, uint8_t *att_data)
{

  if (attr_handle == TaiChiCharHandle + 2){
	  TaiChi_AttributeModified_CB(att_data);
  }else if(attr_handle == ConfigCharHandle + 2){
    if (att_data[0] == 01) {
      FirstConnectionConfig=1;
    } else if (att_data[0] == 0){
      FirstConnectionConfig=0;
    }
  } else if(attr_handle == EnvironmentalCharHandle + 2){
    Environmental_AttributeModified_CB(att_data);
  } else if(attr_handle == AccGyroMagCharHandle + 2) {
    AccGyroMag_AttributeModified_CB(att_data);
  } else if (attr_handle == AudioLevelCharHandle + 2) {
    AudioLevel_AttributeModified_CB(att_data);
  } else if(attr_handle == BatteryFeaturesCharHandle + 2){
    BatteryFeatures_AttributeModified_CB(att_data);
  } else if (attr_handle == FFTAmplitudeCharHandle + 2) {
    FFTAmplitude_AttributeModified_CB(att_data);
  } else if (attr_handle == TimeDomainCharHandle + 2) {
    TimeDomain_AttributeModified_CB(att_data);
  } else if (attr_handle == FFTAlarmSpeedRMS_StatusCharHandle + 2) {
    FFTAlarmSpeedRMS_AttributeModified_CB(att_data);
  } else if (attr_handle == FFTAlarmAccStatusCharHandle + 2) {
    FFTAlarmAccStatus_AttributeModified_CB(att_data);
  } else if (attr_handle == FFTAlarmSubrangeStatusCharHandle + 2) {
      FFTAlarmSubrangeStatus_AttributeModified_CB(att_data);
  } else if(attr_handle == StdErrCharHandle + 2){
    if (att_data[0] == 01) {
      W2ST_ON_CONNECTION(W2ST_CONNECT_STD_ERR);
    } else if (att_data[0] == 0){
      W2ST_OFF_CONNECTION(W2ST_CONNECT_STD_ERR);
    }
  } else if(attr_handle == TermCharHandle + 2){
    if (att_data[0] == 01) {
      W2ST_ON_CONNECTION(W2ST_CONNECT_STD_TERM);
    } else if (att_data[0] == 0){
      W2ST_OFF_CONNECTION(W2ST_CONNECT_STD_TERM);
    }
  } else if (attr_handle == TermCharHandle + 1){
    uint32_t SendBackData =1; /* By default UserAnswer with the same message received */

    if(SizeOfUpdateBlueFW!=0) {
      /* FP-IND-PREDMNT1 firwmare update */
      int8_t RetValue = UpdateFWBlueMS(&SizeOfUpdateBlueFW,att_data, data_length,1);
      if(RetValue!=0) {
        MCR_FAST_TERM_UPDATE_FOR_OTA(((uint8_t *)&RetValue));
        if(RetValue==1) {
          /* if OTA checked */
          BytesToWrite =sprintf((char *)BufferToWrite,"The Board will restart in 5 seconds\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
          PREDMNT1_PRINTF("%s will restart in 5 seconds\r\n",PREDMNT1_PACKAGENAME);
          HAL_Delay(5000);
          HAL_NVIC_SystemReset();
        }
      }
      SendBackData=0;
    } else {
      /* Received one write from Client on Terminal characteristc */
      SendBackData = DebugConsoleCommandParsing(att_data,data_length);
    }

    /* Send it back for testing */
    if(SendBackData) {
      Term_Update(att_data,data_length);
    }
  } else if (attr_handle == ConfigCharHandle + 1) {
    /* Received one write command from Client on Configuration characteristc */
    ConfigCommandParsing(att_data, data_length);    
  } else if (attr_handle==(0x0002+2)){
    /* Force one UUID rescan */
    Force_UUID_Rescan();
  } else {
#ifdef PREDMNT1_DEBUG_CONNECTION
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_ERR)){
      BytesToWrite =sprintf((char *)BufferToWrite, "Notification UNKNOW handle\r\n");
      Stderr_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Notification UNKNOW handle\r\n");
    }
#endif /* PREDMNT1_DEBUG_CONNECTION */
  }
}

/************************************/
/* Hardware Characteristics Service */
/************************************/


/**
 * @brief  This function is called when there is a change on the gatt attribute for TaiChi
 * With this function it's possible to understand if one application
 * is subscribed or not to the Environmental service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void TaiChi_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_TAICHI);


  } else if (att_data[0] == 0){
    W2ST_OFF_CONNECTION(W2ST_CONNECT_TAICHI);



  }
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite =sprintf((char *)BufferToWrite,"--->TaiChi=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_TAICHI) ? " ON\r\n" : " OFF\r\n\n");
    Term_Update(BufferToWrite,BytesToWrite);
  } else
    PREDMNT1_PRINTF("--->TaiChi=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_TAICHI) ? " ON\r\n" : " OFF\r\n\n");
#endif /* PREDMNT1_DEBUG_CONNECTION */
}



/**
 * @brief  This function is called when there is a change on the gatt attribute for Environmental
 * With this function it's possible to understand if one application 
 * is subscribed or not to the Environmental service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void Environmental_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_ENV);

    /* Start the TIM Base generation in interrupt mode */
    if(HAL_TIM_Base_Start_IT(&TimEnvHandle) != HAL_OK){
      /* Starting Error */
      Error_Handler();
    }
  } else if (att_data[0] == 0){
    W2ST_OFF_CONNECTION(W2ST_CONNECT_ENV);

    /* Stop the TIM Base generation in interrupt mode */
    if(HAL_TIM_Base_Stop_IT(&TimEnvHandle) != HAL_OK){
      /* Stopping Error */
      Error_Handler();
    }
  }
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite =sprintf((char *)BufferToWrite,"--->Env=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_ENV) ? " ON\r\n" : " OFF\r\n\n");
    Term_Update(BufferToWrite,BytesToWrite);
  } else
    PREDMNT1_PRINTF("--->Env=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_ENV) ? " ON\r\n" : " OFF\r\n\n");
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for Acc,Gyro and Mag
 * With this function it's possible to understand if one application 
 * is subscribed or not to the Acc,Gyro and Mag service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void AccGyroMag_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_ACC_GYRO_MAG);

    /* Start the TIM Base generation in interrupt mode */
    if(HAL_TIM_OC_Start_IT(&TimCCHandle, TIM_CHANNEL_4) != HAL_OK){
      /* Starting Error */
      Error_Handler();
    }

    /* Set the new Capture compare value */
    {
      uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TimCCHandle);
      /* Set the Capture Compare Register value */
      __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_4, (uhCapture + uhCCR4_Val));
    }
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_ACC_GYRO_MAG);

    /* Stop the TIM Base generation in interrupt mode */
    if(HAL_TIM_OC_Stop_IT(&TimCCHandle, TIM_CHANNEL_4) != HAL_OK){
      /* Stopping Error */
      Error_Handler();
    }      
  }
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite =sprintf((char *)BufferToWrite,"--->Acc/Gyro/Mag=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_ACC_GYRO_MAG) ? " ON\r\n" : " OFF\r\n\n");
    Term_Update(BufferToWrite,BytesToWrite);
  } else
    PREDMNT1_PRINTF("--->Acc/Gyro/Mag=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_ACC_GYRO_MAG) ? " ON\r\n" : " OFF\r\n\n");
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for Audio Level
 * With this function it's possible to understand if one application 
 * is subscribed or not to the Audio Level service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void AudioLevel_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    int32_t Count;
    
    W2ST_ON_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL);
      
    InitMics(AUDIO_IN_SAMPLING_FREQUENCY, AUDIO_VOLUME_INPUT);
    
    for(Count=0;Count<AUDIO_IN_CHANNELS;Count++) {
      RMS_Ch[Count]=0;
      DBNOISE_Value_Old_Ch[Count] =0;
    }
    
    /* Start the TIM Base generation in interrupt mode */
    if(HAL_TIM_Base_Start_IT(&TimAudioDataHandle) != HAL_OK){
      /* Starting Error */
      Error_Handler();
    }
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL);

    DeInitMics();

    /* Stop the TIM Base generation in interrupt mode */
    if(HAL_TIM_Base_Stop_IT(&TimAudioDataHandle) != HAL_OK){
      /* Stopping Error */
      Error_Handler();
    }      
  }
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"--->dB Noise AudioLevel=%s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL)   ? " ON\r\n" : " OFF\r\n\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else
    PREDMNT1_PRINTF("--->dB Noise AudioLevel=%s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL)   ? " ON\r\n" : " OFF\r\n\n"));
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for Battery Features
 * With this function it's possible to understand if one application 
 * is subscribed or not to the Battery Features service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void BatteryFeatures_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_BATTERY_INFO);
   /* Start the TIM Base generation in interrupt mode */
   if(HAL_TIM_Base_Start_IT(&TimEnvHandle) != HAL_OK){
     /* Starting Error */
     Error_Handler();
   }
  } else if (att_data[0] == 0){
    W2ST_OFF_CONNECTION(W2ST_CONNECT_BATTERY_INFO);
   /* Stop the TIM Base generation in interrupt mode */
   if(HAL_TIM_Base_Stop_IT(&TimEnvHandle) != HAL_OK){
     /* Stopping Error */
     Error_Handler();
   }
  }
#ifdef ALLMEMS1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite =sprintf((char *)BufferToWrite,"---BatteryInfo=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_BATTERY_INFO) ? " ON\r\n" : " OFF\r\n\n");
    Term_Update(BufferToWrite,BytesToWrite);
  } else
    ALLMEMS1_PRINTF("--->BatteryInfo=%s", W2ST_CHECK_CONNECTION(W2ST_CONNECT_BATTERY_INFO) ? " ON\r\n" : " OFF\r\n\n");
#endif /* ALLMEMS1_DEBUG_CONNECTION */
}

/***********************************
* Software Characteristics Service *
************************************/

/**
 * @brief  This function is called when there is a change on the gatt attribute for FFT Amplitude
 * With this function it's possible to understand if one application 
 * is subscribed or not to the FFT Amplitude service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void FFTAmplitude_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    
    W2ST_ON_CONNECTION(W2ST_CONNECT_FFT_AMPLITUDE);
    
    if(!PredictiveMaintenance)
    {
      PredictiveMaintenance= 1;
      FFT_Amplitude= 1;
      IsFirstTime = 1;
    }
  } else if (att_data[0] == 0) {
    
    W2ST_OFF_CONNECTION(W2ST_CONNECT_FFT_AMPLITUDE);
    
    disable_FIFO();
    EnableDisable_ACC_HP_Filter(0);
    PredictiveMaintenance= 0;
    FFT_Amplitude= 0;
  }
  
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"\r\n--->FFT Amplitude= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_AMPLITUDE)   ? " ON\r\n" : " OFF\r\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("\r\n--->FFT Amplitude= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_AMPLITUDE)   ? " ON\r\n" : " OFF\r\n"));
  }
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for Time Domain
 * With this function it's possible to understand if one application 
 * is subscribed or not to the Time Domain service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void TimeDomain_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_TIME_DOMAIN);
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_TIME_DOMAIN);
  }

#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"--->Time Domain= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_TIME_DOMAIN)   ? " ON\r\n" : " OFF\r\n\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("--->Time Domain= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_TIME_DOMAIN)   ? " ON\r\n" : " OFF\r\n\n"));
  }
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for FFT Alarm Speed RMS
 * With this function it's possible to understand if one application 
 * is subscribed or not to the FFT Alarm Speed RMS service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void FFTAlarmSpeedRMS_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_FFT_ALARM_SPEED_RMS_STATUS);
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_FFT_ALARM_SPEED_RMS_STATUS);
    disable_FIFO();
    EnableDisable_ACC_HP_Filter(0);
    PredictiveMaintenance= 0;
    FFT_Alarm= 0;
  }
  
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"\r\n--->FFT Alarm Speed RMS Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_SPEED_RMS_STATUS)   ? " ON\r\n" : " OFF\r\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("\r\n--->FFT Alarm Speed RMS Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_SPEED_RMS_STATUS)   ? " ON\r\n" : " OFF\r\n"));
  }
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for FFT Alarm Acc Status
 * With this function it's possible to understand if one application 
 * is subscribed or not to the FFT Alarm Acc Status service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void FFTAlarmAccStatus_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_FFT_ALARM_ACC_STATUS);
    if(!PredictiveMaintenance)
    {
      PredictiveMaintenance= 1;
      FFT_Alarm= 1;
      IsFirstTime = 1;
    }
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_FFT_ALARM_ACC_STATUS);
  }
  
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"--->FFT Alarm Acc Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_ACC_STATUS)   ? " ON\r\n" : " OFF\r\n\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("--->FFT Alarm Acc Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_ACC_STATUS)   ? " ON\r\n" : " OFF\r\n\n"));
  }
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function is called when there is a change on the gatt attribute for FFT Alarm Subrange Status
 * With this function it's possible to understand if one application 
 * is subscribed or not to the FFT Alarm Subrange Status service
 * @param uint8_t *att_data attribute data
 * @retval None
 */
static void FFTAlarmSubrangeStatus_AttributeModified_CB(uint8_t *att_data)
{
  if (att_data[0] == 01) {
    W2ST_ON_CONNECTION(W2ST_CONNECT_FFT_ALARM_SUBRANGE_STATUS);
  } else if (att_data[0] == 0) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_FFT_ALARM_SUBRANGE_STATUS);
  }
  
#ifdef PREDMNT1_DEBUG_CONNECTION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
    BytesToWrite = sprintf((char *)BufferToWrite,"--->FFT Alarm Subrange Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_SUBRANGE_STATUS)   ? " ON\r\n" : " OFF\r\n") );
    Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("--->FFT Alarm Subrange Status= %s", (W2ST_CHECK_CONNECTION(W2ST_CONNECT_FFT_ALARM_SUBRANGE_STATUS)   ? " ON\r\n" : " OFF\r\n"));
  }
#endif /* PREDMNT1_DEBUG_CONNECTION */
}

/**
 * @brief  This function makes the parsing of the Debug Console Commands
 * @param uint8_t *att_data attribute data
 * @param uint8_t data_length length of the data
 * @retval uint32_t SendItBack true/false
 */
static uint32_t DebugConsoleCommandParsing(uint8_t * att_data, uint8_t data_length)
{
  uint32_t SendBackData = 1;
  
  static uint8_t SetVibrParam= 0;

    /* Help Command */
    if(!strncmp("help",(char *)(att_data),4)) {
      /* Print Legend */
      SendBackData=0;

      BytesToWrite =sprintf((char *)BufferToWrite,"Command:\r\n"
         "info -> System Info\r\n"
         "versionFw  -> FW Version\r\n"
         /*"versionBle -> Ble Version\r\n" */
         "getVibrParam  -> Read Vibration Parameters\r\n"
         "setVibrParam [-odr -fs -size -wind - tacq -subrng -ovl] -> Set Vibration Parameters\r\n"
           );
      Term_Update(BufferToWrite,BytesToWrite);
      
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nodr= [13, 26, 52, 104, 208, 416, 833, 1660, 3330, 6660]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nfs= [2, 4, 8, 16]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nsize= [256, 512, 1024, 2048]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nwind= [RECTANGULAR= 0, HANNING= 1, HAMMING= 2, FLAT_TOP= 3]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\ntacq= [500 - 60000]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nsubrng= [8, 16, 32, 64]");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\novl= [5 - 95]\r\n\r\n");
      Term_Update(BufferToWrite,BytesToWrite);
      
      BytesToWrite =sprintf((char *)BufferToWrite,
         "setName xxxxxxx     -> Set the node name (Max 7 characters)\r\n"
           );
      Term_Update(BufferToWrite,BytesToWrite); 
    } else if(!strncmp("versionFw",(char *)(att_data),9)) {
      BytesToWrite =sprintf((char *)BufferToWrite,"%s_%s_%c.%c.%c\r\n",
#ifdef STM32F401xE
                            "F401"
#elif STM32F446xx
                            "F446"
#elif STM32L476xx
                            "L476"
#elif STM32L4R9xx
                            "L4R9"
#else
#error "Undefined STM32 processor type"
#endif
                            ,PREDMNT1_PACKAGENAME,
                            PREDMNT1_VERSION_MAJOR,
                            PREDMNT1_VERSION_MINOR,
                            PREDMNT1_VERSION_PATCH);
#ifdef DISABLE_FOTA
      if(FirstCommandSent)
      {
        FirstCommandSent= 0;
#endif /* DISABLE_FOTA */
        Term_Update(BufferToWrite,BytesToWrite);
        SendBackData=0;
#ifdef DISABLE_FOTA
      }
      else
        SendBackData=1;
#endif /* DISABLE_FOTA */
    } else if(!strncmp("info",(char *)(att_data),4)) {
      SendBackData=0;
      
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nSTMicroelectronics %s:\r\n"
          "\tVersion %c.%c.%c\r\n"
          "\tSTM32F446xx-Nucleo board"
          "\r\n",
          PREDMNT1_PACKAGENAME,
          PREDMNT1_VERSION_MAJOR,PREDMNT1_VERSION_MINOR,PREDMNT1_VERSION_PATCH);
      Term_Update(BufferToWrite,BytesToWrite);

      BytesToWrite =sprintf((char *)BufferToWrite,"\t(HAL %ld.%ld.%ld_%ld)\r\n"
        "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
        " (IAR)\r\n",
#elif defined (__CC_ARM)
        " (KEIL)\r\n",
#elif defined (__GNUC__)
        " (STM32CubeIDE)\r\n",
#endif
          HAL_GetHalVersion() >>24,
          (HAL_GetHalVersion() >>16)&0xFF,
          (HAL_GetHalVersion() >> 8)&0xFF,
           HAL_GetHalVersion()      &0xFF,
           __DATE__,__TIME__);
      Term_Update(BufferToWrite,BytesToWrite);
      
    BytesToWrite =sprintf((char *)BufferToWrite,"Code compiled for STWIN board\r\n");
    
    Term_Update(BufferToWrite,BytesToWrite);
      
#ifndef DISABLE_FOTA
    }  if(!strncmp("upgradeFw",(char *)(att_data),9)) {
      uint32_t uwCRCValue;
      uint8_t *PointerByte = (uint8_t*) &SizeOfUpdateBlueFW;

      SizeOfUpdateBlueFW=atoi((char *)(att_data+9));
      PointerByte[0]=att_data[ 9];
      PointerByte[1]=att_data[10];
      PointerByte[2]=att_data[11];
      PointerByte[3]=att_data[12];

      /* Check the Maximum Possible OTA size */
      if(SizeOfUpdateBlueFW>OTA_MAX_PROG_SIZE) {
        PREDMNT1_PRINTF("OTA %s SIZE=%ld > %d Max Allowed\r\n",PREDMNT1_PACKAGENAME,SizeOfUpdateBlueFW, OTA_MAX_PROG_SIZE);
        /* UserAnswer with a wrong CRC value for signaling the problem to BlueMS application */
        PointerByte[0]= att_data[13];
        PointerByte[1]=(att_data[14]!=0) ? 0 : 1;/* In order to be sure to have a wrong CRC */
        PointerByte[2]= att_data[15];
        PointerByte[3]= att_data[16];
        BytesToWrite = 4;
        Term_Update(BufferToWrite,BytesToWrite);
      } else {
        PointerByte = (uint8_t*) &uwCRCValue;
        PointerByte[0]=att_data[13];
        PointerByte[1]=att_data[14];
        PointerByte[2]=att_data[15];
        PointerByte[3]=att_data[16];

        PREDMNT1_PRINTF("OTA %s SIZE=%ld uwCRCValue=%lx\r\n",PREDMNT1_PACKAGENAME,SizeOfUpdateBlueFW,uwCRCValue);
	  
        /* Reset the Flash */
        StartUpdateFWBlueMS(SizeOfUpdateBlueFW,uwCRCValue);

        /* Reduce the connection interval */
        {
          int ret = aci_l2cap_connection_parameter_update_req(connection_handle,
                                                        10 /* interval_min*/,
                                                        10 /* interval_max */,
                                                        0   /* slave_latency */,
                                                        400 /*timeout_multiplier*/);
          /* Go to infinite loop if there is one error */
          if (ret != BLE_STATUS_SUCCESS) {
            while (1) {
              PREDMNT1_PRINTF("Problem Changing the connection interval\r\n");
            }
          }
        }
        
        /* Signal that we are ready sending back the CRV value*/
        BufferToWrite[0] = PointerByte[0];
        BufferToWrite[1] = PointerByte[1];
        BufferToWrite[2] = PointerByte[2];
        BufferToWrite[3] = PointerByte[3];
        BytesToWrite = 4;
        Term_Update(BufferToWrite,BytesToWrite);
      }
      
      SendBackData=0;      
    } else if(!strncmp("versionBle",(char *)(att_data),10)) {
      uint8_t  hwVersion;
      uint16_t fwVersion;
      /* get the BlueNRG HW and FW versions */
      getBlueNRG2_Version(&hwVersion, &fwVersion);
      BytesToWrite =sprintf((char *)BufferToWrite,"%s_v%d.%d.%c\r\n",
                            "BlueNRG2",
                            (fwVersion>>8)&0xF,
                            (fwVersion>>4)&0xF,
                            ('a' + (fwVersion&0xF)));
      Term_Update(BufferToWrite,BytesToWrite);
      SendBackData=0; 
    } else if(!strncmp("getVibrParam",(char *)(att_data),12)) {
      BytesToWrite =sprintf((char *)BufferToWrite,"\r\nAccelerometer parameters:\r\n");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"FifoOdr= %d fs= %d\r\n",
                            Accelerometer_Parameters.FifoOdr,
                            Accelerometer_Parameters.fs);
      Term_Update(BufferToWrite,BytesToWrite);
      
      BytesToWrite =sprintf((char *)BufferToWrite,"MotionSP parameters:\r\n");
      Term_Update(BufferToWrite,BytesToWrite);
      BytesToWrite =sprintf((char *)BufferToWrite,"size= %d wind= %d tacq= %d subrng= %d ovl= %d\r\n",
                            MotionSP_Parameters.FftSize,
                            MotionSP_Parameters.window,
                            MotionSP_Parameters.tacq,
                            MotionSP_Parameters.subrange_num,
                            MotionSP_Parameters.FftOvl);
      Term_Update(BufferToWrite,BytesToWrite);
      SendBackData=0;
    } else if(!strncmp("setVibrParam",(char *)(att_data),12)) {
      SetVibrParam= 1;
      SendBackData=0;
#endif /* DISABLE_FOTA */
    } else if((att_data[0]=='u') & (att_data[1]=='i') & (att_data[2]=='d')) {
      /* Write back the STM32 UID */
      uint8_t *uid = (uint8_t *)STM32_UUID;
      uint32_t MCU_ID = STM32_MCU_ID[0]&0xFFF;
      BytesToWrite =sprintf((char *)BufferToWrite,"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X_%.3lX\r\n",
                            uid[ 3],uid[ 2],uid[ 1],uid[ 0],
                            uid[ 7],uid[ 6],uid[ 5],uid[ 4],
                            uid[11],uid[ 10],uid[9],uid[8],
                            MCU_ID);
      Term_Update(BufferToWrite,BytesToWrite);
      SendBackData=0;
    } else if(!strncmp("setName ",(char *)(att_data),8)) {
      
      //int NameLength= strcspn((const char *)att_data,"\n");
      int NameLength= data_length -1;
      
      if(NameLength > 8)
      {
        for(int i=1;i<8;i++)
          NodeName[i]= atoi(" ");
 
        if((NameLength - 8) > 7)
          NameLength= 7;
        else NameLength= NameLength - 8;
        
        for(int i=1;i<NameLength+1;i++)
          NodeName[i]= att_data[i+7];
        
        MDM_SaveGMD(GMD_NODE_NAME,(void *)&NodeName);
        NecessityToSaveMetaDataManager=1;
        
        BytesToWrite =sprintf((char *)BufferToWrite,"\nThe node nome has been updated\r\n");
        Term_Update(BufferToWrite,BytesToWrite);
        BytesToWrite =sprintf((char *)BufferToWrite,"Disconnecting and riconnecting to see the new node name\r\n");
        Term_Update(BufferToWrite,BytesToWrite);
      }
      else
      {
        BytesToWrite =sprintf((char *)BufferToWrite,"\nInsert the node name\r\n");
        Term_Update(BufferToWrite,BytesToWrite);
        BytesToWrite =sprintf((char *)BufferToWrite,"Use command: setName 'xxxxxxx'\r\n");
        Term_Update(BufferToWrite,BytesToWrite);
      }

      SendBackData=0;
    }

    if(SetVibrParam)
    {
      uint8_t Index=0;
     
      static uint8_t NumByte= 0;
      static uint8_t CommandLenght=0;
      
      while( (att_data[Index] != '\n') && (att_data[Index] != '\0') )
      {
        VibrParam[20*NumByte + Index]= att_data[Index];
        Index++;
        CommandLenght++;
      }
      
      NumByte++;
      
      if(att_data[Index] == '\n')
      {
        if(VibrationParametersCommandParsing(CommandLenght))
        {
          /* Save vibration parameters values to memory */
          SaveVibrationParamToMemory();
        }
        
        NumByte= 0;
        SetVibrParam=0;
        CommandLenght=0;
      }
      
      SendBackData= 0;
    }
    
#if 1

  /* If it's something not yet recognized... only for testing.. This must be removed*/
   if(SendBackData) {
    if(att_data[0]=='@') {

      if(att_data[1]=='T') {
        uint8_t loc_att_data[6];
        uint8_t loc_data_length=6;

        loc_att_data[0] = (FEATURE_MASK_TEMP1>>24)&0xFF;
        loc_att_data[1] = (FEATURE_MASK_TEMP1>>16)&0xFF;
        loc_att_data[2] = (FEATURE_MASK_TEMP1>>8 )&0xFF;
        loc_att_data[3] = (FEATURE_MASK_TEMP1    )&0xFF;
        loc_att_data[4] = 255;
        
        switch(att_data[2]) {
          case 'L':
            loc_att_data[5] = 50; /* @5S */
          break;
          case 'M':
            loc_att_data[5] = 10; /* @1S */
          break;
          case 'H':
            loc_att_data[5] = 1; /* @100mS */
          break;
          case 'D':
            loc_att_data[5] = 0; /* Default */
          break;
        }
        SendBackData = ConfigCommandParsing(loc_att_data,loc_data_length);
      } else if(att_data[1]=='A') {
        uint8_t loc_att_data[6];
        uint8_t loc_data_length=6;

        loc_att_data[0] = (FEATURE_MASK_ACC>>24)&0xFF;
        loc_att_data[1] = (FEATURE_MASK_ACC>>16)&0xFF;
        loc_att_data[2] = (FEATURE_MASK_ACC>>8 )&0xFF;
        loc_att_data[3] = (FEATURE_MASK_ACC    )&0xFF;
        loc_att_data[4] = 255;

        switch(att_data[2]) {
          case 'L':
            loc_att_data[5] = 50; /* @5S */
          break;
          case 'M':
            loc_att_data[5] = 10; /* @1S */
          break;
          case 'H':
            loc_att_data[5] = 1; /* @100mS */
          break;
          case 'D':
            loc_att_data[5] = 0; /* Default */
          break;
        }
        SendBackData = ConfigCommandParsing(loc_att_data,loc_data_length);
      } else if(att_data[1]=='M') {
        uint8_t loc_att_data[6];
        uint8_t loc_data_length=6;

        loc_att_data[0] = (FEATURE_MASK_MIC>>24)&0xFF;
        loc_att_data[1] = (FEATURE_MASK_MIC>>16)&0xFF;
        loc_att_data[2] = (FEATURE_MASK_MIC>>8 )&0xFF;
        loc_att_data[3] = (FEATURE_MASK_MIC    )&0xFF;
        loc_att_data[4] = 255;

        switch(att_data[2]) {
          case 'L':
            loc_att_data[5] = 50; /* @5S */
          break;
          case 'M':
            loc_att_data[5] = 10; /* @1S */
          break;
          case 'H':
            loc_att_data[5] = 1; /* @100mS */
          break;
          case 'D':
            loc_att_data[5] = 0; /* Default */
          break;
        }
        SendBackData = ConfigCommandParsing(loc_att_data,loc_data_length);
      }    
    }
  }
#endif
  
  return SendBackData;
}

/**
 * @brief  This function makes the parsing of the set Vibration Parameter Commands
 * @param uint8_t CommandLenght length of the data
 * @retval UpdatedParameters
 */
static uint8_t VibrationParametersCommandParsing(uint8_t CommandLenght)
{
  uint8_t UpdatedParameters= 0;
  uint8_t UpdatedAccParameters= 0;
  
  uint8_t i=7;
  uint32_t Param[7];
  uint8_t DigitNumber;
  uint8_t ParamFound;
  
  int Index= 13;
  
  if(Index >= CommandLenght)
  {
    BytesToWrite =sprintf((char *)BufferToWrite,"\r\nParameters not found\r\n");
    Term_Update(BufferToWrite,BytesToWrite);
  }  
  
  while(Index < CommandLenght)
  {
    Index++;
    ParamFound= 0;
    
    if((VibrParam[Index]=='o') & (VibrParam[Index+1]=='d') & (VibrParam[Index+2]=='r'))
    {
      Index+= 4;
      i=0;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='f') & (VibrParam[Index+1]=='s'))
    {
      Index+= 3;
      i=1;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='s') & (VibrParam[Index+1]=='i') & (VibrParam[Index+2]=='z') & (VibrParam[Index+3]=='e'))
    {
      Index+= 5;
      i=2;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='w') & (VibrParam[Index+1]=='i') & (VibrParam[Index+2]=='n') & (VibrParam[Index+3]=='d'))
    {
      Index+= 5;
      i=3;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='t') & (VibrParam[Index+1]=='a') & (VibrParam[Index+2]=='c') & (VibrParam[Index+3]=='q'))
    {
      Index+= 5;
      i=4;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='o') & (VibrParam[Index+1]=='v') & (VibrParam[Index+2]=='l') )
    {
      Index+= 4;
      i=5;
      ParamFound= 1;
    }
    
    if((VibrParam[Index]=='s') & (VibrParam[Index+1]=='u') & (VibrParam[Index+2]=='b') & (VibrParam[Index+3]=='r') & (VibrParam[Index+4]=='n') & (VibrParam[Index+5]=='g'))
    {
      Index+= 7;
      i=6;
      ParamFound= 1;
    }
      
    if(ParamFound == 1)
    {
      ParamFound= 0;
      
      DigitNumber= 0;
      while( (VibrParam[Index + DigitNumber] != ' ') &&
             (VibrParam[Index + DigitNumber] != '\r') &&
             (VibrParam[Index + DigitNumber] != '\0') )
        DigitNumber++;
      
      Param[i]= VibrParam[Index + DigitNumber - 1] & 0x0F;
      
      if(DigitNumber > 1)
      {
        for(int t=1; t<DigitNumber; t++)
        {
          Param[i]= Param[i] + ( (VibrParam[Index + DigitNumber - t - 1] & 0x0F) * ((uint32_t)pow(10.0,t)) );
        }           
      }

      switch(i)
      {
      /* FifoOdr (FIFO Accelerometer Output Data Rate in Hz) */
      case 0:
        if( (Param[i] == 13)   || (Param[i] == 26)   || (Param[i] == 52)   ||
            (Param[i] == 104)  || (Param[i] == 208)  || (Param[i] == 416)  ||
            (Param[i] == 833)  || (Param[i] == 1660) || (Param[i] == 3330) || (Param[i] == 6660)
           )
        {
          Accelerometer_Parameters.FifoOdr= Param[i];
          Accelerometer_Parameters.AccOdr=  Param[i];
          UpdatedParameters= 1;
          UpdatedAccParameters= 1;
        }
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for odr\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /* fs (Full Scale in g) */
      case 1:
        if( (Param[i] == 2) || (Param[i] == 4) || (Param[i] == 8) || (Param[i] == 16) )
        {
          Accelerometer_Parameters.fs= Param[i];
          UpdatedParameters= 1;
          UpdatedAccParameters= 1;
        }
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for fs\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /* size (FFT SIZE) */
      case 2:
        if( (Param[i] == 256) || (Param[i] == 512) || (Param[i] == 1024) || (Param[i] == 2048))
        {
          MotionSP_Parameters.FftSize= Param[i];
          UpdatedParameters= 1;
        }          
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for size\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /*  wind (PRE-FFT WINDOWING Method) */
      case 3:
        if(Param[i] < 4)
        {
          MotionSP_Parameters.window= Param[i];
          UpdatedParameters= 1;
        }          
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for wind\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /* tacq (TIME ACQUISITION WINDOW in ms) */
      case 4:
        if( (Param[i] >= 500) && (Param[i] <= 60000) )
        {
          MotionSP_Parameters.tacq= Param[i];
          UpdatedParameters= 1;
        }          
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for tacq\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /* ovl (FFT OVERLAPPING in %) */
      case 5:
        if( (Param[i] >= 5) && (Param[i] <= 95) )
        {
          MotionSP_Parameters.FftOvl= Param[i];
          UpdatedParameters= 1;
        }
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for ovl\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      /*  subrng (SUBRANGE number for evaluate thresholds) */
      case 6:
        if( (Param[i] == 8) || (Param[i] == 16) || (Param[i] == 32) || (Param[i] == 64) )
        {
          MotionSP_Parameters.subrange_num= Param[i];
          UpdatedParameters= 1;
        }
        else
        {
          BytesToWrite =sprintf((char *)BufferToWrite,"\r\nValue out of range for nsubrng\r\n");
          Term_Update(BufferToWrite,BytesToWrite);
        }
        break;
      }
      
      Index= Index + DigitNumber + 1;
    }
    else
    {
      if(VibrParam[Index] != '-')
      {
        BytesToWrite =sprintf((char *)BufferToWrite,"\r\nParam not found\r\n");
        Term_Update(BufferToWrite,BytesToWrite);
      }
    }
  }
  
  BytesToWrite =sprintf((char *)BufferToWrite,"\r\nOK\r\n");
  Term_Update(BufferToWrite,BytesToWrite);  
  
//  BytesToWrite =sprintf((char *)BufferToWrite,"\r\nNew accelerometer parameters:\r\n");
//  Term_Update(BufferToWrite,BytesToWrite);
//  BytesToWrite =sprintf((char *)BufferToWrite,"FifoOdr= %d fs= %d\r\n",
//                        Accelerometer_Parameters.FifoOdr,
//                        Accelerometer_Parameters.fs);
//  Term_Update(BufferToWrite,BytesToWrite);
//  
//  BytesToWrite =sprintf((char *)BufferToWrite,"New MotionSP parameters:\r\n");
//  Term_Update(BufferToWrite,BytesToWrite);
//  BytesToWrite =sprintf((char *)BufferToWrite,"size= %d wind= %d tacq= %d ovl= %d subrng= %d\r\n",
//                        MotionSP_Parameters.FftSize,
//                        MotionSP_Parameters.window,
//                        MotionSP_Parameters.tacq,
//                        MotionSP_Parameters.FftOvl,
//                        MotionSP_Parameters.subrange_num);
//  Term_Update(BufferToWrite,BytesToWrite);
  
  if(UpdatedAccParameters)
    SetAccelerometerParameters();
  
  return UpdatedParameters;
}

/**
 * @brief  This function makes the parsing of the Configuration Commands
 * @param uint8_t *att_data attribute data
 * @param uint8_t data_length length of the data
 * @retval uint32_t SendItBack true/false
 */
static uint32_t ConfigCommandParsing(uint8_t * att_data, uint8_t data_length)
{
  FeatureMask = (att_data[3]) | (att_data[2]<<8) | (att_data[1]<<16) | (att_data[0]<<24);
  uint8_t Command = att_data[4];
  uint8_t Data    = att_data[5];
  uint32_t SendItBack = 1;

  switch (FeatureMask) {
    /* Environmental features */
    case FEATURE_MASK_TEMP1:
    case FEATURE_MASK_TEMP2:
    case FEATURE_MASK_PRESS:
    case FEATURE_MASK_HUM:
      switch(Command) {
        case 255:
          /* Change the Sending interval */
          if(Data!=0) {
            /* Multiple of 100mS */
            __HAL_TIM_SET_AUTORELOAD(&TimEnvHandle,(((Data*TIM_CLOCK_ENV) / 10U) - 1));
            __HAL_TIM_SET_COUNTER(&TimEnvHandle,0);
            TimEnvHandle.Instance->EGR = TIM_EGR_UG;
          } else {
            /* Default Values */
            __HAL_TIM_SET_AUTORELOAD(&TimEnvHandle,((TIM_CLOCK_ENV  / ALGO_FREQ_ENV) - 1));
            __HAL_TIM_SET_COUNTER(&TimEnvHandle,0);
          }
#ifdef PREDMNT1_DEBUG_CONNECTION
          if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
            BytesToWrite = sprintf((char *)BufferToWrite,"Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
            Term_Update(BufferToWrite,BytesToWrite);
          } else {
            PREDMNT1_PRINTF("Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
          }
#endif /* PREDMNT1_DEBUG_CONNECTION */
          SendItBack = 0;
        break;
      }
    break;
    /* Inertial features */
    case FEATURE_MASK_ACC:
    case FEATURE_MASK_GRYO:
    case FEATURE_MASK_MAG:
      switch(Command) {
        case 255:
          /* Change the Sending interval */
          if(Data!=0) {
            /* Multiple of 100mS */
            uhCCR4_Val  = 1000*Data;
          } else {
            /* Default Value */
            uhCCR4_Val  = DEFAULT_uhCCR4_Val;
          }
#ifdef PREDMNT1_DEBUG_CONNECTION
          if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
            BytesToWrite = sprintf((char *)BufferToWrite,"Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
            Term_Update(BufferToWrite,BytesToWrite);
          } else {
            PREDMNT1_PRINTF("Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
          }
#endif /* PREDMNT1_DEBUG_CONNECTION */
          SendItBack = 0;
        break;
      }
    break;
    /* Environmental features */
    case FEATURE_MASK_MIC:
      switch(Command) {
        case 255:
          /* Change the Sending interval */
          if(Data!=0) {
            /* Multiple of 100mS */
            __HAL_TIM_SET_AUTORELOAD(&TimAudioDataHandle,(((Data*TIM_CLOCK_AUDIO_LEVEL) / 10U) - 1));
            __HAL_TIM_SET_COUNTER(&TimAudioDataHandle,0);
            TimAudioDataHandle.Instance->EGR = TIM_EGR_UG;
          } else {
            /* Default Values */
            __HAL_TIM_SET_AUTORELOAD(&TimAudioDataHandle,((TIM_CLOCK_AUDIO_LEVEL / ALGO_FREQ_AUDIO_LEVEL) - 1));
            __HAL_TIM_SET_COUNTER(&TimAudioDataHandle,0);
          }
#ifdef PREDMNT1_DEBUG_CONNECTION
          if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
            BytesToWrite = sprintf((char *)BufferToWrite,"Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
            Term_Update(BufferToWrite,BytesToWrite);
          } else {
            PREDMNT1_PRINTF("Conf Sig F=%lx C=%2x Data=%2x\n\r",FeatureMask,Command,Data);
          }
#endif /* PREDMNT1_DEBUG_CONNECTION */
          SendItBack = 0;
        break;
      }
    break;
  }
  return SendItBack;
}

/**
 * @brief  Force one UUID rescan
 * @param  None
 * @retval None
 */
static void Force_UUID_Rescan(void)
{
  tBleStatus ret;
  
  uint8_t buff[4];

  /* Delete all the Handles from 0x0001 to 0xFFFF */
  STORE_LE_16(buff  ,0x0001);
  STORE_LE_16(buff+2,0xFFFF);

  ret = aci_gatt_update_char_value(0x0001,0x0002,0,4,buff);

  if (ret == BLE_STATUS_SUCCESS){
    PREDMNT1_PRINTF("UUID Rescan Forced\r\n\r\n");
  } else {
    PREDMNT1_PRINTF("Problem forcing UUID Rescan\r\n\r\n");
  }
}

/* ***************** BlueNRG-2 Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{ 
  connected = TRUE;
  connection_handle = Connection_Handle;

#ifdef PREDMNT1_DEBUG_CONNECTION
  PREDMNT1_PRINTF(">>>>>>CONNECTED %x:%x:%x:%x:%x:%x\r\n\r\n",Peer_Address[5],Peer_Address[4],Peer_Address[3],Peer_Address[2],Peer_Address[1],Peer_Address[0]);
#endif /* PREDMNT1_DEBUG_CONNECTION */

  ConnectionBleStatus=0;
  FirstConnectionConfig  =0;
  
#ifdef DISABLE_FOTA
  FirstCommandSent       =1;
#endif /* DISABLE_FOTA */
}
/* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event occurs when a connection is terminated.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{  
  connected = FALSE;

  /*trigger the device connectable again*/
  isBeacon = TRUE;

#ifdef PREDMNT1_DEBUG_CONNECTION  
  PREDMNT1_PRINTF("\r\n<<<<<<DISCONNECTED\r\n");
#endif /* PREDMNT1_DEBUG_CONNECTION */

  /* Make the device connectable again. */
  set_connectable = TRUE;

  ConnectionBleStatus=0;
  FirstConnectionConfig  =0;

#ifdef DISABLE_FOTA
  FirstCommandSent       =1;
#endif /* DISABLE_FOTA */
  
  /* Reset for any problem during FOTA update */
  SizeOfUpdateBlueFW = 0;
  
  /************************/
  /* Stops all the Timers */
  /************************/
  
  /* Stop Timer For Acc/Gyro/Mag */
  if(HAL_TIM_OC_Stop_IT(&TimCCHandle, TIM_CHANNEL_4) != HAL_OK){
    /* Stopping Error */
    Error_Handler();
  }
  
  /* Stop Timer For Environmental */
  if(HAL_TIM_Base_Stop_IT(&TimEnvHandle) != HAL_OK){
    /* Stopping Error */
    Error_Handler();
  }
  
  /* Stop Timer For Audio Level and Source Localisation */
  if(HAL_TIM_Base_Stop_IT(&TimAudioDataHandle) != HAL_OK){
    /* Stopping Error */
    Error_Handler();
  }
}
/* end hci_disconnection_complete_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_read_permit_req_event.
 * Description    : This event is given when a read request is received
 *                  by the server from the client.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
                                    uint16_t Attribute_Handle,
                                    uint16_t Offset)
{
  Read_Request_CB(Attribute_Handle);    
}

/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : This event is given when an attribute change his value.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
  Attribute_Modified_Request_CB(Connection_Handle, Attr_Handle, Offset, Attr_Data_Length, Attr_Data);
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

/**
  * @}
  */

/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
