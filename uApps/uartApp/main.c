//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.05.2022
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A middle level application.
//
//------------------------------------------------------------------------------------------------

#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

#include <linux/types.h>
#include <linux/serial.h>

struct t_UserInputCheck tUserCmd;

BOOL bReadOnly = TRUE;
BOOL bInputError = FALSE;
BOOL bMultiRW = FALSE;

BOOL bBaudRate = FALSE;
BOOL bStopBit = FALSE;
BOOL bCollDet = FALSE;
BOOL bDuplSel = FALSE;
BOOL bParityMode = FALSE;
BOOL bLaceBusMode = FALSE;
BOOL bTxIrqThr = FALSE;
BOOL bRxIrqThr = FALSE;
BOOL bIrqMask = FALSE;
BOOL bIrqStatus = FALSE;
BOOL bTxFIFO = FALSE;
BOOL bRxFIFO = FALSE;
BOOL bRxTimout = FALSE;
BOOL bLaceBusTimout = FALSE;
BOOL bUartMask = FALSE;
BOOL bUartTimout = FALSE;

int intAddrIdx = 0;
int intLength = 0;

char* pcInputTx = NULL;

uint8_t u8UserLength = 0;
uint8_t* auWriteBuf;
uint8_t* auReadBuf;
uint8_t u8Check = 0;

uint8_t u8Temp = 0;
uint8_t u8BaudRate = 0;
uint8_t u8StopBit = 0;
uint8_t u8CollDet = 0;
uint8_t u8DuplSel = 0;
uint8_t u8ParityMode = 0;
uint8_t u8LaceBusMode = 0;
uint8_t u8TxIrqThr = 0;
uint8_t u8RxIrqThr = 0;
uint8_t u8IrqMask = 0;
uint8_t u8IrqStatus = 0;
uint8_t u8TxFIFO = 0;
uint8_t u8RxFIFO = 0;

uint16_t u16RxTimout = 0;
uint16_t u16LaceBusTimout = 0;
uint16_t u16UartTimout = 0;

uint16_t u16UserLength = 0;
uint16_t u16AddrIdx = 0;

uint16_t u16TxIrqThr = 0;
uint16_t u16RxIrqThr = 0;

uint16_t u16IrqMask = 0;
uint16_t u16UartMask = 0;

uint32_t u32AddrIdx = 0;




static void pabort(const char* s)
{
	perror(s);
	abort();
}

const char* pcDevice = "/dev/ttyspi2";
uint32_t u32Mode = 1;
uint8_t u8Bits = 8;
uint32_t u32SpeedHz = 1000000;
uint16_t u16Delay = 0;
uint8_t u8CS = 0;


static const char* device = "/dev/ttyspi0";


static char* input_file;
static char* output_file;


static int verbose;
static int transfer_size;
static int iterations;
static int interval = 5; /* interval in seconds for showing transfer rate */

uint8_t default_tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0x0D,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = { 0, };
char* input_tx;


static void print_usage(const char* prog)
{
	printf("Usage: %s [-DsbdlHOLC3vpNR24SI]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
		"  -s --speed    max speed (Hz)\n"
		"  -d --delay    delay (usec)\n"
		"  -b --bpw      bits per word\n"
		"  -i --input    input data from a file (e.g. \"test.bin\")\n"
		"  -o --output   output data to a file (e.g. \"results.bin\")\n"
		"  -l --loop     loopback\n"
		"  -H --cpha     clock phase\n"
		"  -O --cpol     Lace Bus Timeout\n"
		"  -L --lsb      Lace bus enable/desable\n"
		"  -C --cs-high  chip select active high\n"
		"  -3 --3wire    SI/SO signals shared\n"
		"  -v --verbose  Verbose (show tx buffer)\n"
		"  -p            Send data (e.g. \"1234\\xde\\xad\")\n"
		"  -N --no-cs    no chip select\n"
		"  -R --ready    slave pulls low to pause\n"
		"  -2 --dual     dual transfer\n"
		"  -4 --quad     quad transfer\n"
		"  -S --size     transfer size\n"
		"  -I --iter     iterations\n");
	exit(1);
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
			{ NULL, 0, 0, 0 },
		};

		int c;
		c = getopt_long(argc, argv, "hD:B:P:M:Ffsi:x:S:T:d:b:H:O:L:C:N:U:A:l:t:c:",
			lopts, NULL);

		if (c == -1)
		{
			break;
		}

		switch (c) {
		case 'D':
			pcDevice = optarg;
			break;
		case 'H':
			u32SpeedHz = (uint32_t)atoi(optarg);
			break;
		case 'd':
			u8DuplSel = (uint8_t)atoi(optarg);
			bDuplSel = TRUE;
			break;
		case 'b':
			u8Bits = (uint8_t)atoi(optarg);
			break;
		case 'c':
			u8CS = (uint8_t)atoi(optarg);
			break;
		case 'B':
			u8BaudRate = (uint8_t)atoi(optarg);
			bBaudRate = TRUE;
			break;
		case 'i':
			u16TxIrqThr = (uint16_t)atoi(optarg);
			bTxIrqThr = TRUE;
			break;
		case 'x':
			u16RxIrqThr = (uint16_t)atoi(optarg);
			bRxIrqThr = TRUE;
			break;
		case 'F':
			//u8TxFIFO = (uint8_t)atoi(optarg);
			bTxFIFO = TRUE;
			break;
		case 'f':
			//u8RxFIFO = (uint8_t)atoi(optarg);
			bRxFIFO = TRUE;
			break;
		case 'P':
			u8ParityMode = (uint8_t)atoi(optarg);
			bParityMode = TRUE;
			break;
		case 'M':
			//u8IrqMask = (uint8_t)atoi(optarg);
			u16IrqMask = (uint16_t)atoi(optarg);
			bIrqMask = TRUE;
			break;
		case 'T':
			u16RxTimout = (uint16_t)atoi(optarg);
			bRxTimout = TRUE;
			break;
		case 'S':
			u8StopBit = (uint8_t)atoi(optarg);
			bStopBit = TRUE;
			break;
		case 's':
			//u8IrqStatus = (uint8_t)atoi(optarg);
			bIrqStatus = TRUE;
			break;
		case 'O':
			u16LaceBusTimout = (uint16_t)atoi(optarg);
			bLaceBusTimout = TRUE;
			break;
		case 'L':
			u8LaceBusMode = (uint8_t)atoi(optarg);
			bLaceBusMode = TRUE;
			break;
		case 'C':
			u8CollDet = (uint8_t)atoi(optarg);
			bCollDet = TRUE;
			break;
		case 'N':
			u16UartMask = (uint16_t)atoi(optarg);
			bUartMask = TRUE;
			break;
		case 'U':
			u16UartTimout = (uint16_t)atoi(optarg);
			bUartTimout = TRUE;
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
				u32AddrIdx = (uint32_t)atoi(optarg);
				tUserCmd.bAddress = TRUE;
			}
			break;
		case 'l':
			check = intCheckChar(optarg);
			if (check < 0)
			{
				bInputError = TRUE;
				printf("Error: incorrect parameter in command line. Try -h for help\n");
			}
			else
			{
				intLength = atoi(optarg);
				u16UserLength = (uint16_t)atoi(optarg);
				u8UserLength = (uint8_t)atoi(optarg);
				tUserCmd.bLength = TRUE;
			}
			break;
		case 'h':
			vPrint_help_cpld(argv[0], CPLD_VERSION_NUMBER);
			break;
		default:
			printf("Error: command line. Try -h for help\n");
			bInputError = TRUE;
			break;
		}
	}
	if (optind < argc) {
		printf("Error: non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
		bInputError = TRUE;
		return -1;
	}

	return 0;
}
static uint64_t _read_count;
static uint64_t _write_count;


int main(int argc, char* argv[])
{
	int ret = 0;
	int fd;
	uint8_t* tx;
	ssize_t bytes;
	struct serial_struct sInfo;
	uint8_t u8uart;
	uint16_t u16uart;

	u8uart = 0;
	u16uart = 0;

	static struct t_SpiDebug* ptSpiBuffer;
	static struct t_SpiTransfer tTransferBuffer;

	int intReturn = 0;
	int intFd;

	uint8_t au8TelTx[SPI_BUF_SIZE];
	uint8_t au8TelRx[SPI_BUF_SIZE];

	size_t size;

	memset(&sInfo, 0, sizeof(sInfo));

	printf("parse_opts");

	//---------------------------------------------------------------------------------------------------
	// User command line input 
	//---------------------------------------------------------------------------------------------------
	intReturn = parse_opts(argc, argv);

	fd = open(pcDevice, O_RDWR);
	if (fd < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", pcDevice);
	}

	if ((tUserCmd.bInput == TRUE) | (tUserCmd.bAddress == TRUE))
	{
		goto writeCmd;
	}

	ret = ioctl(fd, TIOREAD_SB_BAUDRATE, &u8uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("Baudrate | Stop Bit Count: %d\n", u8uart);
		if (bBaudRate == TRUE) 
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8BaudRate: %d\n", u8BaudRate);
			u8uart = (u8uart & 0x0F) | (u8BaudRate << 4);
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_SB_BAUDRATE, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
		if (bStopBit == TRUE)
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8StopBit: %d\n", u8StopBit);
			u8uart = (u8uart & 0xF0) | u8StopBit;
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_SB_BAUDRATE, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
			u8uart = 0;
		}
	}

	ret = ioctl(fd, TIOREAD_CD_DS_PM, &u8uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("LaceBus Mode | Collision Detection | Duplex Select | Parity Mode Select: %d\n", u8uart);
		if (bLaceBusMode == TRUE)
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8LaceBusMode: %d\n", u8LaceBusMode);
			u8uart = (u8uart & 0xEF) | (u8LaceBusMode << 4);
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_CD_DS_PM, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
		if (bCollDet == TRUE)
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8CollDet: %d\n", u8CollDet);
			u8uart = (u8uart & 0xF7) | (u8CollDet << 3);
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_CD_DS_PM, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
		if (bDuplSel == TRUE)
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8DuplSel: %d\n", u8DuplSel);
			u8uart = (u8uart & 0xFB) | (u8DuplSel << 2);
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_CD_DS_PM, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
		if (bParityMode == TRUE)
		{
			printf("u8uart IN: %d\n", u8uart);
			printf("u8ParityMode: %d\n", u8ParityMode);
			u8uart = (u8uart & 0xFC) | u8ParityMode;
			printf("u8uart OUT: %d\n", u8uart);
			ret = ioctl(fd, TIOWRITE_CD_DS_PM, &u8uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
		

	}


	ret = ioctl(fd, TIOREAD_RX_BUS_TOUT, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("RX Bus IDLE Timeout: %d\n", u16uart);
		if (bRxTimout == TRUE)
		{
			printf("u16uart IN: %d\n", u16uart);
			printf("u16RxTimout: %d\n", u16RxTimout);
			u16uart = u16RxTimout;
			printf("u16uart OUT: %d\n", u16uart);
			ret = ioctl(fd, TIOWRITE_RX_BUS_TOUT, &u16uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
	}

	ret = ioctl(fd, TIOREAD_LACE_BUS_TOUT, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("Lace Bus Timeout: %d\n", u16uart);
		if (bLaceBusTimout == TRUE)
		{
			printf("u16uart IN: %d\n", u16uart);
			printf("u16RxTimout: %d\n", u16LaceBusTimout);
			u16uart = u16LaceBusTimout;
			printf("u16uart OUT: %d\n", u16uart);
			ret = ioctl(fd, TIOWRITE_LACE_BUS_TOUT, &u16uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
	}

	ret = ioctl(fd, TIOREAD_TX_IRQ_TS, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("TX IRQ Threshold: %d\n", u16uart);
		if (bTxIrqThr == TRUE)
		{
			printf("u16uart IN: %d\n", u16uart);
			printf("u16TxIrqThr: %d\n", u16TxIrqThr);
			u16uart = u16TxIrqThr;
			printf("u8uart OUT: %d\n", u16uart);
			ret = ioctl(fd, TIOWRITE_TX_IRQ_TS, &u16uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
	}

	ret = ioctl(fd, TIOREAD_RX_IRQ_TS, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("RX IRQ Threshold: %d\n", u16uart);
		if (bRxIrqThr == TRUE)
		{
			printf("u16uart IN: %d\n", u16uart);
			printf("u16RxIrqThr: %d\n", u16RxIrqThr);
			u16uart = u16RxIrqThr;
			printf("u8uart OUT: %d\n", u16uart);
			ret = ioctl(fd, TIOWRITE_RX_IRQ_TS, &u16uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
	}

	ret = ioctl(fd, TIOREAD_IRQ_MASK, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("IRQ Mask Config: %d\n", u16uart);
		if (bIrqMask == TRUE)
		{
			printf("u16uart IN: %d\n", u16uart);
			printf("u16IrqMask: %d\n", u16IrqMask);
			u16uart = u16IrqMask;
			printf("u16uart OUT: %d\n", u16uart);
			ret = ioctl(fd, TIOWRITE_IRQ_MASK, &u16uart);
			if (ret != 0)
			{
				printf("not all bytes ioctl to output file\n");
			}
		}
	}

	ret = ioctl(fd, TIOREAD_IRQ_STATUS, &u8uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("IRQ Status: %d\n", u8uart);
	}

	ret = ioctl(fd, TIOREAD_TX_FIFO_L, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("TX FIFO Level: %d\n", u16uart);
	}

	ret = ioctl(fd, TIOREAD_RX_FIFO_L, &u16uart);
	if (ret != 0)
	{
		printf("not all bytes ioctl to output file\n");
	}
	else
	{
		printf("RX FIFO Level: %d\n", u16uart);
	}

	if (bUartMask == TRUE)
	{
		printf("u16uart IN: %d\n", u16uart);
		printf("u16UartMask: %d\n", u16UartMask);
		u16uart = u16UartMask;
		printf("u16uart OUT: %d\n", u16uart);
		ret = ioctl(fd, TIOWRITE_LOOPBACK_MODE, &u16uart);
		if (ret != 0)
		{
			printf("not all bytes ioctl to output file\n");
		}
	}

	if (bUartTimout == TRUE)
	{
		printf("u16uart IN: %d\n", u16uart);
		printf("u16UartTimout: %d\n", u16UartTimout);
		u16uart = u16UartTimout;
		printf("u16uart OUT: %d\n", u16uart);
		ret = ioctl(fd, TIOWRITE_LOOPBACK_UART, &u16uart);
		if (ret != 0)
		{
			printf("not all bytes ioctl to output file\n");
		}
	}

	goto end_status;
writeCmd:

	// Clearing the transmission buffer
	memset(&au8TelTx, 0, sizeof(au8TelTx));
	memset(&au8TelRx, 0, sizeof(au8TelRx));
	memset(&tTransferBuffer, 0, sizeof(tTransferBuffer));

	ptSpiBuffer = malloc(sizeof(struct t_SpiDebug));

	tTransferBuffer.intFd = fd;
	tTransferBuffer.u32Freq = SPI_SPEED;

	//---------------------------------------------------------------------------------------------------
	// Multiline transfer to CPLD 
	//---------------------------------------------------------------------------------------------------
	// 
	// u16AddrIdx = (uint16_t)atoi(optarg);
	// tUserCmd.bAddress = TRUE;
		// Write to CPLD
	if (bReadOnly == FALSE)
	{

		if (tUserCmd.bAddress)
		{
			tTransferBuffer.u32Address = u32AddrIdx;
		}
		else
		{
			tTransferBuffer.u32Address = SPI_ADDR_TX_BUF + (SPI_ADDR_TX_BUF_OFFSET * u8CS);
		}

		intReturn = intStringSize(pcInputTx);
		size = intReturn;
		auWriteBuf = malloc(size);
		if (!auWriteBuf)
		{
			printf("Error: can't allocate write buffer\n");
			exit(1);
		}
		auReadBuf = malloc(size);
		if (!auReadBuf)
		{
			printf("Error: can't allocate read buffer\n");
			exit(1);
		}

		intReturn = intStringConversion(pcInputTx, auWriteBuf);
		if (intReturn < 0)
		{
			//Conversion error
			printf("Error: command line conversion\n");
			goto end;
		}
		else
		{
			tTransferBuffer.pu8DataOut = auWriteBuf;
			tTransferBuffer.pu8DataIn = auReadBuf;
			tTransferBuffer.u8Length = (uint8_t)intReturn;
			tTransferBuffer.u16Length = (uint16_t)intReturn;
			tTransferBuffer.bRead = FALSE;

			// Calling a low level function with the parameter handover for the SPI transfer.
			ptSpiBuffer = ptSpiReadWriteTelMulti(&tTransferBuffer);
			if (ptSpiBuffer->intRet < 0)
			{
				//Errorhandling.
				printf("Error: write multiline transfer\n");
				intReturn = ptSpiBuffer->intRet;
				goto end;
			}
			else
			{
				vPrintout(FALSE, tTransferBuffer.u32Address, intReturn);
				//Display transfer buffer
				vPrintBufMulti(auWriteBuf, (uint16_t)intReturn);
			}
		}
		free(auWriteBuf);
		free(auReadBuf);
	}
	// Read from CPLD
	else
	{
		if (tUserCmd.bAddress)
		{
			tTransferBuffer.u32Address = u32AddrIdx;
		}
		else
		{
			tTransferBuffer.u32Address = SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * u8CS);
		}
		
		if (tUserCmd.bLength == FALSE)
		{
			printf("Error: the length is out of range\n");
			exit(1);
		}

		auWriteBuf = malloc(u16UserLength);
		if (!auWriteBuf)
		{
			printf("Error: can't allocate write buffer\n");
			exit(1);
		}
		auReadBuf = malloc(u16UserLength);
		if (!auReadBuf)
		{
			printf("Error: can't allocate read buffer\n");
			exit(1);
		}

		tTransferBuffer.pu8DataOut = auWriteBuf;
		tTransferBuffer.pu8DataIn = auReadBuf;
		tTransferBuffer.u8Length = u8UserLength;
		tTransferBuffer.u16Length = u16UserLength;
		tTransferBuffer.bRead = TRUE;

		// Calling a low level function with the parameter handover for the SPI transfer.
		ptSpiBuffer = ptSpiReadWriteTelMulti(&tTransferBuffer);
		if (ptSpiBuffer->intRet < 0)
		{
			//Errorhandling.
			printf("Error: read multiline transfer\n");
			intReturn = ptSpiBuffer->intRet;
			goto end;
		}
		else
		{
			//Display transfer buffer
			vPrintout(TRUE, tTransferBuffer.u32Address, u16UserLength);
			vPrintBufMulti(auReadBuf, (uint16_t)u16UserLength);
		}
		free(auWriteBuf);
		free(auReadBuf);
	}

	free(ptSpiBuffer);

	return intReturn;

end:
	free(auWriteBuf);
	free(auReadBuf);

end_single:

	free(ptSpiBuffer);

end_status:
	close(fd);
	return intReturn;

	return ret;
}
