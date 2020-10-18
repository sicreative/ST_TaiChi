/**
  ******************************************************************************
  * @file    MotionSP_Manager.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   This file includes 
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
#include <stdbool.h>

#include "MotionSP_Manager.h"
#include "sensor_service.h"
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

/** @addtogroup PREDCTIVE_MAINTENANCE_MOTIONSP_MANAGER Predictive Maintenance Motion Signal Processing Manager
  * @{
  */

/* Exported Variables --------------------------------------------------------*/
uint8_t RestartFlag = 1;
sAccelerometer_Parameter_t Accelerometer_Parameters;

uint32_t Start_Tick = 0;

/* X-Y-Z STATUS ALARM for TimeDomain */
sTimeDomainAlarm_t sTdAlarm;
/* X-Y-Z ALARM Threshold for Speed RMS TimeDomain */
sTimeDomainThresh_t sTdRmsThresholds;
/* X-Y-Z ALARM Threshold for Acc Peak TimeDomain */
sTimeDomainThresh_t sTdPkThresholds;
/* X-Y-Z parameter measured for TimeDomain */
sAcceleroParam_t sTimeDomainVal;

/* X-Y-Z Frequency domain Subranges Status */
sFreqDomainAlarm_t THR_Fft_Alarms;

/* Imported Variables --------------------------------------------------------*/
extern volatile uint8_t AccIntReceived;
extern volatile uint8_t FifoEnabled;
extern sAxesMagBuff_t AccAxesAvgMagBuff;

extern float *FDWarnThresh;
extern float *FDAlarmThresh;

/* Private variables ---------------------------------------------------------*/
/* Int_Current_Time1 Value */
volatile uint32_t Int_Current_Time1 = 0;
/* Int_Current_Time2 Value */
volatile uint32_t Int_Current_Time2 = 0;

volatile uint32_t FFT_Amplitude= 0;
volatile uint32_t FFT_Alarm=    0;

/* X-Y-Z Amplitude subranges Values that exceed thresholds */
sSubrange_t THR_Check;

Alarm_Type_t TotalTDAlarm;
Alarm_Type_t TotalFDAlarm;

static uint8_t Accelero_Drdy = 0;
static float MotionSP_Sensitivity;
static bool IsAcceleroFifoToRead = false;

static uint8_t SendingFFT= 0;
static uint8_t MemoryIsAlloc= 0;
static uint16_t CountSendData= 0;

uint8_t *TotalBuffToSending;

/* Private function prototypes -----------------------------------------------*/
static uint8_t MotionSP_AccMeasInit(void);

static uint8_t AccOdrMeas(sAcceleroODR_t *pAcceleroODR);

static void AcceleroFifoRead(MOTION_SENSOR_AxesRaw_t *pSensorAxesRaw);
static void FillCircBuffFromFifo(sCircBuffer_t *pAccCircBuff, float AccSensitivity);

static void PrepareTotalBuffToSending(sAxesMagBuff_t *ArrayToSend, uint16_t ActualMagSize);

static void MotionSP_TimeDomainAlarm (sTimeDomainAlarm_t *pTdAlarm,
                                      sAcceleroParam_t *pTimeDomainVal,
                                      sTimeDomainThresh_t *pTdRmsThreshold,
                                      sTimeDomainThresh_t *pTdPkThreshold,
                                      sAcceleroParam_t *pTimeDomain);

static void MotionSP_FreqDomainAlarm (sSubrange_t *pSRAmplitude,
                                      float *pFDWarnThresh,
                                      float *pFDAlarmThresh,
                                      uint8_t subrange_num, 
                                      sSubrange_t *pTHR_Check, 
                                      sFreqDomainAlarm_t *pTHR_Fft_Alarms);

/* Private defines -----------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/

/**
  * @brief  Init the MotionSP Vibration parameters
  * @param  None
  * @return None
  */   
void MotionSP_SetDefaultVibrationParam(void)
{ 
  PREDMNT1_PRINTF("************************************************************************\r\n");
  PREDMNT1_PRINTF("      Vibration parameter values\r\n");
  
  PREDMNT1_PRINTF("\r\nSet default vibration parameters value\r\n");

  /* Set default parameters for accelerometer */
  Accelerometer_Parameters.hw_hpf_cut=  SENSOR_HPF_CUT_VALUE;
  Accelerometer_Parameters.AccOdr=      SENSOR_ACC_ORD_VALUE;
  Accelerometer_Parameters.FifoOdr=     SENSOR_ACC_FIFO_ORD_VALUE;
  Accelerometer_Parameters.fs=          SENSOR_ACC_FS_DEFAULT;

  /* Set default parameters for MotionSP library */
  MotionSP_Parameters.FftSize=          FFT_SIZE_DEFAULT;
  MotionSP_Parameters.tau=              TAU_DEFAULT;
  MotionSP_Parameters.window=           WINDOW_DEFAULT;
  MotionSP_Parameters.td_type=          TD_DEFAULT;
  MotionSP_Parameters.tacq=             TACQ_DEFAULT;
  MotionSP_Parameters.subrange_num=     SUBRANGE_DEFAULT;
  MotionSP_Parameters.FftOvl=           FFT_OVL_DEFAULT;
}

/**
  * @brief  Init the MotionSP Vibration parameters
  * @param  None
  * @return None
  */ 
void MotionSP_VibrationInit(void)
{
  PREDMNT1_PRINTF("\r\nMotionSP Vibration Init");
  
  if ((MotionSP_Parameters.FftSize) <= FFT_SIZE_MAX)
  {
    // Reset the accelero circular buffer
    memset((void *)(&AccCircBuffer), 0x00, sizeof(sCircBuffer_t));
    
    // Set the initial position of the accelero circular buffer
    AccCircBuffer.IdPos = (uint16_t)(-1);

    /* Create circular buffer and initialize result variables */
    //AccCircBuffer.Size = (uint16_t)((float)(MotionSP_Parameters.FftSize*(((float)CIRC_BUFFER_RATIO)/10)));
    AccCircBuffer.Size = (uint16_t)((MotionSP_Parameters.FftSize * CIRC_BUFFER_RATIO_NUM)/CIRC_BUFFER_RATIO_DEN);
    
    magSize = MotionSP_Parameters.FftSize / 2;
    
    /* Set the mag size to be used */
    AccMagResults.MagSizeTBU = magSize; 

    // Reset the TimeDomain parameter values
    memset((void *)(&sTimeDomain), 0x00, sizeof(sAcceleroParam_t));

    // Reset the counters of the number of sums about the calculation of the average
    memset((void *)(&AccSumCnt), 0x00, sizeof(sSumCnt_t));  

    MotionSP_SetWindFiltArray(Filter_Params, MotionSP_Parameters.FftSize, (Filt_Type_t)MotionSP_Parameters.window);

    /* Reset the flag to enable FFT computation */
    fftIsEnabled = 0;

    arm_rfft_fast_init_f32(&fftS, MotionSP_Parameters.FftSize);

    /* It is the minimum value to do the first FFT */
    accCircBuffIndexForFft = MotionSP_Parameters.FftSize - 1;
    
    if(MemoryIsAlloc)
    {
      free(TotalBuffToSending);
      MemoryIsAlloc= 0;
    }
 
    FinishAvgFlag = 0;
    RestartFlag = 1;
    SendingFFT= 0;
    
    PREDMNT1_PRINTF("\t--> OK\r\n");
  }
  else
  {
    /* Send fault info to terminal */
    PREDMNT1_PRINTF("\t--> FFT size is out of range.\r\n\r\n");
  }  
}

/**
  *  @brief Initialization of Alarm Status on Axes, Alarm Values Reported
  *         and Thresholds to detect WARNING and ALARM conditions
  *  @param pTdAlarm: Pointer to TimeDomain Alarms Result
  *  @param pTimeDomainVal: Pointer to TimeDomain Value Result
  *  @param pTdRmsThreshold: Pointer to TimeDomain RMS Threshlods to initialize
  *  @param pTdPkThreshold:  Pointer to TimeDomain PK Threshlods to initialize
  *  @return Return description
  */
void MotionSP_TimeDomainAlarmInit (sTimeDomainAlarm_t *pTdAlarm,
                                   sAcceleroParam_t *pTimeDomainVal,
                                   sTimeDomainThresh_t *pTdRmsThreshold,
                                   sTimeDomainThresh_t *pTdPkThreshold) 
{
  
  PREDMNT1_PRINTF("MotionSP Time Domain Alarm Init\r\n");
  
  pTdAlarm->RMS_STATUS_AXIS_X = GOOD;  
  pTdAlarm->RMS_STATUS_AXIS_Y = GOOD;  
  pTdAlarm->RMS_STATUS_AXIS_Z = GOOD;  
  pTdAlarm->PK_STATUS_AXIS_X = GOOD;  
  pTdAlarm->PK_STATUS_AXIS_Y = GOOD;  
  pTdAlarm->PK_STATUS_AXIS_Z = GOOD;   
  pTimeDomainVal->SpeedRms.AXIS_X = 0.0f;
  pTimeDomainVal->SpeedRms.AXIS_Y = 0.0f;
  pTimeDomainVal->SpeedRms.AXIS_Z = 0.0f;
  pTimeDomainVal->AccPeak.AXIS_X = 0.0f;
  pTimeDomainVal->AccPeak.AXIS_Y = 0.0f;
  pTimeDomainVal->AccPeak.AXIS_Z = 0.0f;
  pTdRmsThreshold->THR_WARN_AXIS_X = TDSpeedRMSThresh.THR_WARN_AXIS_X;     //0.2f;
  pTdRmsThreshold->THR_WARN_AXIS_Y = TDSpeedRMSThresh.THR_WARN_AXIS_Y;     //0.1f;
  pTdRmsThreshold->THR_WARN_AXIS_Z = TDSpeedRMSThresh.THR_WARN_AXIS_Z;     //1.5f;
  pTdRmsThreshold->THR_ALARM_AXIS_X = TDSpeedRMSThresh.THR_ALARM_AXIS_X;   //0.3f;
  pTdRmsThreshold->THR_ALARM_AXIS_Y = TDSpeedRMSThresh.THR_ALARM_AXIS_Y;   //0.2f;
  pTdRmsThreshold->THR_ALARM_AXIS_Z = TDSpeedRMSThresh.THR_ALARM_AXIS_Z;   //2.0f;
  pTdPkThreshold->THR_WARN_AXIS_X = TDAccPeakThresh.THR_WARN_AXIS_X;     //0.2f;
  pTdPkThreshold->THR_WARN_AXIS_Y = TDAccPeakThresh.THR_WARN_AXIS_Y;     //0.1f;
  pTdPkThreshold->THR_WARN_AXIS_Z = TDAccPeakThresh.THR_WARN_AXIS_Z;     //1.5f;
  pTdPkThreshold->THR_ALARM_AXIS_X = TDAccPeakThresh.THR_ALARM_AXIS_X;   //0.3f;
  pTdPkThreshold->THR_ALARM_AXIS_Y = TDAccPeakThresh.THR_ALARM_AXIS_Y;   //0.2f;
  pTdPkThreshold->THR_ALARM_AXIS_Z = TDAccPeakThresh.THR_ALARM_AXIS_Z;   //2.0f;
}

/**
  *  @brief Frequency domain initialization of Alarm Status
  *  @param pWarnThresh: Pointer to TimeDomain Warnings thresholds to use
  *  @param pAlarmThresh: Pointer to TimeDomain Alarms thresholds to use
  *  @param pTHR_Fft_Alarms: Pointer to Freq Domain Value Arrays Result
  *  @param subrange_num:  Subranges numbers
  *  @return Return description
  *  
  */
void MotionSP_FreqDomainAlarmInit (float **pWarnThresh,
                                   float **pAlarmThresh,
                                   sFreqDomainAlarm_t *pTHR_Fft_Alarms,
                                   uint8_t subrange_num) 
{
  PREDMNT1_PRINTF("MotionSP Frequency Domain Alarm Init\r\n");
  
  /* Reset status value for FFT alarms */
  memset(pTHR_Fft_Alarms, GOOD, 3*64);
  
  switch (subrange_num){
  case 8:
    *pWarnThresh = (float *)FDWarnThresh_Sub8;
    *pAlarmThresh = (float *)FDAlarmThresh_Sub8;
    break;
    
  case 16:
    *pWarnThresh = (float *)FDWarnThresh_Sub16;
    *pAlarmThresh = (float *)FDAlarmThresh_Sub16;
    break; 
    
  case 32:
    *pWarnThresh = (float *)FDWarnThresh_Sub32;
    *pAlarmThresh = (float *)FDAlarmThresh_Sub32;
    break; 
    
  case 64:
    *pWarnThresh = (float *)FDWarnThresh_Sub64;
    *pAlarmThresh = (float *)FDAlarmThresh_Sub64;
    break; 
  }
}

/**
  * @brief  Set accelerometer parameters for MotionSP Vibration
  * @param  None
  * @retval 1 in case of success
  * @retval 0 in case of failure
  */   
uint8_t SetAccelerometerParameters(void)
{
  int8_t BSP_Error= 0;
  uint8_t ret= 1;
  
  PREDMNT1_PRINTF("Set Accelerometer Parameters:\r\n");
 
  /* Reset the real accelero ODR value */
  memset((void *)(&AcceleroODR), 0x00, sizeof(sAcceleroODR_t));
  
 /* Set FS value */
  if ((BSP_Error = MOTION_SENSOR_SetFullScale(ACCELERO_INSTANCE, 
                                              MOTION_ACCELERO,
                                              Accelerometer_Parameters.fs)) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError on FullScale Setting(BSP_ERROR = %d)\r\n", BSP_Error);
    ret= 0;
  }
  else
  {
    /* Get Sensitivity */
    MOTION_SENSOR_GetSensitivity(ACCELERO_INSTANCE,
                                 MOTION_ACCELERO,
                                 &MotionSP_Sensitivity );
    PREDMNT1_PRINTF("\tOk FullScale Setting\r\n");
  }

  /* Set ODR value */
  if ((BSP_Error = MOTION_SENSOR_SetOutputDataRate(ACCELERO_INSTANCE, 
                                                   MOTION_ACCELERO,
                                                   Accelerometer_Parameters.AccOdr)) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set Output Data Rate (BSP_ERROR = %d)\r\n", BSP_Error);
    ret= 0;
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set Output Data Rate\r\n");
  }  

  /* Turn-on time delay */
  HAL_Delay(40);
  
    /* Measure and calculate ODR */
  if (MotionSP_AccMeasInit() == 1)
  {
    PREDMNT1_PRINTF("\tError measure and calculate ODR - Used parameter value (");
    AcceleroODR.Frequency= Accelerometer_Parameters.AccOdr;
    ret= 0;
  }
  else
  {
    PREDMNT1_PRINTF("\tOk measure and calculate ODR (");
  }
  
#ifdef PREDMNT1_ENABLE_PRINTF
  uint32_t IntPart, DecPart;
  MCR_BLUEMS_F2I_2D(AcceleroODR.Frequency, IntPart, DecPart);
#endif /* PREDMNT1_ENABLE_PRINTF */

  /* Send the parameters to terminal */
  PREDMNT1_PRINTF(" %ld.%.2ld Hz )\r\n", IntPart, DecPart);

  return ret;
}

/**
  * @brief  Configure the FIFO in according with the algorithm setting
  * @param  None
  * @retval None
  */
void MotionSP_ConfigFifo(void)
{
  int8_t BSP_Error= 0;
  
  PREDMNT1_PRINTF("\r\nFIFO config:\r\n");

  Accelerometer_Parameters.AccFifoSize=(uint16_t)(((float)MotionSP_Parameters.FftSize*(1.0f-((float)MotionSP_Parameters.FftOvl/100.0f)))*0.75f);

  /* Set FIFO ODR value */
  if ((BSP_Error = MOTION_SENSOR_FIFO_Set_BDR(ACCELERO_INSTANCE,
                                              MOTION_ACCELERO,
                                              Accelerometer_Parameters.FifoOdr)) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set FIFO ODR Value (BSP_ERROR = %d)\r\n", BSP_Error);
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set FIFO ODR Value\r\n");
  } 
  
  /* Set FIFO watermark */
  if ((BSP_Error = MOTION_SENSOR_FIFO_Set_Watermark_Level(ACCELERO_INSTANCE, Accelerometer_Parameters.AccFifoSize)) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set FIFO Threshold Level (BSP_ERROR = %d)\r\n", BSP_Error);
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set FIFO Threshold Level\r\n");
  } 

  /* Set FIFO depth to be limited to watermark threshold level  */
  if ((BSP_Error = MOTION_SENSOR_FIFO_Set_Stop_On_Fth(ACCELERO_INSTANCE, ENABLE)) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set FIFO Stop On Fth (BSP_ERROR = %d)\r\n", BSP_Error);
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set FIFO Stop On Fth\r\n");
  } 

   
  if ( (uint16_t)((Accelerometer_Parameters.AccFifoSize*3*2) > (1024*4)) )
  {
    PREDMNT1_PRINTF("\n FIFO size will be %i byte than exceed its maximum value, 4 kbyte.\n",
                    (uint16_t)(Accelerometer_Parameters.AccFifoSize*3*2));
    PREDMNT1_PRINTF(" Please, reduce the SIZE_DEFAULT or the OVL_DEFAULT. \n");
    Error_Handler();
  }
  
    /* Set FIFO to STREAM Mode (Continuous) */
  if (MOTION_SENSOR_FIFO_Set_Mode(ACCELERO_INSTANCE, ACCELERO_STREAM_MODE ) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set FIFO in Continuous Mode (BSP_ERROR = %d)\r\n", BSP_Error);
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set FIFO in Continuous Mode\r\n");
  }

    /* Disable FIFO full flag interrupt */
  if (MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full(ACCELERO_INSTANCE, ENABLE) != BSP_ERROR_NONE)
  {
    PREDMNT1_PRINTF("\tError Set FIFO FULL INTP on INT2 path (BSP_ERROR = %d)\r\n", BSP_Error);
  }
  else
  {
    PREDMNT1_PRINTF("\tOk Set FIFO FULL INTP on INT2 path\r\n");
    /* Start of the Time Window */
    Start_Tick = HAL_GetTick();
  }
}

/**
  * @brief  Vibration Analysis Data processing
  * @param  None
  * @retval None
  */
void MotionSP_VibrationAnalysis(void)
{
#define FFTSIZEDELTA  (MotionSP_Parameters.FftSize*((100.0-MotionSP_Parameters.FftOvl)/100.0))
  static uint8_t accCircBuffIndexForFftOvf = 0;
  static uint16_t accCircBuffIndexPre = (uint16_t)(-1);;
  uint32_t ActualTick;
  uint16_t accCircBuffIndexTmp = 0;
  uint16_t accCircBuffIndexForFftTmp = 0;
  
  static uint8_t Reset= 0;

  if(!SendingFFT)
  {
    if ( (AccCircBuffer.IdPos != accCircBuffIndexPre) && (AccCircBuffer.IdPos != (uint16_t)(-1)) )
    {
      accCircBuffIndexPre = AccCircBuffer.IdPos;
      
      accCircBuffIndexTmp = AccCircBuffer.IdPos + (AccCircBuffer.Ovf * AccCircBuffer.Size);
      accCircBuffIndexForFftTmp = accCircBuffIndexForFft + (accCircBuffIndexForFftOvf * AccCircBuffer.Size);
      
      if (accCircBuffIndexTmp >= accCircBuffIndexForFftTmp)
      {
        /* Check the Tick value */
        ActualTick = HAL_GetTick(); 
        if ((ActualTick - Start_Tick) > MotionSP_Parameters.tacq)
        {
          FinishAvgFlag = 1;
        }
        
        MotionSP_FrequencyDomainProcess();
        
        /* Status check during Time domain Analysis */
        MotionSP_TimeDomainAlarm(&sTdAlarm,&sTimeDomainVal,
                                 &sTdRmsThresholds,
                                 &sTdPkThresholds,
                                 &sTimeDomain);

        accCircBuffIndexForFftOvf = 0;
        accCircBuffIndexForFft += FFTSIZEDELTA;
        if (accCircBuffIndexForFft >= AccCircBuffer.Size)
        {
          accCircBuffIndexForFft -= AccCircBuffer.Size;
          
          if (!AccCircBuffer.Ovf)
            accCircBuffIndexForFftOvf = 1;
        }
        
        AccCircBuffer.Ovf = 0;
      }
    }
    
    /* Send data to ST BLE Sensor app*/
    if (FinishAvgFlag == 1)
    {
      disable_FIFO();
      
      if(FFT_Amplitude)
      {
        PrepareTotalBuffToSending(&AccAxesAvgMagBuff, magSize);
        
        /* Send Time Domain to ST BLE Sensor app */
        PREDMNT1_PRINTF("Sending Time Domain to ST BLE Sensor app\r\n");
        TimeDomain_Update(&sTimeDomain);
        
        /* Send Accelerometer ARRAYs FFT average values to ST BLE Sensor app */
        PREDMNT1_PRINTF("Sending FFT Amplitude to ST BLE Sensor app\r\n");
        SendingFFT= 1;
        CountSendData= 0;
        FFT_Amplitude_Update(TotalBuffToSending, magSize, &SendingFFT, &CountSendData);
      }
      
      if(FFT_Alarm)
      {
        //SendVibrationResult();

        /* Compare the Frequency domain subrange comparison with external Threshold Arrays */
        MotionSP_FreqDomainAlarm (&SRAmplitude, FDWarnThresh, FDAlarmThresh,
                                                          MotionSP_Parameters.subrange_num,
                                                          &THR_Check, 
                                                          &THR_Fft_Alarms);
        
        /* Send Speed RMS value and status */
        PREDMNT1_PRINTF("Sending Speed RMS value and status to ST BLE Sensor app\r\n");
        FFT_AlarmSpeedRMS_Status_Update(&sTdAlarm, &sTimeDomainVal);
        
        /* Send Acc Peak value and status */
        PREDMNT1_PRINTF("Sending Acc Peak value and status to ST BLE Sensor app\r\n");
        FFT_AlarmAccStatus_Update(&sTdAlarm, &sTimeDomainVal);
        
        /* Send the frequency domain threshold status for max Subrange value */
        PREDMNT1_PRINTF("Sending the frequency domain threshold status for max Subrange value to ST BLE Sensor app\r\n");
        FFT_AlarmSubrangeStatus_Update(&AccAxesMagResults,&THR_Fft_Alarms,MotionSP_Parameters.subrange_num,magSize);
        
        Reset= 1;
      }
    }
  }
  else
  {
    /* Send Accelerometer ARRAYs FFT average values to ST BLE Sensor app */
    FFT_Amplitude_Update(TotalBuffToSending, magSize, &SendingFFT, &CountSendData);
    
    if(!SendingFFT)
      Reset= 1;
  }
    
  if(Reset)
  {
    /* Re-Initializes the vibration parameters */
    MotionSP_VibrationInit();
    
    if(FFT_Alarm)
    {
      /* Initialization of Alarm Status on Axes, Alarm Values Reported
         and Thresholds to detect WARNING and ALARM conditions */
      MotionSP_TimeDomainAlarmInit(&sTdAlarm, &sTimeDomainVal, &sTdRmsThresholds, &sTdPkThresholds);
      
      /* Frequency domain initialization of Alarm Status */
      MotionSP_FreqDomainAlarmInit(&FDWarnThresh, &FDAlarmThresh, &THR_Fft_Alarms, MotionSP_Parameters.subrange_num);
    }
    
    enable_FIFO();
    
    Start_Tick = HAL_GetTick();
    
    Reset= 0;
  }
}

/**
  * @brief  Enable FIFO measuring
  * @param  None
  * @retval 1 in case of success
  * @retval 0 in case of failure
  */
uint8_t enable_FIFO(void)
{  
  /* Set FIFO to STREAM Mode (Continuous) */
  if (MOTION_SENSOR_FIFO_Set_Mode(ACCELERO_INSTANCE, ACCELERO_STREAM_MODE) != BSP_ERROR_NONE)
  {
    return 0;
  }
  
  /* Enable FIFO full flag interrupt */
  if (MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full(ACCELERO_INSTANCE, ENABLE) != BSP_ERROR_NONE)
  {
    return 0;
  }

  FifoEnabled = 1;
  PREDMNT1_PRINTF("Enable FIFO\r\n");
  
  return 1;
}

/**
  * @brief  Disable FIFO measuring
  * @param  None
  * @retval 1 in case of success
  * @retval 0 in case of failure
  */
uint8_t disable_FIFO(void)
{
  /* Set FIFO to bypass mode */
  if (MOTION_SENSOR_FIFO_Set_Mode(ACCELERO_INSTANCE, ACCELERO_BYPASS_MODE) != BSP_ERROR_NONE)
  {
    return 0;
  }
  
  /* Disable FIFO full flag interrupt */
  if (MOTION_SENSOR_FIFO_Set_INT2_FIFO_Full(ACCELERO_INSTANCE, DISABLE) != BSP_ERROR_NONE)
  {
    return 0;
  }  

  FifoEnabled = 0;
  
  PREDMNT1_PRINTF("Disable FIFO\r\n");
  
  return 1;
}

/**
  * @brief  Restart FIFO
  * @param  None
  * @retval 1 in case of success
  * @retval 0 in case of failure
  */
uint8_t restart_FIFO(void)
{
  AccIntReceived = 0;

  /* FIFO bypass */
  if (MOTION_SENSOR_FIFO_Set_Mode(ACCELERO_INSTANCE, ACCELERO_BYPASS_MODE) != BSP_ERROR_NONE)
  {
    return 0;
  }
  /* FIFO Mode*/
  if (MOTION_SENSOR_FIFO_Set_Mode(ACCELERO_INSTANCE, ACCELERO_STREAM_MODE) != BSP_ERROR_NONE)
  {
    return 0;
  }

  return 1;
}

/**
  * @brief  FuncOn DRDY XL
  * @param  None
  * @retval None
  */
void FuncOn_DRDY_XL(void)
{
  Accelero_Drdy = 1;
}

/**
  * @brief  FuncOn Fifo Full
  * @param  None
  * @retval None
  */
void FuncOn_FifoFull(void)
{
  LedOnTargetPlatform();
  
  IsAcceleroFifoToRead = true;
  
  while(IsAcceleroFifoToRead)
  {
    if (EXTI->PR1 & M_INT2_O_PIN)
      while(1);
    
    FillCircBuffFromFifo(&AccCircBuffer, MotionSP_Sensitivity);
    
    /* Time Domain Processing */
    MotionSP_TimeDomainProcess(&sTimeDomain, (Td_Type_t)MotionSP_Parameters.td_type, RestartFlag);

    
    /* Clear the restart flag */
    if (RestartFlag)
      RestartFlag = 0;
  }
}

/**
  * @brief  Enable/Disable the accelerometer HP Filter
  * @param  Cutoff frequency
  * @retval 1 in case of success
  * @retval 0 in case of failure
  */
uint8_t EnableDisable_ACC_HP_Filter(uint8_t FilterIsEnabled)
{
  int8_t BSP_Error= 0;
  uint8_t ret= 1;
  
  if(FilterIsEnabled)
  {
    if ((BSP_Error = MOTION_SENSOR_Enable_HP_Filter(ACCELERO_INSTANCE, Accelerometer_Parameters.hw_hpf_cut)) != BSP_ERROR_NONE)
    {
      PREDMNT1_PRINTF("\r\nError Enable/Disable HP Filter (BSP_ERROR = %d)", BSP_Error);
      ret= 0;
    }
    else
    {
      PREDMNT1_PRINTF("\r\nEnable HP Filter --> OK");
    }
  }
  else
  {
    if( (BSP_Error = MOTION_SENSOR_Write_Register(ACCELERO_INSTANCE, ISM330DHCX_CTRL8_XL, 0x00)) != BSP_ERROR_NONE)
    {
      PREDMNT1_PRINTF("\r\nError Disable HP Filter (BSP_ERROR = %d)", BSP_Error);
      ret= 0;
    }
    else
    {
      PREDMNT1_PRINTF("\r\nDisable HP Filter --> OK");
    }
  }
  
  return ret;
}

/* Private function ----------------------------------------------------------*/

/**
  * @brief  Prepare buffer to send data to Bluetooth
  * @param  sAxesMagBuff_t *ArrayToSend X, Y and Z FFT Amplitude values
  * @param  uint16_t ActualMagSize Number of samples for each component
  * @retval None
  */
static void PrepareTotalBuffToSending(sAxesMagBuff_t *ArrayToSend, uint16_t ActualMagSize)
{
  uint32_t BuffPos;
  
  float BinFreqStep;
  
  uint16_t TotalSize;
  
  float TempFloat;
  uint8_t *TempBuff = (uint8_t *)&TempFloat;
  
  TotalSize= 2 /* nSample */ + 1 /* nComponents */ + 4 /*  Frequency Steps */ + ((3 * ActualMagSize) * 4) /* Samples */;
  TotalBuffToSending=(uint8_t *)malloc(TotalSize);
  
  if(TotalBuffToSending == NULL)
  {
    Error_Handler();
  }
  
  MemoryIsAlloc= 1;

  BinFreqStep= (AcceleroODR.Frequency / 2) / ActualMagSize;
    
  STORE_LE_16(TotalBuffToSending, ActualMagSize);
  TotalBuffToSending[2]= 3;
  BuffPos= 3;
  
  TempFloat = BinFreqStep;
  TotalBuffToSending[BuffPos]= TempBuff[0];
  BuffPos++;
  TotalBuffToSending[BuffPos]= TempBuff[1];
  BuffPos++;
  TotalBuffToSending[BuffPos]= TempBuff[2];
  BuffPos++;
  TotalBuffToSending[BuffPos]= TempBuff[3];
  BuffPos++;
  
  for(int i=0; i<ActualMagSize; i++)
  {
    TempFloat = ArrayToSend->AXIS_X[i];
    TotalBuffToSending[BuffPos]= TempBuff[0];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[1];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[2];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[3];
    BuffPos++; 
  }
  
  for(int i=0; i<ActualMagSize; i++)
  {
    TempFloat = ArrayToSend->AXIS_Y[i];
    TotalBuffToSending[BuffPos]= TempBuff[0];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[1];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[2];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[3];
    BuffPos++;  
  }
  
  for(int i=0; i<ActualMagSize; i++)
  {
    TempFloat = ArrayToSend->AXIS_Z[i];
    TotalBuffToSending[BuffPos]= TempBuff[0];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[1];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[2];
    BuffPos++;
    TotalBuffToSending[BuffPos]= TempBuff[3];
    BuffPos++;
  }
}

/**
  * @brief 	Measurement of the accelerometer output data rate
  * @param 	pAcceleroODR Pointer to be fill with the new value
  * @return 0 in case of success
  * @return 1 in case of failure
  */
static uint8_t AccOdrMeas(sAcceleroODR_t *pAcceleroODR)
{
#define ODRMEASURINGTIME  200   // in ms
#define TOLERANCE         0.10   // from 0.0 to 1.0
#define INTCNTMIN         (uint16_t)((((Accelerometer_Parameters.AccOdr)*(1.0-TOLERANCE))*ODRMEASURINGTIME)/1000)
#define INTCNTMAX         (uint16_t)((((Accelerometer_Parameters.AccOdr)*(1.0+TOLERANCE))*ODRMEASURINGTIME)/1000)
  
  uint8_t retValue = 1;
  uint32_t tkStart;
  uint16_t IntCnt;
  
  /* The data-ready pulses are 75 µs long */
  MOTION_SENSOR_DRDY_Set_Mode(ACCELERO_INSTANCE,ISM330DHCX_DRDY_PULSED);
  
  /* ISM330DLC INT2_DRDY_XL interrupt enable */
  MOTION_SENSOR_Set_INT2_DRDY(ACCELERO_INSTANCE, ENABLE);

  Accelero_Drdy = 0;
  IntCnt = 0;
  tkStart = BSP_GetTick();  
  
  do
  {
    if (Accelero_Drdy)
    {
      Accelero_Drdy = 0;
      IntCnt++;
    }
  } while ( (BSP_GetTick() - tkStart) < ODRMEASURINGTIME);
  
  /* ISM330DLC INT2_DRDY_XL interrupt disable */
  MOTION_SENSOR_Set_INT2_DRDY(ACCELERO_INSTANCE, DISABLE);
  
  if ((IntCnt < INTCNTMIN) || (IntCnt > INTCNTMAX))
  {
    /* ODR is out of range */
    retValue = 1;
  }
  else
  {
    /* ODR is valid */
    /* Calculate measured ODR and Exponential parameters*/
    pAcceleroODR->Frequency = (IntCnt * 1000) / ODRMEASURINGTIME;
    pAcceleroODR->Period = 1/(pAcceleroODR->Frequency);
    pAcceleroODR->Tau= exp(-(float)(1000*pAcceleroODR->Period)/MotionSP_Parameters.tau);
    retValue = 0;
  }
  
  return retValue;
}

/**
  *  @brief  Time Domain Alarm Analysis based just on Speed RMS FAST Moving Average
  *  @param  pTdAlarm: Pointer to TimeDomain Alarms Result
  *  @param  pTimeDomainVal: Pointer to TimeDomain Value Result
  *  @param  pTdRmsThreshold:  Pointer to TimeDomain RMS Threshlods Configured
  *  @param  pTdPkThreshold:  Pointer to TimeDomain PK Threshlods Configured
  *  @param  pTimeDomain:   Pointer to TimeDomain Parameters to monitor
  *  @return None
  */
static void MotionSP_TimeDomainAlarm (sTimeDomainAlarm_t *pTdAlarm,
                                      sAcceleroParam_t *pTimeDomainVal,
                                      sTimeDomainThresh_t *pTdRmsThreshold,
                                      sTimeDomainThresh_t *pTdPkThreshold,
                                      sAcceleroParam_t *pTimeDomain) 
{
  pTimeDomainVal->SpeedRms.AXIS_X = pTimeDomain->SpeedRms.AXIS_X*1000;
  pTimeDomainVal->SpeedRms.AXIS_Y = pTimeDomain->SpeedRms.AXIS_Y*1000;
  pTimeDomainVal->SpeedRms.AXIS_Z = pTimeDomain->SpeedRms.AXIS_Z*1000;
  
  /* Speed RMS comparison with thresholds */      
  if ((pTimeDomain->SpeedRms.AXIS_X*1000) > pTdRmsThreshold->THR_WARN_AXIS_X)
  {
        pTdAlarm->RMS_STATUS_AXIS_X = WARNING;
        pTimeDomainVal->SpeedRms.AXIS_X = pTimeDomain->SpeedRms.AXIS_X*1000;
  }
  if ((pTimeDomain->SpeedRms.AXIS_Y*1000) > pTdRmsThreshold->THR_WARN_AXIS_Y)
  {
        pTdAlarm->RMS_STATUS_AXIS_Y = WARNING;
        pTimeDomainVal->SpeedRms.AXIS_Y = pTimeDomain->SpeedRms.AXIS_Y*1000;
  }
  if ((pTimeDomain->SpeedRms.AXIS_Z*1000) > pTdRmsThreshold->THR_WARN_AXIS_Z)
  {
        pTdAlarm->RMS_STATUS_AXIS_Z = WARNING;
        pTimeDomainVal->SpeedRms.AXIS_Z = pTimeDomain->SpeedRms.AXIS_Z*1000;
  }
  if ((pTimeDomain->SpeedRms.AXIS_X*1000) > pTdRmsThreshold->THR_ALARM_AXIS_X)
  {
        pTdAlarm->RMS_STATUS_AXIS_X = ALARM;
        pTimeDomainVal->SpeedRms.AXIS_X = pTimeDomain->SpeedRms.AXIS_X*1000;
  }
  if ((pTimeDomain->SpeedRms.AXIS_Y*1000) > pTdRmsThreshold->THR_ALARM_AXIS_Y)
  {
        pTdAlarm->RMS_STATUS_AXIS_Y = ALARM;
        pTimeDomainVal->SpeedRms.AXIS_Y = pTimeDomain->SpeedRms.AXIS_Y*1000;
  }
  if ((pTimeDomain->SpeedRms.AXIS_Z*1000) > pTdRmsThreshold->THR_ALARM_AXIS_Z)
  {
        pTdAlarm->RMS_STATUS_AXIS_Z = ALARM;
        pTimeDomainVal->SpeedRms.AXIS_Z = pTimeDomain->SpeedRms.AXIS_Z*1000;
  }
  
  pTimeDomainVal->AccPeak.AXIS_X = pTimeDomain->AccPeak.AXIS_X;
  pTimeDomainVal->AccPeak.AXIS_Y = pTimeDomain->AccPeak.AXIS_Y;
  pTimeDomainVal->AccPeak.AXIS_Z = pTimeDomain->AccPeak.AXIS_Z;
        
  /* Accelerometer Peak comparison with thresholds */      
  if ((pTimeDomain->AccPeak.AXIS_X) > pTdPkThreshold->THR_WARN_AXIS_X)
  {
        pTdAlarm->PK_STATUS_AXIS_X = WARNING;
        pTimeDomainVal->AccPeak.AXIS_X = pTimeDomain->AccPeak.AXIS_X;
  }
  if ((pTimeDomain->AccPeak.AXIS_Y) > pTdPkThreshold->THR_WARN_AXIS_Y)
  {
        pTdAlarm->PK_STATUS_AXIS_Y = WARNING;
        pTimeDomainVal->AccPeak.AXIS_Y = pTimeDomain->AccPeak.AXIS_Y;
  }
  if ((pTimeDomain->AccPeak.AXIS_Z) > pTdPkThreshold->THR_WARN_AXIS_Z)
  {
        pTdAlarm->PK_STATUS_AXIS_Z = WARNING;
        pTimeDomainVal->AccPeak.AXIS_Z = pTimeDomain->AccPeak.AXIS_Z;
  }
  if ((pTimeDomain->AccPeak.AXIS_X) > pTdPkThreshold->THR_ALARM_AXIS_X)
  {
        pTdAlarm->PK_STATUS_AXIS_X = ALARM;
        pTimeDomainVal->AccPeak.AXIS_X = pTimeDomain->AccPeak.AXIS_X;
  }
  if ((pTimeDomain->AccPeak.AXIS_Y) > pTdPkThreshold->THR_ALARM_AXIS_Y)
  {
        pTdAlarm->PK_STATUS_AXIS_Y = ALARM;
        pTimeDomainVal->AccPeak.AXIS_Y = pTimeDomain->AccPeak.AXIS_Y;
  }
  if ((pTimeDomain->AccPeak.AXIS_Z) > pTdPkThreshold->THR_ALARM_AXIS_Z)
  {
        pTdAlarm->PK_STATUS_AXIS_Z = ALARM;
        pTimeDomainVal->AccPeak.AXIS_Z = pTimeDomain->AccPeak.AXIS_Z;
  }
}

/**
  *  @brief  Compare the Frequency domain subrange comparison with external Threshold Arrays
  *  @param  pSRAmplitude: Pointer to Amplitude subranges Array resulting after Freq Analysis
  *  @param  pFDWarnThresh: Pointer to Amplitude Warning Threshold subranges Array
  *  @param  pFDAlarmThresh: Pointer to Amplitude Alarm Threshold subranges Array
  *  @param  subrange_num: Subranges number
  *  @param  pTHR_Check: Pointer to Amplitude subranges Values that exceed thresholds
  *  @param  pTHR_Fft_Alarms: Pointer to Amplitude subranges Threshold Status
  *  @return None
  */
static void MotionSP_FreqDomainAlarm (sSubrange_t *pSRAmplitude,
                                      float *pFDWarnThresh,
                                      float *pFDAlarmThresh,
                                      uint8_t subrange_num, 
                                      sSubrange_t *pTHR_Check, 
                                      sFreqDomainAlarm_t *pTHR_Fft_Alarms)
{
  float warn_thresholds;
  float alarm_thresholds;
  
  for(int i=0; i<subrange_num; i++)
  {
   for(int j=0; j<3; j++) 
   {
    warn_thresholds = *(pFDWarnThresh+(i*3)+j);
    alarm_thresholds = *(pFDAlarmThresh+(i*3)+j);
    switch (j)
    {
      case 0x00:  /* Axis X */
        pTHR_Check->AXIS_X[i] = pSRAmplitude->AXIS_X[i];        
        if(pSRAmplitude->AXIS_X[i] > warn_thresholds) {
            pTHR_Check->AXIS_X[i] = pSRAmplitude->AXIS_X[i];
            pTHR_Fft_Alarms->STATUS_AXIS_X[i] = WARNING;}
        if(pSRAmplitude->AXIS_X[i] > alarm_thresholds){
          pTHR_Check->AXIS_X[i] = pSRAmplitude->AXIS_X[i];
          pTHR_Fft_Alarms->STATUS_AXIS_X[i] = ALARM;}
       break;

      case 0x01:  /* Axis Y */
        pTHR_Check->AXIS_Y[i] = pSRAmplitude->AXIS_Y[i];
        if(pSRAmplitude->AXIS_Y[i] > warn_thresholds){
          pTHR_Check->AXIS_Y[i] = pSRAmplitude->AXIS_Y[i];
          pTHR_Fft_Alarms->STATUS_AXIS_Y[i] = WARNING;}
        if(pSRAmplitude->AXIS_Y[i] > alarm_thresholds){
          pTHR_Check->AXIS_Y[i] = pSRAmplitude->AXIS_Y[i];
          pTHR_Fft_Alarms->STATUS_AXIS_Y[i] = ALARM;}          
       break;
      case 0x02:  /* Axis Z */
        pTHR_Check->AXIS_Z[i] = pSRAmplitude->AXIS_Z[i];
        if(pSRAmplitude->AXIS_Z[i] > warn_thresholds){
          pTHR_Check->AXIS_Z[i] = pSRAmplitude->AXIS_Z[i];
          pTHR_Fft_Alarms->STATUS_AXIS_Z[i] = WARNING;}
        if(pSRAmplitude->AXIS_Z[i] > alarm_thresholds){
          pTHR_Check->AXIS_Z[i] = pSRAmplitude->AXIS_Z[i];
          pTHR_Fft_Alarms->STATUS_AXIS_Z[i] = ALARM;}  
       break;
      default:
        __NOP();
       break;    
    }
   } 
  }
}

/**
  * @brief  Measurement initialization for the accelerometer
  * @param  None
  * @return 1 in case of failure
  */    
static uint8_t MotionSP_AccMeasInit(void)
{
  uint8_t retValue = 1;
  
  /* Set ODR */
  MOTION_SENSOR_SetOutputDataRate(ACCELERO_INSTANCE,
                                  MOTION_ACCELERO,
                                  Accelerometer_Parameters.AccOdr);
  
  /* Set Full-Scale */
  MOTION_SENSOR_SetFullScale(ACCELERO_INSTANCE,
                             MOTION_ACCELERO,
                             Accelerometer_Parameters.fs);
  
  /* Get Sensitivity */
  MOTION_SENSOR_GetSensitivity(ACCELERO_INSTANCE,
                               MOTION_ACCELERO,
                               &MotionSP_Sensitivity );
  
  /* Evaluate the real accelerometer ODR **********************************/
  uint8_t iteration = 0;
  do
  {
    retValue = AccOdrMeas(&AcceleroODR);
    iteration++;
  } while( (retValue != 0) && (iteration < 3) );
  /************************************************************************/
  
  if (retValue != 0)
    return 1;
  
  return 0;
}

/**
  * @brief Read 3-axes acceleration raw data from FIFO
  * @param pSensorAxesRaw Pointer to be filled with the new data
  * @return None
  */
static void AcceleroFifoRead(MOTION_SENSOR_AxesRaw_t *pSensorAxesRaw)
{
  static uint16_t fifoId = 0;
  int16_t *data_out;
  
  data_out = (int16_t *)pSensorAxesRaw;  
  MOTION_SENSOR_FIFO_Get_Data_Word(ACCELERO_INSTANCE, MOTION_ACCELERO, data_out );
  
  fifoId++;
  if (fifoId == Accelerometer_Parameters.AccFifoSize)
  {
    fifoId = 0;
    IsAcceleroFifoToRead = false;
    LedOffTargetPlatform();
  }
}

/**
  * @brief  Measurement initialization for the accelerometer
  * @param  sCircBuffer_t *pAccCircBuff
  * @param  float AccSensitivity
  * @return None
  */
static void FillCircBuffFromFifo(sCircBuffer_t *pAccCircBuff, float AccSensitivity)
{
  MOTION_SENSOR_AxesRaw_t rawAcc;
  SensorVal_f_t mgAcc;
  SensorVal_f_t mgAccNoDC;

  /* Get a complete acceleration from FIFO */
  AcceleroFifoRead(&rawAcc);
  
  /* Convert raw acceleration in float [mg] */
  mgAcc.AXIS_X = (float)(rawAcc.x*AccSensitivity);
  mgAcc.AXIS_Y = (float)(rawAcc.y*AccSensitivity);
  mgAcc.AXIS_Z = (float)(rawAcc.z*AccSensitivity);
  
  // High Pass Filter to delete Accelerometer Offset
  MotionSP_accDelOffset(&mgAccNoDC, &mgAcc, DC_SMOOTH, RestartFlag);
  
  /* Fill the circular buffer with the accelerations without DC component */
  MotionSP_CreateAccCircBuffer(pAccCircBuff, mgAccNoDC);
}

/* Code for MotionSP integration - End Section */

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

/************************ (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
