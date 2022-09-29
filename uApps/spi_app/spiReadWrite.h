//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWrite.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiReadWrite.c"
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


struct t_SpiTransfer
{
	int			intFd;				// Device ID
	uint8_t*	pu8DataOut;			// TX data
	uint8_t*	pu8DataIn;			// RX data
	uint16_t	u16Address;				// Source address
	uint32_t	u32Freq;			// Speed/frequency [Hz]
	uint8_t		u8Length;			// Data length [Byte], not Telegram length
	BOOL		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
	BOOL		bFlag;				// Type of transfer data. 0 - Random, 1 - User (ucDataIn)
};

struct t_SpiTransferSwitch
{
	int			intFd;				// Device ID
	uint8_t* pu8DataOut;			// TX data
	uint8_t* pu8DataIn;			// RX data
	uint8_t		u8Register;				// Register address
	uint16_t	u16RegisterExt;				// Register address extended
	uint32_t	u32Freq;			// Speed/frequency [Hz]
	uint8_t		u8Length;			// Data length [Byte], not Telegram length
	BOOL		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
	BOOL		bByteEndian;		// The order or sequence of bytes. 1 - Bigendian, 0 - little endian
	BOOL		bArrayInvert;		//
};

struct t_CmdToSwitch
{
	int			intFd;				// Device ID
	BOOL bWizard;
	BOOL bReadOnly;
	BOOL bPrintBuf;
	BOOL bClearBuf;
	BOOL bWaitIRQ;
	BOOL bByteOrder;
	BOOL bArrayInvert;
	BOOL bFileIn;
	char* pcInputTx;

	uint8_t* pu8DataOut;			// TX data
	uint8_t u8UserLength;

	uint16_t u16AddrIdx;

	uint32_t u32SpeedHz;
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
// ptSpiReadWriteTel()
//
///\brief					The interface for Read/Write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.pu8DataIn	- Input RX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[in]				.bFlag		- Predefined (user) transfer or	random data
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTelX(struct t_SpiTransfer *ptTransferBlock);

#endif //_SPIREADWRITE_H_