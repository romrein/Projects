#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//------------------------------------------------------------------------------------------------
//  INCLUIDES
//------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>                // Needed for SPI port
#include <sys/ioctl.h>            // Needed for SPI port
#include <linux/spi/spidev.h>     // Needed for SPI port
#include <linux/types.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

#include "func.h"
#include "conversion.h"
#include "spiReadWrite.h"


//------------------------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------------------------
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SPI_MESSAGE		0x03
#define MAXBUF			0x3FF
#define READ			0x1
#define CYCLE_MAX		0xFFFFFFF

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"

static int rwss = 0;

//------------------------------------------------------------------------------------------------
//  STRUCTS
//------------------------------------------------------------------------------------------------
struct t_SpiTransfer
{
	int			fd;					// Device ID
	uint8_t* ucDataOut;			// TX data
	uint8_t* ucDataIn;			// RX data
	uint16_t	unAdr;				// Source address
	uint32_t	udnFreq;			// Speed/frequency [Hz]
	uint8_t		ucLength;			// Data length [Byte], not Telegram length
	uint8_t		bRead;				// Read/Write Selection. 0 - Write, 1 - Read
	uint8_t		bFlag;				// Type of transfer data. 0 - Random, 1 - User (ucDataIn)
};
struct t_SpiDebug
{
	int			ret;				// Return
	uint8_t		ucTelOut[130];		// Transfer buffer
	uint8_t		ucTelIn[130];		// Receive Buffer
};

#endif