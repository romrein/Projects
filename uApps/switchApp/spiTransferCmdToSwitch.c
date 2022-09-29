//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTransferCmdToSwitch.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  23.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - The midle level application contains three functions for communication with the switch via SPI interface.
//  - Requires an low level application to connect with the SPI kernel driver (spiReadWriteSwitch.c).
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S 

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchSingleReg()
//------------------------------------------------------------------------------------------------

///\brief					ptSpiTransferSwitchSingleReg().
///							The interface for read/write transfer to the single switch register.
///
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd			- Device ID
///\param[in]				.bReadOnly		- Read/Write Selection. 0 - Write, 1 - Read
///\param[in]				.pu8DataOut		- TX data
///\param[in]				.u16AddrIdx		- Register address
///\param[in]				.u32SpeedHz		- Transfer speed [Hz]
///\param[out]
//
///\retval		t_SpiDebugSwitch*
//------------------------------------------------------------------------------------------------
struct t_SpiDebugSwitch* ptSpiTransferSwitchSingleReg(struct t_CmdToSwitch* ptCmdSwitch)
{
	static struct t_SpiDebug* ptSpiBuffer;							// A structure for status feedback and debugging
	static struct t_SpiDebugSwitch* ptSpiSwitchData;
	static struct t_SpiTransferSwitch tTransferBufferSW;			// Transfer buffer interface

	uint8_t au8TelTx[SPI_BUF_SIZE];									// Output transmission buffer (TX)

	ptSpiSwitchData = malloc(sizeof(struct t_SpiDebugSwitch));

	// Clearing the transmission buffer and memory allocation
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&tTransferBufferSW, 0, sizeof(tTransferBufferSW));
	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	// Setting structure variables for the transfer
	tTransferBufferSW.intFd = ptCmdSwitch->intFd;
	tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
	tTransferBufferSW.u8Register = (uint8_t)ptCmdSwitch->u16AddrIdx;
	tTransferBufferSW.u32Freq = ptCmdSwitch->u32SpeedHz;

	//---------------------------------------------------------------------------------------------------
	// Write process
	//---------------------------------------------------------------------------------------------------
	if (ptCmdSwitch->bReadOnly == FALSE)
	{
		tTransferBufferSW.pu8DataOut = ptCmdSwitch->pu8DataOut;
		tTransferBufferSW.bRead = FALSE;
	}
	//---------------------------------------------------------------------------------------------------
	// Read process
	//---------------------------------------------------------------------------------------------------
	else
	{
		tTransferBufferSW.pu8DataOut = &au8TelTx[0];
		tTransferBufferSW.bRead = TRUE;
	}

	// Calling a low level function with the parameter handover for the SPI transfer.
	ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
	if (ptSpiBuffer->intRet < 0)
	{
		printf("Error: single register read/write failed\n");
		ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
		goto end;
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function
	//---------------------------------------------------------------------------------------------------
	ptSpiSwitchData->u8TranRegisterData = ptSpiBuffer->au8TelOut[SWITCH_DATA_OFFSET];
	ptSpiSwitchData->u8RecRegisterData = ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET];

end:

	free(ptSpiBuffer);

	return ptSpiSwitchData;
}

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchStaticMac()
//

///\brief					ptSpiTransferSwitchStaticMac().
///							Function to read write data to the static MAC address table.
/// 
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd			- Device ID
///\param[in]				.bReadOnly		- Read/Write Selection. 0 - Write, 1 - Read
///\param[in]				.pu8DataOut		- TX data
///\param[in]				.u16AddrIdx		- Register address
///\param[in]				.u32SpeedHz		- Transfer speed [Hz]
///\param[out]
//
///\retval		t_SpiDebugSwitch*
//------------------------------------------------------------------------------------------------
struct t_SpiDebugSwitch* ptSpiTransferSwitchStaticMac(struct t_CmdToSwitch* ptCmdSwitch)
{
	static struct t_SpiDebug* ptSpiBuffer;						// A structure for status feedback and debugging
	static struct t_SpiDebugSwitch* ptSpiSwitchData;
	static struct t_SpiTransferSwitch tTransferBufferSW;		// Transfer buffer interface

	BOOL	bReadWreitEnd = FALSE;								// Trigger or output condition for the sequence

	uint8_t u8State = 0;
	uint8_t u8SwitchReg = 0;									// Register data
	uint8_t u8SwitchRegAdress = 0;								// Indirect address
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX)
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping

	ptSpiSwitchData = malloc(sizeof(struct t_SpiDebugSwitch));

	// Initialise the variables
	memset(&au8Swap, 0, sizeof(au8Swap));
	memset(&au8TmpSwap, 0, sizeof(au8TmpSwap));
	memset(&tTransferBufferSW, 0, sizeof(tTransferBufferSW));

	// Setting structure variables for the transfer
	tTransferBufferSW.intFd = ptCmdSwitch->intFd;
	tTransferBufferSW.u32Freq = ptCmdSwitch->u32SpeedHz;

	if (ptCmdSwitch->bReadOnly == FALSE)
	{
		u8State = 1;
	}
	else
	{
		u8State = 2;
	}

	// State machine for the Process
	while (bReadWreitEnd == FALSE)
	{
		switch (u8State)
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
			// Write process 
			//---------------------------------------------------------------------------------------------------
			case 1:
			{
				// Clearing the transmission buffer and memory allocation
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				memcpy(&au8Swap[0], ptCmdSwitch->pu8DataOut, SWITCH_STATIC_MAC_LENGTH);

				//STEP 1. Copying the static MAC address to the memory area
				tTransferBufferSW.pu8DataOut = &au8Swap[0];
				tTransferBufferSW.u8Length = SWITCH_STATIC_MAC_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write the static MAC address to the memory area failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				//STEP 2. Config of the advanced control register 110 (Indirect Access Control 0)
				// Clearing the transmission bufferand memory allocation for the next step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchReg &= ~(SWITCH_READ_ACR);

				tTransferBufferSW.pu8DataOut = &u8SwitchReg;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 110 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				//STEP 3. Config of the advanced control register 111 (Indirect Access Control 1)
				// Clearing the transmission buffer and memory allocation for the last step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchRegAdress = (uint8_t)ptCmdSwitch->u16AddrIdx;

				tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 111 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				// Validation
				u8State = 2;
			}
			break;
			//---------------------------------------------------------------------------------------------------
			// Read process 
			//---------------------------------------------------------------------------------------------------
			case 2:
			{
				//STEP 1. Config of the advanced control register 110 (Indirect Access Control 0)
				// Clearing the transmission buffer and memory allocation
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchReg |= SWITCH_READ_ACR;

				tTransferBufferSW.pu8DataOut = &u8SwitchReg;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 110 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				//STEP 2. Config of the advanced control register 111 (Indirect Access Control 1)
				// Clearing the transmission buffer and memory allocation for the next step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchRegAdress = (uint8_t)ptCmdSwitch->u16AddrIdx;

				tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 111 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				//STEP 3. Copying the static MAC address from the memory area
				// Clearing the transmission buffer and memory allocation for the last step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				tTransferBufferSW.pu8DataOut = &au8TelTx[0];
				tTransferBufferSW.u8Length = SWITCH_STATIC_MAC_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
				tTransferBufferSW.bRead = TRUE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: read the static MAC address from the memory area failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				// Validation
				memcpy(&au8Swap[0], &ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET], SWITCH_STATIC_MAC_LENGTH);

				if ((au8Swap[0] & (0x1)) != 0)
				{
					au8Swap[1] |= (0x80);
				}
				au8Swap[0] = au8Swap[0] >> 1;

				if (ptCmdSwitch->bReadOnly == FALSE)
				{
					for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
					{
						if ( (*(ptCmdSwitch->pu8DataOut + i)) != au8Swap[i])
						{
							printf("The bytes are not valid: Byte Nr %d, TX %02X, RX %02X\n", i, (*(ptCmdSwitch->pu8DataOut + i)), au8Swap[i]);
							printf("Error: static MAC address transfer validation failed\n\n");
							goto end;
						}
					}
				}

				free(ptSpiBuffer);

				bReadWreitEnd = TRUE;
			}
			break;
			default:
			{
				bReadWreitEnd = TRUE;
			}
			break;
		}//switch (u8State)
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (SWITCH_STATIC_MAC_LENGTH); i++)
	{
		ptSpiSwitchData->au8TranData[i] = au8Swap[i];
		ptSpiSwitchData->au8RecData[i] = au8Swap[i];
	}
	
	return ptSpiSwitchData;

end:
	free(ptSpiBuffer);
	return ptSpiSwitchData;
}

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchVLan()
//
///\brief					ptSpiTransferSwitchVLan().
///							Function to read write data to the VLAN address table.
/// 
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd			- Device ID
///\param[in]				.bReadOnly		- Read/Write Selection. 0 - Write, 1 - Read
///\param[in]				.pu8DataOut		- TX data
///\param[in]				.u16AddrIdx		- Register address
///\param[in]				.u32SpeedHz		- Transfer speed [Hz]
///\param[out]
//
///\retval		t_SpiDebugSwitch*
//------------------------------------------------------------------------------------------------
struct t_SpiDebugSwitch* ptSpiTransferSwitchVLan(struct t_CmdToSwitch* ptCmdSwitch)
{
	static struct t_SpiDebug* ptSpiBuffer;						// A structure for status feedback and debugging
	static struct t_SpiDebugSwitch* ptSpiSwitchData;
	static struct t_SpiTransferSwitch tTransferBufferSW;		// Transfer buffer interface

	BOOL	bReadWreitEnd = FALSE;
	uint8_t u8Step = 0;											// Read/write step
	uint8_t u8Ret = 0;
	uint8_t u8StepCtrl = 0;
	uint8_t u8ValidByte = 0;
	uint8_t u8Offset = 0;
	uint8_t* tx;												// Output character buffer
	uint8_t u8SwitchReg = 0;									// Register data
	uint8_t u8SwitchRegAdress = 0;								// Indirect address
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX)
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8Valid[8];										// Buffer for validation
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping
	
	uint16_t u16Adresse = 0;
	
	ptSpiSwitchData = malloc(sizeof(struct t_SpiDebugSwitch));

	// Initialise the variables
	memset(&au8Swap, 0, sizeof(au8Swap));
	memset(&au8Valid, 0, sizeof(au8Valid));
	memset(&au8TmpSwap, 0, sizeof(au8TmpSwap));
	memset(&tTransferBufferSW, 0, sizeof(tTransferBufferSW));

	// Setting structure variables for the transfer
	tTransferBufferSW.intFd = ptCmdSwitch->intFd;
	tTransferBufferSW.u32Freq = ptCmdSwitch->u32SpeedHz;

	u8StepCtrl = CTRL_START_PROCESS;

	while (bReadWreitEnd == FALSE)
	{
		// Control state machine
		switch (u8StepCtrl)
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
			// Start process
			//---------------------------------------------------------------------------------------------------
			case CTRL_START_PROCESS:
			{
				u8Step = SWITCH_READ_STEP_1;
				u8StepCtrl++;
			}
			break;
			case CTRL_WAIT_FOR_READ:
			{
				if ((u8Step == SWITCH_READ_DONE) && (ptCmdSwitch->bReadOnly == FALSE))
				{
					u8Step = SWITCH_WRITE_STEP_1;
					u8StepCtrl++;
				}
				else if ((u8Step == SWITCH_READ_DONE) && (ptCmdSwitch->bReadOnly == TRUE))
				{
					u8Step = STANDBY;
					u8StepCtrl = STANDBY;
					bReadWreitEnd = TRUE;
				}
			}
			break;
			case CTRL_WAIT_FOR_WRITE:
			{
				if (u8Step == SWITCH_WRITE_DONE)
				{
					u8Step = STANDBY;
					u8StepCtrl = STANDBY;
					bReadWreitEnd = TRUE;
				}
			}
			break;
			default:
			{
				bReadWreitEnd = TRUE;
			}
			break;
		}//switch (u8StepCtrl)

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
			// Read process
			//---------------------------------------------------------------------------------------------------
			case SWITCH_READ_STEP_1:
			{
				//STEP 1. Config of the advanced control register 110 (Indirect Access Control 0)
				// Clearing the transmission buffer and memory allocation
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				//Ind. address. MAX 1023
				if ((ptCmdSwitch->u16AddrIdx + SWITCH_VLAN_LENGTH_2) > SWITCH_MAX_VLAN_ADDRESS)
				{
					printf("Error: the address is out of range\n");
					ptSpiSwitchData->intRet = -1;
					goto end;
				}

				u16Adresse = ptCmdSwitch->u16AddrIdx / 4;
				u8Offset = (uint8_t)(ptCmdSwitch->u16AddrIdx % 4);

				u8SwitchReg |= SWITCH_READ_ACR;
				//8 MSB from address
				u8SwitchReg |= ((u16Adresse >> 8) | (SWITCH_SELECT_VLAN));

				tTransferBufferSW.pu8DataOut = &u8SwitchReg;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 110 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);
			}
			case SWITCH_READ_STEP_2:
			{
				//STEP 2. Config of the advanced control register 111 (Indirect Access Control 1)
				// Clearing the transmission buffer and memory allocation for the next step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				//8 LSB from address
				u8SwitchRegAdress = (uint8_t)u16Adresse;

				tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 111 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);
			}
			case SWITCH_READ_STEP_3:
			{
				//STEP 3. Copying the VLAN address from the memory area
				// Clearing the transmission buffer and memory allocation for the last step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				tTransferBufferSW.pu8DataOut = &au8TelTx[0];
				tTransferBufferSW.u8Length = SWITCH_VLAN_LENGTH_8;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
				tTransferBufferSW.bRead = TRUE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: read the VLAN address from the memory area failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				memcpy(&au8Swap[0], &ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET], SWITCH_VLAN_LENGTH_8);

				u8Ret = intArrayInvertMAC(&au8Swap[0]);

				if ( (ptCmdSwitch->bReadOnly == FALSE) && (u8StepCtrl == CTRL_WAIT_FOR_WRITE) )
				{
					// Validation
					u8Step = SWITCH_WRITE_DONE;
					for (int i = 0; i < (SWITCH_VLAN_LENGTH_8); i++)
					{
						if (au8Valid[i] != au8Swap[i])
						{
							printf("The bytes are not valid: Byte Nr %d, TX %02X, RX %02X\n", i, au8Valid[i], au8Swap[i]);
							u8ValidByte = au8Valid[i] & 0x1F;
							if (u8ValidByte & au8Swap[i])
							{
								printf("The 3 MSB are deleted. Check HW version from switch\n");
							}
							printf("Error: VLAN address transfer validation failed\n\n");
							goto end;
						}
					}
				}
				else 
				{
					u8Step = SWITCH_READ_DONE;
				}

				free(ptSpiBuffer);
			}
			break;
			case SWITCH_READ_DONE:
			{
				//Read end
			}
			break;
			//---------------------------------------------------------------------------------------------------
			// Write process
			//---------------------------------------------------------------------------------------------------
			case SWITCH_WRITE_STEP_1:
			{
				//STEP 1. Copying the VLAN address to the memory area
				// Clearing the transmission buffer
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				memcpy(&au8Swap[u8Offset * 2], ptCmdSwitch->pu8DataOut, SWITCH_VLAN_LENGTH_2);

				u8Ret = intArrayInvertMAC(&au8Swap[0]);

				tTransferBufferSW.pu8DataOut = &au8Swap[0];
				tTransferBufferSW.u8Length = SWITCH_VLAN_LENGTH_8;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write the VLAN address to the memory area failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);
			}
			case SWITCH_WRITE_STEP_2:
			{
				//STEP 2. Config of the advanced control register 110 (Indirect Access Control 0)
				// Clearing the transmission bufferand memory allocation for the next step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchReg &= ~(SWITCH_READ_ACR);

				tTransferBufferSW.pu8DataOut = &u8SwitchReg;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 110 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);
			}
			case SWITCH_WRITE_STEP_3:
			{
				//STEP 3. Config of the advanced control register 111 (Indirect Access Control 1)
				// Clearing the transmission buffer and memory allocation for the last step
				memset(&au8TelTx, 0, sizeof(au8TelTx));

				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					printf("Error: write to the advanced control register 111 failed\n");
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
					goto end;
				}

				free(ptSpiBuffer);

				memcpy(&au8Valid[0], &au8Swap[0], SWITCH_VLAN_LENGTH_8);

				u8Ret = intArrayInvertMAC(&au8Valid[0]);

				// Start validation
				u8Step = SWITCH_READ_STEP_1;
			}
			break;
			case SWITCH_WRITE_DONE:
			{
				//Write end
			}
			break;
			default:
			{
				bReadWreitEnd = TRUE;
			}
			break;
		}//switch (u8Step)
	}
	
	//---------------------------------------------------------------------------------------------------
	// Return data for calling function (Echo)
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (SWITCH_VLAN_LENGTH_2); i++)
	{
		ptSpiSwitchData->au8TranData[i] = au8Swap[i + u8Offset * 2];
		ptSpiSwitchData->au8RecData[i] = au8Swap[i + u8Offset * 2];
	}

	return ptSpiSwitchData;
end:
	free(ptSpiBuffer);
	return ptSpiSwitchData;
}

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchVLan8Byte()
//
///\brief					ptSpiTransferSwitchVLan().
///							Function to read write data to the VLAN address table.
/// 
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd			- Device ID
///\param[in]				.bReadOnly		- Read/Write Selection. 0 - Write, 1 - Read
///\param[in]				.pu8DataOut		- TX data
///\param[in]				.u16AddrIdx		- Register address
///\param[in]				.u32SpeedHz		- Transfer speed [Hz]
///\param[out]
//
///\retval		t_SpiDebugSwitch*
//------------------------------------------------------------------------------------------------
struct t_SpiDebugSwitch* ptSpiTransferSwitchVLan8Byte(struct t_CmdToSwitch* ptCmdSwitch)
{
	static struct t_SpiDebug* ptSpiBuffer;						// A structure for status feedback and debugging
	static struct t_SpiDebugSwitch* ptSpiSwitchData;
	static struct t_SpiTransferSwitch tTransferBufferSW;		// Transfer buffer interface

	BOOL	bReadWreitEnd = FALSE;
	uint8_t u8Step = 0;											// Read/write step
	uint8_t u8Ret = 0;
	uint8_t u8StepCtrl = 0;
	uint8_t u8ValidByte = 0;
	uint8_t u8Offset = 0;
	uint8_t* tx;												// Output character buffer
	uint8_t u8SwitchReg = 0;									// Register data
	uint8_t u8SwitchRegAdress = 0;								// Indirect address
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX)
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8Valid[8];										// Buffer for validation
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping

	uint16_t u16Adresse = 0;

	ptSpiSwitchData = malloc(sizeof(struct t_SpiDebugSwitch));

	// Initialise the variables
	memset(&au8Swap, 0, sizeof(au8Swap));
	memset(&au8Valid, 0, sizeof(au8Valid));
	memset(&au8TmpSwap, 0, sizeof(au8TmpSwap));
	memset(&tTransferBufferSW, 0, sizeof(tTransferBufferSW));

	// Setting structure variables for the transfer
	tTransferBufferSW.intFd = ptCmdSwitch->intFd;
	tTransferBufferSW.u32Freq = ptCmdSwitch->u32SpeedHz;

	u8StepCtrl = CTRL_START_PROCESS;

	while (bReadWreitEnd == FALSE)
	{
		// Control state machine
		switch (u8StepCtrl)
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
		// Start process
		//---------------------------------------------------------------------------------------------------
		case CTRL_START_PROCESS:
		{
			u8Step = SWITCH_READ_STEP_1;
			u8StepCtrl++;
		}
		break;
		case CTRL_WAIT_FOR_READ:
		{
			if ((u8Step == SWITCH_READ_DONE) && (ptCmdSwitch->bReadOnly == FALSE))
			{
				u8Step = SWITCH_WRITE_STEP_1;
				u8StepCtrl++;
			}
			else if ((u8Step == SWITCH_READ_DONE) && (ptCmdSwitch->bReadOnly == TRUE))
			{
				u8Step = STANDBY;
				u8StepCtrl = STANDBY;
				bReadWreitEnd = TRUE;
			}
		}
		break;
		case CTRL_WAIT_FOR_WRITE:
		{
			if (u8Step == SWITCH_WRITE_DONE)
			{
				u8Step = STANDBY;
				u8StepCtrl = STANDBY;
				bReadWreitEnd = TRUE;
			}
		}
		break;
		default:
		{
			bReadWreitEnd = TRUE;
		}
		break;
		}//switch (u8StepCtrl)

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
		// Read process
		//---------------------------------------------------------------------------------------------------
		case SWITCH_READ_STEP_1:
		{
			//STEP 1. Config of the advanced control register 110 (Indirect Access Control 0)
			// Clearing the transmission buffer and memory allocation
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			
			u16Adresse = ptCmdSwitch->u16AddrIdx;
			//Ind. address. MAX 1023
			if ((u16Adresse + SWITCH_VLAN_LENGTH_8) > SWITCH_MAX_VLAN_INDADDRESS)
			{
				printf("Error: the address is out of range\n");
				ptSpiSwitchData->intRet = -1;
				goto end;
			}
			//u8Offset = (uint8_t)(ptCmdSwitch->u16AddrIdx % 4);

			u8SwitchReg |= SWITCH_READ_ACR;
			//8 MSB from address
			u8SwitchReg |= ((u16Adresse >> 8) | (SWITCH_SELECT_VLAN));

			tTransferBufferSW.pu8DataOut = &u8SwitchReg;
			tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
			tTransferBufferSW.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: write to the advanced control register 110 failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			free(ptSpiBuffer);
		}
		case SWITCH_READ_STEP_2:
		{
			//STEP 2. Config of the advanced control register 111 (Indirect Access Control 1)
			// Clearing the transmission buffer and memory allocation for the next step
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			//8 LSB from address
			u8SwitchRegAdress = (uint8_t)u16Adresse;

			tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
			tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
			tTransferBufferSW.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: write to the advanced control register 111 failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			free(ptSpiBuffer);
		}
		case SWITCH_READ_STEP_3:
		{
			//STEP 3. Copying the VLAN address from the memory area
			// Clearing the transmission buffer and memory allocation for the last step
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			tTransferBufferSW.pu8DataOut = &au8TelTx[0];
			tTransferBufferSW.u8Length = SWITCH_VLAN_LENGTH_8;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
			tTransferBufferSW.bRead = TRUE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: read the VLAN address from the memory area failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			memcpy(&au8Swap[0], &ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET], SWITCH_VLAN_LENGTH_8);

			u8Ret = intArrayInvertMAC(&au8Swap[0]);

			if ((ptCmdSwitch->bReadOnly == FALSE) && (u8StepCtrl == CTRL_WAIT_FOR_WRITE))
			{
				// Validation
				u8Step = SWITCH_WRITE_DONE;
				for (int i = 0; i < (SWITCH_VLAN_LENGTH_8); i++)
				{
					if (au8Valid[i] != au8Swap[i])
					{
						printf("The bytes are not valid: Byte Nr %d, TX %02X, RX %02X\n", i, au8Valid[i], au8Swap[i]);
						u8ValidByte = au8Valid[i] & 0x1F;
						if (u8ValidByte & au8Swap[i])
						{
							printf("The 3 MSB are deleted. Check HW version from switch\n");
						}
						printf("Error: VLAN address transfer validation failed\n\n");
						goto end;
					}
				}
			}
			else
			{
				u8Step = SWITCH_READ_DONE;
			}

			free(ptSpiBuffer);
		}
		break;
		case SWITCH_READ_DONE:
		{
			//Read end
		}
		break;
		//---------------------------------------------------------------------------------------------------
		// Write process
		//---------------------------------------------------------------------------------------------------
		case SWITCH_WRITE_STEP_1:
		{
			//STEP 1. Copying the VLAN address to the memory area
			// Clearing the transmission buffer
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			//memcpy(&au8Swap[u8Offset * 2], ptCmdSwitch->pu8DataOut, SWITCH_VLAN_LENGTH_2);
			memcpy(&au8Swap[0], ptCmdSwitch->pu8DataOut, SWITCH_VLAN_LENGTH_8);

			u8Ret = intArrayInvertMAC(&au8Swap[0]);

			tTransferBufferSW.pu8DataOut = &au8Swap[0];
			tTransferBufferSW.u8Length = SWITCH_VLAN_LENGTH_8;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
			tTransferBufferSW.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: write the VLAN address to the memory area failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			free(ptSpiBuffer);
		}
		case SWITCH_WRITE_STEP_2:
		{
			//STEP 2. Config of the advanced control register 110 (Indirect Access Control 0)
			// Clearing the transmission bufferand memory allocation for the next step
			memset(&au8TelTx, 0, sizeof(au8TelTx));
			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			u8SwitchReg &= ~(SWITCH_READ_ACR);

			tTransferBufferSW.pu8DataOut = &u8SwitchReg;
			tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
			tTransferBufferSW.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: write to the advanced control register 110 failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			free(ptSpiBuffer);
		}
		case SWITCH_WRITE_STEP_3:
		{
			//STEP 3. Config of the advanced control register 111 (Indirect Access Control 1)
			// Clearing the transmission buffer and memory allocation for the last step
			memset(&au8TelTx, 0, sizeof(au8TelTx));

			ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

			tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
			tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
			tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
			tTransferBufferSW.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
			if (ptSpiBuffer->intRet < 0)
			{
				printf("Error: write to the advanced control register 111 failed\n");
				ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				goto end;
			}

			free(ptSpiBuffer);

			memcpy(&au8Valid[0], &au8Swap[0], SWITCH_VLAN_LENGTH_8);

			u8Ret = intArrayInvertMAC(&au8Valid[0]);

			// Start validation
			u8Step = SWITCH_READ_STEP_1;
		}
		break;
		case SWITCH_WRITE_DONE:
		{
			//Write end
		}
		break;
		default:
		{
			bReadWreitEnd = TRUE;
		}
		break;
		}//switch (u8Step)
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function (Echo)
	//---------------------------------------------------------------------------------------------------
	for (int i = 0; i < (SWITCH_VLAN_LENGTH_8); i++)
	{
		ptSpiSwitchData->au8TranData[i] = au8Swap[i];
		ptSpiSwitchData->au8RecData[i] = au8Swap[i];
	}

	return ptSpiSwitchData;
end:
	free(ptSpiBuffer);
	return ptSpiSwitchData;
}