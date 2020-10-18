/**
  ******************** (C) COPYRIGHT 2020 STMicroelectronics *******************
  * @file    readme.txt
  * @author  System Research & Applications Team - Catania Lab.
  * @version V2.2.0
  * @date    16-March-2020
  * @brief   Description of the Application FW.
  ******************************************************************************
  * Attention
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
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

Application Description 

 This firmware package includes Components Device Drivers, Board Support Package
 and example application for the following STMicroelectronics elements:
 - STEVAL-STWINKT1 (SensorTile Wireless Industrial Node) evaluation kit that contains:
    . STEVAL-STWINCSV1 (Core System) that contains HTS221, LPS22HH, IIS3DWB, ISM330DHCX, STTS751, IIS2MDC, IIS2DH, IMP34DT05 and MP23ABS1 components
	. 480mAh Li-Po battery
	. STLINK-V3MINI debugger
	. Plastic box
 - MotionSP software provides real-time vibration analysis in time domain and frequency domain.

 The Example application initializes all the Components and Library creating 4 Custom Bluetooth services:
 - The first service exposes all the HW characteristics related to MEMS sensor devices: Temperature, Humidity, Pressure, Magnetometer, 
   Gyroscope, Accelleromenter, Microphones Signal Noise dB level and Battery Features.
 - The second service exposes the SW characteristic: the Frequency Domain Processing using the MotionSP algorithm.
 - The third Service exposes the console services where we have stdin/stdout and stderr capabilities
 - The last Service is used for configuration purpose
 
 The example application allows the user to control the initialization phase via UART.
 Launch a terminal application and set the UART port to 9600 bps, 8 bit, No Parity, 1 stop bit.
 
 This example must be used with the related ST BLE Sensor Android/iOS application available on Play/itune store (Version 4.1.0 or higher),
 in order to read the sent information by Bluetooth Low Energy protocol

                              --------------------
                              | VERY IMPORTANT : |
                              --------------------
 1) This example support the Firmware-Over-The-Air (FOTA) update using the ST BLE Sensor Android/iOS application (Version 3.0.0 and above)
 2) This example must run starting at address 0x08004000 in memory and works ONLY if the BootLoader 
    is saved at the beginning of the FLASH (address 0x08000000)
 3) For each IDE (IAR/µVision/STM32CubeIDE) and for NUCLEO-F446RE platform,
    there are some scripts *.bat and *.sh that makes the following operations:
     - Full Flash Erase
     - Load the BootLoader on the rigth flash region
     - Load the Program (after the compilation) on the rigth flash region (This could be used for a FOTA)
     - Dump back one single binary that contain BootLoader+Program that could be
       flashed at the flash beginning (address 0x08000000) (This COULD BE NOT used for FOTA)
     - Reset the board


 Inside the Binary Directory there are the following binaries:
Binary/
¦   +-- STM32L4R9ZI-STWIN_PredictiveMaintenance_v2.2.0.bin			(Program without BootLoader. COULD BE USED     for FOTA)
¦   +-- STM32L4R9ZI-STWIN_PredictiveMaintenance_BL_v2.2.0.bin		(Program with BootLoader.    COULD NOT BE USED for FOTA)

@par Hardware and Software environment

  - This example runs on STEVAL-STWINCSV1 Core System to STM32L4R9ZIJ6 devices
    can be easily tailored to any other supported device and development board.
    
  - This example must be used with the related ST BLE Sensor Android/iOS application (Version 4.1.0 or higher) available on Play/itune store,
    in order to read the sent information by Bluetooth Low Energy protocol
    
@par STM32Cube packages:
  - STM32F4xx drivers from STM32CubeL4 V1.15.0
@par X-CUBE packages:
  - X-CUBE-BLE2 V1.0.0
  - X-CUBE-MEMS1 V7.1.0
@par STWIN packages:
  - STSW-STWINKT01 V1.3.0

@par How to use it ? 

This package contains projects for 3 IDEs viz. IAR, µVision and System Workbench. 
In order to make the  program work, you must do the following:
 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.

For IAR:
 - Open IAR toolchain (this firmware has been successfully tested with Embedded Workbench V8.32.3).
 - Open the IAR project file STM32F446RE-Nucleo\Demonstrations\Predictive_Maintenance_BLE\EWARM\PredictiveMaintenance.eww.
 - Rebuild all files and run these script that you find on the same directory:
	- CleanPREDMNT1_IAR_STWIN.bat

For µVision:
 - Open µVision toolchain (this firmware has been successfully tested with MDK-ARM Professional Version: 5.29.0)
 - Open the µVision project file STM32F446RE-Nucleo\Demonstrations\Predictive_Maintenance_BLE\Project.uvprojx.
 - Rebuild all files and run these script that you find on the same directory:
	- CleanPREDMNT1_MDK-ARM_STWIN.bat
		
For System Workbench:
 - Open STM32CubeIDE (this firmware has been successfully tested with Version 1.3.1)
 - Set the default workspace proposed by the IDE (please be sure that there are not spaces in the workspace path).
 - Press "File" -> "Import" -> "Existing Projects into Workspace"; press "Browse" in the "Select root directory" and choose the path where the System
   Workbench project is located (it should be STM32F446RE-Nucleo\Demonstrations\Predictive_Maintenance_BLE\STM32CubeIDE\STWIN_PREDMNT1\. 
 - Rebuild all files and and run these script that you find on the same directory:
	- CleanPREDMNT1_STM32CubeIDE_STWIN.bat
		
 /******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
