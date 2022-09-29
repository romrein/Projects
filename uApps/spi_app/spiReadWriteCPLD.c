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

//---------OLD FUNKTION---------PLEASE USE ptSpiReadWriteTelSingle() ----------------------------------
//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTel()
//
///\brief					The interface for read/write SPI DMA transfer.
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
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
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
			au8TelTx[i + CPLD_DATA_OFFSET] = *(pu8DataOut + i);
		}
	}
	else
	{
		for (int i = 0; i < (u8Length); i++)
		{
			au8TelTx[i + CPLD_DATA_OFFSET] = (uint8_t)rand() % 255;
			*(pu8DataOut + i) = au8TelTx[i + CPLD_DATA_OFFSET];
		}
	}

	// Setting structure variables for the SPI DMA transfer
	tTransfer.tx_buf = (unsigned long)au8TelTx;
	tTransfer.rx_buf = (unsigned long)pu8DataIn;
	tTransfer.len = (uint32_t)(u8Length + CPLD_DATA_OFFSET);
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
	for (int i = 0; i < (u8Length + CPLD_DATA_OFFSET); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = *(pu8DataIn + i);
	}

	return ptSpiTel;
}

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTelSingle()
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
struct t_SpiDebug* ptSpiReadWriteTelSingle(struct t_SpiTransfer *ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));

	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	if (ptTransferBlock->u8Length == 0)
	{
		//Wrong length
		perror("Wrong data length\n");
		ptSpiTel->intRet = -1;
		goto end;
	}

	// Telegram build up according to the transfer protocol
	u8Len = (uint8_t)(ptTransferBlock->u8Length << 1);
	if (ptTransferBlock->bRead == TRUE)
	{
		u8Len |= READ;
	}
	u16AdrSwap = (uint16_t)( (ptTransferBlock->u16Address >> 8) | (ptTransferBlock->u16Address << 8) );
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
	tTransfer.delay_usecs = 0;
	tTransfer.speed_hz = ptTransferBlock->u32Freq;
	tTransfer.bits_per_word = 8;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		perror("Read/Write Error - ioctl - ioctl\n");
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (ptTransferBlock->u8Length + CPLD_DATA_OFFSET); i++)
	{
		ptSpiTel->au8TelOut[i] = au8TelTx[i];
		ptSpiTel->au8TelIn[i] = au8TelRx[i];
	}

end:
	return ptSpiTel;
}

//------------------------------------------------------------------------------------------------
// ptSpiReadWriteTransfer()
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
struct t_SpiDebug* ptSpiReadWriteTransfer(struct t_SpiTransfer* ptTransferBlock)
{
	static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
	ptSpiTel = malloc(sizeof(struct t_SpiDebug));

	uint8_t u8Len = 0;								// Data length. Note: not the telegram length
	uint8_t au8TelTx[SPI_BUF_SIZE];					// Transfer buffer
	uint8_t au8TelRx[SPI_BUF_SIZE];					// Transfer buffer
	uint16_t u16AdrSwap = 0;						// Temporal variable for address swapping
	struct spi_ioc_transfer tTransfer;				// Transfer structure or interface to the kernel SPI-driver

	// Clearing the transmission buffers
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransfer, 0, sizeof(tTransfer));

	if (ptTransferBlock->u8Length == 0)
	{
		//Wrong length
		perror("Wrong data length\n");
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
	tTransfer.delay_usecs = 0;
	tTransfer.speed_hz = ptTransferBlock->u32Freq;
	tTransfer.bits_per_word = 8;

	// Calling a Kernel IO-Control function with the parameter handover for the SPI transfer.
	ptSpiTel->intRet = ioctl(ptTransferBlock->intFd, SPI_IOC_MESSAGE(1), &tTransfer);

	if (ptSpiTel->intRet < 0)
	{
		perror("Read/Write Error - ioctl - ioctl\n");
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function. Echo function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (ptTransferBlock->u8Length); i++)
	{
		ptSpiTel->au8TelIn[i] = au8TelRx[i + 3];
	}

end:
	return ptSpiTel;
}