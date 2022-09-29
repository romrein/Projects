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
//  SVN-Revision    :  $Revision:: 1165              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A high level application. User interface for communication with CPLD via SPI DMA transfer.
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

struct t_UserInputCheck tUserCmd;

BOOL bReadOnly = TRUE;
BOOL bInputError = FALSE;
BOOL bMultiRW = FALSE;

int intAddrIdx = 0;
int intLength = 0;

char* pcInputTx = NULL;

uint8_t u8UserLength = 0;
uint8_t* auWriteBuf;
uint8_t* auReadBuf;
uint8_t u8Check = 0;

uint16_t u16UserLength = 0;
uint16_t u16AddrIdx = 0;


int main(int argc, char* argv[])
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intReturn = 0;
	int intFd;

	uint8_t au8TelTx[SPI_BUF_SIZE];
	uint8_t au8TelRx[SPI_BUF_SIZE];

	size_t size;

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
	u8Check |= ((tUserCmd.bAddress << 2) | (tUserCmd.bLength << 1) | (tUserCmd.bInput << 0));

	//Check status
	if ((u8Check != 5) && (u8Check != 6))
	{
		printf("Error: user command line input incomplete.\n");
		exit(1);
	}

	if ((intAddrIdx < 0) || (intAddrIdx > CPLD_MAX_ADDRESS))
	{
		printf("Error: the address is out of range\n");
		exit(1);
	}

	if (intLength < 0)
	{
		printf("Error: the length is out of range\n");
		exit(1);
	}
	if ((((!u16UserLength) && (bReadOnly == TRUE)) || ((pcInputTx == NULL) && (bReadOnly == FALSE))))
	{
		printf("Error: command line entry. Try -h for help\n");
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

	/*printf("\n\n");
	printf("*************************************************************************************************************\n");
	printf("**                                   SPI transfer to CPLD with Commanad Line                               **\n");
	printf("*************************************************************************************************************\n");*/

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = u16AddrIdx;
	tTransferBuffer.u32Freq = SPI_SPEED;

	//---------------------------------------------------------------------------------------------------
	// Multiline transfer to CPLD 
	//---------------------------------------------------------------------------------------------------
		// Write to CPLD
	if (bReadOnly == FALSE)
	{
		intReturn = intStringSize(pcInputTx);
		size = intReturn;
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
			exit(1);
		}

		intReturn = intStringConversion(pcInputTx, auWriteBuf);
		if (intReturn < 0)
		{
			//Conversion error
			printf("Error: command line conversion\n");
			goto end;
		}
		else
		{
			tTransferBuffer.pu8DataOut = auWriteBuf;
			tTransferBuffer.pu8DataIn = auReadBuf;
			tTransferBuffer.u8Length = (uint8_t)intReturn;
			tTransferBuffer.u16Length = (uint16_t)intReturn;
			tTransferBuffer.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteTelMulti(&tTransferBuffer);
			if (ptSpiBuffer->intRet < 0)
			{
				//Errorhandling.
				printf("Error: write multiline transfer\n");
				intReturn = ptSpiBuffer->intRet;
				goto end;
			}
			else
			{
				vPrintout(FALSE, u16AddrIdx, intReturn);
				//Display transfer buffer
				vPrintBufMulti(auWriteBuf, (uint16_t)intReturn);
			}
		}
		free(auWriteBuf);
		free(auReadBuf);
	}
	// Read from CPLD
	else
	{
		auWriteBuf = malloc(u16UserLength);
		if (!auWriteBuf)
		{
			printf("Error: can't allocate write buffer\n");
			exit(1);
		}
		auReadBuf = malloc(u16UserLength);
		if (!auReadBuf)
		{
			printf("Error: can't allocate read buffer\n");
			exit(1);
		}

		tTransferBuffer.pu8DataOut = auWriteBuf;
		tTransferBuffer.pu8DataIn = auReadBuf;
		tTransferBuffer.u8Length = u8UserLength;
		tTransferBuffer.u16Length = u16UserLength;
		tTransferBuffer.bRead = TRUE;

		// Calling a low level function with the parameter handover for the SPI transfer.
		ptSpiBuffer = ptSpiReadWriteTelMulti(&tTransferBuffer);
		if (ptSpiBuffer->intRet < 0)
		{
			//Errorhandling.
			printf("Error: read multiline transfer\n");
			intReturn = ptSpiBuffer->intRet;
			goto end;
		}
		else
		{
			//Display transfer buffer
			vPrintout(TRUE, u16AddrIdx, u16UserLength);
			vPrintBufMulti(auReadBuf, (uint16_t)u16UserLength);
		}
		free(auWriteBuf);
		free(auReadBuf);
	}

	free(ptSpiBuffer);

	return intReturn;

end:
	free(auWriteBuf);
	free(auReadBuf);

end_single:

	free(ptSpiBuffer);
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
		};

		int c;
		c = getopt_long(argc, argv, "hD:s:d:b:HOLCNA:l:t:",
			lopts, NULL);

		if (c == -1)
		{
			break;
		}

		switch (c) {
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
				u8UserLength = (uint8_t)atoi(optarg);
				tUserCmd.bLength = TRUE;
			}
			break;
		case 'h':
			vPrint_help_cpld(argv[0], CPLD_VERSION_NUMBER);
			break;
		default:
			printf("Error: command line. Try -h for help\n");
			bInputError = TRUE;
			break;
		}
	}
	if (optind < argc) {
		printf("Error: non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
		bInputError = TRUE;
		return -1;
	}

	return 0;
}