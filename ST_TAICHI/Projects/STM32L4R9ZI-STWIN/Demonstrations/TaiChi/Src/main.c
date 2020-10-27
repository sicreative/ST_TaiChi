/**
  ******************************************************************************
  * @file    main.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>

#include <limits.h>
#include "TargetFeatures.h"
#include "main.h"
#include "OTA.h"
#include "sensor_service.h"
#include "config.h"
#include "uuid_ble_service.h"

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
   
/* Private typedef -----------------------------------------------------------*/

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_PRIVATE_DEFINE Predictive Maintenance Main Private Define
  * @{
  */

/* Private define ------------------------------------------------------------*/
#define CHECK_VIBRATION_PARAM ((uint16_t)0x1234)


/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_IMPORTED_VARIABLES Predictive Maintenance Main Imported Variables
  * @{
  */

/* Imported Variables -------------------------------------------------------------*/
extern uint8_t set_connectable;
extern int connected;
extern volatile uint32_t FFT_Alarm;

extern uint8_t isBeacon;
extern uint8_t running_discovery;
    
extern volatile float RMS_Ch[];
extern float DBNOISE_Value_Old_Ch[];
extern uint16_t PCM_Buffer[];
extern uint32_t NumSample;

extern uint32_t Start_Tick;

extern sTimeDomainAlarm_t sTdAlarm;
extern sTimeDomainThresh_t sTdRmsThresholds;
extern sTimeDomainThresh_t sTdPkThresholds;
extern sAcceleroParam_t sTimeDomainVal;

/* X-Y-Z Frequency domain Subranges Status */
extern sFreqDomainAlarm_t THR_Fft_Alarms;

extern sAccelerometer_Parameter_t Accelerometer_Parameters;

extern void *MotionCompObj[MOTION_INSTANCES_NBR];





#ifdef PREDMNT1_ENABLE_PRINTF
  extern TIM_HandleTypeDef  TimHandle;
  extern void CDC_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
#endif /* PREDMNT1_ENABLE_PRINTF */

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_VARIABLES Predictive Maintenance Main Exported Variables
  * @{
  */

/* Exported Variables -------------------------------------------------------------*/
volatile uint8_t AccIntReceived= 0;
volatile uint8_t FifoEnabled = 0;
volatile uint32_t PredictiveMaintenance=     0;

uint8_t IsFirstTime = 0;

float sensitivity;

/* Acc sensitivity multiply by FROM_MG_TO_G constant */
float sensitivity_Mul;

uint32_t ConnectionBleStatus  =0;
uint32_t FirstConnectionConfig =0;

uint8_t BufferToWrite[256];
int32_t BytesToWrite;

TIM_HandleTypeDef    TimCCHandle;
TIM_HandleTypeDef    TimEnvHandle;
TIM_HandleTypeDef    TimAudioDataHandle;

uint8_t bdaddr[6];

uint32_t uhCCR4_Val = DEFAULT_uhCCR4_Val;

uint8_t  NodeName[8];
uint16_t VibrationParam[11];

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_PRIVATE_VARIABLES Predictive Maintenance Main Private Variables
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/* Table with All the known Meta Data */
MDM_knownGMD_t known_MetaData[]={
  {GMD_NODE_NAME,      (sizeof(NodeName))},
  {GMD_VIBRATION_PARAM,(sizeof(VibrationParam))},
  {GMD_END    ,0}/* THIS MUST BE THE LAST ONE */
};

static volatile uint32_t ButtonPressed=         0;
static volatile uint32_t HCI_ProcessEvent=      0;
static volatile uint32_t SendEnv=               0;
static volatile uint32_t SendAudioLevel=        0;
static volatile uint32_t SendAccGyroMag=        0;
static volatile uint32_t SendBatteryInfo=       0;
static volatile uint32_t t_stwin=               0;
static volatile uint32_t TriggerMotionMLInt=			0;
static volatile uint32_t printData=				0;
static volatile uint32_t beaconUpdateTimer=		0;


typedef struct {
	uint16_t type;
	uint32_t start;
	uint32_t end;
}taiChiResult_t;



volatile taiChiResult_t* BufftaiChiResult[BUFF_TAICHIRESULT_SIZE];
volatile uint8_t taiChiResultPos = 0;





/* CRC handler declaration */
static CRC_HandleTypeDef hcrc;

float *FDWarnThresh;
float *FDAlarmThresh;

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_PRIVATE_FUNCTIONS_PROTOTYPES Predictive Maintenance Main Private Functions Prototypes
  * @{
  */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

static void InitTimers(void);
static void Init_BlueNRG_Custom_Services(void);
static void Init_BlueNRG_Stack(void);
static void InitPredictiveMaintenance(void);

static void Set2GAccelerometerFullScale(void);

static void MX_CRC_Init(void);

static unsigned char ReCallNodeNameFromMemory(void);
static unsigned char ReCallVibrationParamFromMemory(void);

static void SendEnvironmentalData(void);
static void SendMotionData(void);
static void SendAudioLevelData(void);
static void SendBatteryInfoData(void);
static void SendTaiChiData(void);

static void ButtonCallback(void);
static void AudioProcess(void);
static void AudioProcess_DB_Noise(void);

static void beaconUpdate(void);

void APP_UserEvtRx(void *pData);

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_PRIVATE_FUNCTIONS Predictive Maintenance Main Private Functions
  * @{
  */










static void _MotionMLIntSet(int status,ism330dhcx_pin_int2_route_t* intRoute){


		stmdev_ctx_t* ctx = &(((ISM330DHCX_Object_t *)MotionCompObj[ISM330DHCX_0])->Ctx);



		intRoute->mlc_int2.int2_mlc1 = status;






		if (!ism330dhcx_pin_int2_route_set(ctx,intRoute) &&
			!ism330dhcx_int_notification_set(ctx, ISM330DHCX_BASE_PULSED_EMB_LATCHED)){
			PREDMNT1_PRINTF("Enabled MLC1 interrupt\r\n");
		}

}



static void MotionMLIntSet(int status){
		stmdev_ctx_t* ctx = &(((ISM330DHCX_Object_t *)MotionCompObj[ISM330DHCX_0])->Ctx);

		ism330dhcx_pin_int2_route_t intRoute;


		ism330dhcx_pin_int2_route_get(ctx,&intRoute);

		_MotionMLIntSet(status,&intRoute);

}

static void MotionMLIntToggle(){

		stmdev_ctx_t* ctx = &(((ISM330DHCX_Object_t *)MotionCompObj[ISM330DHCX_0])->Ctx);

		ism330dhcx_pin_int2_route_t intRoute;

		ism330dhcx_pin_int2_route_get(ctx,&intRoute);

		_MotionMLIntSet(!intRoute.mlc_int2.int2_mlc1,&intRoute);

}

/** @brief Motion ML Initialization
  * @param None
  * @retval None
  */
static void InitMotionML(void){



	int n = sizeof(taichi)/sizeof(taichi[0]);

	// Load config data for ML engine
	for(int i=0;i<n;i++){

		ISM330DHCX_Write_Reg( MotionCompObj[ISM330DHCX_0], taichi[i].address , taichi[i].data );
	}

	ISM330DHCX_ACC_Disable(MotionCompObj[ISM330DHCX_0]);
	ISM330DHCX_GYRO_Disable(MotionCompObj[ISM330DHCX_0]);

	ISM330DHCX_ACC_SetFullScale(MotionCompObj[ISM330DHCX_0],ISM330DHCX_2g);
	ISM330DHCX_GYRO_SetFullScale(MotionCompObj[ISM330DHCX_0],ISM330DHCX_4000dps);

	ISM330DHCX_ACC_Enable(MotionCompObj[ISM330DHCX_0]);
		ISM330DHCX_GYRO_Enable(MotionCompObj[ISM330DHCX_0]);


	// Set MotionML Interrupt
	MotionMLIntSet(1);




}

/** @brief Get and prepare data from MotionML
  * @param None
  * @retval None
  */
static void getMotionMLData(void){



	ism330dhcx_mlc_status_mainpage_t status;
	uint8_t result;



	stmdev_ctx_t* ctx = &(((ISM330DHCX_Object_t *)MotionCompObj[ISM330DHCX_0])->Ctx);


	ism330dhcx_mlc_get(ctx,&result);

	if (!result){
		PREDMNT1_PRINTF("MLC not enable\r\n");
		return;
	}



	ism330dhcx_mlc_status_get(ctx,&status);



	if (status.is_mlc1){




		int32_t ret = ism330dhcx_mem_bank_set(ctx, ISM330DHCX_EMBEDDED_FUNC_BANK);
		  if (ret == 0) {
			  if (ISM330DHCX_OK == ISM330DHCX_Read_Reg(MotionCompObj[ISM330DHCX_0],ISM330DHCX_MLC0_SRC,&result)){



			  			uint32_t current = HAL_GetTick();


			  			taiChiResult_t* last = BufftaiChiResult[taiChiResultPos];

			  		//	PREDMNT1_PRINTF("MLC: 0x%02X\r\n",result);


			  			if (last){
			  				if (result==0 || last->type != result){
			  						 //update last item second and inc pos

			  						 last->end = current;

			  						 if (last->end-last->start<1000){
			  							 free(last);
			  							BufftaiChiResult[taiChiResultPos] = NULL;

			  						 }else{

			  							 ++taiChiResultPos;
			  						 }


			  				}else{

			  					goto end;
			  				}



			  			}

			  			if (result==0)
			  				goto end;


			  			if (taiChiResultPos==BUFF_TAICHIRESULT_SIZE){


			  				free(BufftaiChiResult[0]);

			  				for (int i=1;i<BUFF_TAICHIRESULT_SIZE;i++)
			  						 BufftaiChiResult[i-1]=BufftaiChiResult[i];

			  						  		--taiChiResultPos;

			  						  			}

			  			taiChiResult_t* item = malloc(sizeof(taiChiResult_t));


			  			item->type = result;
			  			item->start = current;


			  			PREDMNT1_PRINTF("MLC: %d - 0x%02X\r\n",taiChiResultPos,item->type);

			  			BufftaiChiResult[taiChiResultPos] = item;

			  }
		  }
		  end:
		  if (ret == 0) {
		    ret = ism330dhcx_mem_bank_set(ctx, ISM330DHCX_USER_BANK);


		}


	}else if (status.is_mlc2){
		PREDMNT1_PRINTF("mlc2\r\n");
	}else if (status.is_mlc3){
		PREDMNT1_PRINTF("mlc3\r\n");
	}else if (status.is_mlc4){
		PREDMNT1_PRINTF("mlc4\r\n");
	}else if (status.is_mlc5){
		PREDMNT1_PRINTF("mlc5\r\n");
	}else if (status.is_mlc6){
		PREDMNT1_PRINTF("mlc6\r\n");
	}else if (status.is_mlc7){
		PREDMNT1_PRINTF("mlc7\r\n");
	}else if (status.is_mlc8){
		PREDMNT1_PRINTF("mlc8\r\n");
	}




//	if (result)
//		PREDMNT1_PRINTF("MLC: 0x%.2X\r\n",result);

}


static void MotionMLTrainingData(void){
	 MOTION_SENSOR_Axes_t ACC_Value;
	  MOTION_SENSOR_Axes_t GYR_Value;
	  MOTION_SENSOR_Axes_t MAG_Value;

	  /* Read the Acc values */
	  if(TargetBoardFeatures.AccSensorIsInit)
	  {
	    MOTION_SENSOR_GetAxes(ACCELERO_INSTANCE, MOTION_ACCELERO, &ACC_Value);
	  }
	  else
	  {
	    ACC_Value.x = ACC_Value.y = ACC_Value.z =0;
	  }

	  /* Read the Magneto values */
	  if(TargetBoardFeatures.MagSensorIsInit)
	  {
	    MOTION_SENSOR_GetAxes(MAGNETO_INSTANCE, MOTION_MAGNETO, &MAG_Value);
	  }
	  else
	  {
	    MAG_Value.x = MAG_Value.y = MAG_Value.z =0;
	  }

	  /* Read the Gyro values */
	  if(TargetBoardFeatures.GyroSensorIsInit)
	  {
	    MOTION_SENSOR_GetAxes(GYRO_INSTANCE,MOTION_GYRO, &GYR_Value);
	  }
	  else
	  {
	    GYR_Value.x = GYR_Value.y = GYR_Value.z =0;
	  }

	  PREDMNT1_PRINTF("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",ACC_Value.x,ACC_Value.y,ACC_Value.z,GYR_Value.x,GYR_Value.y,GYR_Value.z,MAG_Value.x,MAG_Value.y,MAG_Value.z);

}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  HAL_Init();
  
    HAL_PWREx_EnableVddIO2();
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_EnableVddUSB();

  /* Configure the System clock */
  SystemClock_Config();
 
  InitTargetPlatform(TARGET_STWIN);
  
  t_stwin = HAL_GetTick();
  
  /* For enabling CRC clock for using motion libraries (for checking if STM32 microprocessor is used)*/
  MX_CRC_Init();
  
  /* Check the MetaDataManager */
  InitMetaDataManager((void *)&known_MetaData,MDM_DATA_TYPE_GMD,NULL); 
  
  PREDMNT1_PRINTF("\n\t(HAL %ld.%ld.%ld_%ld)\r\n"
        "\tCompiled %s %s"

#if defined (__IAR_SYSTEMS_ICC__)
        " (IAR)\r\n"
#elif defined (__CC_ARM)
        " (KEIL)\r\n"
#elif defined (__GNUC__)
        " (STM32CubeIDE)\r\n"
#endif
         "\tSend Every %4dmS Temperature/Humidity/Pressure\r\n"
         "\tSend Every %4dmS Acc/Gyro/Magneto\r\n"
         "\tSend Every %4dmS dB noise\r\n\n",
           HAL_GetHalVersion() >>24,
          (HAL_GetHalVersion() >>16)&0xFF,
          (HAL_GetHalVersion() >> 8)&0xFF,
           HAL_GetHalVersion()      &0xFF,
         __DATE__,__TIME__,
         ALGO_PERIOD_ENV,
         ALGO_PERIOD_ACC_GYRO_MAG,
         ALGO_PERIOD_AUDIO_LEVEL);

#ifdef PREDMNT1_DEBUG_CONNECTION
  PREDMNT1_PRINTF("Debug Connection         Enabled\r\n");
#endif /* PREDMNT1_DEBUG_CONNECTION */

#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
  PREDMNT1_PRINTF("Debug Notify Trasmission Enabled\r\n\n");
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

  /* Set Node Name */
  ReCallNodeNameFromMemory();
  
  HCI_TL_SPI_Reset();

  /* Initialize the BlueNRG */
  Init_BlueNRG_Stack();

  /* Initialize the BlueNRG Custom services */
  Init_BlueNRG_Custom_Services();  
  
  /* Check the BootLoader Compliance */
  PREDMNT1_PRINTF("\r\n");
  if(CheckBootLoaderCompliance()) {
    PREDMNT1_PRINTF("BootLoader Compliant with FOTA procedure\r\n\n");
  } else {
    PREDMNT1_PRINTF("ERROR: BootLoader NOT Compliant with FOTA procedure\r\n\n");
  }




  /* Set Accelerometer Full Scale to 2G */
  //Set2GAccelerometerFullScale();

  /* initialize timers */
  InitTimers();
  
  /* Predictive Maintenance Initialization */
  //InitPredictiveMaintenance();
  
  InitMotionML();


  /* Infinite loop */
  while (1)
  {
    /* Led Blinking when there is not a client connected */
    if(!connected)
    {
      if(!TargetBoardFeatures.LedStatus) {
        if(!(HAL_GetTick()&0x3FF)) {
          LedOnTargetPlatform();
        }
      } else {
        if(!(HAL_GetTick()&0x3F)) {
          LedOffTargetPlatform();
        }
      }

    }


/*
 *
 * Interval between broadcast iBeacon and connectable mode
 *
 *
 *  iBeacon used for notification to user have data waiting to send when App killed/force closed.
 *  Based on the Apple Spec, iBeacon will send every 100ms at ADV_NONCONN_IND mode,
 *  we flip to normal connectable mode when idle so iOS can detect proximity
 *  and the App able to connect simultaneously.
 *
 *  There no need to broadcast for iBeacon when connected or no data to send.
 *
 * */

    if (!connected && HAL_GetTick()%100 < 50){

    	// Only broadcast iBeacon when Buff have data waiting to send
     	if (!isBeacon && taiChiResultPos){
     		setBeacon();
     		isBeacon=TRUE;
     	}
    }else{

    	if (isBeacon){
    		setConnectable();
    		isBeacon=FALSE;
    	}
     }


//    if(set_connectable){
//      if(NecessityToSaveMetaDataManager) {
//        uint32_t Success = EraseMetaDataManager();
//        if(Success) {
//          SaveMetaDataManager();
//        }
//      }
//
//      /* Now update the BLE advertize data and make the Board connectable */
//      setConnectable();
//      setBeacon();
//      set_connectable = FALSE;
//
//    }

    /* Handle user button */
    if(ButtonPressed) {
      ButtonCallback();
      ButtonPressed=0;       
    }
    
//    if(PredictiveMaintenance){
//      if (IsFirstTime)
//      {
//        IsFirstTime = 0;
//
//        /* Enable the HP Filter */
//        EnableDisable_ACC_HP_Filter(1);
//
//	/* Initializes the MotionSP Vibration parameters */
//        MotionSP_VibrationInit();
//
//        if(FFT_Alarm)
//        {
//          /* Initialization of Alarm Status on Axes, Alarm Values Reported
//             and Thresholds to detect WARNING and ALARM conditions */
//          MotionSP_TimeDomainAlarmInit(&sTdAlarm, &sTimeDomainVal, &sTdRmsThresholds, &sTdPkThresholds);
//
//          /* Frequency domain initialization of Alarm Status */
//          MotionSP_FreqDomainAlarmInit(&FDWarnThresh, &FDAlarmThresh, &THR_Fft_Alarms, MotionSP_Parameters.subrange_num);
//        }
//
//        /* Configure the FIFO settings in according with others parammeters changed */
//       MotionSP_ConfigFifo();
//
//        enable_FIFO();
//
//        /* Start of the Time Window */
//        Start_Tick = HAL_GetTick();
//      }
//
//
//
//      MotionSP_VibrationAnalysis();
//    }

    /* handle BLE event */
    if(HCI_ProcessEvent) {
      HCI_ProcessEvent=0;
      hci_user_evt_proc();
    }

    /* Environmental Data */
    if(SendEnv) {
      SendEnv=0;
      SendEnvironmentalData();
    }

    /* Mic Data */
    if (SendAudioLevel) {
      SendAudioLevel = 0;
      SendAudioLevelData();
    }

    /* Motion Data */
    if(SendAccGyroMag) {
      SendAccGyroMag=0;
      SendMotionData();
    }

    /* Battery Info Data */
    if(SendBatteryInfo){
      SendBatteryInfo=0;
      SendBatteryInfoData();
    }
    
    if(TriggerMotionMLInt){
    		getMotionMLData();
        	TriggerMotionMLInt=0;
    }




//    if (printData && !(HAL_GetTick()%38)){
//
//
//    	MotionMLTrainingData();
//
//    }





    /* taichi Data*/


    SendTaiChiData();









    /* Wait for Event */
    //__WFI();
  }
}

/**
  * @brief  This function sets the ACC FS to 2g
  * @param  None
  * @retval None
  */
static void Set2GAccelerometerFullScale(void)
{
  /* Set Full Scale to +/-2g */
  MOTION_SENSOR_SetFullScale(ACCELERO_INSTANCE, MOTION_ACCELERO, 2.0f);
  
  /* Read the Acc Sensitivity */
  MOTION_SENSOR_GetSensitivity(ACCELERO_INSTANCE, MOTION_ACCELERO, &sensitivity);
  sensitivity_Mul = sensitivity* ((float) FROM_MG_TO_G);
}

/**
  * @brief  Callback for user button
  * @param  None
  * @retval None
  */
static void ButtonCallback(void)
{
  PREDMNT1_PRINTF("\r\nUser Button Pressed   \r\n\r\n");

//  printData = !printData;
//  SendMotionML = !SendMotionML;
}


/**
  * @brief  Send Motion Data Acc/Mag/Gyro to BLE
  * @param  None
  * @retval None
  */
static void SendMotionData(void)
{
  MOTION_SENSOR_Axes_t ACC_Value;
  MOTION_SENSOR_Axes_t GYR_Value;
  MOTION_SENSOR_Axes_t MAG_Value;

  /* Read the Acc values */
  if(TargetBoardFeatures.AccSensorIsInit)
  {
    MOTION_SENSOR_GetAxes(ACCELERO_INSTANCE, MOTION_ACCELERO, &ACC_Value);
  }
  else
  {
    ACC_Value.x = ACC_Value.y = ACC_Value.z =0;
  }

  /* Read the Magneto values */
  if(TargetBoardFeatures.MagSensorIsInit)
  {
    MOTION_SENSOR_GetAxes(MAGNETO_INSTANCE, MOTION_MAGNETO, &MAG_Value);
  }
  else
  {
    MAG_Value.x = MAG_Value.y = MAG_Value.z =0;
  }
  
  /* Read the Gyro values */
  if(TargetBoardFeatures.GyroSensorIsInit)
  {
    MOTION_SENSOR_GetAxes(GYRO_INSTANCE,MOTION_GYRO, &GYR_Value);
  }
  else
  {
    GYR_Value.x = GYR_Value.y = GYR_Value.z =0;
  }
  
  AccGyroMag_Update(&ACC_Value,&GYR_Value,&MAG_Value);
}

/**
* @brief  User function that is called when 1 ms of PDM data is available.
* @param  none
* @retval None
*/
static void AudioProcess(void)
{
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL))
  {
    AudioProcess_DB_Noise();
  }
}

/**
* @brief  User function that is called when 1 ms of PDM data is available.
* @param  none
* @retval None
*/
static void AudioProcess_DB_Noise(void)
{
  int32_t i;
  int32_t NumberMic;
  
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL)) {
    for(i = 0; i < (NumSample/2); i++){
      for(NumberMic=0;NumberMic<AUDIO_IN_CHANNELS;NumberMic++) {
        RMS_Ch[NumberMic] += (float)((int16_t)PCM_Buffer[i*AUDIO_IN_CHANNELS+NumberMic] * ((int16_t)PCM_Buffer[i*AUDIO_IN_CHANNELS+NumberMic]));
      }
    }
  }
}

/**
  * @brief  Send Audio Level Data (Ch1) to BLE
  * @param  None
  * @retval None
  */
static void SendAudioLevelData(void)
{
  int32_t NumberMic;
  uint16_t DBNOISE_Value_Ch[AUDIO_IN_CHANNELS];
  
  for(NumberMic=0;NumberMic<(AUDIO_IN_CHANNELS);NumberMic++) {
    DBNOISE_Value_Ch[NumberMic] = 0;

    RMS_Ch[NumberMic] /= ((float)(NumSample/AUDIO_IN_CHANNELS)*ALGO_PERIOD_AUDIO_LEVEL);

    DBNOISE_Value_Ch[NumberMic] = (uint16_t)((120.0f - 20 * log10f(32768 * (1 + 0.25f * (AUDIO_VOLUME_INPUT /*AudioInVolume*/ - 4))) + 10.0f * log10f(RMS_Ch[NumberMic])) * 0.3f + DBNOISE_Value_Old_Ch[NumberMic] * 0.7f);
    DBNOISE_Value_Old_Ch[NumberMic] = DBNOISE_Value_Ch[NumberMic];
    RMS_Ch[NumberMic] = 0.0f;
  }
  
  AudioLevel_Update(DBNOISE_Value_Ch);
}

/**
  * @brief  Send Environmetal Data (Temperature/Pressure/Humidity) to BLE
  * @param  None
  * @retval None
  */
static void SendEnvironmentalData(void)
{
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
     BytesToWrite = sprintf((char *)BufferToWrite,"Sending: ");
     Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("Sending: ");
  }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

  if(FirstConnectionConfig)
  {
    FirstConnectionConfig=0;
  }

  /* Pressure,Humidity, and Temperatures*/
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_ENV)) {
    float SensorValue;
    int32_t PressToSend=0;
    uint16_t HumToSend=0;
    int16_t Temp2ToSend=0,Temp1ToSend=0;
    int32_t decPart, intPart;

    if(TargetBoardFeatures.PressSensorIsInit)
    {
      ENV_SENSOR_GetValue(PRESSURE_INSTANCE,ENV_PRESSURE, &SensorValue);
      MCR_BLUEMS_F2I_2D(SensorValue, intPart, decPart);
      PressToSend=intPart*100+decPart;
    }
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
      BytesToWrite = sprintf((char *)BufferToWrite,"Press=%ld ",PressToSend);
      Term_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Press=%ld ",PressToSend);
    }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

    if(TargetBoardFeatures.HumSensorIsInit)
    {
      ENV_SENSOR_GetValue(HUMIDITY_INSTANCE,ENV_HUMIDITY,&SensorValue);
      MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
      HumToSend = intPart*10+decPart;
    }
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
      BytesToWrite = sprintf((char *)BufferToWrite,"Hum=%d ",HumToSend);
      Term_Update(BufferToWrite,BytesToWrite);
    } else {
      PREDMNT1_PRINTF("Hum=%d ",HumToSend);
    }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

    if(TargetBoardFeatures.NumTempSensors==2) {
      if(TargetBoardFeatures.TempSensorsIsInit[0])
      {
        ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_1,ENV_TEMPERATURE,&SensorValue);
        MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
        Temp1ToSend = intPart*10+decPart;
      }
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
      if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
        BytesToWrite = sprintf((char *)BufferToWrite,"Temp=%d ",Temp1ToSend);
        Term_Update(BufferToWrite,BytesToWrite);
      } else {
        PREDMNT1_PRINTF("Temp=%d ",Temp1ToSend);
      }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

      if(TargetBoardFeatures.TempSensorsIsInit[1])
      {
        ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_2,ENV_TEMPERATURE,&SensorValue);
        MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
        Temp2ToSend = intPart*10+decPart;
      }
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
      if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
        BytesToWrite = sprintf((char *)BufferToWrite,"Temp2=%d ",Temp2ToSend);
        Term_Update(BufferToWrite,BytesToWrite);
      } else {
        PREDMNT1_PRINTF("Temp2=%d ",Temp2ToSend);
      }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
    } else if(TargetBoardFeatures.NumTempSensors==1) {
      if(TargetBoardFeatures.TempSensorsIsInit[0])
      {
        ENV_SENSOR_GetValue(TEMPERATURE_INSTANCE_1,ENV_TEMPERATURE,&SensorValue);
        MCR_BLUEMS_F2I_1D(SensorValue, intPart, decPart);
        Temp1ToSend = intPart*10+decPart;
      }
#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
      if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
        BytesToWrite = sprintf((char *)BufferToWrite,"Temp1=%d ",Temp1ToSend);
        Term_Update(BufferToWrite,BytesToWrite);
      } else {
        PREDMNT1_PRINTF("Temp1=%d ",Temp1ToSend);
      }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
    }
    Environmental_Update(PressToSend,HumToSend,Temp2ToSend,Temp1ToSend);
  }

#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
     BytesToWrite = sprintf((char *)BufferToWrite,"\r\n");
     Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("\r\n");
  }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
}

/**
  * @brief  Send Battery Info Data (Voltage/Current/Soc) to BLE
  * @param  None
  * @retval None
  */
static void SendBatteryInfoData(void)
{
  uint32_t Voltage;
  uint32_t BatteryLevel= 0;

  stbc02_State_TypeDef BC_State;



  /* Read the voltage value and battery level status */
  BSP_BC_GetVoltageAndLevel(&Voltage,&BatteryLevel);

  BSP_BC_GetState(&BC_State);




#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
     BytesToWrite = sprintf((char *)BufferToWrite,"Battery Report: \r\n");
     Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("Battery Report: ");
  }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */

  /* Battery Informations */
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_BATTERY_INFO)) {
    BatteryInfo_Update(BatteryLevel, Voltage, BC_State);

#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
     if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
       BytesToWrite = sprintf((char *)BufferToWrite,"Charge= %ld%% Voltage=%ld mV BC_State= %d", BatteryLevel, Voltage, BC_State.Id);
       Term_Update(BufferToWrite,BytesToWrite);
     } else {
       PREDMNT1_PRINTF("Charge= %ld%% Voltage=%ld mV BC_State= %d", BatteryLevel, Voltage, BC_State.Id);
     }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
  }

#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
     BytesToWrite = sprintf((char *)BufferToWrite,"\r\n");
     Term_Update(BufferToWrite,BytesToWrite);
  } else {
    PREDMNT1_PRINTF("\r\n");
  }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
}

/**
  * @brief  stop unnecessary IRQ and off all sensor before sleep
  * @param  None
  * @retval None
  */
void SleepDisable(void){

	HAL_NVIC_DisableIRQ(DFSDM_DMA_ANALOG_IRQn);
	HAL_NVIC_DisableIRQ(DFSDM_DMA_DIGITAL_IRQn);
	HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);

	  if(TargetBoardFeatures.AccSensorIsInit==1)
		  BSP_MOTION_SENSOR_Disable(ACCELERO_INSTANCE, MOTION_ACCELERO);

	 if(TargetBoardFeatures.GyroSensorIsInit==1)
		 BSP_MOTION_SENSOR_Disable(GYRO_INSTANCE, MOTION_GYRO);
	if (TargetBoardFeatures.MagSensorIsInit== 1)
		BSP_MOTION_SENSOR_Disable(MAGNETO_INSTANCE, MOTION_MAGNETO);
	if (TargetBoardFeatures.TempSensorsIsInit == 1 && TargetBoardFeatures.HumSensorIsInit == 1 )
		BSP_ENV_SENSOR_Disable(HUMIDITY_INSTANCE,ENV_TEMPERATURE);
	if (TargetBoardFeatures.HumSensorIsInit == 1 )
		BSP_ENV_SENSOR_Disable(HUMIDITY_INSTANCE,ENV_HUMIDITY);
	if(TargetBoardFeatures.TempSensorsIsInit[1])
		 BSP_ENV_SENSOR_Disable(TEMPERATURE_INSTANCE_2, ENV_TEMPERATURE);
    if(TargetBoardFeatures.PressSensorIsInit)
		 BSP_ENV_SENSOR_Disable(PRESSURE_INSTANCE, ENV_PRESSURE);






	//HAL_NVIC_DisableIRQ(HCI_TL_SPI_EXTI_IRQn);
	HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	HAL_NVIC_DisableIRQ(STBC02_USED_TIM_IRQn);
	HAL_NVIC_DisableIRQ(TIM4_IRQn);
	HAL_NVIC_DisableIRQ(TIM5_IRQn);
	HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
#ifdef PREDMNT1_ENABLE_PRINTF
	HAL_NVIC_DisableIRQ(TIMx_IRQn);
#endif



	//STBC02_USED_TIM_CLK_DISABLE();





	__HAL_RCC_TIM1_CLK_DISABLE();
	 __HAL_RCC_TIM4_CLK_DISABLE();
	 __HAL_RCC_TIM5_CLK_DISABLE();

	 /*##-1- Enable peripherals and GPIO Clocks #################################*/
	  /* TIM3 Peripheral clock enable */
	  __HAL_RCC_TIM3_CLK_DISABLE();
	  /* Enable GPIO channels Clock */
	  __HAL_RCC_GPIOB_CLK_DISABLE();


	  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);


	  HAL_GPIO_DeInit(GPIOA, (GPIO_PIN_11 | GPIO_PIN_12));






	//HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
	//HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
}

/**
  * @brief  star  IRQ and on all sensor after sleep
  * @param  None
  * @retval None
  */
void SleepEnable(void){




	HAL_NVIC_EnableIRQ(DFSDM_DMA_ANALOG_IRQn);
	HAL_NVIC_EnableIRQ(DFSDM_DMA_DIGITAL_IRQn);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	//HAL_NVIC_EnableIRQ(HCI_TL_SPI_EXTI_IRQn);
	HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_EnableIRQ(STBC02_USED_TIM_IRQn);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
	HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
#ifdef PREDMNT1_ENABLE_PRINTF
	HAL_NVIC_EnableIRQ(TIMx_IRQn);
#endif






	__HAL_RCC_TIM1_CLK_ENABLE();
	 __HAL_RCC_TIM4_CLK_ENABLE();
	 __HAL_RCC_TIM5_CLK_ENABLE();

	STBC02_USED_TIM_CLK_ENABLE();



	 /*##-1- Enable peripherals and GPIO Clocks #################################*/
	  /* TIM3 Peripheral clock enable */
	  __HAL_RCC_TIM3_CLK_ENABLE();
	  /* Enable GPIO channels Clock */
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  GPIO_InitTypeDef GPIO_InitStruct;

	  /* Configure  (TIMx_Channel) in Alternate function, push-pull and high speed */
	  GPIO_InitStruct.Pin = GPIO_PIN_0;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



//	   /* Configure USB FS GPIOs */
//	   __HAL_RCC_GPIOA_CLK_ENABLE();

	   /* Configure DM DP Pins */
	   GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	   GPIO_InitStruct.Pull = GPIO_NOPULL;
	   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	   /* Enable USB FS Clock */
//	   __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

//	HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
//	HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);


	  /*  Enable all the sensors */

	if(TargetBoardFeatures.AccSensorIsInit==1)
		 BSP_MOTION_SENSOR_Enable(ACCELERO_INSTANCE, MOTION_ACCELERO);

	if(TargetBoardFeatures.GyroSensorIsInit==1)
		 BSP_MOTION_SENSOR_Enable(GYRO_INSTANCE, MOTION_GYRO);
	if (TargetBoardFeatures.MagSensorIsInit== 1)
		BSP_MOTION_SENSOR_Enable(MAGNETO_INSTANCE, MOTION_MAGNETO);
	if (TargetBoardFeatures.TempSensorsIsInit == 1)
		BSP_MOTION_SENSOR_Enable(HUMIDITY_INSTANCE,ENV_TEMPERATURE);
	if (TargetBoardFeatures.HumSensorIsInit == 1 )
		BSP_MOTION_SENSOR_Enable(HUMIDITY_INSTANCE,ENV_HUMIDITY);
	if(TargetBoardFeatures.TempSensorsIsInit[1])
		BSP_MOTION_SENSOR_Enable(TEMPERATURE_INSTANCE_2, ENV_TEMPERATURE);
    if(TargetBoardFeatures.PressSensorIsInit)
    	BSP_MOTION_SENSOR_Enable(PRESSURE_INSTANCE, ENV_PRESSURE);



}

/**
  * @brief  Send TaiChi Data to BLE
  * @param  None
  * @retval None
  */
static void SendTaiChiData(void)
{

  /* Check have data waiting to send*/
	if (!taiChiResultPos){


	    if (W2ST_CHECK_CONNECTION(W2ST_CONNECT_TAICHI) && !taiChiResultPos && BufftaiChiResult[0]==NULL){

	    	// Entry to Sleep mode for Power Save when no activity
#ifdef PREDMNT1_ENABLE_PRINTF



	    	  PREDMNT1_PRINTF("Enter Sleep mode\r\n");




#endif



	    	 HAL_SuspendTick();
	    //	 SleepDisable();
	    	 HAL_PWREx_EnableLowPowerRunMode();
	    	// HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);
	    	 HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);
	     	 HAL_PWREx_DisableLowPowerRunMode();
	    //	 SleepEnable();
	    	 HAL_ResumeTick();






	     }



			  return;
	}


  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_TAICHI)) {

	  uint8_t buff[4+W2ST_TAICHI_MAX_BUFF_LEN*6];

	  uint32_t timestamp = HAL_GetTick();
	  STORE_LE_16(buff,timestamp>>3);


	  // num of items have been send in this event, max 4

	  uint16_t num_send = taiChiResultPos>4?4:taiChiResultPos;



	  STORE_LE_16(buff+2,num_send);

	  int i=0;

	 for(;i<num_send;i++){


		  STORE_LE_16(buff+4+i*6,BufftaiChiResult[i]->type);

		  // Start and End time are the seconds before current timestamp
		  uint16_t start = (timestamp-BufftaiChiResult[i]->start)/1000;
		  uint16_t end = (timestamp-BufftaiChiResult[i]->end)/1000;


		  STORE_LE_16(buff+6+i*6,start);
		  STORE_LE_16(buff+8+i*6,end);



		  free(BufftaiChiResult[i]);




	  }


	  do
		  if(i<taiChiResultPos)
			  BufftaiChiResult[i-num_send]=BufftaiChiResult[i];
		  else
			  BufftaiChiResult[i-num_send]=NULL;
	  while(++i<BUFF_TAICHIRESULT_SIZE);

	  taiChiResultPos -= num_send;


	  i=0;

	  PREDMNT1_PRINTF("Send Hex:\r\n");

	  do {
	 	 PREDMNT1_PRINTF("%02X ",buff[i++])
	  }
	  while(++i<(4+W2ST_TAICHI_MAX_BUFF_LEN*6));

	  PREDMNT1_PRINTF("\r\n");


	  TaiChi_Update(buff);




    

#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
     if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
       BytesToWrite = sprintf((char *)BufferToWrite,"send taichi data");
       Term_Update(BufferToWrite,BytesToWrite);
     } else {
       PREDMNT1_PRINTF("send taichi data\r\n");
     }
#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */



  }

//#ifdef PREDMNT1_DEBUG_NOTIFY_TRAMISSION
//  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_STD_TERM)) {
//     BytesToWrite = sprintf((char *)BufferToWrite,"\r\n");
//     Term_Update(BufferToWrite,BytesToWrite);
//  } else {
//    PREDMNT1_PRINTF("\r\n");
//  }
//#endif /* PREDMNT1_DEBUG_NOTIFY_TRAMISSION */
}


/**
  * @brief  CRC init function.
  * @param  None
  * @retval None
  */
static void MX_CRC_Init(void)
{
  hcrc.Instance = CRC;

  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
* @brief  Function for initializing timers for sending the information to BLE:
 *  - 1 for sending MotionFX/AR/CP and Acc/Gyro/Mag
 *  - 1 for sending the Environmental info
 * @param  None
 * @retval None
 */
static void InitTimers(void)
{
  uint32_t uwPrescalerValue;
  
  /* Timer Output Compare Configuration Structure declaration */
  TIM_OC_InitTypeDef sConfig;
  
  /* Compute the prescaler value to have TIM4 counter clock equal to TIM_CLOCK_ENV Hz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / TIM_CLOCK_ENV) - 1);
  
  /* Set TIM4 instance ( Environmental ) */
  TimEnvHandle.Instance = TIM4;
  /* Initialize TIM4 peripheral */
  TimEnvHandle.Init.Period = (TIM_CLOCK_ENV  / ALGO_FREQ_ENV) - 1;
  TimEnvHandle.Init.Prescaler = uwPrescalerValue;
  TimEnvHandle.Init.ClockDivision = 0;
  TimEnvHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_Base_Init(&TimEnvHandle) != HAL_OK) {
    /* Initialization Error */
  }

  /* Compute the prescaler value to have TIM1 counter clock equal to 10 KHz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 10000) - 1); 
  
  /* Set TIM1 instance ( Motion ) */
  TimCCHandle.Instance = TIM1;  
  TimCCHandle.Init.Period        = 65535;
  TimCCHandle.Init.Prescaler     = uwPrescalerValue;
  TimCCHandle.Init.ClockDivision = 0;
  TimCCHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
  if(HAL_TIM_OC_Init(&TimCCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
 /* Configure the Output Compare channels */
 /* Common configuration for all channels */
  sConfig.OCMode     = TIM_OCMODE_TOGGLE;
  sConfig.OCPolarity = TIM_OCPOLARITY_LOW;
  
  /* Output Compare Toggle Mode configuration: Channel4 */
  sConfig.Pulse = DEFAULT_uhCCR4_Val;
  if(HAL_TIM_OC_ConfigChannel(&TimCCHandle, &sConfig, TIM_CHANNEL_4) != HAL_OK)
  {
    /* Configuration Error */
    Error_Handler();
  }
  
  /* Compute the prescaler value to have TIM5 counter clock equal to TIM_CLOCK_AUDIO_LEVEL Hz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / TIM_CLOCK_AUDIO_LEVEL) - 1);
  
  /* Set TIM5 instance ( Mic ) */
  TimAudioDataHandle.Instance = TIM5;
  TimAudioDataHandle.Init.Period = (TIM_CLOCK_AUDIO_LEVEL / ALGO_FREQ_AUDIO_LEVEL) - 1;
  TimAudioDataHandle.Init.Prescaler = uwPrescalerValue;
  TimAudioDataHandle.Init.ClockDivision = 0;
  TimAudioDataHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_Base_Init(&TimAudioDataHandle) != HAL_OK) {
    /* Initialization Error */
    Error_Handler();
  }
}




/** @brief Initialize the BlueNRG Stack
 * @param None
 * @retval None
 */
static void Init_BlueNRG_Stack(void)
{
  char BoardName[8];
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  int ret;
  uint8_t data_len_out;
  uint8_t  hwVersion;
  uint16_t fwVersion;
  


  for(int i=0; i<7; i++)
    BoardName[i]= NodeName[i+1];
    
  BoardName[7]= 0;

  /* Initialize the BlueNRG SPI driver */
  hci_init(APP_UserEvtRx, NULL);
  
  /* get the BlueNRG HW and FW versions */
  getBlueNRG2_Version(&hwVersion, &fwVersion);

  aci_hal_read_config_data(0x80, &data_len_out, bdaddr);

  if ((bdaddr[5] & 0xC0) != 0xC0) {
    PREDMNT1_PRINTF("\r\nStatic Random address not well formed.\r\n");
    while(1);
  }
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, data_len_out,
                                  bdaddr); 
  
  ret = aci_gatt_init();    
  if(ret){
     PREDMNT1_PRINTF("\r\nGATT_Init failed\r\n");
     goto fail;
  }

  ret = aci_gap_init(GAP_PERIPHERAL_ROLE | GAP_BROADCASTER_ROLE, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  if(ret != BLE_STATUS_SUCCESS){
     PREDMNT1_PRINTF("\r\nGAP_Init failed\r\n");
     goto fail;
  }



  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
                                   7/*strlen(BoardName)*/, (uint8_t *)BoardName);

  if(ret){
     PREDMNT1_PRINTF("\r\naci_gatt_update_char_value failed\r\n");
    while(1);
  }

  ret = aci_gap_set_authentication_requirement(BONDING,
                                               MITM_PROTECTION_REQUIRED,
                                               SC_IS_SUPPORTED,
                                               KEYPRESS_IS_NOT_SUPPORTED,
                                               7, 
                                               16,
                                               USE_FIXED_PIN_FOR_PAIRING,
                                               123456,
                                               0x00);
  if (ret != BLE_STATUS_SUCCESS) {
     PREDMNT1_PRINTF("\r\nGAP setting Authentication failed\r\n");
     goto fail;
  }



  /* Set output power level: -2,1 dBm */
  aci_hal_set_tx_power_level(1,4);











  PREDMNT1_PRINTF("SERVER: BLE Stack Initialized \r\n"
         "\t\tHWver= %d.%d\r\n"
         "\t\tFWver= %d.%d.%c\r\n"
         "\t\tBoardName= %s\r\n"
         "\t\tBoardMAC = %x:%x:%x:%x:%x:%x\r\n\n",
         ((hwVersion>>4)&0x0F),(hwVersion&0x0F),
         (fwVersion>>8)&0xF,
         (fwVersion>>4)&0xF,
         ('a' + (fwVersion&0xF)),
         BoardName,
         bdaddr[5],bdaddr[4],bdaddr[3],bdaddr[2],bdaddr[1],bdaddr[0]);





  return;

fail:
  return;
}





void APP_UserEvtRx(void *pData)
{
  uint32_t i;

  hci_spi_pckt *hci_pckt = (hci_spi_pckt *)pData;

  if(hci_pckt->type == HCI_EVENT_PKT) {
    hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

    if(event_pckt->evt == EVT_LE_META_EVENT) {
      evt_le_meta_event *evt = (void *)event_pckt->data;

      for (i = 0; i < (sizeof(hci_le_meta_events_table)/sizeof(hci_le_meta_events_table_type)); i++) {
        if (evt->subevent == hci_le_meta_events_table[i].evt_code) {
          hci_le_meta_events_table[i].process((void *)evt->data);
        }
      }
    } else if(event_pckt->evt == EVT_VENDOR) {
      evt_blue_aci *blue_evt = (void*)event_pckt->data;        

      for (i = 0; i < (sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table_type)); i++) {
        if (blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code) {
          hci_vendor_specific_events_table[i].process((void *)blue_evt->data);
        }
      }
    } else {
      for (i = 0; i < (sizeof(hci_events_table)/sizeof(hci_events_table_type)); i++) {
        if (event_pckt->evt == hci_events_table[i].evt_code) {
          hci_events_table[i].process((void *)event_pckt->data);
        }
      }
    }
  }
}

/**
 * @brief  Get hardware and firmware version
 *
 * @param  Hardware version
 * @param  Firmware version
 * @retval Status
 */
uint8_t getBlueNRG2_Version(uint8_t *hwVersion, uint16_t *fwVersion)
{
  uint8_t status;
  uint8_t hci_version, lmp_pal_version;
  uint16_t hci_revision, manufacturer_name, lmp_pal_subversion;
  uint8_t DTM_version_major, DTM_version_minor, DTM_version_patch, DTM_variant, BTLE_Stack_version_major, BTLE_Stack_version_minor, BTLE_Stack_version_patch, BTLE_Stack_development;
  uint16_t DTM_Build_Number, BTLE_Stack_variant, BTLE_Stack_Build_Number;


  status = hci_read_local_version_information(&hci_version, &hci_revision, &lmp_pal_version, 
				                              &manufacturer_name, &lmp_pal_subversion);

  if (status == BLE_STATUS_SUCCESS) {
    *hwVersion = hci_revision >> 8;
  }
  else {
    PREDMNT1_PRINTF("Error= %x \r\n", status);
  }
  
  
  status = aci_hal_get_firmware_details(&DTM_version_major,
                                        &DTM_version_minor,
                                        &DTM_version_patch,
                                        &DTM_variant,
                                        &DTM_Build_Number,
                                        &BTLE_Stack_version_major,
                                        &BTLE_Stack_version_minor,
                                        &BTLE_Stack_version_patch,
                                        &BTLE_Stack_development,
                                        &BTLE_Stack_variant,
                                        &BTLE_Stack_Build_Number);
  
  if (status == BLE_STATUS_SUCCESS) {
    *fwVersion = BTLE_Stack_version_major  << 8;  // Major Version Number
    *fwVersion |= BTLE_Stack_version_minor << 4;  // Minor Version Number
    *fwVersion |= BTLE_Stack_version_patch;       // Patch Version Number
  }
  else {
    PREDMNT1_PRINTF("Error= %x \r\n", status);
  }
  
    
  return status;
}







/** @brief Initialize all the Custom BlueNRG services
  * @param None
  * @retval None
  */
static void Init_BlueNRG_Custom_Services(void)
{
  int ret;
  

  ret = Add_HW_ServW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS)
  {
     PREDMNT1_PRINTF("HW      Service W2ST added successfully\r\n");
  }
  else
  {
     PREDMNT1_PRINTF("\r\nError while adding HW & SW Service W2ST\r\n");
  }

  ret = Add_SW_ServW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS)
  {
     PREDMNT1_PRINTF("SW      Service W2ST added successfully\r\n");
  } else
  {
     PREDMNT1_PRINTF("\r\nError while adding SW Service W2ST\r\n");
  }




  ret = Add_ConsoleW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS)
  {
     PREDMNT1_PRINTF("Console Service W2ST added successfully\r\n");
  }
  else
  {
     PREDMNT1_PRINTF("\r\nError while adding Console Service W2ST\r\n");
  }


  ret = Add_ConfigW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS)
  {
     PREDMNT1_PRINTF("Config  Service W2ST added successfully\r\n");
  }
  else
  {
     PREDMNT1_PRINTF("\r\nError while adding Config Service W2ST\r\n");
  }


  ret = Add_TaiChi_ServW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS)
    {
       PREDMNT1_PRINTF("Config TaiChi Char added successfully\r\n");
    }
    else
    {
       PREDMNT1_PRINTF("\r\nError while adding Config TaiChi Char\n");
    }


}

/** @brief Predictive Maintenance Initialization
  * @param None
  * @retval None
  */
static void InitPredictiveMaintenance(void)
{
  /* Set the vibration parameters with default values */
  MotionSP_SetDefaultVibrationParam();
  
  /* Read Vibration Parameters From Memory */
  ReCallVibrationParamFromMemory();
  
  PREDMNT1_PRINTF("\r\nAccelerometer parameters:\r\n");
  PREDMNT1_PRINTF("AccOdr= %d\t", Accelerometer_Parameters.AccOdr);
  PREDMNT1_PRINTF("FifoOdr= %d\t", Accelerometer_Parameters.FifoOdr);   
  PREDMNT1_PRINTF("fs= %d\t", Accelerometer_Parameters.fs);   
  PREDMNT1_PRINTF("\r\n");

  PREDMNT1_PRINTF("\r\nMotionSP parameters:\r\n");
  PREDMNT1_PRINTF("size= %d\t", MotionSP_Parameters.FftSize); 
  PREDMNT1_PRINTF("wind= %d\t", MotionSP_Parameters.window);  
  PREDMNT1_PRINTF("tacq= %d\t", MotionSP_Parameters.tacq);
  PREDMNT1_PRINTF("ovl= %d\t", MotionSP_Parameters.FftOvl);
  PREDMNT1_PRINTF("subrange_num= %d\t", MotionSP_Parameters.subrange_num);
  PREDMNT1_PRINTF("\r\n\n");
  
  PREDMNT1_PRINTF("************************************************************************\r\n\r\n");
  
  /* Initializes accelerometer with vibration parameters values */
  if(SetAccelerometerParameters()) {
    PREDMNT1_PRINTF("\tOK Set Accelerometer Parameters\r\n\n");
  } else {
    PREDMNT1_PRINTF("\tFailed Set Accelerometer Parameters\r\n\n");
  }
  
  IsFirstTime = 1;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 
  *            HCLK(Hz)                       = 
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 
  *            PLL_M                          = 2
  *            PLL_N                          = 30
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
//   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
//  
//  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /* Configure the main internal regulator output voltage */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 30;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV5; //RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  


  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC
                                      |RCC_PERIPHCLK_DFSDM1
                                      |RCC_PERIPHCLK_USB
                                      |RCC_PERIPHCLK_ADC
                                      |RCC_PERIPHCLK_I2C2
                                      |RCC_PERIPHCLK_SAI1;
  PeriphClkInit.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_PCLK2; //RCC_DFSDM1CLKSOURCE_PCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 5;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 96;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV25;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV4;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV4;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK|RCC_PLLSAI1_SAI1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Configure the Systick interrupt time */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  /* Configure the Systick */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
 * @brief  Check if there are a valid Node Name Values in Memory and read them
 * @param  None
 * @retval unsigned char Success/Not Success
 */
static unsigned char ReCallNodeNameFromMemory(void)
{
  const char DefaultBoardName[7] = {NAME_BLUEMS};
  
  /* ReLoad the Node Name Values from RAM */
  unsigned char Success=0;

  /* Recall the node name Credential saved */
  MDM_ReCallGMD(GMD_NODE_NAME,(void *)&NodeName);
  
  if(NodeName[0] != 0x12)
  {
    NodeName[0]= 0x12;
    
    for(int i=0; i<7; i++)
      NodeName[i+1]= DefaultBoardName[i];
    
    MDM_SaveGMD(GMD_NODE_NAME,(void *)&NodeName);
    NecessityToSaveMetaDataManager=1;
  }

  return Success;
}

/**
 * @brief  Check if there are a valid Vibration Parameters Values in Memory and read them
 * @param pAccelerometer_Parameters Pointer to Accelerometer parameter structure
 * @param pMotionSP_Parameters Pointer to Board parameter structure
 * @retval unsigned char Success/Not Success
 */
static unsigned char ReCallVibrationParamFromMemory(void)
{
  /* ReLoad the Vibration Parameters Values from RAM */
  unsigned char Success=0;
  
  PREDMNT1_PRINTF("Recall the vibration parameter values from FLASH\r\n");

  /* Recall the Vibration Parameters Values saved */
  MDM_ReCallGMD(GMD_VIBRATION_PARAM,(void *)VibrationParam);
  
  if(VibrationParam[0] == CHECK_VIBRATION_PARAM)
  {
    Accelerometer_Parameters.AccOdr=    VibrationParam[1];
    Accelerometer_Parameters.FifoOdr=   VibrationParam[2];
    Accelerometer_Parameters.fs=        VibrationParam[3];
    MotionSP_Parameters.FftSize=        VibrationParam[4];
    MotionSP_Parameters.tau=            VibrationParam[5];
    MotionSP_Parameters.window=         VibrationParam[6];
    MotionSP_Parameters.td_type=        VibrationParam[7];
    MotionSP_Parameters.tacq=           VibrationParam[8];
    MotionSP_Parameters.FftOvl=         VibrationParam[9];
    MotionSP_Parameters.subrange_num=   VibrationParam[10];
    
    PREDMNT1_PRINTF("Vibration parameter values read from FLASH\r\n");
    
    NecessityToSaveMetaDataManager=0;
  }
  else
  {
    PREDMNT1_PRINTF("Vibration parameters values not present in FLASH\r\n");
    SaveVibrationParamToMemory();
  }

  return Success;
}

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_CALLBACK_FUNCTIONS Predictive Maintenance Main CallBack Functions
  * @{
  */

/**
  * @brief  Output Compare callback in non blocking mode 
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t uhCapture=0;

  /* TIM1_CH4 toggling with frequency = 20 Hz */
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
  {
     uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_4, (uhCapture + uhCCR4_Val));
    SendAccGyroMag=1;
  }
}

/**
  * @brief  Period elapsed callback in non blocking mode for Environmental timer
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim == (&TimEnvHandle)) {
    /* Environmental */
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_ENV))
      SendEnv=1;
    
    /* Battery Info */
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_BATTERY_INFO))
      SendBatteryInfo= 1;
    
  } else if(htim == (&TimAudioDataHandle)) {
    /* Mic Data */
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_AUDIO_LEVEL))
      SendAudioLevel=1;
  } else if (htim->Instance == STBC02_USED_TIM) {
    BC_CmdMng();
#ifdef PREDMNT1_ENABLE_PRINTF
    } else if(htim == (&TimHandle)) {
      CDC_TIM_PeriodElapsedCallback(htim);
#endif /* PREDMNT1_ENABLE_PRINTF */
  }
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  htim : hadc handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
  {
    BSP_BC_ChgPinHasToggled();
  }
}

/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  AudioProcess();
}

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  AudioProcess();
}

/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t GPIO_Pin Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{  


  switch(GPIO_Pin){
  case HCI_TL_SPI_EXTI_PIN:
    hci_tl_lowlevel_isr();
    HCI_ProcessEvent=1;
    break;

  case M_INT2_O_PIN:
	  PREDMNT1_PRINTF("M_INT2_0_PIN\r\n");
	  TriggerMotionMLInt = 1;
//    AccIntReceived = 1;
//    if(FifoEnabled)
//      FuncOn_FifoFull();
//    else
     FuncOn_DRDY_XL();
    break;
    
  case USER_BUTTON_PIN:

    ButtonPressed = 1;
    break;
    
  case GPIO_PIN_10:
    if(HAL_GetTick() - t_stwin > 4000)
    {
      BSP_BC_CmdSend(SHIPPING_MODE_ON);
    }
    break;
  }
}

/**
  * @}
  */

/** @defgroup PREDCTIVE_MAINTENANCE_MAIN_EXPORTED_FUNCTIONS Predictive Maintenance Main Exported Functions
  * @{
  */

/**
  * @brief This function provides accurate delay (in milliseconds) based 
  *        on variable incremented.
  * @note This is a user implementation using WFI state
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay(__IO uint32_t Delay)
{
  uint32_t tickstart = 0;
  tickstart = HAL_GetTick();
  while((HAL_GetTick() - tickstart) < Delay){
    __WFI();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1){
  }
}

/**
  *  @brief   Get MCU Identifiers
  *  @details See "MCU device ID code" paragraph into the MCU Reference Manual
  *  @param   pMcuId Pointer to the MCU ID structure to be filled
  *  @return  None
  */
void Get_McuId(sMcuId_t *pMcuId)
{
#define DEV_ID_MASK       (uint32_t)0x00000FFF
#define REV_ID_MASK       (uint32_t)0xFFFF0000
#define DEV_ID_BIT        0
#define REV_ID_BIT        16

#define UNQ_DEV_ID_BASE   0x1FFF7A10U //!< Unique device ID register (96 bits) base address
#define FLASH_SIZE_BASE   0x1FFF7A22U //!< Flash size register base address
#define PACKAGE_DATA_BASE 0x1FFF7BF0U //!< Package data register base address
  
  if (pMcuId != NULL)
  {
    /* Read the device Id code */
    pMcuId->McuDevId = (uint16_t)(((uint32_t)(DBGMCU->IDCODE) & DEV_ID_MASK) >> DEV_ID_BIT);
    pMcuId->McuRevId = (uint16_t)(((uint32_t)(DBGMCU->IDCODE) & REV_ID_MASK) >> REV_ID_BIT);
    
    /* Read the unique device ID registers (96 bits) */
    pMcuId->u_id_31_0 = *((uint32_t *)(UNQ_DEV_ID_BASE+0x00));
    pMcuId->u_id_63_32 = *((uint32_t *)(UNQ_DEV_ID_BASE+0x04));
    pMcuId->u_id_95_64 = *((uint32_t *)(UNQ_DEV_ID_BASE+0x08));
    
    /* Read the size of the device Flash memory expressed in Kbytes */
    pMcuId->FlashSize = *((uint16_t *)(FLASH_SIZE_BASE));

    /* Read the package data */
    pMcuId->Package = (uint8_t)(((*((uint16_t *)(PACKAGE_DATA_BASE)))>>8)&0x07);
  }
}

/**
 * @brief  Save vibration parameters values to memory
 * @param pAccelerometer_Parameters Pointer to Accelerometer parameter structure
 * @param pMotionSP_Parameters Pointer to Board parameter structure
 * @retval unsigned char Success/Not Success
 */
unsigned char SaveVibrationParamToMemory(void)
{
  /* ReLoad the Vibration Parameters Values from RAM */
  unsigned char Success=0;

  VibrationParam[0]= CHECK_VIBRATION_PARAM;
  VibrationParam[1]=  (uint16_t)Accelerometer_Parameters.AccOdr;
  VibrationParam[2]=  (uint16_t)Accelerometer_Parameters.FifoOdr;
  VibrationParam[3]=  (uint16_t)Accelerometer_Parameters.fs;
  VibrationParam[4]=  (uint16_t)MotionSP_Parameters.FftSize;
  VibrationParam[5]=  (uint16_t)MotionSP_Parameters.tau;
  VibrationParam[6]=  (uint16_t)MotionSP_Parameters.window;
  VibrationParam[7]=  (uint16_t)MotionSP_Parameters.td_type;
  VibrationParam[8]=  (uint16_t)MotionSP_Parameters.tacq;
  VibrationParam[9]=  (uint16_t)MotionSP_Parameters.FftOvl;
  VibrationParam[10]= (uint16_t)MotionSP_Parameters.subrange_num;
  
  PREDMNT1_PRINTF("Vibration parameters values will be saved in FLASH\r\n");
  MDM_SaveGMD(GMD_VIBRATION_PARAM,(void *)VibrationParam);
  NecessityToSaveMetaDataManager=1;

  return Success;
}

/**
  * @}
  */

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: PREDMNT1_PRINTF("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1){
  }
}
#endif

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
