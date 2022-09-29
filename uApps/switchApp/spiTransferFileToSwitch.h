//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  spiTransferFileToSwitch.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  28.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - header for "spiTransferFileToSwitch.c"
//
//------------------------------------------------------------------------------------------------
#ifndef _SPITRANSFERFILETOSWITCH_H_
#define _SPITRANSFERFILETOSWITCH_H_

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (declaration)
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// iFileTransferToSwitch()
//
///\brief			The interface for config file transfer to the switch.
//
///\param[in]			ptCmdSwitch.
///\param[in]				.intFd;				 Device ID
///\param[in]				.bReadOnly;			 Read/Write Selection. 0 - Write, 1 - Read 
///\param[in]				.bArrayInvert;			 Static MAC or VLAN address invert
///\param[in]				.u32SpeedHz;			 Transfer speed [Hz]
///\param[out]
//
///\retval		int
//------------------------------------------------------------------------------------------------
int iFileTransferToSwitch(int intFd, char* filename, struct t_FileToSwitch* tFileToSwitch);

#endif //_SPITRANSFERFILETOSWITCH_H_
