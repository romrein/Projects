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
	uint8_t u8ch;													// Received character

	uint8_t binHex = 0;

	// Clearing the transmission buffer and set new line
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	bLineStart = TRUE;

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
	if (bytes != sb.st_size)
		printf("Error: failed to read input file\n");


	//---------------------------------------------------------------------------------------------------
	// File read process
	//---------------------------------------------------------------------------------------------------
	while (iIdx < (sb.st_size))
	{
		ch = *(txC + iIdx);

		if ( (*(txC + iIdx)) == 10 )
		{
			ch = '^';
		}
		if ((*(txC + iIdx)) == 13)
		{
			ch = '´';
		}
		switch (ch)
		{
			case '´':
			{
				bLineStart = TRUE;
			}
			break;
			case '^':
			{
				bLineStart = TRUE;
			}
			break;
			case '#':
			{
				bLineStart = FALSE;
			}
			break;
			case ' ':
			{
				//
			}
			break;
			case '	':
			{
				//
			}
			break;
			case '\n':
			{
				//
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
					if ( (ch == '0') && (*(txC + iIdx + 1) == '|') && (*(txC + iIdx + 5) == '|') )
					{
						//Single register
						printf("Switch single register:\n");
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						au8TelTx[0] |= (((intConvert(*(txC + iIdx + 6)) << 4)) + (intConvert(*(txC + iIdx + 7))));

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchSingleReg(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling
							intReturn = ptSpiDataFromSwitch->intRet;
							printf("Error: write single register from file to the switch\n");
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
						{
							vPrintoutLite(TRUE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->u8RecRegisterData), SWITCH_SINGLE_REG_LENGTH);
						}
						else
						{
							vPrintoutLite(FALSE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->u8TranRegisterData), SWITCH_SINGLE_REG_LENGTH);
						}
						printf("\n\n");
					}
					else if ( (ch == '1') && (*(txC + iIdx + 1) == '|') && (*(txC + iIdx + 5) == '|') && (*(txC + iIdx + 8) == '_') && (*(txC + iIdx + 11) == '_') && (*(txC + iIdx + 14) == '_') 
							&& (*(txC + iIdx + 17) == '_') && (*(txC + iIdx + 20) == '_') && (*(txC + iIdx + 23) == '_') && (*(txC + iIdx + 26) == '_')
						)
					{
						//Static MAC address
						printf("Switch static MAC address:\n");
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						for (int i = 0; i < 8; i++)
						{
							au8TelTx[i] |= (((intConvert(*(txC + iIdx + 6 + (i * 3))) << 4)) + (intConvert(*(txC + iIdx + 7 + (i * 3)))));
						}

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchStaticMac(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling
							intReturn = ptSpiDataFromSwitch->intRet;
							printf("Error: write MAC address from file to the switch\n");
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
						{
							vPrintoutLite(TRUE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8RecData), SWITCH_STATIC_MAC_LENGTH);
						}
						else
						{
							vPrintoutLite(FALSE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8TranData), SWITCH_STATIC_MAC_LENGTH);
						}
						printf("\n\n");
					}
					else if ( (ch == '2') && (*(txC + iIdx + 1) == '|') && (*(txC + iIdx + 5) == '|') && (*(txC + iIdx + 8) == '_') && (*(txC + iIdx + 11) == '_') && (*(txC + iIdx + 14) == '_') 
							&& (*(txC + iIdx + 17) == '_') && (*(txC + iIdx + 20) == '_') && (*(txC + iIdx + 23) == '_') && (*(txC + iIdx + 26) == '_')
							)
					{
						//VLAN address
						printf("Switch 8 Byte VLAN address:\n");
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						for (int i = 0; i < 8; i++)
						{
							au8TelTx[i] |= (((intConvert(*(txC + iIdx + 6 + (i * 3))) << 4)) + (intConvert(*(txC + iIdx + 7 + (i * 3)))));
						}

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchVLan8Byte(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling
							intReturn = ptSpiDataFromSwitch->intRet;
							printf("Error: write VLAN address from file to the switch\n");
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
						{
							vPrintoutLite(TRUE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8RecData), SWITCH_VLAN_LENGTH_8);
						}
						else
						{
							vPrintoutLite(FALSE, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8TranData), SWITCH_VLAN_LENGTH_8);
						}
						printf("\n\n");
					}
					else if ( (ch == '2') && (*(txC + iIdx + 1) == '|') && (*(txC + iIdx + 5) == '|') && (*(txC + iIdx + 8) == '_') )
					{
						//VLAN address
						printf("Switch 2 Byte VLAN address:\n");
						// Setting structure variables for the transfer
						tCmdFileToSW.u16AddrIdx = (intConvert(*(txC + iIdx + 2)) << 8);
						tCmdFileToSW.u16AddrIdx |= (((intConvert(*(txC + iIdx + 3)) << 4)) + (intConvert(*(txC + iIdx + 4))));

						for (int i = 0; i < 2; i++)
						{
							au8TelTx[i] |= (((intConvert(*(txC + iIdx + 6 + (i * 3))) << 4)) + (intConvert(*(txC + iIdx + 7 + (i * 3)))));
						}

						tCmdFileToSW.intFd = intFd;
						tCmdFileToSW.bReadOnly = tFileToSwitch->bReadOnly;
						tCmdFileToSW.pu8DataOut = &au8TelTx[0];
						tCmdFileToSW.u32SpeedHz = tFileToSwitch->u32SpeedHz;

						// Calling a midle level function with the parameter handover for the SPI transfer.
						ptSpiDataFromSwitch = ptSpiTransferSwitchVLan(&tCmdFileToSW);
						if (ptSpiDataFromSwitch->intRet < 0)
						{
							//Errorhandling
							intReturn = ptSpiDataFromSwitch->intRet;
							printf("Error: write VLAN address from file to the switch\n");
							iIdx = (sb.st_size);
							break;
						}

						// Display transmission buffer
						if (tCmdFileToSW.bReadOnly == TRUE)
						{
							vPrintoutVlan(TRUE, (tCmdFileToSW.u16AddrIdx)/4, (tCmdFileToSW.u16AddrIdx) % 4, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8RecData), SWITCH_VLAN_LENGTH_2);
						}
						else
						{
							vPrintoutVlan(FALSE, (tCmdFileToSW.u16AddrIdx)/4, (tCmdFileToSW.u16AddrIdx) % 4, tCmdFileToSW.u16AddrIdx);
							// Printout from buffer
							vPrintBufMulti(&(ptSpiDataFromSwitch->au8TranData), SWITCH_VLAN_LENGTH_2);
						}
						printf("\n\n");
					}
					else
					{
						//WRONG method
						printf("Error: file wrong format. Idx: %d, Char int: %d, Char hex: %x, Char ASCII: %c\n", iIdx, ch, ch, ch, ch);
						iIdx = (sb.st_size);
						break;
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

	}//while (iIdx < (sb.st_size))

ende:

	free(txC);
	close(tx_fd);

	return intReturn;
}