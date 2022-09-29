//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWriteCPLD.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiReadWriteCPLD.c"
//
//------------------------------------------------------------------------------------------------
#ifndef _SPIREADWRITE_H_
#define _SPIREADWRITE_H_

#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

#include <stdlib.h>
#include <stdint.h>
#include "WinTypes.h"

//------------------------------------------------------------------------------------------------
//  STRUCTS
//------------------------------------------------------------------------------------------------
struct t_SpiTransfer
{
	int			intFd;				// Device ID
	uint8_t*	pu8DataOut;			// TX data
	uint8_t*	pu8DataIn;			// RX data
	uint16_t	u16Address;			// Source address
	uint32_t	u32Freq;			// Speed/frequency [Hz]
	uint8_t		u8Length;			// Data length [Byte], not Telegram length
	uint16_t	u16Length;			// Data length [Byte], not Telegram length
	BOOL		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
};

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelSingle()
//
///\brief					The interface for SPI DMA read/write single transfer to CPLD.
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
struct t_SpiDebug* ptSpiReadWriteTelSingle(struct t_SpiTransfer* ptTransferBlock);

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelMulti()
//
///\brief					The interface for SPI DMA read/write multiline transfer to CPLD.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]				.pu8DataIn	- Input RX buffer
//		
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTelMulti(struct t_SpiTransfer* ptTransferBlock);

//------------------------------------------------------------------------------------------------
// iSwitchON()
//
///\brief				Function to activate the switch in the CPLD
//
///\param[in]			intFd		- Device ID
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSwitchON(int intFd);

//------------------------------------------------------------------------------------------------
// iSwitchOFF()
//
///\brief				Function to deactivate the switch in the CPLD
//
///\param[in]			intFd		- Device ID
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSwitchOFF(int intFd);

#endif //_SPIREADWRITE_H_