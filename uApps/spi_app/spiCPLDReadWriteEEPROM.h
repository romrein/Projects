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
// iEEPROM()
//
///\brief			
//
///\param[in]			
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iWriteEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress);

//------------------------------------------------------------------------------------------------
// iReadEEPROM()
//
///\brief			
//
///\param[in]			
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress);

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROMStatus()
//
///\brief					The interface for SPI DAM read/write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROMStatus(int intFd);

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROM()
//
///\brief					The interface for SPI DAM read/write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROM(int intFd, uint8_t* pu8DataOut);

#endif //_SPICPLDREADWRITEEEPROM_H_