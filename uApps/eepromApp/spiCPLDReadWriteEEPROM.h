//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiCPLDReadWriteEEPROM.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  06.08.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiCPLDReadWriteEEPROM.c"
//
//------------------------------------------------------------------------------------------------
#ifndef _SPICPLDREADWRITEEEPROM_H_
#define _SPICPLDREADWRITEEEPROM_H_

#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// iWriteEEPROM()
//
///\brief			Write data to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		pu8DataOut	- Output TX buffer
///\param[in]		u8Length	- Length of buffer
///\param[in]		u16StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iWriteEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress);

//------------------------------------------------------------------------------------------------
// iReadEEPROM()
//
///\brief			Read command to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		pu8DataOut	- Input RX buffer
///\param[in]		u8Length	- Length of buffer
///\param[in]		u16StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress);

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROMStatus()
//
///\brief					Read EEPROM status
//
///\param[in]				intFd		- Device ID
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROMStatus(int intFd);

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROM()
//
///\brief					Read data from EEPROM via CPLD and SPI DMA transfer
//
///\param[in]				intFd		- Device ID
///\param[in]				
///\param[out]				pu8DataOut	- Input RX buffer
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// iReadWriteEEPROMData()
//
///\brief			Write data to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		bReadOnly	- Read/Write
///\param[in]		pu8DataOut	- Output TX buffer
///\param[in]		pu8DataIn	- Output RX buffer
///\param[in]		u32Length	- Length of buffer
///\param[in]		u32StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadWriteEEPROMData(int intFd, BOOL bReadOnly, uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint32_t u32Length, uint32_t u32StartAddress);

#endif //_SPICPLDREADWRITEEEPROM_H_