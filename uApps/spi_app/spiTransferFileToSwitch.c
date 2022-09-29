//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTransferFileToSwitch.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  28.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - The high level application contains function for data import from the config file.
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// iFileTransferToSwitch()
//
///\brief			The interface for config file transfer to the switch.
//
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd;				 Device ID
///\param[in]				.bReadOnly;			 Read/Write Selection. 0 - Write, 1 - Read 
///\param[in]				.bByteOrder;			 The order or sequence of bytes. 1 - Bigendian, 0 - little endian
///\param[in]				.bArrayInvert;			 Static MAC or VLAN address invert
///\param[in]				.u32SpeedHz;			 Transfer speed [Hz]
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iFileTransferToSwitch(int intFd, char* filename, struct t_FileToSwitch* tFileToSwitch)
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
	bLineStart = TRUE;

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
	if (bytes != sb.st_size)
		perror("failed to read input file");

	printf("File size: %d\n", sb.st_size);

	printf("Char: %d\n", *(txC + iIdx));

	printf("Char1: %d\n", *(txC + 1));

	printf("Char2: %d\n", *(txC + 2));

	binHex = *(txC + 3);

	printf("Char3: %d\n", binHex);

	goto ende;

	//---------------------------------------------------------------------------------------------------
	// File read process
	//---------------------------------------------------------------------------------------------------
	while (iIdx < (sb.st_size))
	{
		ch = *(txC + iIdx);
		switch (ch)
		{
			case '#':
			{
				printf("COMMENT\n");
				bLineStart = FALSE;
			}
			break;
			case ' ':
			{
				printf("LEER\n");
			}
			break;
			case '	':
			{
				printf("TAB\n");
			}
			break;
			case '\n':
			{
				printf("END OF LINE\n");
				bLineStart = TRUE;
			}
			break;
			case EOF:
			{
				printf("END OF FILE\n");
			}
			break;
			default:
			{
				ptSpiDataFromSwitch = malloc(sizeof(struct t_SpiDebugSwitch));
				if (bLineStart == TRUE)
				{
					bLineStart = FALSE;
					if ((ch == '0') && (*(txC + iIdx + 1) == '|'))
					{
						//Single register
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						au8TelTx[0] |= (((intConvert(*(txC + iIdx + 6)) << 4)) + (intConvert(*(txC + iIdx + 7))));

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.bByteOrder = tFileToSwitch->bByteOrder;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchSingleReg(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling. Not yet implemented
							intReturn = ptSpiDataFromSwitch->intRet;
							//Error
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
						{
							printf("Received register: %02X\n", ptSpiDataFromSwitch->u8RecRegisterData);
						}
						else
						{
							printf("Transmited register: %02X\n", ptSpiDataFromSwitch->u8TranRegisterData);
						}
					}
					else if ((ch == '1') && (*(txC + iIdx + 1) == '|'))
					{
						//Static MAC address
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						for (int i = 0; i < 8; i++)
						{
							au8TelTx[i] |= (((intConvert(*(txC + iIdx + 6 + (i * 3))) << 4)) + (intConvert(*(txC + iIdx + 7 + (i * 3)))));
						}

						if (tFileToSwitch->bArrayInvert == TRUE)
						{
							intReturn = intArrayInvertMAC(&au8TelTx[0]);
							if (intReturn < 0)
							{
								//Error
								iIdx = (sb.st_size);
								break;
							}
						}

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.bByteOrder = tFileToSwitch->bByteOrder;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchStaticMac(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling. Not yet implemented
							intReturn = ptSpiDataFromSwitch->intRet;
							//Error
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
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
					else if ((ch == '2') && (*(txC + iIdx + 1) == '|'))
					{
						//VLAN address
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						for (int i = 0; i < 8; i++)
						{
							au8TelTx[i] |= (((intConvert(*(txC + iIdx + 6 + (i * 3))) << 4)) + (intConvert(*(txC + iIdx + 7 + (i * 3)))));
						}

						if (tFileToSwitch->bArrayInvert == TRUE)
						{
							intReturn = intArrayInvertVLAN(&au8TelTx[0]);
							if (intReturn < 0)
							{
								//Error
								iIdx = (sb.st_size);
								break;
							}
						}
	
						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.bByteOrder = tFileToSwitch->bByteOrder;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchVLan(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling. Not yet implemented
							intReturn = ptSpiDataFromSwitch->intRet;
							//Error
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
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
					else
					{
						//WRONG method
						printf("File wrong format: %d\n", iIdx);
						printf("File wrong format: %s\n", ch);
						//iIdx = (sb.st_size);
						//break;
					}
				}

				// Clearing the transmission buffer for the next line
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&au8TelRx, 0, sizeof(au8TelRx));
				free(ptSpiDataFromSwitch);
			}
			break;
		}
		iIdx++;
		printf("IDX: %d\n", iIdx);

	}//while (iIdx < (sb.st_size))

ende:
	printf("IDX: %d\n", iIdx);

	free(txC);
	close(tx_fd);

	return intReturn;
}