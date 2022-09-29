//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  conversion.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.05.2022
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "conversion.c"
//
//------------------------------------------------------------------------------------------------
#ifndef _CONVERSION_H_
#define _CONVERSION_H_
#include <stdlib.h>
//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// conversion()
//
///\brief       Unescape - process hexadecimal escape character converts shell input "\x23" -> 0x23
///				or "|12_A0_2B" to 0x12 0xA0 0x2B ...
//
///\param[in]   _dst		- Destination adress for conversion
///\param[in]	len		- Length of Buffer
///\param[out]  _src		- Source adress for conversion
//
///\retval      ret			- Size of Buffer
//------------------------------------------------------------------------------------------------
int conversion(char* _dst, char* _src, size_t len);

#endif //_CONVERSION_H_