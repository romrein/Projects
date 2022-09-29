//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTestFuncCPLD.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - Two test methods to check SPI DMA transfer for reliability and to verify error-free transfer
//	- Only for debugging, not for use
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// iSpiTransferTest()
//
///\brief					SPI DMA data transfer test relating to predefined memory area
//
///\param[in]				intFD				- Device ID
///\param[in]				u32ZykCnt			- The number of test cycles
///\param[in]				u16Address			- Destination adress
///\param[in]				u8UserLen			- Length of Buffer
///\param[in]				u32Speed			- Transfer speed [Hz]
//
///\retval					intRet
//------------------------------------------------------------------------------------------------
int iSpiTransferTest(int intFD, uint32_t u32ZykCnt, uint16_t u16Address, uint8_t u8UserLen, uint32_t u32Speed)
{
	static struct t_SpiDebug* ptSpiBufferTest;				// A structure for status feedback and debugging
	ptSpiBufferTest = malloc(sizeof(struct t_SpiDebug));
	int intRet = 0;											// Return value
	int intLpad = 0;										// Status bar left value
	int intRpad = 0;										// Status bar right value
	int intErr = 0;											// Status value for buffer compare
	int intCR;												// Carriage return for table display

	uint8_t u8LenIdx = 0;									// Data length. Note: not the telegram length
	uint8_t au8LenDebug[SPI_BUF_SIZE];						// Debugging buffer for the length analysis
	uint8_t au8AddressDebug[MEM_AREA_SIZE];					// Debugging buffer for the adress analysis
	uint8_t au8BufTx[MEM_AREA_SIZE];						// Debugging buffer for the transfer memory arrea
	uint8_t au8BufRxR[MEM_AREA_SIZE];						// Debugging buffer for the receive memory arrea
	uint8_t au8TelTx[SPI_BUF_SIZE];							// Transfer buffer (TX)
	uint8_t au8TelRx[SPI_BUF_SIZE];							// Receive buffer (RX)

	uint16_t u16ValMax = 0;									// Maximum value for normalisation
	uint16_t u16TxIdxRead = 0;								// Running index for the address
	uint16_t u16DestAddress = 0;							// Destination adress


	uint32_t u32ByteCount = 0;								// Transferred byte counter
	uint32_t u32IdxErr = 0;									// Read/Write Error counter
	uint32_t u32Idx = 0;									// Cycle counter

	float fval = 0.0;										// Status bar actual value

	// Timer for calculating the speed of the byte transfer
	uint32_t u32Diff = 0;
	float fErrRate = 0.0;
	time_t start;
	time_t stop;

	// Clearing the transmission and debugging buffer
	memset(&au8LenDebug, 0, sizeof(au8LenDebug));
	memset(&au8AddressDebug, 0, sizeof(au8AddressDebug));
	memset(&au8BufTx, 0, sizeof(au8BufTx));
	memset(&au8BufRxR, 0, sizeof(au8BufRxR));
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// avoid NULL - Pointer exception
	if (ptSpiBufferTest == NULL)
	{
		return -EFAULT;
	}

	// Start from the cycle timer
	time(&start);

	// Write cycle
	while (u32Idx < u32ZykCnt)
	{
		// Generate the random address and the length
		u16DestAddress = (uint16_t)rand() % 1024;
		u8LenIdx = (uint8_t)rand() % 128;

		// Counter for the generated addresses
		au8AddressDebug[u16DestAddress]++;

		// Write cycle with generated length
		if (!u8UserLen)
		{
			// Counter for the generated length
			au8LenDebug[u8LenIdx]++;

			// Calling a transfer function and sending the parameters
			ptSpiBufferTest = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16DestAddress, u32Speed, u8LenIdx, FALSE, FALSE);
			if (ptSpiBufferTest->intRet < 0)
			{
				return ptSpiBufferTest->intRet;
			}

			// Calculate the next address to write from the length and fill the memory area
			for (int i = 0; i < (u8LenIdx); i++)
			{
				au8BufTx[u16DestAddress] = au8TelTx[i];
				u16DestAddress = (u16DestAddress + 1) & MAXBUF;
			}
			u32ByteCount += u8LenIdx;
		}
		// Write cycle with predefined length
		else
		{
			au8LenDebug[u8UserLen]++;

			// Calling a transfer function and sending the parameters
			ptSpiBufferTest = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16DestAddress, u32Speed, u8UserLen, FALSE, FALSE);
			if (ptSpiBufferTest->intRet < 0)
			{
				return ptSpiBufferTest->intRet;
			}

			// Calculating the next address to write from the length and filling the transmit memory area
			for (int i = 0; i < (u8UserLen); i++)
			{
				au8BufTx[u16DestAddress] = au8TelTx[i];
				u16DestAddress = (u16DestAddress + 1) & MAXBUF;
			}
			u32ByteCount += u8UserLen;
		}

		// Clearing the buffers for the next cycle
		memset(&au8TelTx, 0, sizeof(au8TelTx));
		memset(&au8TelRx, 0, sizeof(au8TelRx));

		u32Idx++;
	}// while (u32Idx < u32ZykCnt)

	// Stop from the cycle timer
	time(&stop);

	// Display of the time required for the transfer and the calculated speed [byte/sec]
	u32Diff = (uint32_t)difftime(stop, start);
	printf("Cycle: %d\n", u32Idx);
	printf("Speed: %d Bytes in %d seconds \n", u32ByteCount, u32Diff);

	sleep(1);

	// Clearing the buffers for the next read cycle
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// Generate the start address
	u16DestAddress = (uint16_t)rand() % 1024;

	// Read cycle
	while (1)
	{
		// Generate the random length
		u8LenIdx = (uint8_t)rand() % 128;

		// Calling a transfer function and sending the parameters
		ptSpiBufferTest = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16DestAddress, u32Speed, u8LenIdx, TRUE, FALSE);
		if (ptSpiBufferTest->intRet < 0)
		{
			return ptSpiBufferTest->intRet;
		}

		// Calculating the next address to read from the length and filling the receive memory area
		for (int i = CPLD_DATA_OFFSET; i < (u8LenIdx + CPLD_DATA_OFFSET); i++)
		{
			au8BufRxR[u16DestAddress] = au8TelRx[i];
			u16DestAddress = (u16DestAddress + 1) & MAXBUF;
			u16TxIdxRead++;
		}

		// Clearing the buffers for the next cycle
		memset(&au8TelTx, 0, sizeof(au8TelTx));
		memset(&au8TelRx, 0, sizeof(au8TelRx));

		// Exit condition for the cycle
		if (u16TxIdxRead > 2000)
		{
			break;
		}
	}//while (1)

	// Display of the memory area after executed write/read cycle
	printf("\n\n");
	printf("***************************************************************************************\n");
	printf("**                         Receive buffer after WRITE command:                       **\n");
	printf("***************************************************************************************\n");
	for (int i = 0; i < 20; i++)
	{
		printf(" %02d ", i);
	}
	printf("\n");
	for (int i = 0; i < 20; i++)
	{
		printf("  | ");
	}
	printf("\n");
	intCR = 1;
	u32IdxErr = 0;
	for (int i = 0; i < MEM_AREA_SIZE; i++)
	{
		if (au8BufTx[i] != au8BufRxR[i])
		{
			printf(ANSI_COLOR_RED);
			printf(" %02X ", au8BufRxR[i]);
			printf(ANSI_COLOR_RESET);
			u32IdxErr++;
		}
		else
		{
			printf(" %02X ", au8BufRxR[i]);
		}
		if (intCR >= 20)
		{
			printf(" - %d\n", (i - 19));
			intCR = 0;
		}
		intCR++;
	}
	printf("\n");
	fErrRate = (float)u32IdxErr * 100 / 1024;
	printf("Error rate: %f\n", fErrRate);

	// Display how often a specific length was used for the cycle
	printf("\n\n");
	printf("***********************************************************************************************************\n");
	printf("**                              Address statistics after WRITE command:                                  **\n");
	printf("***********************************************************************************************************\n");
	for (int i = 0; i < 20; i++)
	{
		printf(" %03d ", i);
	}
	printf("\n");
	for (int i = 0; i < 20; i++)
	{
		printf("  |  ");
	}
	printf("\n");
	intCR = 1;
	u32IdxErr = 0;
	for (int i = 0; i < MEM_AREA_SIZE; i++)
	{

		if (!au8AddressDebug[i])
		{
			printf(ANSI_COLOR_RED);
			printf(" %03d ", au8AddressDebug[i]);
			printf(ANSI_COLOR_RESET);
			u32IdxErr++;
		}
		else
		{
			printf(" %03d ", au8AddressDebug[i]);
		}
		if (intCR >= 20)
		{
			printf(" - %d\n", (i - 19));
			intCR = 0;
		}
		intCR++;
	}
	printf("\n");
	fErrRate = (float)u32IdxErr * 100 / 1024;
	printf("Error rate: %f\n", fErrRate);

	printf("\n");

	intErr = memcmp(au8BufTx, au8BufRxR, sizeof(au8BufTx)); //WR TO DO!!!

	if (intErr != 0)
	{
		printf("Read/write buffers are different!\n");
	}

	// Display of length statistics normalised to maximum value
	printf("\n\n");
	printf("**********************************************************************************************************************************\n");
	printf("**                                               Lenght statistics after WRITE command:                                         **\n");
	printf("**                                                      normalized to MAX value                                                 **\n");
	printf("**********************************************************************************************************************************\n");
	u16ValMax = 0;
	for (int i = 0; i < 128; i++)
	{
		if (u16ValMax < au8LenDebug[i])
		{
			u16ValMax = au8LenDebug[i];
		}
	}
	if (!u16ValMax)
	{
		u16ValMax = 1;
	}
	for (int i = 0; i < 128; i++)
	{
		fval = (float)au8LenDebug[i] * 100 / u16ValMax;
		intLpad = (int)(au8LenDebug[i] * 100 / u16ValMax);
		intRpad = 100 - intLpad;

		if (i < 10)
		{
			printf("\rLen: %d, %03d >> %3f%%   [%.*s%*s]", i, au8LenDebug[i], fval, intLpad, PBSTR, intRpad, "");
		}
		else if ((i < 100) && (i >= 10))
		{
			printf("\rLen: %d, %03d >> %3f%%  [%.*s%*s]", i, au8LenDebug[i], fval, intLpad, PBSTR, intRpad, "");
		}
		else
		{
			printf("\rLen: %d, %03d >> %3f%% [%.*s%*s]", i, au8LenDebug[i], fval, intLpad, PBSTR, intRpad, "");
		}

		printf("\n");
	}

	printf("\n");
	printf("Test completed.\n");

	free(ptSpiBufferTest);

	return intRet;
}

//------------------------------------------------------------------------------------------------
// iSpiTransferTestSingle()
//
///\brief					SPI DMA data transfer test for single telegram
//
///\param[in]				intFD					- Device ID
///\param[in]				u32ZykCnt				- The number of test cycles
///\param[in]				u16Address				- Destination adress
///\param[in]				u8UserLen				- Length of Buffer
///\param[in]				u32Speed				- Transfer speed [Hz]
//
///\retval					intRet
//------------------------------------------------------------------------------------------------
int iSpiTransferTestSingle(int intFD, uint32_t u32ZykCnt, uint16_t u16Address, uint8_t u8UserLen, uint32_t u32Speed)
{
	static struct t_SpiDebug* ptSpiBufferTestSingle;			// A structure for status feedback and debugging
	ptSpiBufferTestSingle = malloc(sizeof(struct t_SpiDebug));
	int intRet = 0;												// Return value
	int intCR;													// Carriage return for table display

	uint8_t u8LenIdx = 0;										// Data length. Note: not the telegram length
	uint8_t au8TelTxTmp[SPI_BUF_SIZE];							// Temporal transfer buffer
	uint8_t au8BufRxR[MEM_AREA_SIZE];							// Debugging buffer for the receive memory arrea
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Transfer buffer (TX)
	uint8_t au8TelRx[SPI_BUF_SIZE];								// Receive buffer (RX)

	uint16_t u16TxIdxRead = 0;									// Running index for the address
	uint16_t u16DestAddress = 0;								// Destination adress

	uint32_t u32ByteCount = 0;									// Transferred byte counter
	uint32_t u32IdxErr = 0;										// Read/Write Error counter
	uint32_t u32Idx = 0;										// Cycle counter

	// Timer for calculating the speed of the byte transfer
	uint32_t u32Diff = 0;
	float fErrRate = 0.0;
	time_t start;
	time_t stop;

	// Clearing the transmission and debugging buffer
	memset(&au8TelTxTmp, 0, sizeof(au8TelTxTmp));
	memset(&au8BufRxR, 0, sizeof(au8BufRxR));
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// avoid NULL - Pointer exception
	if (ptSpiBufferTestSingle == NULL)
	{
		return -EFAULT;
	}

	// Start from the cycle timer
	time(&start);

	// Write cycle
	while (u32Idx < u32ZykCnt)
	{
		// Write cycle with generated length
		if (!u8UserLen)
		{
			u8LenIdx = (uint8_t)rand() % 128;
			ptSpiBufferTestSingle = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16Address, u32Speed, u8LenIdx, FALSE, FALSE);
			if (ptSpiBufferTestSingle->intRet < 0)
			{
				return ptSpiBufferTestSingle->intRet;
			}
			memcpy(&au8TelTxTmp[3], &au8TelTx, 127);

			memset(&au8TelTx, 0, sizeof(au8TelTx));

			ptSpiBufferTestSingle = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16Address, u32Speed, u8LenIdx, TRUE, FALSE);
			if (ptSpiBufferTestSingle->intRet < 0)
			{
				return ptSpiBufferTestSingle->intRet;
			}
			for (int i = 3; i < (u8LenIdx); i++)
			{
				if (au8TelTxTmp[i] != (*(ptSpiBufferTestSingle->au8TelIn + i)))
				{
					u32IdxErr++;
					break;
				}
			}
			memset(&au8TelTxTmp, 0, sizeof(au8TelTxTmp));
			u32ByteCount += u8LenIdx;
		}
		// Write cycle with predefined lengt
		else
		{
			ptSpiBufferTestSingle = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16Address, u32Speed, u8UserLen, FALSE, FALSE);
			if (ptSpiBufferTestSingle->intRet < 0)
			{
				return ptSpiBufferTestSingle->intRet;
			}
			memcpy(&au8TelTxTmp[3], &au8TelTx, 127);

			memset(&au8TelTx, 0, sizeof(au8TelTx));

			ptSpiBufferTestSingle = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16Address, u32Speed, u8UserLen, TRUE, FALSE);
			if (ptSpiBufferTestSingle->intRet < 0)
			{
				return ptSpiBufferTestSingle->intRet;
			}
			for (int i = 0; i < (u8UserLen + 3); i++)
			{
				if (au8TelTxTmp[i] != (*(ptSpiBufferTestSingle->au8TelIn + i)))
				{
					u32IdxErr++;
					break;
				}
			}
			memset(&au8TelTxTmp, 0, sizeof(au8TelTxTmp));
			u32ByteCount += u8UserLen;
		}

		u32Idx++;
	}//while (u32Idx < u32ZykCnt)

	// Stop from the cycle timer
	time(&stop);

	// Display of the time required for the transfer and the calculated speed [byte/sec]
	u32Diff = (uint32_t)difftime(stop, start);
	printf("Cycle: %d\n", u32Idx);
	printf("Speed: %d Bytes in %d seconds \n", u32ByteCount, u32Diff);

	// Calculate und display the error rate for the read/write cycles
	if (u32Idx != 0)
	{
		fErrRate = (float)u32IdxErr * 100 / (float)u32Idx;
	}
	else
	{
		fErrRate = (float)u32IdxErr * 100 / 1;
	}
	printf("Total Error: %d\n", u32IdxErr);
	printf("Total Error in %: %f\n", fErrRate);

	sleep(1);
	// Clearing the buffers for the next read cycle
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));

	// Generate the start address
	u16DestAddress = (uint16_t)rand() % 1024;

	// Read cycle
	while (1)
	{
		// Generate the random length
		u8LenIdx = (uint8_t)rand() % 128;

		// Calling a transfer function and sending the parameters
		ptSpiBufferTestSingle = ptSpiReadWriteTel(intFD, &au8TelTx[0], &au8TelRx[0], u16DestAddress, u32Speed, u8LenIdx, TRUE, FALSE);
		if (ptSpiBufferTestSingle->intRet < 0)
		{
			return ptSpiBufferTestSingle->intRet;
		}

		// Calculating the next address to read from the length and filling the receive memory area
		for (int i = CPLD_DATA_OFFSET; i < (u8LenIdx + CPLD_DATA_OFFSET); i++)
		{
			au8BufRxR[u16DestAddress] = au8TelRx[i];
			u16DestAddress = (u16DestAddress + 1) & MAXBUF;
			u16TxIdxRead++;
		}

		// Clearing the buffers for the next cycle
		memset(&au8TelTx, 0, sizeof(au8TelTx));
		memset(&au8TelRx, 0, sizeof(au8TelRx));

		// Exit condition for the cycle
		if (u16TxIdxRead > 2000)
		{
			break;
		}
	}//while (1)

	// Display of the memory area after executed write/read cycle
	printf("\n\n");
	printf("***************************************************************************************\n");
	printf("**                         Receive buffer after WRITE command:                       **\n");
	printf("***************************************************************************************\n");
	for (int i = 0; i < 20; i++)
	{
		printf(" %02d ", i);
	}
	printf("\n");
	for (int i = 0; i < 20; i++)
	{
		printf("  | ");
	}
	printf("\n");
	intCR = 1;
	u32IdxErr = 0;
	for (int i = 0; i < MEM_AREA_SIZE; i++)
	{
		printf(" %02X ", au8BufRxR[i]);
		if (intCR >= 20)
		{
			printf(" - %d\n", (i - 19));
			intCR = 0;
		}
		intCR++;
	}
	printf("\n");
	printf("Test completed.\n");

	free(ptSpiBufferTestSingle);

	return intRet;
}