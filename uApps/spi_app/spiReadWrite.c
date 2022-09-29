//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWrite.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
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
// ptSpiReadWriteTel()
//
///\brief					The interface for Read/Write transfer.
//
///\param[in]				intFd		- Device ID
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u16Address	- Destination adress
///\param[in]				u32Freq		- Transfer speed [Hz]
///\param[in]				u8Length	- Length of Buffer
///\param[in]				bRead		- Read/Write flag
///\param[in]				bFlag		- Predefined (user) transfer or	random data
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTel(int intFd, uint8_t* pu8DataOut, uint8_t* pu8DataIn, uint16_t u16Address, uint32_t u32Freq, uint8_t u8Length, BOOL bRead, BOOL bFlag)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));

	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t au8TelTx[130];							// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	// Telegram build up according to the transfer protocol
	u8Len = (uint8_t)(u8Length << 1);
	if (bRead == TRUE)
	{
		u8Len |= READ;
	}
	u16AdrSwap = (uint16_t)( (u16Address >> 8) | (u16Address << 8) );
	memcpy(&au8TelTx[0], &u16AdrSwap, 2);
	memcpy(&au8TelTx[2], &u8Len, 1);
	if (bFlag == TRUE)
	{
		for (int i = 0; i < (u8Length); i++)
		{
			au8TelTx[i + 3] = *(pu8DataOut + i);
		}
	}
	else
	{
		for (int i = 0; i < (u8Length); i++)
		{
			au8TelTx[i + 3] = (uint8_t)rand() % 255;
			*(pu8DataOut + i) = au8TelTx[i + 3];
		}
	}

	// Filling variables of the structure for the transfer
	tTransfer.tx_buf = (unsigned long)au8TelTx;
	tTransfer.rx_buf = (unsigned long)pu8DataIn;
	tTransfer.len = (uint32_t)(u8Length + 3);
	tTransfer.delay_usecs = 0;
	tTransfer.speed_hz = u32Freq;
	tTransfer.bits_per_word = 8;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		perror("Read/Write Error - ioctl");
	}

	// Return data for calling function
	for (int i = 0; i < (u8Length + 3); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = *(pu8DataIn + i);
	}

	return ptSpiTel;
}

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTel()
//
///\brief					The interface for Read/Write transfer.
//
///\param[in]			t_SpiTransfer.
///\param[in]				.intFd		- Device ID
///\param[in]				.pu8DataOut	- Output TX buffer
///\param[in]				.pu8DataIn	- Input RX buffer
///\param[in]				.u16Address	- Destination adress
///\param[in]				.u32Freq		- Transfer speed [Hz]
///\param[in]				.u8Length	- Length of Buffer
///\param[in]				.bRead		- Read/Write flag
///\param[in]				.bFlag		- Predefined (user) transfer or	random data
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteTelX(struct t_SpiTransfer *ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));

	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t au8TelTx[130];							// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	// Telegram build up according to the transfer protocol
	u8Len = (uint8_t)(ptTransferBlock->u8Length << 1);
	if (ptTransferBlock->bRead == TRUE)
	{
		u8Len |= READ;
	}
	u16AdrSwap = (uint16_t)( (ptTransferBlock->u16Address >> 8) | (ptTransferBlock->u16Address << 8) );
	memcpy(&au8TelTx[0], &u16AdrSwap, 2);
	memcpy(&au8TelTx[2], &u8Len, 1);
	if (ptTransferBlock->bFlag == TRUE)
	{
		for (int i = 0; i < (ptTransferBlock->u8Length); i++)
		{
			au8TelTx[i + 3] = *(ptTransferBlock->pu8DataOut + i);
		}
	}
	else
	{
		for (int i = 0; i < (ptTransferBlock->u8Length); i++)
		{
			au8TelTx[i + 3] = (uint8_t)rand() % 255;
			*(ptTransferBlock->pu8DataOut + i) = au8TelTx[i + 3];
		}
	}

	// Filling variables of the structure for the transfer
	tTransfer.tx_buf = (unsigned long)au8TelTx;
	tTransfer.rx_buf = (unsigned long)ptTransferBlock->pu8DataIn;
	tTransfer.len = (uint32_t)(ptTransferBlock->u8Length + 3);
	tTransfer.delay_usecs = 0;
	tTransfer.speed_hz = ptTransferBlock->u32Freq;
	tTransfer.bits_per_word = 8;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		perror("Read/Write Error - ioctl - ioctl");
	}

	// Return data for calling function
	for (int i = 0; i < (ptTransferBlock->u8Length + 3); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = *(ptTransferBlock->pu8DataIn + i);
	}

	return ptSpiTel;
}