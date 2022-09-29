#pragma once
//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.05.2022
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
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// vPrintBuf()
//
///\brief					Complete printout of the transfer and telegram buffer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBuf(uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// vPrintBufMulti()
//
///\brief					Complete printout of the Multiline transfer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				u32Length	- Length of Buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBufMulti(uint8_t* pu8DataOut, uint32_t u32Length);

//------------------------------------------------------------------------------------------------
// vPrintBufSingle()
//
///\brief					Complete printout of the single transfer buffer
//
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of Buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBufSingle(uint8_t* pu8DataIn, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// vPrintout()
//
///\brief					Printout
//
///\param[in]				bReadWrite	- Read/write
///\param[in]				u16Address	- Address
///\param[in]				u32Length	- Length of buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintout(BOOL bReadWrite, uint16_t u16Address, uint32_t u32Length);

//------------------------------------------------------------------------------------------------
// vPrintoutLite()
//
///\brief					Printout
//
///\param[in]				bReadWrite	- Read/write
///\param[in]				u16Address	- Address
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintoutLite(BOOL bReadWrite, uint16_t u16Address);

//------------------------------------------------------------------------------------------------
// vPrintoutVlan()
//
///\brief					Printout
//
///\param[in]				bReadWrite	- Read/write
///\param[in]				u16Address	- Address
///\param[in]				u8Offset	- Offset
///\param[in]				u16VlanIdx	- VLAN index
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintoutVlan(BOOL bReadWrite, uint16_t u16Address, uint8_t u8Offset, uint16_t u16VlanIdx);

//------------------------------------------------------------------------------------------------
// vPrint_help_cpld()
//
///\brief		Printout of the command line parameters
//
///\param[in]	pcProg	- Command line parameters
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrint_help_cpld(const char* pcProg, uint16_t u16VersionNr);

//------------------------------------------------------------------------------------------------
// vPrint_help_eeprom()
//
///\brief		Printout of the command line parameters
//
///\param[in]	pcProg		- Command line parameters
///\param[in]	u16VersionNr	- Version number
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrint_help_eeprom(const char* pcProg, uint16_t u16VersionNr);

//------------------------------------------------------------------------------------------------
// vPrint_help_switch()
//
///\brief		Printout of the command line parameters
//
///\param[in]	pcProg		- Command line parameters
///\param[in]	u16VersionNr	- Version number
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrint_help_switch(const char* pcProg, uint16_t u16VersionNr);

//------------------------------------------------------------------------------------------------
// intConvert()
//
///\brief		Simple character conversion to integer
//
///\param[in]	cSymbol		- Symbol for conversion
//
///\param[out]
//
///\retval		number
//------------------------------------------------------------------------------------------------
int intConvert(char cSymbol);

//------------------------------------------------------------------------------------------------
// intCheckChar()
//
///\brief		Simple character checking
//
///\param[in]	cSymbol		- Symbol for checking
//
///\param[out]
//
///\retval		number
//------------------------------------------------------------------------------------------------
int intCheckChar(char cSymbol);

//------------------------------------------------------------------------------------------------
// intArrayInvertMAC()
//
///\brief		Function for inverting a MAC address		
//
///\param[in]	pu8DataInOut		- Input buffer
//
///\param[out]	pu8DataInOut		- Output buffer
//
///\retval		int
//------------------------------------------------------------------------------------------------
int intArrayInvertMAC(uint8_t* pu8DataInOut);

//------------------------------------------------------------------------------------------------
// intArrayInvertVLAN()
//
///\brief		Function for inverting a VLAN address
//
///\param[in]	pu8DataInOut		- Input buffer
//
///\param[out]	pu8DataInOut		- Output buffer
//
///\retval		int
//------------------------------------------------------------------------------------------------
int intArrayInvertVLAN(uint8_t* pu8DataInOut);

//------------------------------------------------------------------------------------------------
// intStringConversion()
//
///\brief		Character string conversion to integer
//
///\param[in]	pcDataIn		- Input char buffer
//
///\param[out]	pu8DataOut	- Output integer buffer
//
///\retval		int
//------------------------------------------------------------------------------------------------
int intStringConversion(char* pcDataIn, uint8_t* pu8DataOut);

//------------------------------------------------------------------------------------------------
// intStringSize()
//
///\brief		Function for the calculation of the string size
//
///\param[in]	pcDataIn		- Input char buffer
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int intStringSize(char* pcDataIn);

//------------------------------------------------------------------------------------------------
// iGet_signature()
//
///\brief		Function for the calculation of the signature (CRC)					
//
///\param[in]	pu8DataOut	- Output buffer
///\param[in]	u8Length	- Length of buffer
///\param[out]
//
///\retval		new_sign	- Signature
//------------------------------------------------------------------------------------------------
int iGet_signature(uint8_t* pu8DataOut, uint8_t u8Length);

//------------------------------------------------------------------------------------------------
// iReadBinFileSize()
//
///\brief		Reading a binary file size		
//
///\param[in]	intFd		- File ID number
///\param[in]	filename		- File name
//
///\param[out]	
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadBinFileSize(int intFd, char* filename);

//------------------------------------------------------------------------------------------------
// iReadBinFile()
//
///\brief		Reading a binary file		
//
///\param[in]	intFd		- File ID number
///\param[in]	filename		- File name
///\param[in]	pu8DataOut	- Output buffer
///\param[in]	u8Length	- Length of buffer
//
///\param[out]	
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadBinFile(int intFd, char* filename, uint8_t* pu8DataOut, uint8_t u8Length);

#endif //_FUNC_H_