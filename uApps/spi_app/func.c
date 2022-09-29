//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
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
///\brief					Complete printout of the Transfer and Telegram Buffer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of Buffer
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
void vPrint_help(const char* pcProg)
{
	printf("\n");
	printf("Usage: %s [-ixpwrDsdbHOLCNAltSIB]\n", pcProg);
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
		"  ----Common-----------------------------------------------------------------------------------------------------\n"
		"  -A --address		destination adress (DEC)\n"
		"  -t			send data (e.g. \"\\xde\\xad\" or \"|0A\" or \"|00_01_02_03_A4_5B_06_07\")\n"
		"  -r --read		read data from adress\n"
		"  -p --print		print data buffer\n"
		"\n"
		"  ----CPLD-------------------------------------------------------------------------------------------------------\n"
		"  -l --length		data length\n"
		"  -x --clear		clear data buffer\n"
		"  -w --wizard		wizard assistant for spi-test\n"
		"\n"
		"  ----Interrupt (not in use)-------------------------------------------------------------------------------------\n"
		"  -i --irq		wait for interrupts\n"
		"\n"
		"  ----Switch-----------------------------------------------------------------------------------------------------\n"
		"  -S --Switch		Switch mode: 1 = Write regular register, 2 = Write Static Mac Entry, 3 = Write VLAN Entry\n"
		"  -I --Invert		Invert from static MAC or VLAN adresses\n"
		"  -f --file		input data from a file (e.g. \"test.bin\")\n"
		"  -B --Byte order	Big or little endian for the register adresses\n"
		"\n"
		"  ----EEPROM-----------------------------------------------------------------------------------------------------\n"
		"  -n --binary data	input config data from a file (e.g. \"config.bin\")\n"
		"\n");
	printf("Example: %s -A 10 -t \"\\x01\\x02\\x0A\\x07\\xB3\" -p \n", pcProg);
	printf("Example: %s -A 10 -l 5 -r -p\n", pcProg);
	printf("Example: %s -S 1 -A 10 -t \"|0D\" -p\n", pcProg);
	printf("Example: %s -S 1 -A 10 -r -I -B -p\n", pcProg);
	printf("Example: %s -S 2 -A 5 -t \"|00_22_01_00_5E_00_00_EB\" -p\n", pcProg);
	printf("Example: %s -f \"test.bin\"\n", pcProg);
	printf("Example: %s -A 0 -f \"Config.bin\" -n\n", pcProg);
	printf("Example: %s -A 0 -r -n -p\n", pcProg);
	printf("\n");
	exit(1);
}
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
///\retval		ptSpiBufferClear->intRet or NULL
//------------------------------------------------------------------------------------------------
int iClearBuf(int intFd, uint32_t u32Freq)
{
	uint8_t u8LenIdxClear = 0;								// Running index for length
	uint8_t au8TelTxClear[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRxClear[SPI_BUF_SIZE];					// Receive buffer
	uint16_t u16TxIdxClear = 0;								// Running index for the address
	uint16_t u16TxIdxReadClear = 0;							// Cycle counter
	static struct t_SpiDebug* ptSpiBufferClear;				// A structure for status feedback and debugging
	ptSpiBufferClear = malloc(sizeof(struct t_SpiDebug));
	
	// avoid NULL - Pointer exception
	if (ptSpiBufferClear == NULL)
	{
		return -EFAULT;
	}

	printf("\n\n");
	printf("***************************************************************************************\n");
	printf("**                                 Clear memory area                                 **\n");
	printf("***************************************************************************************\n");

	// Clearing the transmission buffer and resetting the cycle counter
	memset(&au8TelTxClear, 0, sizeof(au8TelTxClear));
	memset(&au8TelRxClear, 0, sizeof(au8TelRxClear));
	u16TxIdxReadClear = 0;
	
	// Generate Starting Address
	u16TxIdxClear = (uint16_t)rand() % 1024;
	
	// Memory Clear Cycle
	while (1)
	{
		// Generate Starting Length
		u8LenIdxClear = (uint8_t)rand() % 128;

		// Calling the Read/Write-function and parameter transfer
		ptSpiBufferClear = ptSpiReadWriteTel(intFd, &au8TelTxClear[0], &au8TelRxClear[0], u16TxIdxClear, u32Freq, 128, FALSE, TRUE);
		if (ptSpiBufferClear->intRet < 0)
		{
			printf("Error in %s function\n", __func__);
			return ptSpiBufferClear->intRet;
		}

		// Calculating the next address from the length
		for (int i = 3; i < (u8LenIdxClear + 3); i++)
		{
			u16TxIdxClear = (u16TxIdxClear + 1) & MAXBUF;
			u16TxIdxReadClear++;
		}
		
		memset(&au8TelTxClear, 0, sizeof(au8TelTxClear));
		memset(&au8TelRxClear, 0, sizeof(au8TelRxClear));

		// Exit condition for the cycle 
		if (u16TxIdxReadClear > 2000)
		{
			printf("Memory cleared\n");
			free(ptSpiBufferClear);
			break;
		}
	}//while (1)

	return 0;
}
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
// intArrayInvertMAC()
//
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intArrayInvertMAC(uint8_t* pu8DataInOut)
{
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping

	// avoid NULL - Pointer exception
	if (pu8DataInOut == NULL)
	{
		printf("NULL pointer in %s\n", __func__);
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
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
int intArrayInvertVLAN(uint8_t* pu8DataInOut)
{
	uint8_t au8Swap[2];											// Buffer for address swapping
	uint8_t au8TmpSwap[2];										// Temporary buffer for address swapping

	// avoid NULL - Pointer exception
	if (pu8DataInOut == NULL)
	{
		printf("NULL pointer in %s\n", __func__);
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
///\brief		
//
///\param[in]	
//
///\param[out]
//
///\retval int
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
		printf("can't allocate tx buffer\n");
		intRet = -1;
		goto end;
	}
	if (size == 0)
	{
		printf("Wrong data length\n");
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
int iGet_signature(uint8_t* pu8DataOut, uint8_t u8Length)
{
	uint8_t new_sign = 0xAA;
	uint8_t prev_sign = 0;
	uint8_t new_sign_lsb = 0;


	for (int i = 0; i < (u8Length); i++)
	{
		prev_sign = new_sign;
		new_sign_lsb = ((prev_sign & 0x01) ^ ((prev_sign & 0x80) >> 7)) ^ ((*(pu8DataOut + i)) & 0x01);
		new_sign = (( (*(pu8DataOut + i)) ^ ((prev_sign << 1) & 0xFF)) & 0xFE) | new_sign_lsb;
	}
	return new_sign;
}
//------------------------------------------------------------------------------------------------
// iReadBinFile()
//
///\brief			
//
///\param[in]			
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
		perror("can't stat input file");

	tx_fd = open(filename, O_RDONLY);
	if (tx_fd < 0)
		perror("can't open input file");

	txC = malloc(sb.st_size);
	if (!txC)
		perror("can't allocate rx buffer");

	bytes = read(tx_fd, txC, sb.st_size);
	intReturn = bytes;

	if (bytes != sb.st_size)
		perror("failed to read input file");


	free(txC);
	close(tx_fd);

	return intReturn;
}

//------------------------------------------------------------------------------------------------
// iReadBinFile()
//
///\brief			
//
///\param[in]			
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadBinFile(int intFd, char* filename, uint8_t* pu8DataOut, uint8_t u8Length)
{
	static struct t_CmdToSwitch tCmdFileToSW;
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
		perror("can't stat input file");

	tx_fd = open(filename, O_RDONLY);
	if (tx_fd < 0)
		perror("can't open input file");

	txC = malloc(sb.st_size);
	if (!txC)
		perror("can't allocate rx buffer");

	bytes = read(tx_fd, txC, sb.st_size);
	u8Length = bytes;

	if (bytes != sb.st_size)
		perror("failed to read input file");


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