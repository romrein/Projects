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
	uint16_t	u16Address;			// Source address
	uint32_t	u32Freq;			// Speed/frequency [Hz]
	uint8_t		u8Length;			// Data length [Byte], not Telegram length
	BOOL		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
};

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTel()
//
///\brief					The interface for Read/Write transfer.
//
///\param[in]				intFd		- Device ID
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u16Address	- Destination adress
///\param[in]				u32Freq		- Transfer speed [Hz]
///\param[in]				u8Length	- Length of Buffer
///\param[in]				bRead		- Read/Write flag
///\param[in]				bFlag		- Predefined (user) transfer or	random data
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTel(int intFd, uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint16_t u16Address, uint32_t u32Freq, uint8_t u8Length, BOOL bRead, BOOL bFlag);


//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelSingle()
//
///\brief					The interface for Read/Write transfer.
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
struct t_SpiDebug* ptSpiReadWriteTelSingle(struct t_SpiTransfer *ptTransferBlock);

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTransfer()
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
struct t_SpiDebug* ptSpiReadWriteTransfer(struct t_SpiTransfer* ptTransferBlock);

#endif //_SPIREADWRITE_H_