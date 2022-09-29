//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTransferCmdToSwitch.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  23.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiTransferCmdToSwitch.c"
//
//------------------------------------------------------------------------------------------------

#ifndef _SPITRANSFERCMDTOSWITCH_H_
#define _SPITRANSFERCMDTOSWITCH_H_

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
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
struct t_SpiDebugSwitch* ptSpiTransferSwitchSingleReg(struct t_CmdToSwitch* ptCmdSwitch);

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchStaticMac()
//
///\brief					Function to read write data to the static MAC address table.
//
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
struct t_SpiDebugSwitch* ptSpiTransferSwitchStaticMac(struct t_CmdToSwitch* ptCmdSwitch);

//------------------------------------------------------------------------------------------------
// ptSpiTransferSwitchVLan()
//
///\brief					Function to read write data to the VLAN address table.
//
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
struct t_SpiDebugSwitch* ptSpiTransferSwitchVLan(struct t_CmdToSwitch* ptCmdSwitch);

#endif //_SPITRANSFERCMDTOSWITCH_H_