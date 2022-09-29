//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  main.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.06.2021
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A high level application. User interface for communication with a Switch.
//	- Transfering the configuration file to switch
//  - main.c - program starts here
//
//------------------------------------------------------------------------------------------------

#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

//------------------------------------------------------------------------------------------------
//  FUNCTION PROTOTYPEN (declaration)
//------------------------------------------------------------------------------------------------
static int parse_opts(int argc, char* argv[]);

//------------------------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------------------------

const char* pcDevice = "/dev/spidev0.0";						// Default Switch device
const char* pcDeviceCPLD = "/dev/spidev1.0";					// Default CPLD device
uint32_t u32Mode = 1;
uint8_t u8Bits = 8;
uint32_t u32SpeedHz = 1000000;
uint16_t u16Delay = 0;
static char* input_file;

struct t_UserInputCheck tUserCmd;

int intAddrIdx = 0;

uint8_t u8Switch = 0;
uint8_t u8Check = 0;

BOOL bReadOnly = TRUE;
BOOL bInputError = FALSE;

char* pcInputTx = NULL;

uint16_t u16AddrIdx = 0;

uint32_t u32AddrIdx = 0;


int main(int argc, char* argv[])
{
	static struct t_SpiDebugSwitch* ptSpiDataFromSwitch;
	static struct t_CmdToSwitch tCmdBufferSW;
	static struct t_FileToSwitch tFileBufferSW;

	int intReturn = 0;
	int intFd;
	int intFdCPLD;

	uint8_t au8TelTx[SPI_BUF_SIZE];

	memset(&tUserCmd, 0, sizeof(tUserCmd));

	printf("\n");

	intReturn = parse_opts(argc, argv);

	//---------------------------------------------------------------------------------------------------
	// Control of user entry  
	//---------------------------------------------------------------------------------------------------

	if (bInputError == TRUE)
	{
		exit(1);
	}

	// Status coding
	u8Check |= ( (tUserCmd.bSwitch << 3) | (tUserCmd.bAddress << 2) | (tUserCmd.bFile << 1) | (tUserCmd.bInput << 0) );

	//Check status
	if ( (u8Check != 2) && (u8Check != 12) && (u8Check != 13))
	{
		printf("Error: user command line input incomplete.\n");
		exit(1);
	}

	if ( (intAddrIdx < 0) )
	{
		printf("Error: the address is out of range\n");
		exit(1);
	}
	if ( (u8Switch != 1) && (u8Switch != 2) && (u8Switch != 3) && (input_file == NULL) )
	{
		printf("Error: command line entry. Try -h for help\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Device open for the switch 
	//---------------------------------------------------------------------------------------------------
	if ((intFd = open(pcDevice, O_RDWR)) < 0)
	{
		printf("Error: Open Device\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Device open for the CPLD
	//---------------------------------------------------------------------------------------------------
	if ((intFdCPLD = open(pcDeviceCPLD, O_RDWR)) < 0)
	{
		printf("Error: Open Device\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Switch ON
	//---------------------------------------------------------------------------------------------------
	intReturn = iSwitchON(intFdCPLD);
	if (intReturn < 0)
	{
		printf("Error: switch enable failed\n");
		exit(1);
	}
	else
	{
		/*printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                                 Switch enabled                                    **\n");
		printf("***************************************************************************************\n");*/
	}

	//---------------------------------------------------------------------------------------------------
	// Function: Input config file (Only for switch)
	//---------------------------------------------------------------------------------------------------
	if ( (input_file) )
	{
		/*printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                          Config file transfer to switch                           **\n");
		printf("***************************************************************************************\n");*/

		tFileBufferSW.intFd = intFd;
		tFileBufferSW.bReadOnly = FALSE;
		tFileBufferSW.u32SpeedHz = SPI_SPEED;

		// Calling a low level function with the parameter handover for the file transfer to the switch
		intReturn = iFileTransferToSwitch(intFd, input_file, &tFileBufferSW);
		if (intReturn < 0)
		{
			printf("Error: Read from the file\n");
			exit(1);
		}

		//---------------------------------------------------------------------------------------------------
		// Function: Switch OFF
		//---------------------------------------------------------------------------------------------------
		intReturn = iSwitchOFF(intFdCPLD);
		if (intReturn < 0)
		{
			printf("Error: switch disable failed\n");
			exit(1);
		}
		else
		{
			/*printf("\n\n");
			printf("***************************************************************************************\n");
			printf("**                                 Switch disabled                                   **\n");
			printf("***************************************************************************************\n");*/
		}

		return intReturn;
	}

	// Memory allocation for switch-transfer
	ptSpiDataFromSwitch = malloc(sizeof(struct t_SpiDebugSwitch));
	if (!ptSpiDataFromSwitch)
	{
		printf("Error: can't allocate TX buffer\n");
		exit(1);
	}

	//---------------------------------------------------------------------------------------------------
	// Switch. Single register transfer 
	//---------------------------------------------------------------------------------------------------
	if (u8Switch == SWITCH_SINGLE)
	{	
		//printf("\n\n");
		//printf("***************************************************************************************\n");
		//printf("**                    Single register SPI transfer to switch                         **\n");
		//printf("***************************************************************************************\n");

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		if ((u32AddrIdx > SWITCH_MAX_REG_ADDRESS))
		{
			printf("Error: the address is out of range\n");
			exit(1);
		}

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			// Calling string conversion function with the parameter handover
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				printf("Error: comand line conversion\n");
				goto end;
			}
		}

		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = SPI_SPEED;

		// Calling a midle level function for the single register with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchSingleReg(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			printf("Error: write single register to the switch\n");
			intReturn = ptSpiDataFromSwitch->intRet;
			goto end;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			vPrintoutLite(TRUE, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->u8RecRegisterData), BYTE);
		}
		else
		{
			vPrintoutLite(FALSE, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->u8RecRegisterData), BYTE);
		}
		printf("\n\n");
	}
	//---------------------------------------------------------------------------------------------------
	// Switch. Static MAC address transfer 
	//---------------------------------------------------------------------------------------------------
	else if (u8Switch == SWITCH_MAC)
	{
		/*printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                    Static MAC adress SPI-transfer to switch                       **\n");
		printf("***************************************************************************************\n");*/

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		if ((u32AddrIdx > SWITCH_MAX_MAC_ADDRESS))
		{
			printf("Error: the address is out of range\n");
			exit(1);
		}

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			// Calling string conversion function with the parameter handover
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				printf("Error: comand line conversion\n");
				goto end;
			}
			if (intReturn != L8_BYTE)
			{
				printf("Error: MAC entry length\n");
				goto end;
			}
		}

		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = SPI_SPEED;

		// Calling a midle level function for the static MAC address with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchStaticMac(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			printf("Error: write MAC address to the switch\n");
			intReturn = ptSpiDataFromSwitch->intRet;
			goto end;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			vPrintoutLite(TRUE, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->au8RecData), SWITCH_STATIC_MAC_LENGTH);
		}
		else
		{
			vPrintoutLite(FALSE, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->au8TranData), SWITCH_STATIC_MAC_LENGTH);
		}
		printf("\n\n");
	}
	//---------------------------------------------------------------------------------------------------
	// Switch. VLAN address transfer 
	//---------------------------------------------------------------------------------------------------
	else if (u8Switch == SWITCH_VLAN)
	{
		/*printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                        VLAN adress SPI-transfer to switch                         **\n");
		printf("***************************************************************************************\n");*/

		// Clearing the transmission buffer
		memset(&au8TelTx, 0, sizeof(au8TelTx));

		if ((u32AddrIdx + SWITCH_VLAN_LENGTH_2) > SWITCH_MAX_VLAN_ADDRESS)
		{
			printf("Error: the address is out of range\n");
			exit(1);
		}

		tCmdBufferSW.intFd = intFd;
		tCmdBufferSW.bReadOnly = bReadOnly;

		if (tCmdBufferSW.bReadOnly == FALSE)
		{
			// Calling string conversion function with the parameter handover
			intReturn = intStringConversion(pcInputTx, &au8TelTx[0]);
			if (intReturn < 0)
			{
				//Conversion error
				printf("Error: comand line conversion\n");
				goto end;
			}
			if (intReturn != L2_BYTE)
			{
				printf("Error: VLAN entry length\n");
				goto end;
			}
		}
		
		tCmdBufferSW.pu8DataOut = &au8TelTx[0];
		tCmdBufferSW.u16AddrIdx = u16AddrIdx;
		tCmdBufferSW.u32SpeedHz = SPI_SPEED;

		// Calling a midle level function for the VLAN address with the parameter handover
		ptSpiDataFromSwitch = ptSpiTransferSwitchVLan(&tCmdBufferSW);
		if (ptSpiDataFromSwitch->intRet < 0)
		{
			printf("Error: write VLAN address to the switch\n");
			intReturn = ptSpiDataFromSwitch->intRet;
			goto end;
		}

		//Display transfer buffer
		if (tCmdBufferSW.bReadOnly == TRUE)
		{
			vPrintoutVlan(TRUE, (u16AddrIdx) / 4, (u16AddrIdx) % 4, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->au8RecData), SWITCH_VLAN_LENGTH_2);
		}
		else
		{
			vPrintoutVlan(FALSE, (u16AddrIdx) / 4, (u16AddrIdx) % 4, u16AddrIdx);
			// Printout from buffer
			vPrintBufMulti(&(ptSpiDataFromSwitch->au8TranData), SWITCH_VLAN_LENGTH_2);
		}
		printf("\n\n");
	}

end:
	free(ptSpiDataFromSwitch);

	//---------------------------------------------------------------------------------------------------
	// Function: Switch OFF
	//---------------------------------------------------------------------------------------------------
	intReturn = iSwitchOFF(intFdCPLD);
	if (intReturn < 0)
	{
		printf("Error: switch disable failed\n");
		exit(1);
	}
	else
	{
		/*printf("\n\n");
		printf("***************************************************************************************\n");
		printf("**                                 Switch disabled                                   **\n");
		printf("***************************************************************************************\n");*/
	}

	return intReturn;
}

//------------------------------------------------------------------------------------------------
// parse_opts()
//
///\brief					The interface for command line.
//
///\param[in]				argc		- name of the option
///\param[in]				argv[]		- Decode options from the vector argv
///\param[out]
//
///\retval		int			The option character when a short option is recognized. For a long option, 
//							they return val if flag is NULL
//------------------------------------------------------------------------------------------------
static int parse_opts(int argc, char* argv[])
{
	int option_index = 0;
	int check = 0;

	if (argc < 2)
	{
		bInputError = TRUE;
		printf("Error: too few parameters in command line. Try -h for help\n");
		return -1;
	}

	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "no-cs",   0, 0, 'N' },
			{ "switch",  1, 0, 'S' },
			{ NULL, 0, 0, 0 },
		};
		
		int c;
		c = getopt_long(argc, argv, "hD:s:d:b:HOLCNA:t:f:S:",
			lopts, &option_index);

		if (c == -1)
		{
			break;
		}

		switch (c) {
		case 0:
			printf("option %s", lopts[option_index].name);
			if (optarg)
				printf(" with arg %s", optarg);
			printf("\n");
			break;
		case 'D':
			pcDevice = optarg;
			break;
		case 's':
			u32SpeedHz = (uint32_t)atoi(optarg);
			break;
		case 'd':
			u16Delay = (uint16_t)atoi(optarg);
			break;
		case 'b':
			u8Bits = (uint8_t)atoi(optarg);
			break;
		case 'H':
			u32Mode |= SPI_CPHA;
			break;
		case 'O':
			u32Mode |= SPI_CPOL;
			break;
		case 'L':
			u32Mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			u32Mode |= SPI_CS_HIGH;
			break;
		case 'N':
			u32Mode |= SPI_NO_CS;
			break;
		case 't':
			pcInputTx = optarg;
			bReadOnly = FALSE;
			tUserCmd.bInput = TRUE;
			break;
		case 'A':
			check = intCheckChar(optarg);
			if (check < 0)
			{
				bInputError = TRUE;
				printf("Error: incorrect parameter in command line. Try -h for help\n");
			}
			else
			{
				intAddrIdx = atoi(optarg);
				u16AddrIdx = (uint16_t)atoi(optarg);
				u32AddrIdx = (uint32_t)atoi(optarg);
				tUserCmd.bAddress = TRUE;
			}
			break;
		case 'f':
			input_file = optarg;
			tUserCmd.bFile = TRUE;
			break;
		case 'S':
			check = intCheckChar(optarg);
			if (check < 0)
			{
				bInputError = TRUE;
				printf("Error: incorrect parameter in command line. Try -h for help\n");
			}
			else
			{
				u8Switch = (uint8_t)atoi(optarg);
				tUserCmd.bSwitch = TRUE;
			}
			break;
		case 'h':
			vPrint_help_switch(argv[0], SWITCH_VERSION_NUMBER);
			break;
		default:
			bInputError = TRUE;
			printf("Error: command line. Try -h for help\n");
			break;
		}	
	}
	if (optind < argc) {
		printf("Error: non-option ARGV-elements - ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
		bInputError = TRUE;
		return -1;
	}

	return 0;
}