//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiReadWriteSwitch.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  21.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiReadWriteSwitch.c"
//
//------------------------------------------------------------------------------------------------

#ifndef _SPIREADWRITESWITCH_H_
#define _SPIREADWRITESWITCH_H_

//------------------------------------------------------------------------------------------------
//  STRUCTS
//------------------------------------------------------------------------------------------------
struct t_SpiTransferSwitch
{
	int			intFd;				// Device ID
	BOOL		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
	BOOL		bByteEndian;		// The order or sequence of bytes. 1 - Bigendian, 0 - little endian
	uint8_t*	pu8DataOut;			// TX data
	uint8_t		u8Register;			// Register address
	uint8_t		u8Length;			// Data length [Byte], not Telegram length
	uint32_t	u32Freq;			// Speed/frequency [Hz]
};

struct t_CmdToSwitch
{
	int			intFd;				// Device ID
	BOOL		bReadOnly;			// Read/Write Selection. 0 - Write, 1 - Read 
	BOOL		bByteOrder;			// The order or sequence of bytes. 1 - Bigendian, 0 - little endian
	uint8_t*	pu8DataOut;			// TX data
	uint16_t	u16AddrIdx;			// Register address
	uint32_t	u32SpeedHz;			// Transfer speed [Hz]
};

struct t_FileToSwitch
{
	int			intFd;				// Device ID
	BOOL		bReadOnly;			// Read/Write Selection. 0 - Write, 1 - Read 
	BOOL		bByteOrder;			// The order or sequence of bytes. 1 - Bigendian, 0 - little endian
	BOOL		bArrayInvert;		// Static MAC or VLAN address invert
	uint32_t	u32SpeedHz;			// Transfer speed [Hz]
};


//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

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
///\param[in]				.bByteEndian	- The order or sequence of bytes. 1 - big endian, 0 - little endian
///\param[out]
//
///\retval		struct t_SpiDebug*
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptSpiReadWriteRegSwitch(struct t_SpiTransferSwitch* ptTransferBlock);

#endif //_SPIREADWRITESWITCH_H_