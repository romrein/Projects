//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.05.2022
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - Includes common functions
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
void vPrintBuf(uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint8_t u8Length)
{
	printf("Telegram length [Byte]: %d\n", (u8Length + 3));
	printf("Buff TX 32:  ");
	for (int i = 0; i < 32; i++)
	{
		printf(" %02X", *(pu8DataOut + i));
	}
	printf("\n");
	printf("Buff RX 32:  ");
	for (int i = 0; i < 32; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff TX 64:  ");
	for (int i = 32; i < 64; i++)
	{
		printf(" %02X", *(pu8DataOut + i));
	}
	printf("\n");
	printf("Buff RX 64:  ");
	for (int i = 32; i < 64; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff TX 96:  ");
	for (int i = 64; i < 96; i++)
	{
		printf(" %02X", *(pu8DataOut + i));
	}
	printf("\n");
	printf("Buff RX 96:  ");
	for (int i = 64; i < 96; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff TX 128: ");
	for (int i = 96; i < 128; i++)
	{
		printf(" %02X", *(pu8DataOut + i));
	}
	printf("\n");
	printf("Buff RX 128: ");
	for (int i = 96; i < 128; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
}

//------------------------------------------------------------------------------------------------
// vPrintBufMulti()
//
///\brief					Complete printout of the Multiline transfer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				u32Length	- Length of buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBufMulti(uint8_t* pu8DataOut, uint32_t u32Length)
{
	printf("\n");
	printf("         00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n ");
	printf("      __________________________________________________________________________________________________\n\n ");
	printf("0000 | ");
	for (int i = 0; i < u32Length; i++)
	{
		if (((i % 32) == 0) && (i != 0))
		{
			printf(" \n");
			printf(" %04d ", i);
			printf("| ");
		}
		printf(" %02X", *(pu8DataOut + i));
	}
	printf("\n\n");
}

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
void vPrintout(BOOL bReadWrite, uint16_t u16Address, uint32_t u32Length)
{
	printf("\n");
	if (bReadWrite == TRUE)
	{
		printf(" READ | Address: %d | Length: %d\n", u16Address, u32Length);
	}
	else
	{
		printf(" WRITE | Address: %d | Length: %d\n", u16Address, u32Length);
	}
}

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
void vPrintoutLite(BOOL bReadWrite, uint16_t u16Address)
{
	printf("\n");
	if (bReadWrite == TRUE)
	{
		printf(" READ | Address: %d \n", u16Address);
	}
	else
	{
		printf(" WRITE | Address: %d \n", u16Address);
	}
}

//------------------------------------------------------------------------------------------------
// vPrintoutVlan()
//
///\brief					Printout
//
///\param[in]				bReadWrite	- Read/write
///\param[in]				u16Address	- Address
///\param[in]				u8Offset	- Offset
///\param[in]				u16VlanIdx	- index
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintoutVlan(BOOL bReadWrite, uint16_t u16Address, uint8_t u8Offset, uint16_t u16VlanIdx)
{
	printf("\n");
	if (bReadWrite == TRUE)
	{
		printf(" READ | Address: %d | Offset: %d | VLAN Idx: %d\n", u16Address, u8Offset, u16VlanIdx);
	}
	else
	{
		printf(" WRITE | Address: %d | Offset: %d | VLAN Idx: %d\n", u16Address, u8Offset, u16VlanIdx);
	}
}

//------------------------------------------------------------------------------------------------
// vPrintBufSingle()
//
///\brief		Complete printout of the single transfer buffer
//
///\param[in]		pu8DataIn	- Input RX buffer
///\param[in]		u8Length	- Length of buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
void vPrintBufSingle(uint8_t* pu8DataIn, uint8_t u8Length)
{
	printf("Buff RX 32:  ");
	for (int i = 0; i < 32; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff RX 64:  ");
	for (int i = 32; i < 64; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff RX 96:  ");
	for (int i = 64; i < 96; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
	printf("Buff RX 128: ");
	for (int i = 96; i < 128; i++)
	{
		printf(" %02X", *(pu8DataIn + i));
	}
	printf("\n");
}

//------------------------------------------------------------------------------------------------
// vPrint_help_cpld()
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
void vPrint_help_cpld(const char* pcProg, uint16_t u16VersionNr)
{
	printf("\n");
	printf("CPLD Application version number: %d\n", u16VersionNr);
	printf("\n");
	printf("Usage: %s [-mpDsdbHOLCNAlt]\n", pcProg);
	puts(" ----Driver-----------------------------------------------------------------------------------------------------\n"
		"  -D --device		device to use (default /dev/spidev1.0)\n"
		"  -s --speed		max speed (Hz)\n"
		"  -d --delay		delay (usec)\n"
		"  -b --bpw		bits per word\n"
		"  -H --cpha		clock phase\n"
		"  -O --cpol		clock polarity\n"
		"  -L --lsb		least significant bit first\n"
		"  -C --cs-high		chip select active high\n"
		"  -N --no-cs		no chip select\n"
		"\n"
		"  ----CPLD-------------------------------------------------------------------------------------------------------\n"
		"  -A --address		destination adress (DEC)\n"
		"  -t			send data (e.g. \"\\xde\\xad\" or \"|0A\" or \"|00_01_02_03_A4_5B_06_07\")\n"
		"  -l --length		data length\n"
		"\n");
	printf("Example: %s -A 10 -l 5\n", pcProg);
	printf("Example: %s -A 10 -t \"\\x01\\x02\\x0A\\x07\\xB3\"\n", pcProg);
	printf("\n");
	exit(1);
}

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
void vPrint_help_eeprom(const char* pcProg, uint16_t u16VersionNr)
{
	printf("\n");
	printf("EEPROM Application version number: %d\n", u16VersionNr);
	printf("\n");
	printf("\n");
	printf("Usage: %s [-DsdbHOLCNAflf]\n", pcProg);
	puts(" ----Driver-----------------------------------------------------------------------------------------------------\n"
		"  -D --device		device to use (default /dev/spidev1.0)\n"
		"  -s --speed		max speed (Hz)\n"
		"  -d --delay		delay (usec)\n"
		"  -b --bpw		bits per word\n"
		"  -H --cpha		clock phase\n"
		"  -O --cpol		clock polarity\n"
		"  -L --lsb		least significant bit first\n"
		"  -C --cs-high		chip select active high\n"
		"  -N --no-cs		no chip select\n"
		"\n"
		"  ----EEPROM-----------------------------------------------------------------------------------------------------\n"
		"  -A --address		destination adress (DEC)\n"
		"  -t			send data (e.g. \"\\xde\\xad\" or \"|0A\" or \"|00_01_02_03_A4_5B_06_07\")\n"
		"  -l --length		data length\n"
		"  -f --binary data	input config data from a file (e.g. \"config.bin\")\n"
		"\n");
	printf("Example: %s -A 0 -f \"Config.bin\"\n", pcProg);
	printf("Example: %s -A 0 -l 2\n", pcProg);
	printf("\n");
	exit(1);
}

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
void vPrint_help_switch(const char* pcProg, uint16_t u16VersionNr)
{
	printf("\n");
	printf("Switch Application version number: %d\n", u16VersionNr);
	printf("\n");
	printf("\n");
	printf("Usage: %s [-DsdbHOLCNAft]\n", pcProg);
	puts(" ----Driver-----------------------------------------------------------------------------------------------------\n"
		"  -D --device		device to use (default /dev/spidev1.0)\n"
		"  -s --speed		max speed (Hz)\n"
		"  -d --delay		delay (usec)\n"
		"  -b --bpw		bits per word\n"
		"  -H --cpha		clock phase\n"
		"  -O --cpol		clock polarity\n"
		"  -L --lsb		least significant bit first\n"
		"  -C --cs-high		chip select active high\n"
		"  -N --no-cs		no chip select\n"
		"\n"
		"  ----Switch-----------------------------------------------------------------------------------------------------\n"
		"  -A --address		destination adress (DEC)\n"
		"  -t			send data (e.g. \"\\xde\\xad\" or \"|0A\" or \"|00_01_02_03_A4_5B_06_07\")\n"
		"  -S --Switch		Switch mode: 1 = Write regular register, 2 = Write Static Mac Entry, 3 = Write VLAN Entry\n"
		"  -f --file		input data from a file (e.g. \"test.cfg\")\n"
		"\n");
	printf("Example: %s -S 1 -A 10 -t \"|0D\"\n", pcProg);
	printf("Example: %s -S 1 -A 10 -t \"\\x01\"\n", pcProg);
	printf("Example: %s -S 1 -A 10\n", pcProg);
	printf("Example: %s -S 2 -A 5 -t \"|00_22_01_00_5E_00_00_EB\"\n", pcProg);
	printf("Example: %s -S 3 -A 2050 -t \"|22_0A\" // -A is VLAN Idx, not ind. address\n", pcProg);
	printf("Example: %s -S 3 -A 2050 // -A is VLAN Idx, not ind. address\n", pcProg);
	printf("Example: %s -f \"test.cfg\"\n", pcProg);
	printf("\n");
	exit(1);
}

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
int intConvert(char cSymbol)
{
	int number = 0;

	if ((cSymbol == 'a') || (cSymbol == 'A'))
	{
		number = 10;
	}
	else if ((cSymbol == 'b') || (cSymbol == 'B'))
	{
		number = 11;
	}
	else if ((cSymbol == 'c') || (cSymbol == 'C'))
	{
		number = 12;
	}
	else if ((cSymbol == 'd') || (cSymbol == 'D'))
	{
		number = 13;
	}
	else if ((cSymbol == 'e') || (cSymbol == 'E'))
	{
		number = 14;
	}
	else if ((cSymbol == 'f') || (cSymbol == 'F'))
	{
		number = 15;
	}
	else
	{
		number = atoi(&cSymbol);
	}
	return number;
}

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
int intCheckChar(char cSymbol)
{
	int number = 0;

	if (((*optarg) == '0') || ((*optarg) == '1') || ((*optarg) == '2') || ((*optarg) == '3') || ((*optarg) == '4')
		|| ((*optarg) == '5') || ((*optarg) == '6') || ((*optarg) == '7') || ((*optarg) == '8') || ((*optarg) == '9'))
	{
		number = 0;
	}
	else
	{
		number = -1;
	}
	return number;
}

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
int intArrayInvertMAC(uint8_t* pu8DataInOut)
{
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping

	// avoid NULL - Pointer exception
	if (pu8DataInOut == NULL)
	{
		printf("Error: NULL pointer in %s\n", __func__);
		return -EFAULT;
	}
	memcpy(&au8TmpSwap[0], pu8DataInOut, SWITCH_STATIC_MAC_LENGTH);

	for (int i = 0; i < SWITCH_STATIC_MAC_LENGTH; i++)
	{
		au8Swap[SWITCH_STATIC_MAC_LENGTH - i - 1] = au8TmpSwap[i];
	}

	for (int i = 0; i < SWITCH_STATIC_MAC_LENGTH; i++)
	{
		*(pu8DataInOut + i) = au8Swap[i];
	}

	return 0;
}

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
int intArrayInvertVLAN(uint8_t* pu8DataInOut)
{
	uint8_t au8Swap[2];											// Buffer for address swapping
	uint8_t au8TmpSwap[2];										// Temporary buffer for address swapping

	// avoid NULL - Pointer exception
	if (pu8DataInOut == NULL)
	{
		printf("Error: NULL pointer in %s\n", __func__);
		return -EFAULT;
	}
	memcpy(&au8TmpSwap[0], pu8DataInOut, SWITCH_VLAN_LENGTH_2);

	for (int i = 0; i < SWITCH_VLAN_LENGTH_2; i++)
	{
		au8Swap[SWITCH_VLAN_LENGTH_2 - i - 1] = au8TmpSwap[i];
	}

	for (int i = 0; i < SWITCH_VLAN_LENGTH_2; i++)
	{
		*(pu8DataInOut + i) = au8Swap[i];
	}

	return 0;
}

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
int intStringConversion(char* pcDataIn, uint8_t* pu8DataOut)
{
	size_t size;													// Buffer size before conversion

	int intBuffsize = 0;											// Calculated buffer size after conversion
	int intRet = 0;
	uint8_t* tx;													// Output character buffer 	

	size = strlen(pcDataIn);
	tx = (uint8_t*)malloc(size);
	if (!tx)
	{
		printf("Error: can't allocate tx buffer\n");
		intRet = -1;
		goto end;
	}
	if (size == 0)
	{
		printf("Error: wrong data length\n");
		intRet = -1;
		goto end;
	}
	intBuffsize = conversion((char*)tx, pcDataIn, size);
	if (intBuffsize < 0)
	{
		intRet = -1;
		goto end;
	}
	else
	{
		intRet = intBuffsize;
	}
	for (int i = 0; i < (intBuffsize); i++)
	{
		*(pu8DataOut + i) = *(tx + i);
	}

end:
	free(tx);
	return intRet;
}

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
int intStringSize(char* pcDataIn)
{
	size_t size;													// Buffer size before conversion

	int intBuffsize = 0;											// Calculated buffer size after conversion
	int intRet = 0;
	uint8_t* tx;													// Output character buffer 	

	size = strlen(pcDataIn);
	tx = (uint8_t*)malloc(size);
	if (!tx)
	{
		printf("Error: can't allocate tx buffer\n");
		intRet = -1;
		goto end;
	}
	if (size == 0)
	{
		printf("Error: wrong data length\n");
		intRet = -1;
		goto end;
	}
	intBuffsize = conversion((char*)tx, pcDataIn, size);
	if (intBuffsize < 0)
	{
		intRet = -1;
		goto end;
	}
	else
	{
		intRet = intBuffsize;
	}

end:
	free(tx);
	return intRet;
}

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
int iGet_signature(uint8_t* pu8DataOut, uint8_t u8Length)
{
	uint8_t new_sign = 0xAA;
	uint8_t prev_sign = 0;
	uint8_t new_sign_lsb = 0;

	for (int i = 0; i < (u8Length); i++)
	{
		prev_sign = new_sign;
		new_sign_lsb = ((prev_sign & 0x01) ^ ((prev_sign & 0x80) >> 7)) ^ ((*(pu8DataOut + i)) & 0x01);
		new_sign = (((*(pu8DataOut + i)) ^ ((prev_sign << 1) & 0xFF)) & 0xFE) | new_sign_lsb;
	}
	return new_sign;
}

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
int iReadBinFileSize(int intFd, char* filename)
{
	struct stat sb;												// Buffer for file information
	ssize_t bytes;												// File size
	int tx_fd;													// File ID
	int iIdx = 0;												// File character index
	int intReturn = 0;
	char* txC;													// Char pointer on the receive buffer
	char ch;													// Received character

	// Prepare import data file
	if (stat(filename, &sb) == -1)
		printf("Error: can't stat input file\n");

	tx_fd = open(filename, O_RDONLY);
	if (tx_fd < 0)
		printf("Error: can't open input file\n");

	txC = malloc(sb.st_size);
	if (!txC)
		printf("Error: can't allocate rx buffer\n");

	bytes = read(tx_fd, txC, sb.st_size);
	intReturn = bytes;

	if (bytes != sb.st_size)
		printf("Error: failed to read input file\n");

	free(txC);
	close(tx_fd);

	return intReturn;
}

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
int iReadBinFile(int intFd, char* filename, uint8_t* pu8DataOut, uint8_t u8Length)
{
	static struct t_SpiDebugSwitch* ptSpiDataFromSwitch;		// Transfer buffer interface

	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)

	BOOL bLineStart;											// Merker for new valid line
	struct stat sb;												// Buffer for file information
	ssize_t bytes;												// File size
	int tx_fd;													// File ID
	int iIdx = 0;												// File character index
	int intReturn = 0;
	char* txC;													// Char pointer on the receive buffer
	char ch;													// Received character

	uint8_t binHex = 0;

	// Clearing the transmission buffer and set new line
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// Prepare import data file
	if (stat(filename, &sb) == -1)
		printf("Error: can't stat input file\n");

	tx_fd = open(filename, O_RDONLY);
	if (tx_fd < 0)
		printf("Error: can't open input file\n");

	txC = malloc(sb.st_size);
	if (!txC)
		printf("Error: can't allocate rx buffer\n");

	bytes = read(tx_fd, txC, sb.st_size);
	u8Length = bytes;

	if (bytes != sb.st_size)
		printf("Error: failed to read input file\n");

	binHex = *(txC + 3);

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (u8Length); i++)
	{
		*(pu8DataOut + i) = *(txC + i);
	}

	free(txC);
	close(tx_fd);

	return intReturn;
}