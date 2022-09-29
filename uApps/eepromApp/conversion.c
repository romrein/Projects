//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  conversion.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - Includes converting functions from string charackter to Hex
//
//------------------------------------------------------------------------------------------------
#include <stdlib.h>
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
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
int conversion(char* _dst, char* _src, size_t len)
{
	int ret = 0;
	int match;
	char* src = _src;
	char* dst = _dst;
	unsigned int ch;

	// Conversion from a sting
	while (*src) 
	{
		// Detecting from the hexadecimal escape sequence
		if (*src == '\\' && *(src + 1) == 'x') {
			// reading formatted input from a string
			match = sscanf(src + 2, "%2x", &ch);
			if (!match)
			{
				printf("Error: malformed input string\n");
				return -1;
			}
			src += 4;
			*dst++ = (unsigned char)ch;
		}
		else if ( (*src == '|') || (*src == '_') )
		{
			// reading formatted input from a string
			match = sscanf(src + 1, "%2x", &ch);
			if (!match)
			{
				printf("Error: malformed input string\n");
				return -1;
			}
			src += 3;
			*dst++ = (unsigned char)ch;
		}
		// Simple recopying of the characters
		else 
		{
			printf("Error: wrong input string\n");
			return -1;
		}
		ret++;
	}
	return ret;
}