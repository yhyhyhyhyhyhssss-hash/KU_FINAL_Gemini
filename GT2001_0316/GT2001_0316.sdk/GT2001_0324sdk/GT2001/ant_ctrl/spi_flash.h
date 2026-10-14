/**
  ******************************************************************************
  * @file    SPI/SPI_FLASH/spi_flash.h
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    13-November-2013
  * @brief   This file contains all the functions prototypes for the spi_flash
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "all_include.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define __FRAM_SPI0__
// #define __FRAM_SPI1__
// #define __FRAM_SPI2__
// #define __FRAM_SPI3__
// #define __FRAM_SPI4__


#define FRAM_SPI_MODE            1  // 1: SPI REG MODE ; 0: SPI GPIO SIMULATION MODE
#define FRAM_SPI_MODE_REG        1
#define FRAM_SPI_MODE_SIMULATION 0 

#define FRAM_CMD_WRSR           0x01  /* Write Status Register instruction */	 
#define FRAM_CMD_WRITE          0x02  /* Write to Memory instruction */
#define FRAM_CMD_READ           0x03  /* Read from Memory instruction */ 
#define FRAM_CMD_WRDS           0x04  /* Write disable instruction */
#define FRAM_CMD_RDSR           0x05  /* Read Status Register instruction  */	 
#define FRAM_CMD_WREN           0x06  /* Write enable instruction */
#define FRAM_CMD_RDID           0x9F  /* Read identification */

#define FRAM_DUMMY_BYTE         0x00
#define FRAM_LENGTH             0x1FFF



/* FM25CL128 CS PIN : PA4 */
#define FRAM_CS_PORT                        GPIOA
#define FRAM_CS_RCU                         RCU_GPIOA
#define FRAM_CS_PIN                         GPIO_PIN_4

#define FRAM_CS_ENABLE()                    gpio_bit_reset(FRAM_CS_PORT,FRAM_CS_PIN)
#define FRAM_CS_DISABLE()                   gpio_bit_set(FRAM_CS_PORT,FRAM_CS_PIN)

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
#if (FRAM_SPI_MODE == FRAM_SPI_MODE_SIMULATION)
    #define FRAM_SPI_INIT()                 SPIx_Init()
    #define FRAM_SPI_ReadWriteByte(x)       SPIx_ReadWriteByte(x)
#else
    #ifdef  __FRAM_SPI0__
        #define FRAM_SPI_INIT()                 spi0_init()
        #define FRAM_SET_SPEED(x)               SPI0_Set_Speed(x)
        #define FRAM_SPI_ReadWriteByte(x)       spi0_ReadWriteByte(x)
    #endif // __FRAM_SPI0__
    #ifdef  __FRAM_SPI1__
        #define FRAM_SPI_INIT()                 spi1_init()
        #define FRAM_SET_SPEED(x)               SPI1_Set_Speed(x)
        #define FRAM_SPI_ReadWriteByte(x)       spi1_ReadWriteByte(x)
    #endif // __FRAM_SPI1__
    #ifdef  __FRAM_SPI3__
        #define FRAM_SPI_INIT()                 spi3_init()
        #define FRAM_SET_SPEED(x)               SPI3_Set_Speed(x)
        #define FRAM_SPI_ReadWriteByte(x)       spi3_ReadWriteByte(x)
    #endif // __FRAM_SPI3__    
#endif // end of SPI_GPIO_SIMULATE_MODE



/* High layer functions  */

// void sFLASH_Init(void);
// void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
// void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
// uint8_t sFLASH_ReadByte(void);
// uint8_t sFLASH_SendByte(uint8_t byte);
// void sFLASH_WriteEnable(void);
// u8 sFLASH_ReadStatus(void);

void FRAM_Init(void);
void FRAM_WriteEnable(void);
void FRMA_WriteDisable(void);
uint8_t FRAM_ReadSR(void);
uint32_t FRAM_ReadID(void);
void FRAM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void FRAM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void FRAM_Write_Read_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
