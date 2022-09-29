//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  main.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A high level application. User interface for communication with EEPROM via CPLD transfer.
//	- Transfering the binary configuration file to EEPROM
//  - main.c - program starts here
//
//------------------------------------------------------------------------------------------------

#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

//------------------------------------------------------------------------------------------------
//  FUNCTION PROTOTYPEN (declaration)
//------------------------------------------------------------------------------------------------
static int parse_opts(int argc, char* argv[]);

//------------------------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------------------------

const char* pcDevice = "/dev/spidev1.0";
uint32_t u32Mode = 1;
uint8_t u8Bits = 8;
uint32_t u32SpeedHz = 1000000;
uint16_t u16Delay = 0;
static char* input_file;

struct t_UserInputCheck tUserCmd;

BOOL bInputError = FALSE;
BOOL bReadOnly = TRUE;

char* pcInputTx = NULL;

int intAddrIdx = 0;
int intLength = 0;

uint8_t u8Idx = 0;
uint8_t u8Check = 0;

uint8_t u8UserLength = 0;
uint8_t* auWriteBuf;
uint8_t* auReadBuf;

uint16_t u16AddrIdx = 0;
uint16_t u16UserLength = 0;

uint32_t u32AddrIdx = 0;
uint32_t u32UserLength = 0;


int main(int argc, char* argv[])
{
	BOOL bFileBin = FALSE;
	int intReturn = 0;
	int intFd;
	int intFileSize = 0;

	size_t size;

	uint8_t au8TelTx[SPI_BUF_SIZE];
	uint8_t* auFileBuf;
	uint8_t u8Idx = 0;
	uint8_t u8ReadWrite = 0;

	memset(&tUserCmd, 0, sizeof(tUserCmd));
	
	printf("\n");

	//---------------------------------------------------------------------------------------------------
	// User command line input 
	//---------------------------------------------------------------------------------------------------
	intReturn = parse_opts(argc, argv);

	//---------------------------------------------------------------------------------------------------
	// Control of user entry  
	//---------------------------------------------------------------------------------------------------
	if (bInputError == TRUE)
	{
		exit(1);
	}

	// Status coding
	u8Check |= ((tUserCmd.bLength << 3) | (tUserCmd.bAddress << 2) | (tUserCmd.bFile << 1) | (tUserCmd.bInput << 0));

	//Check status
	if ((u8Check != 2) && (u8Check != 5) && (u8Check != 6) && (u8Check != 12))
	{
		printf("Error: user command line input incomplete.\n");
		exit(1);
	}

	if ((intAddrIdx < 0) || (intAddrIdx > EEPROM_MAX_ADDRESS))
	{
		printf("Error: the address is out of range\n");
		exit(1);
	}

	if ( (intLength < 0) || ((intLength == 0) && (bReadOnly == TRUE) && (!input_file)) )
	{
		printf("Error: the length is out of range or null\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Device open 
	//---------------------------------------------------------------------------------------------------
	if ((intFd = open(pcDevice, O_RDWR)) < 0)
	{
		printf("Error: Open Device\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Input config file
	//---------------------------------------------------------------------------------------------------
    if (input_file)
	{
		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		size = strlen(input_file);

		// Control of file name
		for (int i = 0; i < (size); i++)
		{
			if ( ((*(input_file + i)) == '.') 
				&& ( ((*(input_file + i + 1)) == 'b') || ((*(input_file + i + 1)) == 'B') )
				&& ( ((*(input_file + i + 2)) == 'i') || ((*(input_file + i + 2)) == 'I') )
				&& ( ((*(input_file + i + 3)) == 'n') || ((*(input_file + i + 3)) == 'N') )
				)
			{
				bFileBin = TRUE;
				break;
			}
		}
		if (bFileBin == FALSE)
		{
			printf("Error: file name is wrong\n");
			exit(1);
		}

		// Reading a binary file size
		intReturn = iReadBinFileSize(intFd, input_file);
		if (intReturn < 0)
		{
			printf("Error: read of the file size\n");
			exit(1);
		}

		intFileSize = intReturn;
		auFileBuf = malloc(intFileSize);
		if (!auFileBuf)
		{
			printf("Error: can't allocate file buffer\n");
			exit(1);
		}
		auReadBuf = malloc(intFileSize);
		if (!auReadBuf)
		{
			printf("Error: can't allocate read buffer\n");
			goto endFreeFile;
		}

		// Reading a binary file
		intReturn = iReadBinFile(intFd, input_file, auFileBuf, intFileSize);
		if (intReturn < 0)
		{
			printf("Error: read from the file\n");
			goto endFreeReadFile;
		}
		
		if ((intFileSize + u32AddrIdx) > EEPROM_MAX_ADDRESS)
		{
			printf("Error: the address plus data length is out of range\n");
			goto endFreeReadFile;
		}
		//---------------------------------------------------------------------------------------------------
		// EEPROM DATA WRITE FROM FILE 
		//---------------------------------------------------------------------------------------------------

		// Calling a midle level function. Write to EEPROM 
		intReturn = iReadWriteEEPROMData(intFd, FALSE, auFileBuf, auReadBuf, intFileSize, u32AddrIdx);
		if (intReturn < 0)
		{
			printf("Error: Function to EEPROM\n");
		}

		goto endFreeReadFile;
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Read/Write from CMD to EEPROM
	//---------------------------------------------------------------------------------------------------
	if (bReadOnly == FALSE)
	{
		u8ReadWrite = EEPROM_MAIN_WRITE;
	}
	else
	{
		u8ReadWrite = EEPROM_MAIN_READ;
	}

	switch (u8ReadWrite)
	{
		//---------------------------------------------------------------------------------------------------
		// EEPROM DATA WRITE FROM CMD  
		//---------------------------------------------------------------------------------------------------
		case EEPROM_MAIN_WRITE:
		{
			// Clearing the transmission buffer
			memset(&au8TelTx, 0, sizeof(au8TelTx));

			intReturn = intStringSize(pcInputTx);
			size = intReturn;

			if ((size + u32AddrIdx) > EEPROM_MAX_ADDRESS)
			{
				printf("Error: the address plus data length is out of range\n");
				exit(1);
			}

			auWriteBuf = malloc(size);
			if (!auWriteBuf)
			{
				printf("Error: can't allocate write buffer\n");
				exit(1);
			}
			auReadBuf = malloc(size);
			if (!auReadBuf)
			{
				printf("Error: can't allocate read buffer\n");
				goto endFreeWrite;
			}

			// Calling service function for string conversion 
			intReturn = intStringConversion(pcInputTx, auWriteBuf);
			if (intReturn < 0)
			{
				//Conversion error
				printf("Error: comand line conversion\n");
			}
			else
			{
				// Calling a midle level function. Write to EEPROM 
				intReturn = iReadWriteEEPROMData(intFd, FALSE, auWriteBuf, auReadBuf, size, u32AddrIdx);
				if (intReturn < 0)
				{
					printf("Error: Function to EEPROM\n");
				}
			}
			goto endFreeReadWrite;
		}
		//---------------------------------------------------------------------------------------------------
		// EEPROM DATA READ  
		//---------------------------------------------------------------------------------------------------
		case EEPROM_MAIN_READ:
		{
			// Clearing the transmission buffer
			memset(&au8TelTx, 0, sizeof(au8TelTx));

			if ((u32UserLength + u32AddrIdx) > EEPROM_MAX_ADDRESS)
			{
				printf("Error: the address plus data length is out of range\n");
				exit(1);
			}

			auReadBuf = malloc(u32UserLength);
			if (!auReadBuf)
			{
				printf("Error: can't allocate read buffer\n");
				exit(1);
			}
			auWriteBuf = malloc(u32UserLength);
			if (!auWriteBuf)
			{
				printf("Error: can't allocate write buffer\n");
				goto endFreeRead;
			}

			// Calling a midle level function. Read from EEPROM
			intReturn = iReadWriteEEPROMData(intFd, TRUE, auWriteBuf, auReadBuf, u32UserLength, u32AddrIdx);
			if (intReturn < 0)
			{
				printf("Error: Function to EEPROM\n");
			}

			goto endFreeReadWrite;
		}
		break;
		default:
		break;
	}
	
endFreeWrite:
	free(auWriteBuf);
	return intReturn;

endFreeRead:
	free(auReadBuf);
	return intReturn;

endFreeFile:
	free(auFileBuf);
	return intReturn;

endFreeReadFile:
	free(auFileBuf);
	free(auReadBuf);
	return intReturn;

endFreeReadWrite:
	free(auWriteBuf);
	free(auReadBuf);
	return intReturn;
}

//------------------------------------------------------------------------------------------------
// parse_opts()
//
///\brief					The interface for command line.
//
///\param[in]				argc		- name of the option
///\param[in]				argv[]		- Decode options from the vector argv
///\param[out]
//
///\retval		int			The option character when a short option is recognized. For a long option, 
//							they return val if flag is NULL
//------------------------------------------------------------------------------------------------
static int parse_opts(int argc, char* argv[])
{
	int option_index = 0;
	int check = 0;

	if (argc < 2)
	{
		bInputError = TRUE;
		printf("Error: too few parameters in command line. Try -h for help\n");
		return -1;
	}

	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "no-cs",   0, 0, 'N' },
			{ NULL, 0, 0, 0 },
			{ 0, 0, 0, NULL },
		};
		
		int c;
		c = getopt_long(argc, argv, "hD:s:d:b:HOLCNA:f:t:l:",
			lopts, NULL);

		if (c == -1)
		{
			break;
		}

		switch (c) {
		case 0:
			printf("option %s", lopts[option_index].name);
			if (optarg)
				printf(" with arg %s", optarg);
			printf("\n");
		case 'D':
			pcDevice = optarg;
			break;
		case 's':
			u32SpeedHz = (uint32_t)atoi(optarg);
			break;
		case 'd':
			u16Delay = (uint16_t)atoi(optarg);
			break;
		case 'b':
			u8Bits = (uint8_t)atoi(optarg);
			break;
		case 'H':
			u32Mode |= SPI_CPHA;
			break;
		case 'O':
			u32Mode |= SPI_CPOL;
			break;
		case 'L':
			u32Mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			u32Mode |= SPI_CS_HIGH;
			break;
		case 'N':
			u32Mode |= SPI_NO_CS;
			break;
		case 't':
			pcInputTx = optarg;
			bReadOnly = FALSE;
			tUserCmd.bInput = TRUE;
			break;
		case 'A':
			check = intCheckChar(optarg);
			if (check < 0)
			{
				bInputError = TRUE;
				printf("Error: incorrect parameter in command line. Try -h for help\n");
			}
			else
			{
				intAddrIdx = atoi(optarg);
				u16AddrIdx = (uint16_t)atoi(optarg);
				u32AddrIdx = (uint32_t)atoi(optarg);
				tUserCmd.bAddress = TRUE;
			}
			break;
		case 'l':
			check = intCheckChar(optarg);
			if (check < 0)
			{
				bInputError = TRUE;
				printf("Error: incorrect parameter in command line. Try -h for help\n");
			}
			else
			{
				intLength = atoi(optarg);
				u16UserLength = (uint16_t)atoi(optarg);
				u32UserLength = (uint32_t)atoi(optarg);
				u8UserLength = (uint8_t)atoi(optarg);
				tUserCmd.bLength = TRUE;
			}
			break;
		case 'f':
			input_file = optarg;
			tUserCmd.bFile = TRUE;
			break;
		case 'h':
			vPrint_help_eeprom(argv[0], EEPROM_VERSION_NUMBER);
			break;
		default:
			bInputError = TRUE;
			printf("Error: command line. Try -h for help\n");
			break;
		}	
	}
	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
		bInputError = TRUE;
		return -1;
	}

	return 0;
}