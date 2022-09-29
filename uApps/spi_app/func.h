//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
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

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// vPrintBuf()
//
///\brief		Complete printout of the Transfer and Telegram Buffer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBuf(uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// vPrintBufSingle()
//
///\brief					Complete printout of the Transfer and Telegram Buffer
//
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBufSingle(uint8_t* pu8DataIn, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// vPrint_help()
//
///\brief		Printout of the command line parameters
//
///\param[in]	pcProg	- Command line parameters
//
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrint_help(const char* pcProg);

//------------------------------------------------------------------------------------------------
// iClearBuf()
//
///\brief		Erase function for memory area 0..1023 (CPLD)
//
///\param[in]	intFd		- Device ID
///\param[in]	u32Freq		- Transfer speed [Hz]
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int iClearBuf(int intFd, uint32_t u32Freq);

//------------------------------------------------------------------------------------------------
// intConvert()
//
///\brief		Simple character conversion to Integer
//
///\param[in]	cSymbol		- Symbol for conversion
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intConvert(char cSymbol);

//------------------------------------------------------------------------------------------------
// intArrayInvertMAC
//
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intArrayInvertMAC(uint8_t* pu8DataInOut);

//------------------------------------------------------------------------------------------------
// intArrayInvertVLAN
//
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intArrayInvertVLAN(uint8_t* pu8DataInOut);

//------------------------------------------------------------------------------------------------
// intStringConversion()
//
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intStringConversion(char* pcDataIn, uint8_t* pu8DataOut);

//------------------------------------------------------------------------------------------------
// iGet_signature()
//
///\brief					
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval					new_sign	- Signatur
//------------------------------------------------------------------------------------------------
int iGet_signature(uint8_t* pu8DataOut, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// iReadBinFile()
//
///\brief					
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval					new_sign	- Signatur
//------------------------------------------------------------------------------------------------
int iReadBinFileSize(int intFd, char* filename);

//------------------------------------------------------------------------------------------------
// iReadBinFile()
//
///\brief					
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval					new_sign	- Signatur
//------------------------------------------------------------------------------------------------
int iReadBinFile(int intFd, char* filename, uint8_t* pu8DataOut, uint8_t u8Length);

#endif //_FUNC_H_