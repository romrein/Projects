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
//	- A low level application.
//  - 
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
// iWriteEEPROM()
//
///\brief			
//
///\param[in]			
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iWriteEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	BOOL	bReadWreitEnd = FALSE;

	int intSignatur = 0;
	int intReturn = 0;
	int intStatus = 0;

	uint8_t u8Step = 0;											// Read/write step
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)

	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	uint16_t u16IdxTest = 0;

	uint32_t u32FreqHz = 1000000;

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	u8Step = EEPROM_READ_STATUS;

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
				printf("\n\n");
				printf("*************************************************************************************************************\n");
				printf("**                                      EEPROMSTATUS                                                       **\n");
				printf("*************************************************************************************************************\n");

				// Calling a low level function with the parameter handover for the SPI transfer.
				intStatus = iSpiReadEEPROMStatus(intFd);
				//Display transfer buffer
				printf("Received Status [HEX]:\n");
				printf(" %02X", intStatus);
				intStatus = 0;
				memset(&au8TelRx, 0, sizeof(au8TelRx));
				printf("\n");
			}
			case EEPROM_WRITE_DATA_1:
			{

				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_WRITE_DATA_0;
				tTransferBuffer.u32Freq = u32FreqHz;
				tTransferBuffer.pu8DataOut = pu8DataOut;
				if (u8Length > 127)
				{
					tTransferBuffer.u8Length = 127;
				}
				else
				{
					tTransferBuffer.u8Length = (uint8_t)u8Length;
				}
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);

				if (u8Length > 127)
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


				au8TelTx[0] = *(pu8DataOut + 127);


				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_WRITE_DATA_127;
				tTransferBuffer.u32Freq = u32FreqHz;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = 1;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);
			}
			case EEPROM_WRITE_CTRL_DATA:
			{

				intSignatur = iGet_signature(pu8DataOut, (uint8_t)u8Length);

				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				au8TelTx[0] = intSignatur;
				u16AdrSwap = u16StartAddress;
				u16AdrSwap = (uint16_t)((u16AdrSwap >> 8) | (u16AdrSwap << 8));
				memcpy(&au8TelTx[1], &u16AdrSwap, 2);
				au8TelTx[3] = u8Length;

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_SIGNATURE;
				tTransferBuffer.u32Freq = u32FreqHz;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = 4;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);
			}
			case EEPROM_WRITE_CMD:
			{

				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				au8TelTx[0] = EEPROM_CMD_WRITE;

				tTransferBuffer.intFd = intFd;
				tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_CMD;
				tTransferBuffer.u32Freq = u32FreqHz;
				tTransferBuffer.pu8DataOut = &au8TelTx[0];
				tTransferBuffer.u8Length = 1;
				tTransferBuffer.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
				if (ptSpiBuffer->intRet < 0)
				{
					intReturn = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);

				u8Step = EEPROM_READ_WRITE_STATUS;

			}
			break;
			case EEPROM_READ_WRITE_STATUS:
			{

				usleep(EEPROM_WAIT_STATUS_US);

				printf("\n\n");
				printf("*************************************************************************************************************\n");
				printf("**                                      EEPROMSTATUS                                                       **\n");
				printf("*************************************************************************************************************\n");


				// Calling a low level function with the parameter handover for the SPI transfer.
				intStatus = iSpiReadEEPROMStatus(intFd);
				//Display transfer buffer
				printf("Received Status [HEX]:\n");
				printf(" %02X", intStatus);
				printf("\n");

				if ((intStatus != 0) && (u16IdxTest < 5))
				{
					if (((intStatus & (0x01)) != 0) && ((intStatus & (0x02)) == 0) && ((intStatus & (0x04)) == 0))
					{
						//Wait for counter
					}
					else if (((intStatus & (0x01)) == 0) && ((intStatus & (0x02)) != 0) && ((intStatus & (0x04)) == 0))
					{
						u8Step = EEPROM_WRITE_CMD;
					}
					u16IdxTest++;
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
///\brief			
//
///\param[in]			
//
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iReadEEPROM(int intFd, uint8_t* pu8DataOut, uint8_t u8Length, uint16_t u16StartAddress)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	BOOL	bReadWreitEnd = FALSE;

	int intSignatur = 0;
	int intReturn = 0;
	int intStatus = 0;

	uint8_t u8Step = 0;											// Read/write step
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX) 
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Input transmission buffer (RX)

	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	uint16_t u16IdxTest = 0;

	uint32_t u32FreqHz = 1000000;

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	u8Step = EEPROM_READ_STATUS;

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
			printf("\n\n");
			printf("*************************************************************************************************************\n");
			printf("**                                      EEPROMSTATUS                                                       **\n");
			printf("*************************************************************************************************************\n");

			// Calling a low level function with the parameter handover for the SPI transfer.
			intStatus = iSpiReadEEPROMStatus(intFd);
			//Display transfer buffer
			printf("Received Status [HEX]:\n");
			printf(" %02X", intStatus);
			intStatus = 0;
			memset(&au8TelRx, 0, sizeof(au8TelRx));
			printf("\n");
		}
		case EEPROM_WRITE_CTRL_DATA:
		{

			// Clearing the transmission buffer
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			u16AdrSwap = u16StartAddress;
			u16AdrSwap = (uint16_t)((u16AdrSwap >> 8) | (u16AdrSwap << 8));
			memcpy(&au8TelTx[0], &u16AdrSwap, 2);
			au8TelTx[2] = u8Length;

			tTransferBuffer.intFd = intFd;
			tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_BYTE_ADDRESS;
			tTransferBuffer.u32Freq = u32FreqHz;
			tTransferBuffer.pu8DataOut = &au8TelTx[0];
			tTransferBuffer.u8Length = 3;
			tTransferBuffer.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
			if (ptSpiBuffer->intRet < 0)
			{
				intReturn = ptSpiBuffer->intRet;
			}

			free(ptSpiBuffer);
		}
		case EEPROM_WRITE_CMD:
		{

			// Clearing the transmission buffer
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			au8TelTx[0] = EEPROM_CMD_READ;

			tTransferBuffer.intFd = intFd;
			tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_CMD;
			tTransferBuffer.u32Freq = u32FreqHz;
			tTransferBuffer.pu8DataOut = &au8TelTx[0];
			tTransferBuffer.u8Length = 1;
			tTransferBuffer.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
			if (ptSpiBuffer->intRet < 0)
			{
				intReturn = ptSpiBuffer->intRet;
			}

			free(ptSpiBuffer);

			u8Step = EEPROM_READ_WRITE_STATUS;

		}
		break;
		case EEPROM_READ_WRITE_STATUS:
		{

			usleep(10000);

			printf("\n\n");
			printf("*************************************************************************************************************\n");
			printf("**                                      EEPROMSTATUS                                                       **\n");
			printf("*************************************************************************************************************\n");


			// Calling a low level function with the parameter handover for the SPI transfer.
			intStatus = iSpiReadEEPROMStatus(intFd);
			//Display transfer buffer
			printf("Received Status [HEX]:\n");
			printf(" %02X", intStatus);
			printf("\n");

			if ((intStatus != 0) && (u16IdxTest < 1000))
			{
				u16IdxTest++;
			}
			else
			{
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
///\brief					The interface for SPI DAM read/write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROMStatus(int intFd)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intRet = 0;

	uint8_t u8Len = 1;								// Data length. Note: not the telegram length
	uint32_t u32FreqHz = 1000000;
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_STATUS;
	tTransferBuffer.u32Freq = u32FreqHz;
	tTransferBuffer.pu8DataOut = &au8TelTx[0];
	tTransferBuffer.u8Length = (uint8_t)1;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTransfer(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intRet = ptSpiBuffer->intRet;
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------

	return ptSpiBuffer->au8TelIn[0];

	free(ptSpiBuffer);
}

//------------------------------------------------------------------------------------------------
// iSpiReadEEPROM()
//
///\brief					The interface for SPI DAM read/write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
int iSpiReadEEPROM(int intFd, uint8_t* pu8DataOut)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intRet = 0;

	uint8_t u8Len = 127;							// Data length. Note: not the telegram length
	uint32_t u32FreqHz = 1000000;
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_READ_DATA_0;
	tTransferBuffer.u32Freq = u32FreqHz;
	tTransferBuffer.pu8DataOut = &au8TelTx[0];
	tTransferBuffer.u8Length = (uint8_t)127;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTransfer(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intRet = ptSpiBuffer->intRet;
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < EEPROM_PAGE_SIZE; i++)
	{
		*(pu8DataOut + i) = ptSpiBuffer->au8TelIn[i];
	}

	free(ptSpiBuffer);

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = EEPROM_CPLD_ADDR_READ_DATA_127;
	tTransferBuffer.u32Freq = u32FreqHz;
	tTransferBuffer.pu8DataOut = &au8TelTx[0];
	tTransferBuffer.u8Length = (uint8_t)1;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTransfer(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intRet = ptSpiBuffer->intRet;
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	*(pu8DataOut + EEPROM_PAGE_SIZE) = ptSpiBuffer->au8TelIn[0];

	free(ptSpiBuffer);

	return intRet;
}