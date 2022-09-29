#pragma once
//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.07.2022
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "func.c"
//
//------------------------------------------------------------------------------------------------

#ifndef _FUNC_H_
#define _FUNC_H_

#include <stdlib.h>
#include <stdint.h>
#include "WinTypes.h"

//------------------------------------------------------------------------------------------------
//  STRUCTS
//------------------------------------------------------------------------------------------------
struct t_BufTXgen
{
	uint8_t*	pu8DataOut;			// TX data
	uint8_t*	pu8DataIn;			// RX data
	uint8_t		u8Length;
	uint8_t		u8Header;
	uint8_t		u8PortID;
	uint8_t*		pui0;
	uint8_t*		puj0;
	uint8_t*		pui1;
	uint8_t*		puj1;
	uint8_t*		pui2;
	uint8_t*		puj2;
	uint8_t		u8LenPT1;
	uint8_t		u8LenPT2;
	uint8_t		u8LenPT3;
};

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// random_number()
//
//------------------------------------------------------------------------------------------------

int random_number(int min_num, int max_num);

//------------------------------------------------------------------------------------------------
// ptTXgenerate()
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
struct t_SpiDebug* ptTXgenerate(struct t_BufTXgen* ptTelGen);


#endif //_FUNC_H_