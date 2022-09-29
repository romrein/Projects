//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  globals.h
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.05.2022
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  global settings and includes for almost every file
//
//------------------------------------------------------------------------------------------------
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//------------------------------------------------------------------------------------------------
//  INCLUIDES
//------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>                // Needed for SPI port
#include <sys/ioctl.h>            // Needed for SPI port
#include <linux/spi/spidev.h>     // Needed for SPI port
#include <linux/types.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <error.h>

#include "func.h"
#include "conversion.h"
#include "spiReadWriteCPLD.h"
#include "WinTypes.h"

#define __ARMEB__
#include <asm/byteorder.h>


//------------------------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------------------------
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define BYTE										1
#define L2_BYTE										2
#define L3_BYTE										3
#define L4_BYTE										4
#define L8_BYTE										8

#define CPLD_VERSION_NUMBER							1175
#define EEPROM_VERSION_NUMBER						1175
#define SWITCH_VERSION_NUMBER						1175

#define SPI_SPEED									1000000
#define SPI_MODE									1
#define SPI_BITS									8
#define SPI_DELAY									0

#define SPI_MESSAGE									0x03
#define SPI_BUF_SIZE								130
#define MEM_AREA_SIZE								1024
#define CPLD_DATA_OFFSET							3
#define SWITCH_DATA_OFFSET							2
#define MAXBUF										0x3FF
#define READ										0x1
#define CYCLE_MAX									0xFFFFFFF
#define CPLD_MAX_ADDRESS							1024
#define CPLD_MAX_LEN								127

#define SWITCH_SINGLE								1
#define SWITCH_MAC									2
#define SWITCH_VLAN									3
#define SWITCH_MAX_REG_ADDRESS						0xBE
#define SWITCH_MAX_MAC_ADDRESS						0x01F
#define SWITCH_MAX_VLAN_ADDRESS						4096
#define SWITCH_BYTE_LENGTH							0x01
//#define SWITCH_READ_ACR								((uint8_t)(0x10))
#define SWITCH_READ_ACR								0x10
#define SWITCH_STATIC_MAC_LENGTH					0x08
#define SWITCH_SELECT_VLAN							0x04
#define SWITCH_VLAN_LENGTH_2						0x02
#define SWITCH_VLAN_LENGTH_8						0x08
#define SWITCH_ADVANCED_CONTROL_REGISTER_110		0x6E
#define SWITCH_ADVANCED_CONTROL_REGISTER_111		0x6F
#define SWITCH_ADVANCED_CONTROL_REGISTER_113		0x71

#define STANDBY										0
#define CTRL_START_PROCESS							1
#define CTRL_WAIT_FOR_READ							2
#define CTRL_WAIT_FOR_WRITE							3

#define SWITCH_READ_DONE							10
#define SWITCH_READ_STEP_1							1
#define SWITCH_READ_STEP_2							2
#define SWITCH_READ_STEP_3							3

#define SWITCH_WRITE_STEP_1							11
#define SWITCH_WRITE_STEP_2							12
#define SWITCH_WRITE_STEP_3							13

#define SWITCH_WRITE_DONE							14

#define SWITCH_ENABLE								0x01
#define SWITCH_DISABLE								0x00
#define SWITCH_CPLD_ADDR							133

#define EEPROM_MAX_ADDRESS							65536
#define EEPROM_READ_DONE							10
#define EEPROM_READ_STATUS							1
#define EEPROM_WRITE_DATA_1							2
#define EEPROM_WRITE_DATA_2							3
#define EEPROM_WRITE_CTRL_DATA						4
#define EEPROM_WRITE_CMD							5
#define EEPROM_READ_WRITE_STATUS					6

#define EEPROM_MAIN_WRITE							0
#define EEPROM_MAIN_READ							1

#define EEPROM_READ_DATA_1							7
#define EEPROM_READ_DATA_2							8

#define EEPROM_CMD_WRITE							0x5A
#define EEPROM_CMD_READ								0xCB

#define EEPROM_CPLD_ADDR_READ_DATA_0				256
#define EEPROM_CPLD_ADDR_READ_DATA_127				383
#define EEPROM_CPLD_ADDR_STATUS						384

#define EEPROM_CPLD_ADDR_WRITE_DATA_0				512
#define EEPROM_CPLD_ADDR_WRITE_DATA_127				639
#define EEPROM_CPLD_ADDR_SIGNATURE					640
#define EEPROM_CPLD_ADDR_BYTE_ADDRESS				641
#define EEPROM_CPLD_ADDR_SIZE						643
#define EEPROM_CPLD_ADDR_CMD						644

#define EEPROM_PAGE_SIZE							128
#define EEPROM_WAIT_STATUS_US						10000

#define EEPROM_BUSY									0x01
#define EEPROM_CRC_FAIL								0x02
#define EEPROM_CMD_ERROR							0x04

#define ANSI_COLOR_RED								"\x1b[31m"
#define ANSI_COLOR_GREEN							"\x1b[32m"
#define ANSI_COLOR_YELLOW							"\x1b[33m"
#define ANSI_COLOR_BLUE								"\x1b[34m"
#define ANSI_COLOR_MAGENTA							"\x1b[35m"
#define ANSI_COLOR_CYAN								"\x1b[36m"
#define ANSI_COLOR_RESET							"\x1b[0m"

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"

/* SPI frame opcodes */
#define SPI_ADDR_S									12
#define SPI_ADDR_M									((1 << SPI_ADDR_S) - 1)
#define SPI_TURNAROUND_S							1

#define SPICMD_RD									3
#define SPICMD_WR									2

//UART TO SPI

#define CPLD_DATA_OFFSET_UART						4
#define TIOWRITE		0x6600
#define TIOMESSAGE		0x6601

//Baudrate | Stop Bit Count
#define TIOWRITE_SB_BAUDRATE	0x6602
#define TIOREAD_SB_BAUDRATE	0x6603

//Collision Detection | Duplex Select | Parity Mode Select
#define TIOWRITE_CD_DS_PM		0x6604
#define TIOREAD_CD_DS_PM		0x6605

//RX Bus IDLE Timeout
#define TIOWRITE_RX_BUS_TOUT	0x6606
#define TIOREAD_RX_BUS_TOUT		0x6607

//TX IRQ Threshold
#define TIOWRITE_TX_IRQ_TS		0x6608
#define TIOREAD_TX_IRQ_TS		0x6609

//RX IRQ Threshold
#define TIOWRITE_RX_IRQ_TS		0x6610
#define TIOREAD_RX_IRQ_TS		0x6611

//IRQ Mask Config
#define TIOWRITE_IRQ_MASK		0x6612
#define TIOREAD_IRQ_MASK		0x6613

//UART Status
#define TIOREAD_IRQ_STATUS		0x6614
#define TIOREAD_TX_FIFO_L		0x6615
#define TIOREAD_RX_FIFO_L		0x6616

#define TIOREAD_INFO			0x6617

#define TIO_SPI_SPEED			0x6618
#define TIO_PORT_POLL			0x6619
#define TIOSPI					0x6620
#define TIO_DRIVER_TYP			0x6621
#define TIO_PORT_TYP_W			0x6622
#define TIO_PORT_TYP_R			0x6623

//Lace Bus Timeout
#define TIOWRITE_LACE_BUS_TOUT	0x6624
#define TIOREAD_LACE_BUS_TOUT	0x6625

//Loopback
#define TIOWRITE_LOOPBACK_MODE	0x6626
#define TIOWRITE_LOOPBACK_UART	0x6627


//SPI Addressraum
//Main Config
#define SPI_MAIN_CONF_LB_MODE	272

#define SPI_MAIN_CONF_LB_UART	274

#define SPI_ADDR_STATUS_OFFSET	8
#define SPI_ADDR_RX_BUF_OFFSET	2048
#define SPI_ADDR_CONFIG_OFFSET	16
#define SPI_ADDR_TX_BUF_OFFSET	2048

#define SPI_ADDR_IRQ_STATUS		512
#define SPI_ADDR_TX_FIFO_L		514
#define SPI_ADDR_RX_FIFO_L		516

#define SPI_ADDR_SB_BAUDRATE	33408
#define SPI_ADDR_CD_DS_PM		33409
#define SPI_ADDR_RX_BUS_TOUT	33410
#define SPI_ADDR_LACE_BUS_TOUT	33412
#define SPI_ADDR_TX_IRQ_TS		33414
#define SPI_ADDR_RX_IRQ_TS		33416
#define SPI_ADDR_IRQ_MASK		33418


//UART 0 TX Buffer (128 adresses)
#define SPI_ADDR_TX_BUF			33664

//UART 0 RX Buffer (128 adresses)
#define SPI_ADDR_RX_BUF			640


//------------------------------------------------------------------------------------------------
//  STRUCTS
//------------------------------------------------------------------------------------------------
// A structure for status feedback and debugging from kernel to user space.
// From low level to midle/higth level
struct t_SpiDebug
{
	int			intRet;				// Return
	uint8_t		au8TelOut[130];		// Transfer buffer
	uint8_t		au8TelIn[130];		// Receive Buffer
};

// A structure for status feedback and debugging, from midle to higth level
struct t_SpiDebugSwitch
{
	int			intRet;				// Return
	uint8_t		u8TranRegisterData;	// Transfered data to register
	uint8_t		u8RecRegisterData;	// Received data from register
	uint8_t		au8RecData[8];		// Received static MAC or VLAN adress
	uint8_t		au8TranData[8];		// Transfered static MAC or VLAN adress
};
// A structure for User Input Check
struct t_UserInputCheck
{
	BOOL		bSwitch;
	BOOL		bAddress;
	BOOL		bLength;
	BOOL		bFile;
	BOOL		bInput;
};
#endif //_GLOBAL_H_
