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
///\param[in]				.bByteOrder		- The order or sequence of bytes. 1 - Bigendian, 0 - little endian
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
	tTransferBufferSW.bByteEndian = ptCmdSwitch->bByteOrder;

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
		//Errorhandling. Not yet implemented
		ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
	}

	//---------------------------------------------------------------------------------------------------
	// Return data for calling function
	//---------------------------------------------------------------------------------------------------
	ptSpiSwitchData->u8TranRegisterData = ptSpiBuffer->au8TelOut[SWITCH_DATA_OFFSET];
	ptSpiSwitchData->u8RecRegisterData = ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET];

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
///\param[in]				.bByteOrder		- The order or sequence of bytes. 1 - Bigendian, 0 - little endian
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
	tTransferBufferSW.bByteEndian = ptCmdSwitch->bByteOrder;

	//---------------------------------------------------------------------------------------------------
	// Write process
	//---------------------------------------------------------------------------------------------------
	if (ptCmdSwitch->bReadOnly == FALSE)
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
		}
		
		free(ptSpiBuffer);
	}
	//---------------------------------------------------------------------------------------------------
	// Read process
	//---------------------------------------------------------------------------------------------------
	else
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
			//Errorhandling. Not yet implemented
			ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
		}

		memcpy(&au8Swap[0], &ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET], SWITCH_STATIC_MAC_LENGTH);

		free(ptSpiBuffer);
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
///\param[in]				.bByteOrder		- The order or sequence of bytes. 1 - Bigendian, 0 - little endian
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
	uint8_t u8StepCtrl = 0;
	uint8_t u8Offset = 0;
	uint8_t* tx;												// Output character buffer
	uint8_t u8SwitchReg = 0;									// Register data
	uint8_t u8SwitchRegAdress = 0;								// Indirect address
	uint8_t au8TelTx[SPI_BUF_SIZE];								// Output transmission buffer (TX)
	uint8_t au8Swap[8];											// Buffer for address swapping
	uint8_t au8TmpSwap[8];										// Temporary buffer for address swapping
	
	uint16_t u16Adresse = 0;
	
	ptSpiSwitchData = malloc(sizeof(struct t_SpiDebugSwitch));

	// Initialise the variables
	memset(&au8Swap, 0, sizeof(au8Swap));
	memset(&au8TmpSwap, 0, sizeof(au8TmpSwap));
	memset(&tTransferBufferSW, 0, sizeof(tTransferBufferSW));

	// Setting structure variables for the transfer
	tTransferBufferSW.intFd = ptCmdSwitch->intFd;
	tTransferBufferSW.u32Freq = ptCmdSwitch->u32SpeedHz;
	tTransferBufferSW.bByteEndian = ptCmdSwitch->bByteOrder;

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

				u16Adresse = ptCmdSwitch->u16AddrIdx / 4;
				u8Offset = (uint8_t)(ptCmdSwitch->u16AddrIdx % 4);

				u8SwitchReg |= SWITCH_READ_ACR;
				u8SwitchReg |= ((u16Adresse >> 8) | (SWITCH_SELECT_VLAN));

				tTransferBufferSW.pu8DataOut = &u8SwitchReg;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_110;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);
			}
			case SWITCH_READ_STEP_2:
			{
				//STEP 2. Config of the advanced control register 111 (Indirect Access Control 1)
				// Clearing the transmission buffer and memory allocation for the next step
				memset(&au8TelTx, 0, sizeof(au8TelTx));
				ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

				u8SwitchRegAdress = (uint8_t)u16Adresse;

				tTransferBufferSW.pu8DataOut = &u8SwitchRegAdress;
				tTransferBufferSW.u8Length = SWITCH_BYTE_LENGTH;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_111;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				}

				memcpy(&au8Swap[0], &ptSpiBuffer->au8TelIn[SWITCH_DATA_OFFSET], SWITCH_VLAN_LENGTH_8);

				free(ptSpiBuffer);

				u8Step = SWITCH_READ_DONE;
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

				tTransferBufferSW.pu8DataOut = &au8Swap[0];
				tTransferBufferSW.u8Length = SWITCH_VLAN_LENGTH_8;
				tTransferBufferSW.u8Register = SWITCH_ADVANCED_CONTROL_REGISTER_113;
				tTransferBufferSW.bRead = FALSE;

				// Calling a low level function with the parameter handover for the SPI transfer.
				ptSpiBuffer = ptSpiReadWriteRegSwitch(&tTransferBufferSW);
				if (ptSpiBuffer->intRet < 0)
				{
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
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
					//Errorhandling. Not yet implemented
					ptSpiSwitchData->intRet = ptSpiBuffer->intRet;
				}

				free(ptSpiBuffer);

				u8Step = SWITCH_WRITE_DONE;
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
}