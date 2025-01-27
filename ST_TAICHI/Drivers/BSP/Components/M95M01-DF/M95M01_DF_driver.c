/**
  ******************************************************************************
  * @file    M95M01_DF_driver.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version V1.0.1
  * @date    08-Feb-2019
  * @brief   M95M01_DF Platform Independent Driver
 ******************************************************************************
 * @attention
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

/* Includes ------------------------------------------------------------------*/
#include "M95M01_DF_driver.h"

#ifdef  USE_FULL_ASSERT_M95M01_DF
#include <stdio.h>
#endif

/** @addtogroup BSP BSP
  * @{
  */

/** @addtogroup Component Component
  * @{
  */

/** @addtogroup M95M01_DF M95M01-DF
  * @{
  */

/** @addtogroup M95M01_DF_DRIVER M95M01_DF_DRIVER
  * @{
  */

/** @defgroup M95M01_DF_DRIVER_Imported_Function M95M01_DF_DRIVER Imported Function
  * @{
  */

extern uint8_t EEPROM_PIN_Read( void *handle, uint8_t PinName, uint8_t *PinState );
extern uint8_t EEPROM_PIN_Write( void *handle, uint8_t PinName, uint8_t PinState );
extern uint8_t EEPROM_Send_Instruction( void *handle, uint32_t Instruction, uint8_t *Data, uint16_t DataSize );
extern uint32_t BSP_GetTick( void );

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/

//static M95M01_DF_Error_et M95M01_DF_ReadPin( void *handle, M95M01_DF_PinName_et PinName, uint8_t *PinState );
static M95M01_DF_Error_et M95M01_DF_WritePin( void *handle, M95M01_DF_PinName_et PinName, uint8_t PinState );

/** @defgroup M95M01_DF_DRIVER_Private_Functions M95M01_DF_DRIVER Private Functions
  * @{
  */

/**
  * @brief M95M01-DF Read Pin
  * @param  *handle Device handle.
  * @param PinName The M95M01-DF pin to be read
  * @param *PinState The actual pin state
  * @return Error code [M95M01_DF_OK, M95M01_DF_ERROR]
  */
//static M95M01_DF_Error_et M95M01_DF_ReadPin( void *handle, M95M01_DF_PinName_et PinName, uint8_t *PinState )
//{
//  if ( EEPROM_PIN_Read( handle, PinName, PinState ) )
//    return M95M01_DF_ERROR;
//  else
//    return M95M01_DF_OK;
//}

/**
  * @brief M95M01-DF Write Pin
  * @param  *handle Device handle.
  * @param PinName The M95M01-DF pin to be written
  * @param PinState The new pin state
  * @return Error code [M95M01_DF_OK, M95M01_DF_ERROR]
  */
static M95M01_DF_Error_et M95M01_DF_WritePin( void *handle, M95M01_DF_PinName_et PinName, uint8_t PinState )
{
  if ( EEPROM_PIN_Write( handle, PinName, PinState ) )
    return M95M01_DF_ERROR;
  else
    return M95M01_DF_OK;
}

/**
  * @}
  */

/** @defgroup M95M01_DF_DRIVER_Exported_Functions M95M01_DF_DRIVER Exported Functions
  * @{
  */

/**
  * @brief  Get the M95M01_DF driver version.
  * @param  Version Driver version
  * @retval Status [M95M01_DF_ERROR, M95M01_DF_OK]
  */
M95M01_DF_Error_et M95M01_DF_Get_DriverVersion(M95M01_DF_DriverVersion_st *Version)
{
  Version->Major = M95M01_DF_DRIVER_VERSION_MAJOR;
  Version->Minor = M95M01_DF_DRIVER_VERSION_MINOR;
  Version->Point = M95M01_DF_DRIVER_VERSION_POINT;
  
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM activation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_Activate(void *handle)
{
  if(M95M01_DF_WritePin(handle, S, 0))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM deactivation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_DeActivate(void *handle)
{
  if(M95M01_DF_WritePin(handle, S, 1))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM Hold condition activation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_Hold(void *handle)
{
  if (M95M01_DF_Activate(handle))
    return M95M01_DF_ERROR;
    
  if(M95M01_DF_WritePin(handle, HOLD, 0))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM Hold condition deactivation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_NotHold(void *handle)
{
  if(M95M01_DF_WritePin(handle, HOLD, 1))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM Write Protect condition activation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WriteProtect(void *handle)
{
  if(M95M01_DF_WritePin(handle, W, 0))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  EEPROM Write Protect condition deactivation.
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_NotWriteProtect(void *handle)
{
  if(M95M01_DF_WritePin(handle, W, 1))
    return M95M01_DF_ERROR;
    
  return M95M01_DF_OK;
}

/**
  * @brief  Write Enable for the M95M01_DF memory
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WREN(void *handle)
{
  uint32_t instruction = (WREN<<24);
  
  if ( EEPROM_Send_Instruction( handle, instruction, 0, 0 ) )
    return M95M01_DF_ERROR;
  else
    return M95M01_DF_OK;
}

/**
  * @brief  Write Disable for the M95M01_DF memory
  * @param  *handle Device handle.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WRDI(void *handle)
{
  uint32_t instruction = (WRDI<<24);
  
  if ( EEPROM_Send_Instruction( handle, instruction, 0, 0 ) )
    return M95M01_DF_ERROR;
  else
    return M95M01_DF_OK;
}

/**
  * @brief  Read Status Register of the M95M01_DF memory
  * @param  *handle Device handle.
  * @param  *data Data to be read.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_RDSR(void *handle, uint8_t *data)
{
  uint32_t instruction = (RDSR<<24);
  
  if ( EEPROM_Send_Instruction( handle, instruction, data, 1 ) )
    return M95M01_DF_ERROR;
  else
    return M95M01_DF_OK;
}

/**
  * @brief  Write Status Register of the M95M01_DF memory
  * @param  *handle Device handle.
  * @param  *data Data to be written.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WRSR(void *handle, uint8_t *data)
{
  uint32_t instruction = (WRSR<<24);
  uint8_t wip = 1;
  
  /* wait for WIP going low */
  do {
    
    if ( M95M01_DF_WIP_CHECK(handle, &wip) != M95M01_DF_OK )
      return M95M01_DF_ERROR;
  } while (wip);
  
  /* Set WEL */
  if ( M95M01_DF_WREN(handle) != M95M01_DF_OK )
    return M95M01_DF_ERROR;
  
  /* write data into the status register */
  if ( EEPROM_Send_Instruction( handle, instruction, data, 1 ) )
    return M95M01_DF_ERROR;
  else {
    
    /* wait for WIP going low */
    do {
      
      if ( M95M01_DF_WIP_CHECK(handle, &wip) != M95M01_DF_OK )
        return M95M01_DF_ERROR;
    } while (wip);
    
    return M95M01_DF_OK;
  }
}

/**
  * @brief  Read from Memory Array of the M95M01_DF memory
  * @param  *handle Device handle.
  * @param  address the first memory address to be read
  * @param  *data the data that have been read
  * @param  datasize the data size
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_READ(void *handle, uint32_t address, uint8_t *data, uint16_t datasize)
{
  uint32_t instruction;
  uint8_t wip = 1;
  
  if (address < M95M01_DF_MEMORY_SIZE) {
    
    /* wait for WIP going low */
    do {
      
      if ( M95M01_DF_WIP_CHECK(handle, &wip) != M95M01_DF_OK )
        return M95M01_DF_ERROR;
    } while (wip);
    
    /* read data from the eeprom */
    /* combine instruction and address */
    instruction = (READ<<24)|(address&0x00FFFFFF);  
    
    if ( EEPROM_Send_Instruction( handle, instruction, data, datasize ) )
      return M95M01_DF_ERROR;
    else
      return M95M01_DF_OK;
  }
  else
    return M95M01_DF_ERROR;
}

/**
  * @brief  Write to Memory Array of the M95M01_DF memory
  * @param  *handle Device handle.
  * @param  address the first memory address to be written
  * @param  *data Data to be written.
  * @param  datasize the data size
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WRITE(void *handle, uint32_t address, uint8_t *data, uint16_t datasize)
{
  uint32_t instruction;
  uint8_t wip = 1;
  
  if (address < M95M01_DF_MEMORY_SIZE) {
    
    /* wait for WIP going low */
    do {
      
      if ( M95M01_DF_WIP_CHECK(handle, &wip) != M95M01_DF_OK )
        return M95M01_DF_ERROR;
    } while (wip);
    
    /* Set WEL */
    if ( M95M01_DF_WREN(handle) != M95M01_DF_OK )
      return M95M01_DF_ERROR;
    
    /* write data into the eeprom */
    /* combine instruction and address */
    instruction = (WRITE<<24)|(address&0x00FFFFFF);  
    
    if ( EEPROM_Send_Instruction( handle, instruction, data, datasize ) )
      return M95M01_DF_ERROR;
    else{
      
      /* wait for WIP going low */
      do {
        
        if ( M95M01_DF_WIP_CHECK(handle, &wip) != M95M01_DF_OK )
          return M95M01_DF_ERROR;
      } while (wip);
      
      return M95M01_DF_OK;
    }
  }
  else
    return M95M01_DF_ERROR;
}

/**
  * @brief  Check for WIP status
  * @param  *handle Device handle.
  * @param  *wip The wip bit status.
  * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
  */
M95M01_DF_Error_et M95M01_DF_WIP_CHECK(void *handle, uint8_t *wip )
{
  uint8_t SR = 0x00;
  
  if ( M95M01_DF_RDSR( handle, &SR ) == M95M01_DF_ERROR )
    return M95M01_DF_ERROR;
  
  *wip = (SR & M95M01_DF_WIP_MASK) >> M95M01_DF_WIP_BIT;
  
  return M95M01_DF_OK;
}

/**
 * @brief Get the block protect of the M95M01_DF eeprom
 * @details The block protect defines the size of the area to be software-protected against Write instructions
 * @param *handle Device handle.
 * @param *bp pointer where the block protect is written
 * @retval Error code [M95M01_DF_OK, M95M01_DF_ERROR].
 */
M95M01_DF_Error_et M95M01_DF_GET_BP(void *handle, uint8_t *bp )
{
  uint8_t SR = 0x00;
  
  if ( M95M01_DF_RDSR( handle, &SR ) == M95M01_DF_ERROR )
  {
    return M95M01_DF_ERROR;
  }
  
  *bp = (SR & M95M01_DF_BP_MASK) >> M95M01_DF_BP_BIT;
  
  return M95M01_DF_OK;
  
}

/**
 * @brief Set the block protect of the M95M01_DF eeprom
 * @details The block protect defines the size of the area to be software-protected against Write instructions
 * @param *handle Device handle.
 * @param bp The block protect to be set
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
M95M01_DF_Error_et M95M01_DF_SET_BP(void *handle, uint8_t bp )
{
  uint8_t SR = bp<<M95M01_DF_BP_BIT;

  if ( M95M01_DF_WRSR( handle, &SR ) == M95M01_DF_ERROR )
  {
    return M95M01_DF_ERROR;
  }
  
  return M95M01_DF_OK;
}

#ifdef  USE_FULL_ASSERT_M95M01_DF
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void M95M01_DF_assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number */
  printf("Wrong parameters tmp: file %s on line %d\r\n", file, (int)line);
  
  /* Infinite loop */
  while (1)
  {
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

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
