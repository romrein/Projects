//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiCPLDReadWriteEEPROM.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  06.08.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A midle level application.
//  - The functions for communication with the EEPROM via CPLD
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// iWriteEEPROM()
//
///\brief			Write data to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		pu8DataOut	- Output TX buffer
///\param[in]		u8Length	- Length of buffer
///\param[in]		u16StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iWriteEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	BOOL	bReadWreitEnd = FALSE;								// Trigger or output condition for the sequence

	int intSignatur = 0;										// Calculated CRC of the data to be transmitted
	int intReturn = 0;											// Return variable for the function
	int intStatus = 0;											// EEPROM status

	uint8_t u8Step = 0;											// Read/write step
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)

	uint16_t u16AdrSwap = 0;									// Temporal variable for address swapping
	uint16_t u16IdxTest = 0;									// Index variable for the status check

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// Start condition
	u8Step = EEPROM_READ_STATUS;

	// State machine for the Process
	while (bReadWreitEnd == FALSE)
	{

		// Read/Write state machine
		switch (u8Step)
		{
			//---------------------------------------------------------------------------------------------------
			// Standby
			//---------------------------------------------------------------------------------------------------
			case STANDBY:
			{
				//Idle
			}
			break;
			//---------------------------------------------------------------------------------------------------
			// Read/Write process 
			//---------------------------------------------------------------------------------------------------
			case EEPROM_READ_STATUS:
			{
				//printf("\n\n");
				//printf("*************************************************************************************************************\n");
				//printf("**                                      EEPROMSTATUS                                                       **\n");
				//printf("*************************************************************************************************************\n");

				// Calling a low level function with the parameter handover for read EEPROM status.
				intStatus = iSpiReadEEPROMStatus(intFd);
				if (intStatus < 0)
				{
					printf("Error: CPLD read EEPROM status\n");
					exit(1);
					break;
				}

				// Identification of the error sources from EEPROM
				if (intStatus != 0)
				{
					if (((intStatus & (EEPROM_BUSY)) != 0) && (u16IdxTest == 0))
					{
						//Wait for counter
						printf("EEPROM Status: busy processing USER CMDs. Wait for 100ms\n");
						u16IdxTest++;
						intStatus = 0;
						u8Step = EEPROM_READ_STATUS;
						usleep(EEPROM_WAIT_STATUS_US);
						break;
					}
					if ( (intStatus & (EEPROM_CRC_FAIL)) != 0 )
					{
						printf("EEPROM Status: CRC FAIL\n");
						exit(1);
						break;
					}
					if ( ((intStatus & (EEPROM_CMD_ERROR)) != 0) )
					{
						printf("EEPROM Status: CMD error\n");
						exit(1);
						break;
					}
					if (u16IdxTest != 0)
					{
						printf("EEPROM Status: Timeout\n");
						exit(1);
						break;
					}
				}
				else
				{
					intStatus = 0;
					memset(&au8TelRx, 0, sizeof(au8TelRx));
				}
			}
			case EEPROM_WRITE_DATA_1:
			{
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_WRITE_DATA_0;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = pu8DataOut;
				if (u8Length > CPLD_MAX_LEN)
				{
					tTransferBuffer.u8Length = CPLD_MAX_LEN;
				}
				else
				{
					tTransferBuffer.u8Length = (uint8_t)u8Length;
				}
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the single SPI transfer
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write data to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);

				if (u8Length > CPLD_MAX_LEN)
				{
					u8Step = EEPROM_WRITE_DATA_2;
				}
				else
				{
					u8Step = EEPROM_WRITE_CTRL_DATA;
					break;
				}
			}
			case EEPROM_WRITE_DATA_2:
			{
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				au8TelTx[0] = *(pu8DataOut + CPLD_MAX_LEN);

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_WRITE_DATA_127;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = BYTE;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the single SPI transfer
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write data to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);
			}
			case EEPROM_WRITE_CTRL_DATA:
			{
				// Calculation CRC of data
				intSignatur = iGet_signature(pu8DataOut, (uint8_t)u8Length);

				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				au8TelTx[0] = intSignatur;
				u16AdrSwap = u16StartAddress;
				u16AdrSwap = (uint16_t)((u16AdrSwap >> 8) | (u16AdrSwap << 8));
				memcpy(&au8TelTx[1], &u16AdrSwap, 2);
				au8TelTx[3] = u8Length;

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_SIGNATURE;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = L4_BYTE;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write control data to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);
			}
			case EEPROM_WRITE_CMD:
			{
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				au8TelTx[0] = EEPROM_CMD_WRITE;

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_CMD;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = BYTE;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the single SPI transfer
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write CMD to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);

				u16IdxTest = 0;
				u8Step = EEPROM_READ_WRITE_STATUS;

			}
			break;
			case EEPROM_READ_WRITE_STATUS:
			{
				usleep(EEPROM_WAIT_STATUS_US);

				//printf("\n\n");
				//printf("*************************************************************************************************************\n");
				//printf("**                                      EEPROMSTATUS                                                       **\n");
				//printf("*************************************************************************************************************\n");

				// Calling a low level function with the parameter handover for read EEPROM status.
				intStatus = iSpiReadEEPROMStatus(intFd);
				if (intStatus < 0)
				{
					printf("Error: CPLD read EEPROM status\n");
					exit(1);
					break;
				}

				// Identification of the error sources from EEPROM
				if (intStatus != 0)
				{
					if (((intStatus & (EEPROM_BUSY)) != 0) && (u16IdxTest == 0))
					{
						//Wait for counter
						printf("EEPROM Status: busy processing USER CMDs. Wait for 100ms\n");
						u16IdxTest++;
						u8Step = EEPROM_READ_WRITE_STATUS;
						intStatus = 0;
						break;
					}
					if ((intStatus & (EEPROM_CRC_FAIL)) != 0)
					{
						printf("EEPROM Status: CRC FAIL\n");
						exit(1);
						break;
					}
					if (((intStatus & (EEPROM_CMD_ERROR)) != 0))
					{
						printf("EEPROM Status: CMD error\n");
						exit(1);
						break;
					}
					if (u16IdxTest != 0)
					{
						printf("EEPROM Status: Timeout\n");
						exit(1);
						break;
					}
				}
				else
				{
					intReturn = intStatus;
					bReadWreitEnd = TRUE;
				}
			}
			break;
			default:
			{
				bReadWreitEnd = TRUE;
			}
			break;
		}//switch (u8Step)
	}

	return intReturn;
}

//------------------------------------------------------------------------------------------------
// iReadEEPROM()
//
///\brief			Read command to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		pu8DataOut	- Input RX buffer
///\param[in]		u8Length	- Length of buffer
///\param[in]		u16StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	BOOL	bReadWreitEnd = FALSE;								// Trigger or output condition for the sequence

	int intReturn = 0;											// Return variable for the function
	int intStatus = 0;											// EEPROM status

	uint8_t u8Step = 0;											// Read/write step
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)

	uint16_t u16AdrSwap = 0;									// Temporal variable for address swapping
	uint16_t u16IdxTest = 0;									// Index variable for the status check

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// Start condition
	u8Step = EEPROM_READ_STATUS;

	// State machine for the Process
	while (bReadWreitEnd == FALSE)
	{
		// Read/Write state machine
		switch (u8Step)
		{
			//---------------------------------------------------------------------------------------------------
			// Standby
			//---------------------------------------------------------------------------------------------------
			case STANDBY:
			{
				//Idle
			}
			break;
			//---------------------------------------------------------------------------------------------------
			// Read/Write process 
			//---------------------------------------------------------------------------------------------------
			case EEPROM_READ_STATUS:
			{
				//printf("\n\n");
				//printf("*************************************************************************************************************\n");
				//printf("**                                      EEPROMSTATUS                                                       **\n");
				//printf("*************************************************************************************************************\n");

				// Calling a low level function with the parameter handover for read EEPROM status.
				intStatus = iSpiReadEEPROMStatus(intFd);
				if (intStatus < 0)
				{
					printf("Error: CPLD read EEPROM status\n");
					exit(1);
					break;
				}

				// Identification of the error sources from EEPROM 
				if (intStatus != 0)
				{
					if (((intStatus & (EEPROM_BUSY)) != 0) && (u16IdxTest == 0))
					{
						//Wait for counter
						printf("EEPROM Status: busy processing USER CMDs. Wait for 100ms\n");
						u16IdxTest++;
						intStatus = 0;
						usleep(EEPROM_WAIT_STATUS_US);
						u8Step = EEPROM_READ_STATUS;
						break;
					}
					if ((intStatus & (EEPROM_CRC_FAIL)) != 0)
					{
						printf("EEPROM Status: CRC FAIL\n");
						exit(1);
						break;
					}
					if (((intStatus & (EEPROM_CMD_ERROR)) != 0))
					{
						printf("EEPROM Status: CMD error\n");
						exit(1);
						break;
					}
					if (u16IdxTest != 0)
					{
						printf("EEPROM Status: Timeout\n");
						exit(1);
						break;
					}
				}
				else
				{
					intStatus = 0;
					memset(&au8TelRx, 0, sizeof(au8TelRx));
				}
			}
			case EEPROM_WRITE_CTRL_DATA:
			{
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				u16AdrSwap = u16StartAddress;
				u16AdrSwap = (uint16_t)((u16AdrSwap >> 8) | (u16AdrSwap << 8));
				memcpy(&au8TelTx[0], &u16AdrSwap, 2);
				au8TelTx[2] = u8Length;

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_BYTE_ADDRESS;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = L3_BYTE;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the single SPI transfer
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write control data to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);
			}
			case EEPROM_WRITE_CMD:
			{
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				// Build of the transfer buffer up according to the transfer protocol
				au8TelTx[0] = EEPROM_CMD_READ;
				
				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_CMD;
				tTransferBuffer.u32Freq = SPI_SPEED;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = BYTE;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the single SPI transfer
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
					printf("Error: CPLD write CMD to EEPROM\n");
					exit(1);
					break;
				}

				free(ptSpiBuffer);

				u16IdxTest = 0;

				u8Step = EEPROM_READ_WRITE_STATUS;
			}
			break;
			case EEPROM_READ_WRITE_STATUS:
			{
				usleep(EEPROM_WAIT_STATUS_US);

				//printf("\n\n");
				//printf("*************************************************************************************************************\n");
				//printf("**                                      EEPROMSTATUS                                                       **\n");
				//printf("*************************************************************************************************************\n");

				// Calling a low level function with the parameter handover for read EEPROM status.
				intStatus = iSpiReadEEPROMStatus(intFd);
				if (intStatus < 0)
				{
					printf("Error: CPLD read EEPROM status\n");
					exit(1);
					break;
				}

				// Identification of the error sources from EEPROM
				if (intStatus != 0)
				{
					if (((intStatus & (EEPROM_BUSY)) != 0) && (u16IdxTest == 0))
					{
						//Wait for counter
						printf("EEPROM Status: busy processing USER CMDs. Wait for 100ms\n");
						u16IdxTest++;
						intStatus = 0;
						u8Step = EEPROM_READ_WRITE_STATUS;
						break;
					}
					if ((intStatus & (EEPROM_CRC_FAIL)) != 0)
					{
						printf("EEPROM Status: CRC FAIL\n");
						exit(1);
						break;
					}
					if (((intStatus & (EEPROM_CMD_ERROR)) != 0))
					{
						printf("EEPROM Status: CMD error\n");
						exit(1);
						break;
					}
					if (u16IdxTest != 0)
					{
						printf("EEPROM Status: Timeout\n");
						exit(1);
						break;
					}
				}
				else
				{
					intReturn = intStatus;
					bReadWreitEnd = TRUE;
				}
			}
			break;
			default:
			{
				bReadWreitEnd = TRUE;
			}
			break;
		}//switch (u8Step)
	}

	return intReturn;
}

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROMStatus()
//
///\brief					Read EEPROM status
//
///\param[in]				intFd		- Device ID
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROMStatus(int intFd)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intRet = 0;

	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	// Build of the transfer buffer up according to the transfer protocol
	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_STATUS;
	tTransferBuffer.u32Freq = SPI_SPEED;
	tTransferBuffer.pu8DataOut = &au8TelTx[0];
	tTransferBuffer.pu8DataIn = &au8TelRx[0];
	tTransferBuffer.u8Length = BYTE;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the single SPI transfer
	ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intRet = ptSpiBuffer->intRet;
		printf("Error: CPLD read from EEPROM\n");
		exit(1);
	}

	free(ptSpiBuffer);
	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------

	return au8TelRx[0];
}

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROM()
//
///\brief					Read data from EEPROM via CPLD and SPI DMA transfer
//
///\param[in]				intFd		- Device ID
///\param[in]				
///\param[out]				pu8DataOut	- Input RX buffer
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intRet = 0;

	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	if (u8Length > CPLD_MAX_LEN)
	{
		// Build of the transfer buffer up according to the transfer protocol
		tTransferBuffer.intFd = intFd;
		tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_READ_DATA_0;
		tTransferBuffer.u32Freq = SPI_SPEED;
		tTransferBuffer.pu8DataOut = &au8TelTx[0];
		tTransferBuffer.pu8DataIn = &au8TelRx[0];
		tTransferBuffer.u8Length = CPLD_MAX_LEN;
		tTransferBuffer.bRead = TRUE;

		// Calling a low level function with the parameter handover for the single SPI transfer
		ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
		if (ptSpiBuffer->intRet < 0)
		{
			intRet = ptSpiBuffer->intRet;
			printf("Error: CPLD read data from EEPROM\n");
			exit(1);
		}

		//---------------------------------------------------------------------------------------------------
		// Return data for calling function. Echo function
		//---------------------------------------------------------------------------------------------------
		for (int i = 0; i < CPLD_MAX_LEN; i++)
		{
			*(pu8DataOut + i) = au8TelRx[i];
		}

		free(ptSpiBuffer);

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));
		memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

		ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

		// Build of the transfer buffer up according to the transfer protocol
		tTransferBuffer.intFd = intFd;
		tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_READ_DATA_127;
		tTransferBuffer.u32Freq = SPI_SPEED;
		tTransferBuffer.pu8DataOut = &au8TelTx[0];
		tTransferBuffer.pu8DataIn = &au8TelRx[0];
		tTransferBuffer.u8Length = BYTE;
		tTransferBuffer.bRead = TRUE;

		// Calling a low level function with the parameter handover for the single SPI transfer
		ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
		if (ptSpiBuffer->intRet < 0)
		{
			intRet = ptSpiBuffer->intRet;
			printf("Error: CPLD read data from EEPROM\n");
			exit(1);
		}

		//---------------------------------------------------------------------------------------------------
		// Return data for calling function. Echo function
		*(pu8DataOut + CPLD_MAX_LEN) = au8TelRx[0];

		free(ptSpiBuffer);
	}
	else
	{
		// Build of the transfer buffer up according to the transfer protocol
		tTransferBuffer.intFd = intFd;
		tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_READ_DATA_0;
		tTransferBuffer.u32Freq = SPI_SPEED;
		tTransferBuffer.pu8DataOut = &au8TelTx[0];
		tTransferBuffer.pu8DataIn = &au8TelRx[0];
		tTransferBuffer.u8Length = u8Length;
		tTransferBuffer.bRead = TRUE;

		// Calling a low level function with the parameter handover for the single SPI transfer
		ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
		if (ptSpiBuffer->intRet < 0)
		{
			intRet = ptSpiBuffer->intRet;
			printf("Error: CPLD read data from EEPROM\n");
			exit(1);
		}

		//---------------------------------------------------------------------------------------------------
		// Return data for calling function. Echo function
		//---------------------------------------------------------------------------------------------------
		for (int i = 0; i < u8Length; i++)
		{
			*(pu8DataOut + i) = au8TelRx[i];
		}

		free(ptSpiBuffer);
	}
	
	return intRet;
}

//------------------------------------------------------------------------------------------------
// iReadWriteEEPROMData()
//
///\brief			Write data to EEPROM via CPLD and SPI DMA transfer			
//
///\param[in]		intFd		- Device ID
///\param[in]		bReadOnly	- Read/Write
///\param[in]		pu8DataOut	- Output TX buffer
///\param[in]		pu8DataIn	- Output RX buffer
///\param[in]		u32Length	- Length of buffer
///\param[in]		u32StartAddress	- Start address
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadWriteEEPROMData(int intFd, BOOL bReadOnly, uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint32_t u32Length, uint32_t u32StartAddress)
{
	static struct t_SpiDebug* ptSpiBuffer;

	int intReturn = 0;											// Return variable for the function

	uint8_t* auWriteBuf;
	uint8_t* auReadBuf;

	uint8_t u8Idx = 0;
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)
	uint8_t u8ReadWrite = 0;

	uint16_t u16Addr = 0;
	uint16_t u16Adresse = 0;
	uint16_t u16Zyklus = 0;
	uint16_t u16StartAddrIdx = 0;

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));


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

			if ((u32Length + u32StartAddress) > EEPROM_MAX_ADDRESS)
			{
				printf("Error: the address plus data length is out of range\n");
				return -1;
			}

			auWriteBuf = malloc(u32Length);
			if (!auWriteBuf)
			{
				printf("Error: can't allocate write buffer\n");
				return -1;
			}

			intReturn = memcpy(auWriteBuf, pu8DataOut, u32Length);
			if (intReturn == NULL)
			{
				printf("Error: memory copy failed\n");
				printf("intReturn: %d\n", intReturn);
				goto endWriteLine;
			}

			u16StartAddrIdx = (uint16_t)u32StartAddress;									// Start address for a data write

			u16Adresse = u16StartAddrIdx / EEPROM_PAGE_SIZE;

			// Start condition
			u16Addr = u16StartAddrIdx;
			u8Idx = 0;

			// Splintered page transfer. The data is transferred page by page depending on the start address in EEPROM
			for (int i = 0; i < (u32Length); i++)
			{
				if (u16Addr < ((u16Adresse + 1) * EEPROM_PAGE_SIZE))		// End of page or the page border
				{
					au8TelTx[u8Idx] = *(auWriteBuf + i);						// Ring buffer
					u8Idx = (u8Idx + 1) & 0x7F;								// Index for a ring buffer. 0x7F - Page size. (0..127)
				}
				else
				{
					// Special case. The page limit has been reachedand and ring buffer is ful
					if (u8Idx == 0)
					{
						// Calling a midle level function with the parameter handover for write to EEPROM
						intReturn = iWriteEEPROM(intFd, &au8TelTx[0], EEPROM_PAGE_SIZE, u16StartAddrIdx);
						if (intReturn < 0)
						{
							printf("Error: write to EEPROM\n");
							goto endWriteLine;
						}
					}
					// The page limit has been reachedand but ring buffer is not ful
					else
					{
						// Calling a midle level function with the parameter handover for write to EEPROM
						intReturn = iWriteEEPROM(intFd, &au8TelTx[0], u8Idx, u16StartAddrIdx);
						if (intReturn < 0)
						{
							printf("Error: write to EEPROM\n");
							goto endWriteLine;
						}
					}
					u16StartAddrIdx = ((u16Adresse + 1) * EEPROM_PAGE_SIZE);
					memset(&au8TelTx, 0, sizeof(au8TelTx));
					u8Idx = 0;

					au8TelTx[u8Idx] = *(auWriteBuf + i);
					u8Idx = (u8Idx + 1) & 0x7F;

					u16Adresse++;
				}
				u16Addr++;
			}
			// Special case. The page limit has been reachedand and ring buffer is ful
			if (u8Idx == 0)
			{
				// Calling a midle level function with the parameter handover for write to EEPROM
				intReturn = iWriteEEPROM(intFd, &au8TelTx[0], EEPROM_PAGE_SIZE, u16StartAddrIdx);
				if (intReturn < 0)
				{
					printf("Error: write to EEPROM\n");
					goto endWriteLine;
				}
			}
			// The page limit has been reachedand but ring buffer is not ful
			else
			{
				// Calling a midle level function with the parameter handover for write to EEPROM
				intReturn = iWriteEEPROM(intFd, &au8TelTx[0], u8Idx, u16StartAddrIdx);
				if (intReturn < 0)
				{
					printf("Error: write to EEPROM\n");
					goto endWriteLine;
				}
			}
			// End of page transfer
			// VALIDATION ->
		}
		//---------------------------------------------------------------------------------------------------
		// EEPROM DATA READ  
		//---------------------------------------------------------------------------------------------------
		case EEPROM_MAIN_READ:
		{
			// Clearing the transmission buffer
			memset(&au8TelRx, 0, sizeof(au8TelRx));

			if ((u32Length + u32StartAddress) > EEPROM_MAX_ADDRESS)
			{
				printf("Error: the address plus data length is out of range\n");
				return -1;
			}

			auReadBuf = malloc(u32Length);
			if (!auReadBuf)
			{
				printf("Error: can't allocate read buffer\n");
				intReturn = -1;
				goto endReadLine;
			}

			// Read mamory page from EEPROM
			u8Idx = 1;
			u16Zyklus = 0;
			u16StartAddrIdx = (uint16_t)u32StartAddress;

			for (int i = 0; i < u32Length; i++)
			{
				if (u8Idx >= EEPROM_PAGE_SIZE)
				{
					u8Idx = 0;
					// Clearing the transmission buffer
					memset(&au8TelRx, 0, sizeof(au8TelRx));
					// Calling a midle level function with the parameter handover for read EEPROM
					intReturn = iReadEEPROM(intFd, &au8TelRx[0], EEPROM_PAGE_SIZE, u16StartAddrIdx);
					if (intReturn != 0)
					{
						printf("Error: read from EEPROM\n");
						goto endReadLine;
					}
					else
					{
						// Clearing the transmission buffer
						memset(&au8TelRx, 0, sizeof(au8TelRx));

						// Calling a midle level function with the parameter handover for read data from EEPROM
						intReturn = iSpiReadEEPROM(intFd, &au8TelRx, EEPROM_PAGE_SIZE);
						if (intReturn != 0)
						{
							printf("Unknown errors\n");
							goto endReadLine;
						}
						else
						{
							// Return data for calling function. Echo function
							for (int idx = 0; idx < (EEPROM_PAGE_SIZE); idx++)
							{
								*(auReadBuf + idx + (EEPROM_PAGE_SIZE * u16Zyklus)) = au8TelRx[idx];
							}
							u16StartAddrIdx += EEPROM_PAGE_SIZE;
							u16Zyklus++;
						}
					}
				}
				u8Idx++;
			}
			// Special case. The page limit has been reachedand and ring buffer is ful
			if (u8Idx != 0)
			{
				// Clearing the transmission buffer
				memset(&au8TelRx, 0, sizeof(au8TelRx));
				// Calling a midle level function with the parameter handover for read EEPROM
				intReturn = iReadEEPROM(intFd, &au8TelRx[0], (uint8_t)u8Idx, u16StartAddrIdx);
				if (intReturn != 0)
				{
					printf("Error: read from EEPROM\n");
					goto endReadLine;
				}
				else
				{
					// Clearing the transmission buffer
					memset(&au8TelRx, 0, sizeof(au8TelRx));

					// Calling a midle level function with the parameter handover for read data from EEPROM
					intReturn = iSpiReadEEPROM(intFd, &au8TelRx, (uint8_t)u8Idx);
					if (intReturn != 0)
					{
						printf("Unknown errors\n");
						goto endReadLine;
					}
					else
					{
						// Return data for calling function. Echo function
						for (int idx = 0; idx < (u8Idx); idx++)
						{
							*(auReadBuf + idx + (EEPROM_PAGE_SIZE * u16Zyklus)) = au8TelRx[idx];
						}
					}
				}
			}

			intReturn = memcpy(pu8DataIn, auReadBuf, u32Length);
			if (intReturn == NULL)
			{
				printf("Error: memory copy failed\n");
				goto endReadLine;
			}

			if (bReadOnly == FALSE)
			{
				// Printout from buffer
				vPrintout(FALSE, u32StartAddress, u32Length);
				vPrintBufMulti(auWriteBuf, u32Length);
				//VALIDATION:
				intReturn = memcmp(auReadBuf, auWriteBuf, u32Length);
				if (intReturn != 0)
				{
					printf("Error: transfer validation failed\n");
					goto endReadLine;
				}
				free(auWriteBuf);
			}
			if (bReadOnly == TRUE)
			{
				// Printout from buffer
				vPrintout(TRUE, u32StartAddress, u32Length);
				vPrintBufMulti(auReadBuf, u32Length);
				printf("\n\n");
			}

			free(auReadBuf);

			return intReturn;
		}
		break;
		default:
			break;
	}

	return 0;

endWriteLine:
	free(auWriteBuf);
	return intReturn;

endReadLine:
	free(auReadBuf);
	if (bReadOnly == FALSE)
	{
		free(auWriteBuf);
	}
	return intReturn;
}