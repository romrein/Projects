//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTestFuncCPLD.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiTestFuncCPLD.c"
//
//------------------------------------------------------------------------------------------------

#ifndef _SPITESTFUNC_H_
#define _SPITESTFUNC_H_
#include <stdlib.h>
#include <stdint.h>

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// spiTransferTest()
//
///\brief					SPI DMA data transfer test relating to predefined memory area
//
///\param[in]				intFD				- Device ID
///\param[in]				u32ZykCnt			- The number of test cycles
///\param[in]				u16Address			- Destination adress
///\param[in]				u8UserLen			- Length of Buffer
///\param[in]				u32Speed			- Transfer speed [Hz]
//
///\param[out]				ptSpiBufferTest->au8TelOut[130]	- Transfer buffer
///\param[out]				ptSpiBufferTest->au8TelIn[130]	- Receive buffer
//
///\retval					ptSpiBufferTest->intRet
//------------------------------------------------------------------------------------------------
int iSpiTransferTest(int intFD, uint32_t u32ZykCnt, uint16_t u16Address, uint8_t u8UserLen, uint32_t u32Speed);

//------------------------------------------------------------------------------------------------
// iSpiTransferTestSingle()
//
///\brief					SPI DMA data transfer test for single telegram
//
///\param[in]				intFD					- Device ID
///\param[in]				u32ZykCnt				- The number of test cycles
///\param[in]				u16Address				- Destination adress
///\param[in]				u8UserLen				- Length of Buffer
///\param[in]				u32Speed				- Transfer speed [Hz]
//
///\param[out]				ptSpiBufferTestSingle->au8TelOut[130]	- Transfer buffer
///\param[out]				ptSpiBufferTestSingle->au8TelIn[130]	- Receive buffer
//
///\retval					ptSpiBufferTestSingle->intRet
//------------------------------------------------------------------------------------------------
int iSpiTransferTestSingle(int intFD, uint32_t u32ZykCnt, uint16_t u16Address, uint8_t u8UserLen, uint32_t u32Speed);

#endif //_SPITESTFUNC_H_