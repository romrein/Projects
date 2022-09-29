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
//	- A high level application.
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

uint8_t u8Switch = 0;
uint8_t u8Idx = 0;

BOOL bWizard = FALSE;
BOOL bReadOnly = FALSE;
BOOL bPrintBuf = FALSE;
BOOL bWaitIRQ = FALSE;
BOOL bByteOrder = FALSE;
BOOL bArrayInvert = FALSE;
BOOL bBinFile = FALSE;

char* pcInputTx = NULL;

uint8_t u8UserLength = 0;

uint16_t u16Addr = 0;
uint16_t u16AddrIdx = 0;
uint16_t u16StartAddrIdx = 0;


int main(int argc, char* argv[])
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiDebugSwitch* ptSpiDataFromSwitch;
	static struct t_SpiTransfer tTransferBuffer;
	static struct t_CmdToSwitch tCmdBufferSW;
	static struct t_FileToSwitch tFileBufferSW;

	size_t size;

	int intReturn = 0;
	int intStatus = 0;
	int nBuffsize = 0;
	int intUser = 0;
	int intFd;
	int intIRQs = 0;
	int intSignatur = 0;
	int intFileSize = 0;
	int intIdx = 0;

	uint8_t u8SingleTest = 0;
	uint8_t au8TelTx[SPI_BUF_SIZE];
	uint8_t u8SwitchReg = 0;
	uint8_t u8SwitchReg1 = 0;
	uint8_t* auFileBuf;

	uint8_t u8Offset = 0;
	uint16_t u16Adresse = 0;
	

	uint32_t u32ZykCount = 0;

	intReturn = parse_opts(argc, argv);


	printf("spi Application\n");

	return 0;


	//---------------------------------------------------------------------------------------------------
	// Device open 
	//---------------------------------------------------------------------------------------------------
	if ((intFd = open(pcDevice, O_RDWR)) < 0)
	{
		perror("Error Open Device");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Interuppt routine. Not yet implemented
	//---------------------------------------------------------------------------------------------------
	if (bWaitIRQ == TRUE)
	{
		while (1)
		{
			read(intFd, &intIRQs, sizeof(intIRQs));
			printf("interrupts: %d\n", intIRQs);
			bWaitIRQ = FALSE;
			break;
		}
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Input config file (Only for switch)
	//---------------------------------------------------------------------------------------------------
	if ( (input_file) && (bBinFile == FALSE) )
	{
		printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                          Config file transfer to switch                           **\n");
		printf("***************************************************************************************\n");

		tFileBufferSW.intFd = intFd;
		tFileBufferSW.bReadOnly = bReadOnly;
		tFileBufferSW.bByteOrder = bByteOrder;
		tFileBufferSW.bArrayInvert = bArrayInvert;
		tFileBufferSW.u32SpeedHz = u32SpeedHz;

		intReturn = iFileTransferToSwitch(intFd, input_file, &tFileBufferSW);
		if (intReturn < 0)
		{
			//Errorhandling. Not yet implemented
		}

		return intReturn;
	}
	else if ((input_file) && (bBinFile == TRUE))
	{

		memset(&au8TelTx, 0, sizeof(au8TelTx));
		intReturn = iReadBinFileSize(intFd, input_file);
		intFileSize = intReturn;
		auFileBuf = malloc(intFileSize);
		if (!auFileBuf)
			perror("can't allocate rx buffer");

		intReturn = iReadBinFile(intFd, input_file, auFileBuf, intFileSize);

		u16StartAddrIdx = u16AddrIdx;

		u16Adresse = u16StartAddrIdx / 128;

		u16Addr = u16StartAddrIdx;
		u8Idx = 0;

		for (int i = 0; i < (intFileSize); i++)
		{
			if (u16Addr < ((u16Adresse + 1) * 128))
			{
				au8TelTx[u8Idx] = *(auFileBuf + i);
				u8Idx = (u8Idx + 1) & 0x7F;
			}
			else
			{
				if (u8Idx == 0)
				{
					intReturn = iWriteEEPROM(intFd, &au8TelTx[0], 128, u16StartAddrIdx);
				}
				else
				{
					intReturn = iWriteEEPROM(intFd, &au8TelTx[0], u8Idx, u16StartAddrIdx);
				}
				u16StartAddrIdx = ((u16Adresse + 1) * 128);
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				u8Idx = 0;
				
				au8TelTx[u8Idx] = *(auFileBuf + i);
				u8Idx = (u8Idx + 1) & 0x7F;
				
				u16Adresse++;
			}
			u16Addr++;
		}
		if (u8Idx == 0)
		{
			intReturn = iWriteEEPROM(intFd, &au8TelTx[0], 128, u16StartAddrIdx);
		}
		else
		{
			intReturn = iWriteEEPROM(intFd, &au8TelTx[0], u8Idx, u16StartAddrIdx);
		}


		free(auFileBuf);

	
		return intReturn;
	}

	// Memory allocation for switch-transfer
	ptSpiDataFromSwitch = malloc(sizeof(struct t_SpiDebugSwitch));

	//---------------------------------------------------------------------------------------------------
	// CPLD. SPI DMA transfer tests.  
	//---------------------------------------------------------------------------------------------------
	if (u8Switch == 0)
	{
		//Command line user interface
			if (bBinFile == TRUE)
			{
				printf("\n\n");
				printf("*************************************************************************************************************\n");
				printf("**                                      EEPROMDATEN READ                                                   **\n");
				printf("*************************************************************************************************************\n");

				memset(&au8TelTx, 1, sizeof(au8TelTx));
				u16StartAddrIdx = u16AddrIdx;

				// Calling a low level function with the parameter handover for the SPI transfer.
				intReturn = iReadEEPROM(intFd, &au8TelTx[0], 128, u16StartAddrIdx);

				printf("\n\n");
				printf("*************************************************************************************************************\n");
				printf("**                                      EEPROMDATEN                                                        **\n");
				printf("*************************************************************************************************************\n");

				memset(&au8TelTx, 1, sizeof(au8TelTx));

				// Calling a low level function with the parameter handover for the SPI transfer.
				intReturn = iSpiReadEEPROM(intFd, &au8TelTx);

				if (bPrintBuf == TRUE)
				{
					vPrintBufSingle(&au8TelTx[0], (uint8_t)128);
				}
				printf("\n\n");

				return intReturn;
			}
			else
			{
				if ( (((!u8UserLength) && (bReadOnly == TRUE)) || ((pcInputTx == NULL) && (bReadOnly == FALSE))) )
				{
					perror("Error command line. Try -h for help\n");
					exit(1);
				}

				printf("\n\n");
				printf("*************************************************************************************************************\n");
				printf("**                                      SPI transfer with Commanad Line                                    **\n");
				printf("*************************************************************************************************************\n");

				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = u16AddrIdx;
				tTransferBuffer.u32Freq = u32SpeedHz;

				if (bReadOnly == FALSE)
				{
					intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
					if (intReturn < 0)
					{
						//Conversion error
						//Errorhandling. Not yet implemented
					}
					else
					{

						intSignatur = iGet_signature(&au8TelTx[0], (uint8_t)intReturn);
						printf("Signatur: %d\n", intSignatur);

						tTransferBuffer.pu8DataOut = &au8TelTx[0];
						tTransferBuffer.u8Length = (uint8_t)intReturn;
						tTransferBuffer.bRead = FALSE;

						// Calling a low level function with the parameter handover for the SPI transfer.
						ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
						if (ptSpiBuffer->intRet < 0)
						{
							intReturn = ptSpiBuffer->intRet;
						}
						else
						{
							printf("Transmited data [HEX]:\n");
							for (int i = 3; i < (intReturn + 3); i++)
							{
								printf(" %02X", ptSpiBuffer->au8TelOut[i]);
							}
							printf("\n");
							if (bPrintBuf == TRUE)
							{
								vPrintBuf(&ptSpiBuffer->au8TelOut[0], &ptSpiBuffer->au8TelIn[0], (uint8_t)intReturn);
							}
							printf("\n\n");
						}
					}
				}
				else
				{
					tTransferBuffer.pu8DataOut = &au8TelTx[0];
					tTransferBuffer.u8Length = u8UserLength;
					tTransferBuffer.bRead = TRUE;

					// Calling a low level function with the parameter handover for the SPI transfer.
					ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
					if (ptSpiBuffer->intRet < 0)
					{
						//Errorhandling. Not yet implemented
						intReturn = ptSpiBuffer->intRet;
					}
					else
					{
						//Display transfer buffer
						printf("Received data [HEX]:\n");
						for (int i = 3; i < (u8UserLength + 3); i++)
						{
							printf(" %02X", ptSpiBuffer->au8TelIn[i]);
						}
						printf("\n");
						if (bPrintBuf == TRUE)
						{
							vPrintBuf(&ptSpiBuffer->au8TelOut[0], &ptSpiBuffer->au8TelIn[0], u8UserLength);
						}
						printf("\n\n");
					}
				}
				free(ptSpiBuffer);
			}
	}
	//---------------------------------------------------------------------------------------------------
	// Switch. Single register transfer 
	//---------------------------------------------------------------------------------------------------
	else if (u8Switch == 1)
	{	
		printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                    Single register SPI transfer to switch                         **\n");
		printf("***************************************************************************************\n");

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;
		tCmdBufferSW.bByteOrder = bByteOrder;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				//Errorhandling. Not yet implemented
				goto end;
			}
		}

		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = u32SpeedHz;

		// Calling a midle level function for the single register with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchSingleReg(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			//Errorhandling. Not yet implemented
			intReturn = ptSpiDataFromSwitch->intRet;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			printf("Received register: %02X\n", ptSpiDataFromSwitch->u8RecRegisterData);
		}
		else
		{
			printf("Transmited register: %02X\n", ptSpiDataFromSwitch->u8TranRegisterData);
		}
	}
	//---------------------------------------------------------------------------------------------------
	// Switch. Static MAC address transfer 
	//---------------------------------------------------------------------------------------------------
	else if (u8Switch == 2)
	{
		printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                    Static MAC adress SPI-transfer to switch                       **\n");
		printf("***************************************************************************************\n");

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;
		tCmdBufferSW.bByteOrder = bByteOrder;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				//Errorhandling. Not yet implemented
				goto end;
			}
		}
		if (bArrayInvert == TRUE)
		{
			intReturn = intArrayInvertMAC(&au8TelTx[0]);
			if (intReturn < 0)
			{
				//Error
				//Errorhandling. Not yet implemented
				goto end;
			}
		}
		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = u32SpeedHz;

		// Calling a midle level function for the static MAC address with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchStaticMac(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			//Errorhandling. Not yet implemented
			intReturn = ptSpiDataFromSwitch->intRet;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			printf("IN MAC:");
			for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
			{
				printf(" %02X", ptSpiDataFromSwitch->au8RecData[i]);
			}
			printf("\n");
		}
		else
		{
			printf("OUT MAC:");
			for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
			{
				printf(" %02X", ptSpiDataFromSwitch->au8TranData[i]);
			}
			printf("\n");
		}
	}
	//---------------------------------------------------------------------------------------------------
	// Switch. VLAN address transfer 
	//---------------------------------------------------------------------------------------------------
	else if (u8Switch == 3)
	{
		printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                        VLAN adress SPI-transfer to switch                         **\n");
		printf("***************************************************************************************\n");

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;
		tCmdBufferSW.bByteOrder = bByteOrder;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				//Errorhandling. Not yet implemented
				goto end;
			}
		}
		
		if (bArrayInvert == TRUE)
		{
			intReturn = intArrayInvertVLAN(&au8TelTx[0]);
			if (intReturn < 0)
			{
				//Error
				//Errorhandling. Not yet implemented
				goto end;
			}
		}
		
		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = u32SpeedHz;

		// Calling a midle level function for the VLAN address with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchVLan(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			//Errorhandling. Not yet implemented
			intReturn = ptSpiDataFromSwitch->intRet;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			printf("IN VLAN:");
			for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
			{
				printf(" %02X", ptSpiDataFromSwitch->au8RecData[i]);
			}
			printf("\n");
		}
		else
		{
			printf("OUT VLAN:");
			for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
			{
				printf(" %02X", ptSpiDataFromSwitch->au8TranData[i]);
			}
			printf("\n");
		}
	}

end:
	free(ptSpiDataFromSwitch);

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
		c = getopt_long(argc, argv, "nIBipwrD:s:d:b:HOLCNA:l:t:f:S:",
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
			break;
		case 'A':
			u16AddrIdx = (uint16_t)atoi(optarg);
			break;
		case 'l':
			u8UserLength = (uint8_t)atoi(optarg);
			break;
		case 'w':
			bWizard = TRUE;
			break;
		case 'r':
			bReadOnly = TRUE;
			break;
		case 'p':
			bPrintBuf = TRUE;
			break;
		case 'i':
			bWaitIRQ = TRUE;
			break;
		case 'B':
			bByteOrder = TRUE;
			break;
		case 'I':
			bArrayInvert = TRUE;
			break;
		case 'n':
			bBinFile = TRUE;
			break;
		case 'f':
			input_file = optarg;
			break;
		case 'S':
			u8Switch = (uint8_t)atoi(optarg); 
			break;
		default:
			vPrint_help(argv[0]);
			break;
		}	
	}

	return 0;
}