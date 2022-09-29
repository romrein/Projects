//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWriteCPLD.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A low level application.
//  - An interface to the SPI DMA kernel driver for a data transfer between user and kernel space.
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelSingle()
//
///\brief					The interface for SPI DMA read/write single transfer to CPLD.
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
struct t_SpiDebug* ptSpiReadWriteTelSingle(struct t_SpiTransfer* ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));
	if (!ptSpiTel)
	{
		printf("Error: can't allocate memory buffer\n");
		printf("NULL pointer in %s\n", __func__);
		ptSpiTel->intRet = -1;
		goto end;
	}

	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer TX buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer RX buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	if ( (ptTransferBlock->u8Length == 0) || (ptTransferBlock->u16Length > CPLD_MAX_LEN) )
	{
		//Wrong length
		printf("Error: wrong data length\n");
		ptSpiTel->intRet = -1;
		goto end;
	}

	// Telegram build up according to the transfer protocol
	u8Len = (uint8_t)(ptTransferBlock->u8Length << 1);
	if (ptTransferBlock->bRead == TRUE)
	{
		u8Len |= READ;
	}
	u16AdrSwap = (uint16_t)((ptTransferBlock->u16Address >> 8) | (ptTransferBlock->u16Address << 8));
	memcpy(&au8TelTx[0], &u16AdrSwap, 2);
	memcpy(&au8TelTx[2], &u8Len, 1);
	for (int i = 0; i < (ptTransferBlock->u8Length); i++)
	{
		au8TelTx[i + CPLD_DATA_OFFSET] = *(ptTransferBlock->pu8DataOut + i);
	}

	// Setting structure variables for the SPI DMA transfer
	tTransfer.tx_buf = (unsigned long)au8TelTx;
	tTransfer.rx_buf = (unsigned long)au8TelRx;
	tTransfer.len = (uint32_t)(ptTransferBlock->u8Length + CPLD_DATA_OFFSET);
	tTransfer.delay_usecs = SPI_DELAY;
	tTransfer.speed_hz = ptTransferBlock->u32Freq;
	tTransfer.bits_per_word = SPI_BITS;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		printf("Error: read/write - driver ioctl - ioctl\n");
		goto end;
	}

	// Transfer validation
	if (ptTransferBlock->bRead == FALSE)
	{
		// Telegram build up according to the transfer protocol
		u8Len = (uint8_t)(ptTransferBlock->u8Length << 1);
		u8Len |= READ;
		u16AdrSwap = (uint16_t)((ptTransferBlock->u16Address >> 8) | (ptTransferBlock->u16Address << 8));
		memcpy(&au8TelTx[0], &u16AdrSwap, 2);
		memcpy(&au8TelTx[2], &u8Len, 1);
		for (int i = 0; i < (ptTransferBlock->u8Length); i++)
		{
			au8TelTx[i + CPLD_DATA_OFFSET] = *(ptTransferBlock->pu8DataOut + i);
		}

		// Setting structure variables for the SPI DMA transfer
		tTransfer.tx_buf = (unsigned long)au8TelTx;
		tTransfer.rx_buf = (unsigned long)au8TelRx;
		tTransfer.len = (uint32_t)(ptTransferBlock->u8Length + CPLD_DATA_OFFSET);
		tTransfer.delay_usecs = SPI_DELAY;
		tTransfer.speed_hz = ptTransferBlock->u32Freq;
		tTransfer.bits_per_word = SPI_BITS;

		// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
		ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

		if (ptSpiTel->intRet < 0)
		{
			printf("Error: read/write - driver ioctl - ioctl\n");
			goto end;
		}

		for (int i = CPLD_DATA_OFFSET; i < (ptTransferBlock->u8Length + CPLD_DATA_OFFSET); i++)
		{
			if (au8TelTx[i] != au8TelRx[i])
			{
				printf("Error: transfer validation failed\n\n");
				ptSpiTel->intRet = -1;
				goto end;
			}
		}
	}// End of validation

	//---------------------------------------------------------------------------------------------------
	// Return compliete transfer data buffer for calling function. Echo function for debugging
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (ptTransferBlock->u8Length + CPLD_DATA_OFFSET); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = au8TelRx[i];
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	if (ptTransferBlock->bRead == TRUE)
	{
		for (int i = 0; i < (ptTransferBlock->u8Length); i++)
		{
			*(ptTransferBlock->pu8DataIn + i) = au8TelRx[i + 3];
		}
	}

end:
	return ptSpiTel;
}

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelMulti()
//
///\brief					The interface for SPI DMA read/write multiline transfer to CPLD.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[out]				.pu8DataIn	- Input RX buffer
//		
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTelMulti(struct t_SpiTransfer* ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));
	if (!ptSpiTel)
	{
		printf("Error: can't allocate memory buffer\n");
		printf("NULL pointer in %s\n", __func__);
		ptSpiTel->intRet = -1;
		goto end;
	}
		
	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t u8Idx = 0;								// Index for the ring buffer
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer TX buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer RX buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	uint16_t u16StartAddrIdx = 0;					// Calculated start address for each transfer
	uint16_t u16Zyklus = 0;							// Cycle counter for the splintered transfer

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	if ( (ptTransferBlock->u16Length + ptTransferBlock->u16Address) > CPLD_MAX_ADDRESS)
	{
		printf("Error: the address plus data length is out of range\n");
		ptSpiTel->intRet = -1;
		goto end;
	}

	if (ptTransferBlock->u16Length == 0)
	{
		//Wrong length
		printf("Error: wrong data length\n");
		ptSpiTel->intRet = -1;
		goto end;
	}

	// Start condition
	u16StartAddrIdx = ptTransferBlock->u16Address;
	u16Zyklus = 0;

	// Splintered transfer
	for (int i = 0; i < (ptTransferBlock->u16Length); i++)
	{
		au8TelTx[u8Idx + CPLD_DATA_OFFSET] = *(ptTransferBlock->pu8DataOut + i);
		u8Idx = (u8Idx + 1) & 0x7F;
		if (u8Idx >= CPLD_MAX_LEN)
		{
			u8Idx = (u8Idx + 1) & 0x7F;
			// Telegram build up according to the transfer protocol
			u8Len = (uint8_t)(CPLD_MAX_LEN << 1);
			if (ptTransferBlock->bRead == TRUE)
			{
				u8Len |= READ;
			}
			u16AdrSwap = (uint16_t)((u16StartAddrIdx >> 8) | (u16StartAddrIdx << 8));
			memcpy(&au8TelTx[0], &u16AdrSwap, 2);
			memcpy(&au8TelTx[2], &u8Len, 1);

			// Setting structure variables for the SPI DMA transfer
			tTransfer.tx_buf = (unsigned long)au8TelTx;
			tTransfer.rx_buf = (unsigned long)au8TelRx;
			tTransfer.len = (uint32_t)(CPLD_MAX_LEN + CPLD_DATA_OFFSET);
			tTransfer.delay_usecs = SPI_DELAY;
			tTransfer.speed_hz = ptTransferBlock->u32Freq;
			tTransfer.bits_per_word = SPI_BITS;

			// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
			ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

			if (ptSpiTel->intRet < 0)
			{
				printf("Error: read/write - driver ioctl - ioctl\n");
				goto end;
			}

			// Transfer validation
			if (ptTransferBlock->bRead == FALSE)
			{
				// Telegram build up according to the transfer protocol
				u8Len = (uint8_t)(CPLD_MAX_LEN << 1);
				u8Len |= READ;
				u16AdrSwap = (uint16_t)((u16StartAddrIdx >> 8) | (u16StartAddrIdx << 8));
				memcpy(&au8TelTx[0], &u16AdrSwap, 2);
				memcpy(&au8TelTx[2], &u8Len, 1);

				// Setting structure variables for the SPI DMA transfer
				tTransfer.tx_buf = (unsigned long)au8TelTx;
				tTransfer.rx_buf = (unsigned long)au8TelRx;
				tTransfer.len = (uint32_t)(CPLD_MAX_LEN + CPLD_DATA_OFFSET);
				tTransfer.delay_usecs = SPI_DELAY;
				tTransfer.speed_hz = ptTransferBlock->u32Freq;
				tTransfer.bits_per_word = SPI_BITS;

				// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
				ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

				if (ptSpiTel->intRet < 0)
				{
					printf("Error: read/write - driver ioctl - ioctl\n");
					goto end;
				}

				for (int i = CPLD_DATA_OFFSET; i < (CPLD_MAX_LEN + CPLD_DATA_OFFSET); i++)
				{
					if (au8TelTx[i] != au8TelRx[i])
					{
						printf("Error: transfer validation failed\n\n");
						ptSpiTel->intRet = -1;
						goto end;
					}
				}
			}// End of validation

			// Return data for calling function. Echo function
			for (int idx = 0; idx < (CPLD_MAX_LEN); idx++)
			{
				*(ptTransferBlock->pu8DataIn + idx + (CPLD_MAX_LEN * u16Zyklus)) = au8TelRx[idx + CPLD_DATA_OFFSET];
			}
			u16StartAddrIdx += CPLD_MAX_LEN;
			u16Zyklus++;
		}
	}
	if (u8Idx > 0)
	{
		// Telegram build up according to the transfer protocol
		u8Len = (uint8_t)(u8Idx << 1);
		if (ptTransferBlock->bRead == TRUE)
		{
			u8Len |= READ;
		}
		u16AdrSwap = (uint16_t)((u16StartAddrIdx >> 8) | (u16StartAddrIdx << 8));
		memcpy(&au8TelTx[0], &u16AdrSwap, 2);
		memcpy(&au8TelTx[2], &u8Len, 1);

		// Setting structure variables for the SPI DMA transfer
		tTransfer.tx_buf = (unsigned long)au8TelTx;
		tTransfer.rx_buf = (unsigned long)au8TelRx;
		tTransfer.len = (uint32_t)(u8Idx + CPLD_DATA_OFFSET);
		tTransfer.delay_usecs = SPI_DELAY;
		tTransfer.speed_hz = ptTransferBlock->u32Freq;
		tTransfer.bits_per_word = SPI_BITS;

		// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
		ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

		if (ptSpiTel->intRet < 0)
		{
			printf("Error: read/write - driver ioctl - ioctl\n");
			goto end;
		}

		// Transfer validation
		if (ptTransferBlock->bRead == FALSE)
		{
			u8Len = (uint8_t)(u8Idx << 1);
			u8Len |= READ;
			u16AdrSwap = (uint16_t)((u16StartAddrIdx >> 8) | (u16StartAddrIdx << 8));
			memcpy(&au8TelTx[0], &u16AdrSwap, 2);
			memcpy(&au8TelTx[2], &u8Len, 1);

			// Setting structure variables for the SPI DMA transfer
			tTransfer.tx_buf = (unsigned long)au8TelTx;
			tTransfer.rx_buf = (unsigned long)au8TelRx;
			tTransfer.len = (uint32_t)(u8Idx + CPLD_DATA_OFFSET);
			tTransfer.delay_usecs = SPI_DELAY;
			tTransfer.speed_hz = ptTransferBlock->u32Freq;
			tTransfer.bits_per_word = SPI_BITS;

			// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
			ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

			if (ptSpiTel->intRet < 0)
			{
				printf("Error: read/write - driver ioctl - ioctl\n");
				goto end;
			}

			for (int i = CPLD_DATA_OFFSET; i < (u8Idx + CPLD_DATA_OFFSET); i++)
			{
				if (au8TelTx[i] != au8TelRx[i])
				{
					printf("Error: transfer validation failed\n\n");
					ptSpiTel->intRet = -1;
					goto end;
				}
			}
		}// End of validation

		// Return data for calling function. Echo function
		for (int idx = 0; idx < (u8Idx); idx++)
		{
			*(ptTransferBlock->pu8DataIn + idx + (CPLD_MAX_LEN * u16Zyklus)) = au8TelRx[idx + CPLD_DATA_OFFSET];
		}
	}

end:
	return ptSpiTel;
}

//------------------------------------------------------------------------------------------------
// iSwitchON()
//
///\brief				Function to activate the switch in the CPLD
//
///\param[in]			intFd		- Device ID
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSwitchON(int intFd)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intReturn = 0;
	int intSwitchCPLD = 0;
	int intSwitchIn = 0;

	// Clearing the transmission buffer
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	intSwitchCPLD = SWITCH_ENABLE;

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = SWITCH_CPLD_ADDR;
	tTransferBuffer.u32Freq = SPI_SPEED;
	tTransferBuffer.pu8DataOut = &intSwitchCPLD;
	tTransferBuffer.pu8DataIn = &intSwitchIn;
	tTransferBuffer.u8Length = (uint8_t)BYTE;
	tTransferBuffer.bRead = FALSE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intReturn = ptSpiBuffer->intRet;
	}

	free(ptSpiBuffer);

	//READ. Validate

	// Clearing the transmission buffer
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	intSwitchCPLD = SWITCH_ENABLE;

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = SWITCH_CPLD_ADDR;
	tTransferBuffer.u32Freq = SPI_SPEED;
	tTransferBuffer.pu8DataOut = &intSwitchCPLD;
	tTransferBuffer.pu8DataIn = &intSwitchIn;
	tTransferBuffer.u8Length = (uint8_t)BYTE;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intReturn = ptSpiBuffer->intRet;
	}

	if (ptSpiBuffer->au8TelIn[CPLD_DATA_OFFSET] == SWITCH_ENABLE)
	{
		intReturn = 0;
	}
	else
	{
		intReturn = -1;
	}

	free(ptSpiBuffer);

	return intReturn;
}

//------------------------------------------------------------------------------------------------
// iSwitchOFF()
//
///\brief				Function to deactivate the switch in the CPLD
//
///\param[in]			intFd		- Device ID
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iSwitchOFF(int intFd)
{
	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intReturn = 0;
	int intSwitchCPLD = 0;
	int intSwitchIn = 0;

	// Clearing the transmission buffer
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	intSwitchCPLD = SWITCH_DISABLE;

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = SWITCH_CPLD_ADDR;
	tTransferBuffer.u32Freq = SPI_SPEED;
	tTransferBuffer.pu8DataOut = &intSwitchCPLD;
	tTransferBuffer.pu8DataIn = &intSwitchIn;
	tTransferBuffer.u8Length = (uint8_t)BYTE;
	tTransferBuffer.bRead = FALSE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intReturn = ptSpiBuffer->intRet;
	}

	free(ptSpiBuffer);

	//READ. Validate

	// Clearing the transmission buffer
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	intSwitchCPLD = SWITCH_ENABLE;

	tTransferBuffer.intFd = intFd;
	tTransferBuffer.u16Address = SWITCH_CPLD_ADDR;
	tTransferBuffer.u32Freq = SPI_SPEED;
	tTransferBuffer.pu8DataOut = &intSwitchCPLD;
	tTransferBuffer.pu8DataIn = &intSwitchIn;
	tTransferBuffer.u8Length = (uint8_t)BYTE;
	tTransferBuffer.bRead = TRUE;

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteTelSingle(&tTransferBuffer);
	if (ptSpiBuffer->intRet < 0)
	{
		intReturn = ptSpiBuffer->intRet;
	}

	if (ptSpiBuffer->au8TelIn[CPLD_DATA_OFFSET] == SWITCH_DISABLE)
	{
		intReturn = 0;
	}
	else
	{
		intReturn = -1;
	}

	free(ptSpiBuffer);

	return intReturn;
}
