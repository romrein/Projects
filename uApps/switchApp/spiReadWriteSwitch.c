//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWriteSwitch.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  21.06.2021
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
// ptSpiReadWriteRegSwitch()
//
///\brief					The interface for SPI DMA read/write transfer.
//
///\param[in]		ptTransferBlock.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.u8Register	- Register address
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write Selection. 0 - Write, 1 - Read
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteRegSwitch(struct t_SpiTransferSwitch* ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));

	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer (TX)
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Receive buffer (RX)
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	uint16_t* txw;									// Variable for cmd and register address

	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	// Telegram build up according to the transfer protocol
	if (ptTransferBlock->bRead == TRUE)
	{
		txw = ((SPICMD_RD << SPI_ADDR_S) | ptTransferBlock->u8Register);
		txw = ((SPICMD_RD << SPI_ADDR_S) | ptTransferBlock->u8Register) << SPI_TURNAROUND_S;
		txw = __cpu_to_le16(txw);
	}
	else
	{
		txw = ((SPICMD_WR << SPI_ADDR_S) | ptTransferBlock->u8Register);
		txw = ((SPICMD_WR << SPI_ADDR_S) | ptTransferBlock->u8Register) << SPI_TURNAROUND_S;
		txw = __cpu_to_le16(txw);
	}

	memcpy(&au8TelTx[0], &txw, 2);
	for (int i = 0; i < (ptTransferBlock->u8Length); i++)
	{
		au8TelTx[i + SWITCH_DATA_OFFSET] = *(ptTransferBlock->pu8DataOut + i);
	}

	// Setting structure variables for the SPI DMA transfer
	tTransfer.tx_buf = (unsigned long)au8TelTx;
	tTransfer.rx_buf = (unsigned long)au8TelRx;
	tTransfer.len = (uint32_t)(ptTransferBlock->u8Length + SWITCH_DATA_OFFSET);
	tTransfer.delay_usecs = SPI_DELAY;
	tTransfer.speed_hz = ptTransferBlock->u32Freq;
	tTransfer.bits_per_word = SPI_BITS;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		printf("Error: read/write - driver ioctl - ioctl\n");
		ptSpiTel->intRet = -1;
		goto end;
	}

	// Transfer single byte validation
	if ( (ptTransferBlock->bRead == FALSE) && (ptTransferBlock->u8Length == BYTE) )
	{
		// Clearing the transmission buffers
		memset(&au8TelTx, 0, sizeof(au8TelTx));
		memset(&au8TelRx, 0, sizeof(au8TelRx));
		// Telegram build up according to the transfer protocol
		txw = ((SPICMD_RD << SPI_ADDR_S) | ptTransferBlock->u8Register);
		txw = ((SPICMD_RD << SPI_ADDR_S) | ptTransferBlock->u8Register) << SPI_TURNAROUND_S;
		txw = __cpu_to_le16(txw);

		memcpy(&au8TelTx[0], &txw, 2);
		for (int i = 0; i < (ptTransferBlock->u8Length); i++)
		{
			au8TelTx[i + SWITCH_DATA_OFFSET] = *(ptTransferBlock->pu8DataOut + i);
		}

		// Setting structure variables for the SPI DMA transfer
		tTransfer.tx_buf = (unsigned long)au8TelTx;
		tTransfer.rx_buf = (unsigned long)au8TelRx;
		tTransfer.len = (uint32_t)(ptTransferBlock->u8Length + SWITCH_DATA_OFFSET);
		tTransfer.delay_usecs = SPI_DELAY;
		tTransfer.speed_hz = ptTransferBlock->u32Freq;
		tTransfer.bits_per_word = SPI_BITS;

		// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
		ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

		if (ptSpiTel->intRet < 0)
		{
			printf("Error: read/write - driver ioctl - ioctl\n");
			ptSpiTel->intRet = -1;
			goto end;
		}
		for (int i = SWITCH_DATA_OFFSET; i < (ptTransferBlock->u8Length + SWITCH_DATA_OFFSET); i++)
		{
			if (au8TelTx[i] != au8TelRx[i])
			{
				printf("The bytes are not valid: TX %02X, RX %02X\n", au8TelTx[i], au8TelRx[i]);
				printf("Error: switch single byte transfer validation failed\n\n");
				ptSpiTel->intRet = -1;
				goto end;
			}
		}
	}// End of Single Byte validation

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (ptTransferBlock->u8Length + SWITCH_DATA_OFFSET); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = au8TelRx[i];
	}

end:

	return ptSpiTel;
}