//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.08.2022
//  File            :  polling16t16P3TLace.c
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//	- A middle level application.
//  - UART SPI testing utility (using uart-spidev driver)
//
//------------------------------------------------------------------------------------------------

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/serial.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//UART TO SPI
#define TIOWRITE		0x6600
#define TIOMESSAGE		0x6601

//Baudrate | Stop Bit Count
#define TIOWRITE_SB_BAUDRATE	0x6602
#define TIOREAD_SB_BAUDRATE	0x6603

//Collision Detection | Duplex Select | Parity Mode Select
#define TIOWRITE_CD_DS_PM		0x6604
#define TIOREAD_CD_DS_PM		0x6605

//RX Bus IDLE Timeout
#define TIOWRITE_RX_BUS_TOUT		0x6606
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
#define TIO_PORT_POLL			0x6619
#define TIOSPI				    0x6620
#define TIO_DRIVER_TYP			0x6621
#define TIO_PORT_TYP_W			0x6622
#define TIO_PORT_TYP_R			0x6623


//SPI Addressraum
//#define SPI_ADDR_STATUS_OFFSET		8
//#define SPI_ADDR_RX_BUF_OFFSET		2048
//#define SPI_ADDR_CONFIG_OFFSET		16
//#define SPI_ADDR_TX_BUF_OFFSET		2048

//#define SPI_ADDR_IRQ_STATUS		512
//#define SPI_ADDR_TX_FIFO_L		513
//#define SPI_ADDR_RX_FIFO_L		515

//#define SPI_ADDR_SB_BAUDRATE		33408
//#define SPI_ADDR_CD_DS_PM		33409
//#define SPI_ADDR_RX_BUS_TOUT		33410
//#define SPI_ADDR_TX_IRQ_TS		33412
//#define SPI_ADDR_RX_IRQ_TS		33414
//#define SPI_ADDR_IRQ_MASK		33416


//UART 0 TX Buffer (128 adresses)
//#define SPI_ADDR_TX_BUF			33664

//UART 0 RX Buffer (128 adresses)
//#define SPI_ADDR_RX_BUF			640


static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/ttyspi0";
static const char *device1 = "/dev/ttyspi1";
static const char *device2 = "/dev/ttyspi2";
static const char *device3 = "/dev/ttyspi3";
static const char *device4 = "/dev/ttyspi4";
static const char *device5 = "/dev/ttyspi5";
static const char *device6 = "/dev/ttyspi6";
static const char *device7 = "/dev/ttyspi7";
static const char *device8 = "/dev/ttyspi8";
static const char *device9 = "/dev/ttyspi9";
static const char *device10 = "/dev/ttyspi10";
static const char *device11 = "/dev/ttyspi11";
static const char *device12 = "/dev/ttyspi12";
static const char *device13 = "/dev/ttyspi13";
static const char *device14 = "/dev/ttyspi14";
static const char *device15 = "/dev/ttyspi15";

static uint32_t mode;
static uint8_t bits = 8;
static char *input_file;
static char *output_file;
static uint32_t speed = 500000;
static uint16_t delay;
static int verbose;
static int transfer_size;
static int iterations;
static int interval = 5; /* interval in seconds for showing transfer rate */
int PortTypW = 1;
int PortTypR = 1;
int DriverTyp = 2;


uint8_t default_tx[2048] = {0, };
uint8_t default_tx1[2048] = {0, };
uint8_t default_rx[2048] = {0, };
uint8_t default_rx1[2048] = {0, };
char *input_tx;

int random_number(int min_num, int max_num);

static void print_usage(const char *prog)
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
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
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

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "input",   1, 0, 'i' },
			{ "output",  1, 0, 'o' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "dual",    0, 0, '2' },
			{ "verbose", 0, 0, 'v' },
			{ "quad",    0, 0, '4' },
			{ "size",    1, 0, 'S' },
			{ "iter",    1, 0, 'I' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:i:o:lHOLC3NR24p:vS:I:",
				lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case 'p':
			input_tx = optarg;
			printf("optarg: %s\n", optarg);
			printf("input_tx: %s\n", input_tx);
			break;
		case '2':
			mode |= SPI_TX_DUAL;
			break;
		case '4':
			mode |= SPI_TX_QUAD;
			break;
		case 'S':
			transfer_size = atoi(optarg);
			break;
		case 'I':
			iterations = atoi(optarg);
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
	if (mode & SPI_LOOP) {
		if (mode & SPI_TX_DUAL)
			mode |= SPI_RX_DUAL;
		if (mode & SPI_TX_QUAD)
			mode |= SPI_RX_QUAD;
	}
}


int main(int argc, char *argv[])
{
	int ret = 0;
	int lenTel = 0;
	int lenTelTotal = 0;
	int fd, fd1, fd2, fd3, fd4, fd5, fd6, fd7, fd8, fd9, fd10, fd11, fd12, fd13, fd14, fd15;
	uint8_t *tx;
	ssize_t bytes;
	struct serial_struct sInfo;
	uint16_t uart;
	uint64_t countPollZyc = 0;
	uint64_t countPollErr = 0;
	uint64_t countPollProz = 0;
	int i = 0;
	int j = 0;
	int t = 0;
	
	int i0 = 3;
	int i0_1 = 13;
	int i0_2 = 23;
	int j0 = 0;
	int j0_1 = 0;
	int j0_2 = 0;
	
	int i1 = 3;
	int i1_1 = 13;
	int i1_2 = 23;
	int j1 = 0;
	int j1_1 = 0;
	int j1_2 = 0;
	
	int i2 = 3;
	int i2_1 = 13;
	int i2_2 = 23;
	int j2 = 0;
	int j2_1 = 0;
	int j2_2 = 0;
	
	int i3 = 3;
	int i3_1 = 13;
	int i3_2 = 23;
	int j3 = 0;
	int j3_1 = 0;
	int j3_2 = 0;
	
	int i4 = 3;
	int i4_1 = 13;
	int i4_2 = 23;
	int j4 = 0;
	int j4_1 = 0;
	int j4_2 = 0;
	
  int i5 = 3;
	int i5_1 = 13;
	int i5_2 = 23;
	int j5 = 0;
	int j5_1 = 0;
	int j5_2 = 0;
	
	int i6 = 3;
	int i6_1 = 13;
	int i6_2 = 23;
	int j6 = 0;
	int j6_1 = 0;
	int j6_2 = 0;
	
	int i7 = 3;
	int i7_1 = 13;
	int i7_2 = 23;
	int j7 = 0;
	int j7_1 = 0;
	int j7_2 = 0;
	
	int i8 = 3;
	int i8_1 = 13;
	int i8_2 = 23;
	int j8 = 0;
	int j8_1 = 0;
	int j8_2 = 0;
	
	int i9 = 3;
	int i9_1 = 13;
	int i9_2 = 23;
	int j9 = 0;
	int j9_1 = 0;
	int j9_2 = 0;
	
	int i10 = 3;
	int i10_1 = 13;
	int i10_2 = 23;
	int j10 = 0;
	int j10_1 = 0;
	int j10_2 = 0;
	
	int i11 = 3;
	int i11_1 = 13;
	int i11_2 = 23;
	int j11 = 0;
	int j11_1 = 0;
	int j11_2 = 0;
	
	int i12 = 3;
	int i12_1 = 13;
	int i12_2 = 23;
	int j12 = 0;
	int j12_1 = 0;
	int j12_2 = 0;
	
	int i13 = 3;
	int i13_1 = 13;
	int i13_2 = 23;
	int j13 = 0;
	int j13_1 = 0;
	int j13_2 = 0;
	
	int i14 = 3;
	int i14_1 = 13;
	int i14_2 = 23;
	int j14 = 0;
	int j14_1 = 0;
	int j14_2 = 0;
	
	int i15 = 3;
	int i15_1 = 13;
	int i15_2 = 23;
	int j15 = 0;
	int j15_1 = 0;
	int j15_2 = 0;
	
	
	unsigned int Z = 0;
	int cntErrTime0 = 0;
	int cntErrTime1 = 0;
	int cntErrTime2 = 0;
	int cntErrTime3 = 0;
	int cntErrTime4 = 0;
	int cntErrTime5 = 0;
	int cntErrTime6 = 0;
	int cntErrTime7 = 0;
  int cntErrTime8 = 0;
  int cntErrTime9 = 0;
  int cntErrTime10 = 0;
  int cntErrTime11 = 0;
  int cntErrTime12 = 0;
  int cntErrTime13 = 0;
  int cntErrTime14 = 0;
  int cntErrTime15 = 0;
  
  int cntErrRTime0 = 0;
	int cntErrRTime1 = 0;
	int cntErrRTime2 = 0;
	int cntErrRTime3 = 0;
	int cntErrRTime4 = 0;
	int cntErrRTime5 = 0;
	int cntErrRTime6 = 0;
	int cntErrRTime7 = 0;
  int cntErrRTime8 = 0;
  int cntErrRTime9 = 0;
  int cntErrRTime10 = 0;
  int cntErrRTime11 = 0;
  int cntErrRTime12 = 0;
  int cntErrRTime13 = 0;
  int cntErrRTime14 = 0;
  int cntErrRTime15 = 0;

	int count = 2048;
	
	uint8_t u8LenP0T1 = 0;
	uint8_t u8LenP0T2 = 0;
	uint8_t u8LenP0T3 = 0;
	
	uint8_t u8LenP1T1 = 0;
	uint8_t u8LenP1T2 = 0;
	uint8_t u8LenP1T3 = 0;
	
	uint8_t u8LenP2T1 = 0;
	uint8_t u8LenP2T2 = 0;
	uint8_t u8LenP2T3 = 0;
	
	uint8_t u8LenP3T1 = 0;
	uint8_t u8LenP3T2 = 0;
	uint8_t u8LenP3T3 = 0;
	
	uint8_t u8LenP4T1 = 0;
	uint8_t u8LenP4T2 = 0;
	uint8_t u8LenP4T3 = 0;
	
	uint8_t u8LenP5T1 = 0;
	uint8_t u8LenP5T2 = 0;
	uint8_t u8LenP5T3 = 0;
	
	uint8_t u8LenP6T1 = 0;
	uint8_t u8LenP6T2 = 0;
	uint8_t u8LenP6T3 = 0;
	
	uint8_t u8LenP7T1 = 0;
	uint8_t u8LenP7T2 = 0;
	uint8_t u8LenP7T3 = 0;
	
	uint8_t u8LenP8T1 = 0;
	uint8_t u8LenP8T2 = 0;
	uint8_t u8LenP8T3 = 0;
	
	uint8_t u8LenP9T1 = 0;
	uint8_t u8LenP9T2 = 0;
	uint8_t u8LenP9T3 = 0;
	
	uint8_t u8LenP10T1 = 0;
	uint8_t u8LenP10T2 = 0;
	uint8_t u8LenP10T3 = 0;
	
	uint8_t u8LenP11T1 = 0;
	uint8_t u8LenP11T2 = 0;
	uint8_t u8LenP11T3 = 0;
	
	uint8_t u8LenP12T1 = 0;
	uint8_t u8LenP12T2 = 0;
	uint8_t u8LenP12T3 = 0;
	
	uint8_t u8LenP13T1 = 0;
	uint8_t u8LenP13T2 = 0;
	uint8_t u8LenP13T3 = 0;
	
	uint8_t u8LenP14T1 = 0;
	uint8_t u8LenP14T2 = 0;
	uint8_t u8LenP14T3 = 0;
	
	uint8_t u8LenP15T1 = 0;
	uint8_t u8LenP15T2 = 0;
	uint8_t u8LenP15T3 = 0;
	
	uart = 0;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)default_tx,
		.rx_buf = (unsigned long)default_rx,
		.len = 2048,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	uint64_t read_countX = 0;
	uint64_t countRead = 0;
	uint64_t countReadTeleg = 0;
	uint64_t countReadByte = 0;
	uint64_t prev_read_count = 0;
	uint32_t cntErr = 0;
	uint32_t cntErrTx = 0;
	uint32_t cntErrRx = 0;
	double rx_rate = 0;
	double tx_rate = 0;
	double rx_rate1 = 0;
	double tx_rate1 = 0;
	double tx_rate_min = 100;
	double tx_rate_max = 0;
	double rx_rate_min = 100;
	double rx_rate_max = 0;
	double rate;
	double pers;
	double persTx;
	double persRx;
  long tv_nsec = 0;
  long tv_nsec1 = 0;
  unsigned char *bufTX0;
  unsigned char *bufTX1;
  unsigned char *bufTX2;
  unsigned char *bufTX3;
  unsigned char *bufTX4;
  unsigned char *bufTX5;
  unsigned char *bufTX6;
  unsigned char *bufTX7;
  unsigned char *bufTX8;
  unsigned char *bufTX9;
  unsigned char *bufTX10;
  unsigned char *bufTX11;
  unsigned char *bufTX12;
  unsigned char *bufTX13;
  unsigned char *bufTX14;
  unsigned char *bufTX15;
  
  unsigned char *bufRX0;
  unsigned char *bufRX1;
  unsigned char *bufRX2;
  unsigned char *bufRX3;
  unsigned char *bufRX4;
  unsigned char *bufRX5;
  unsigned char *bufRX6;
  unsigned char *bufRX7;
  unsigned char *bufRX8;
  unsigned char *bufRX9;
  unsigned char *bufRX10;
  unsigned char *bufRX11;
  unsigned char *bufRX12;
  unsigned char *bufRX13;
  unsigned char *bufRX14;
  unsigned char *bufRX15;
  
  bufTX0 = malloc(255);
  bufTX1 = malloc(255);
  bufTX2 = malloc(255);
  bufTX3 = malloc(255);
  bufTX4 = malloc(255);
  bufTX5 = malloc(255);
  bufTX6 = malloc(255);
  bufTX7 = malloc(255);
  bufTX8 = malloc(255);
  bufTX9 = malloc(255);
  bufTX10 = malloc(255);
  bufTX11 = malloc(255);
  bufTX12 = malloc(255);
  bufTX13 = malloc(255);
  bufTX14 = malloc(255);
  bufTX15 = malloc(255);
    
  memset(bufTX0, 0, 255);
  memset(bufTX1, 0, 255);
  memset(bufTX2, 0, 255);
  memset(bufTX3, 0, 255);
  memset(bufTX4, 0, 255);
  memset(bufTX5, 0, 255);
  memset(bufTX6, 0, 255);
  memset(bufTX7, 0, 255);
  memset(bufTX8, 0, 255);
  memset(bufTX9, 0, 255);
  memset(bufTX10, 0, 255);
  memset(bufTX11, 0, 255);
  memset(bufTX12, 0, 255);
  memset(bufTX13, 0, 255);
  memset(bufTX14, 0, 255);
  memset(bufTX15, 0, 255);
  
  bufRX0 = malloc(2048);
  bufRX1 = malloc(2048);
  bufRX2 = malloc(2048);
  bufRX3 = malloc(2048);
  bufRX4 = malloc(2048);
  bufRX5 = malloc(2048);
  bufRX6 = malloc(2048);
  bufRX7 = malloc(2048);
  bufRX8 = malloc(2048);
  bufRX9 = malloc(2048);
  bufRX10 = malloc(2048);
  bufRX11 = malloc(2048);
  bufRX12 = malloc(2048);
  bufRX13 = malloc(2048);
  bufRX14 = malloc(2048);
  bufRX15 = malloc(2048);
   
  memset(bufRX0, 0, 2048);
  memset(bufRX1, 0, 2048);
  memset(bufRX2, 0, 2048);
  memset(bufRX3, 0, 2048);
  memset(bufRX4, 0, 2048);
  memset(bufRX5, 0, 2048);
  memset(bufRX6, 0, 2048);
  memset(bufRX7, 0, 2048);
  memset(bufRX8, 0, 2048);
  memset(bufRX9, 0, 2048);
  memset(bufRX10, 0, 2048);
  memset(bufRX11, 0, 2048);
  memset(bufRX12, 0, 2048);
  memset(bufRX13, 0, 2048);
  memset(bufRX14, 0, 2048);
  memset(bufRX15, 0, 2048);
  
  memset(&sInfo, 0, sizeof(sInfo));
  
  ///////////////////////////////////////////////////////////////////////////////
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", device);
	}
	fd1 = open(device1, O_RDWR);
	if (fd1 < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", device1);
	}
	fd2 = open(device2, O_RDWR);
	if (fd2 < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", device2);
	}
	fd3 = open(device3, O_RDWR);
	if (fd2 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device3);
	}
	fd4 = open(device4, O_RDWR);
	if (fd4 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device4);
	}
	fd5 = open(device5, O_RDWR);
	if (fd5 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device5);
	}
	fd6 = open(device6, O_RDWR);
	if (fd6 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device6);
	}
	fd7 = open(device7, O_RDWR);
	if (fd7 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device7);
	}
	fd8 = open(device8, O_RDWR);
	if (fd8 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device8);
	}
	fd9 = open(device9, O_RDWR);
	if (fd9 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device9);
	}
	fd10 = open(device10, O_RDWR);
	if (fd10 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device10);
	}
	fd11 = open(device11, O_RDWR);
	if (fd11 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device11);
	}
	fd12 = open(device12, O_RDWR);
	if (fd12 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device12);
	}
	fd13 = open(device13, O_RDWR);
	if (fd13 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device13);
	}
	fd14 = open(device14, O_RDWR);
	if (fd14 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device14);
	}
	fd15 = open(device15, O_RDWR);
	if (fd15 < 0)
	{
		printf("ERR");
	}
	else
	{
		printf("OK: %s\n", device15);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	
	ret = ioctl(fd, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd1, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd2, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd3, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd4, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd5, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd6, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd7, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd8, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd9, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd10, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd11, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd12, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd13, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd14, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd15, TIO_PORT_TYP_W, &PortTypW);
	
	ret = ioctl(fd, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd1, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd2, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd3, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd4, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd5, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd6, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd7, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd8, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd9, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd10, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd11, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd12, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd13, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd14, TIO_PORT_TYP_R, &PortTypR);
	
	ret = ioctl(fd15, TIO_PORT_TYP_R, &PortTypR);
	
	//PortTypR = 2;
	//ret = ioctl(fd, TIO_PORT_TYP_R, &PortTypR);
	//ret = ioctl(fd, TIO_DRIVER_TYP, &DriverTyp);
	
	
		
	///////////////////////////////////////////////////////////////////////////////
	
	struct timespec startT;
	clock_gettime(CLOCK_MONOTONIC, &startT);
	srand(time(NULL));
	
	//-----0
  u8LenP0T1 = (uint8_t)random_number(9,64);
  printf("LEN: %d\n", u8LenP0T1);
  
  u8LenP0T2 = u8LenP0T1 +(uint8_t)random_number(9,64);
  printf("LEN1: %d, PUNKT1: %d\n", u8LenP0T2 - u8LenP0T1, u8LenP0T2);
  i0_1= u8LenP0T1 + 3;
  
  u8LenP0T3 = u8LenP0T2 + (uint8_t)random_number(9,64);
  printf("LEN2: %d, PUNKT2: %d\n", u8LenP0T3 - u8LenP0T2, u8LenP0T3);
  i0_2= u8LenP0T2 + 3;
  
  //-----1
  u8LenP1T1 = (uint8_t)random_number(9,64);
  
  u8LenP1T2 = u8LenP1T1 +(uint8_t)random_number(9,64);
  i1_1= u8LenP1T1 + 3;
  
  u8LenP1T3 = u8LenP1T2 + (uint8_t)random_number(9,64);
  i1_2= u8LenP1T2 + 3;
  
  //-----2
  u8LenP2T1 = (uint8_t)random_number(9,64);
  
  u8LenP2T2 = u8LenP2T1 +(uint8_t)random_number(9,64);
  i2_1= u8LenP2T1 + 3;
  
  u8LenP2T3 = u8LenP2T2 + (uint8_t)random_number(9,64);
  i2_2= u8LenP2T2 + 3;
  
  //-----3
  u8LenP3T1 = (uint8_t)random_number(9,64);
  
  u8LenP3T2 = u8LenP3T1 +(uint8_t)random_number(9,64);
  i3_1= u8LenP3T1 + 3;
  
  u8LenP3T3 = u8LenP3T2 + (uint8_t)random_number(9,64);
  i3_2= u8LenP3T2 + 3;
  
  //-----4
  u8LenP4T1 = (uint8_t)random_number(9,64);
  
  u8LenP4T2 = u8LenP4T1 +(uint8_t)random_number(9,64);
  i4_1= u8LenP4T1 + 3;
  
  u8LenP4T3 = u8LenP4T2 + (uint8_t)random_number(9,64);
  i4_2= u8LenP4T2 + 3;
  
  //-----5
  u8LenP5T1 = (uint8_t)random_number(9,64);
  
  u8LenP5T2 = u8LenP5T1 +(uint8_t)random_number(9,64);
  i5_1= u8LenP5T1 + 3;
  
  u8LenP5T3 = u8LenP5T2 + (uint8_t)random_number(9,64);
  i5_2= u8LenP5T2 + 3;
  
  //-----6
  u8LenP6T1 = (uint8_t)random_number(9,64);
  
  u8LenP6T2 = u8LenP6T1 +(uint8_t)random_number(9,64);
  i6_1= u8LenP6T1 + 3;
  
  u8LenP6T3 = u8LenP6T2 + (uint8_t)random_number(9,64);
  i6_2= u8LenP6T2 + 3;
  
  //-----7
  u8LenP7T1 = (uint8_t)random_number(9,64);
  
  u8LenP7T2 = u8LenP7T1 +(uint8_t)random_number(9,64);
  i7_1= u8LenP7T1 + 3;
  
  u8LenP7T3 = u8LenP7T2 + (uint8_t)random_number(9,64);
  i7_2= u8LenP7T2 + 3;
  
  //-----8
  u8LenP8T1 = (uint8_t)random_number(9,64);
  
  u8LenP8T2 = u8LenP8T1 +(uint8_t)random_number(9,64);
  i8_1= u8LenP8T1 + 3;
  
  u8LenP8T3 = u8LenP8T2 + (uint8_t)random_number(9,64);
  i8_2= u8LenP8T2 + 3;
  
  //-----9
  u8LenP9T1 = (uint8_t)random_number(9,64);
  
  u8LenP9T2 = u8LenP9T1 +(uint8_t)random_number(9,64);
  i9_1= u8LenP9T1 + 3;
  
  u8LenP9T3 = u8LenP9T2 + (uint8_t)random_number(9,64);
  i9_2= u8LenP9T2 + 3;
  
  //-----10
  u8LenP10T1 = (uint8_t)random_number(9,64);
  
  u8LenP10T2 = u8LenP10T1 +(uint8_t)random_number(9,64);
  i10_1= u8LenP10T1 + 3;
  
  u8LenP10T3 = u8LenP10T2 + (uint8_t)random_number(9,64);
  i10_2= u8LenP10T2 + 3;
  
  //-----11
  u8LenP11T1 = (uint8_t)random_number(9,64);
  
  u8LenP11T2 = u8LenP11T1 +(uint8_t)random_number(9,64);
  i11_1= u8LenP11T1 + 3;
  
  u8LenP11T3 = u8LenP11T2 + (uint8_t)random_number(9,64);
  i11_2= u8LenP11T2 + 3;
  
  //-----12
  u8LenP12T1 = (uint8_t)random_number(9,64);
  
  u8LenP12T2 = u8LenP12T1 +(uint8_t)random_number(9,64);
  i12_1= u8LenP12T1 + 3;
  
  u8LenP12T3 = u8LenP12T2 + (uint8_t)random_number(9,64);
  i12_2= u8LenP12T2 + 3;
  
  //-----13
  u8LenP13T1 = (uint8_t)random_number(9,64);
  
  u8LenP13T2 = u8LenP13T1 +(uint8_t)random_number(9,64);
  i13_1= u8LenP13T1 + 3;
  
  u8LenP13T3 = u8LenP13T2 + (uint8_t)random_number(9,64);
  i13_2= u8LenP13T2 + 3;
  
  //-----14
  u8LenP14T1 = (uint8_t)random_number(9,64);
  
  u8LenP14T2 = u8LenP14T1 +(uint8_t)random_number(9,64);
  i14_1= u8LenP14T1 + 3;
  
  u8LenP14T3 = u8LenP14T2 + (uint8_t)random_number(9,64);
  i14_2= u8LenP14T2 + 3;
  
  //-----15
  u8LenP15T1 = (uint8_t)random_number(9,64);
  
  u8LenP15T2 = u8LenP15T1 +(uint8_t)random_number(9,64);
  i15_1= u8LenP15T1 + 3;
  
  u8LenP15T3 = u8LenP15T2 + (uint8_t)random_number(9,64);
  i15_2= u8LenP15T2 + 3;
  
	
	///////////////////////////////////////////////////////////////////////////////
	while (1)   //t < 2
	{
    //----------------- WRITE PORT 0 -----------------------
    //Port 0 Tel 1		
    
    *(bufTX0) = 0xA9;
    *(bufTX0 + 1) = 0;        //HEADER
    *(bufTX0 + u8LenP0T1 - 1) = 0xA9;
    *(bufTX0 + u8LenP0T1 - 2) = 201;
    *(bufTX0 + u8LenP0T1 - 3) = 1;
   
    if (i0 >= (u8LenP0T1 - 2)) 
    {
      i0= 3;
      j0= 0;
      //memset(bufTX0, 0, 9);
      *(bufTX0) = 0xA9;
      for (i = 1; i < (u8LenP0T1-3); i++)
      {
        *(bufTX0 + i) = 0;
      }
      *(bufTX0 + u8LenP0T1 - 1) = 0xA9;
      *(bufTX0 + u8LenP0T1 - 2) = 201;
      *(bufTX0 + u8LenP0T1 - 3) = 1;
    }
    *(bufTX0 + i0) = *(bufTX0 + i0) + 1;
    j0++;
    if (j0 >= 160)
    {
      i0++;
      j0 = 0;
    }
    // Port 0 Tel 2
    *(bufTX0 + u8LenP0T1) = 0xA9;
    *(bufTX0 + u8LenP0T1 + 1) = 0;   //HEADER
    *(bufTX0 + u8LenP0T2 - 1) = 0xA9;
    *(bufTX0 + u8LenP0T2 - 2) = 201;
    *(bufTX0 + u8LenP0T2 - 3) = 2;
   
    if (i0_1 >= (u8LenP0T2 - 2)) 
    {
      i0_1= u8LenP0T1 + 3;
      j0_1= 0;
      //memset(bufTX0, 0, 9);
      *(bufTX0 + u8LenP0T1) = 0xA9;
      for (i = (u8LenP0T1+1); i < (u8LenP0T2-3); i++)
      {
        *(bufTX0 + i) = 0;
      }
      *(bufTX0 + u8LenP0T2 - 1) = 0xA9;
      *(bufTX0 + u8LenP0T2 - 2) = 201;
      *(bufTX0 + u8LenP0T2 - 3) = 2;
    }
    *(bufTX0 + i0_1) = *(bufTX0 + i0_1) + 1;
    j0_1++;
    if (j0_1 >= 160)
    {
      i0_1++;
      j0_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX0 + u8LenP0T2) = 0xA9;
    *(bufTX0 + u8LenP0T2 + 1) = 0;   //HEADER
    *(bufTX0 + u8LenP0T3 - 1) = 0xA9;
    *(bufTX0 + u8LenP0T3 - 2) = 201;
    *(bufTX0 + u8LenP0T3 - 3) = 3;
   
    if (i0_2 >= (u8LenP0T3 - 2)) 
    {
      i0_2= u8LenP0T2 + 3;
      j0_2= 0;
      //memset(bufTX0, 0, 9);
      *(bufTX0 + u8LenP0T2) = 0xA9;
      for (i = (u8LenP0T2+1); i < (u8LenP0T3-3); i++)
      {
        *(bufTX0 + i) = 0;
      }
      *(bufTX0 + u8LenP0T3 - 1) = 0xA9;
      *(bufTX0 + u8LenP0T3 - 2) = 201;
      *(bufTX0 + u8LenP0T3 - 3) = 3;
    }
    *(bufTX0 + i0_2) = *(bufTX0 + i0_2) + 1;
    j0_2++;
    if (j0_2 >= 160)
    {
      i0_2++;
      j0_2 = 0;
    }

    //----------------- WRITE PORT 1 -----------------------
    //Port 1 Tel 1
    *(bufTX1) = 0xA9;
    *(bufTX1 + 1) = 0;        //HEADER
    *(bufTX1 + u8LenP1T1 - 1) = 0xA9;
    *(bufTX1 + u8LenP1T1 - 2) = 202;
    *(bufTX1 + u8LenP1T1 - 3) = 1;
   
    if (i1 >= (u8LenP1T1 - 2)) 
    {
      i1= 3;
      j1= 0;
      //memset(bufTX1, 0, 9);
      *(bufTX1) = 0xA9;
      for (i = 1; i < (u8LenP1T1-3); i++)
      {
        *(bufTX1 + i) = 0;
      }
      *(bufTX1 + u8LenP1T1 - 1) = 0xA9;
      *(bufTX1 + u8LenP1T1 - 2) = 202;
      *(bufTX1 + u8LenP1T1 - 3) = 1;
    }
    *(bufTX1 + i1) = *(bufTX1 + i1) + 1;
    j1++;
    if (j1 >= 160)
    {
      i1++;
      j1 = 0;
    }
    // Port 0 Tel 2
    *(bufTX1 + u8LenP1T1) = 0xA9;
    *(bufTX1 + u8LenP1T1 + 1) = 0;   //HEADER
    *(bufTX1 + u8LenP1T2 - 1) = 0xA9;
    *(bufTX1 + u8LenP1T2 - 2) = 202;
    *(bufTX1 + u8LenP1T2 - 3) = 2;
   
    if (i1_1 >= (u8LenP1T2 - 2)) 
    {
      i1_1= u8LenP1T1 + 3;
      j1_1= 0;
      *(bufTX1 + u8LenP1T1) = 0xA9;
      for (i = (u8LenP1T1+1); i < (u8LenP1T2-3); i++)
      {
        *(bufTX1 + i) = 0;
      }
      *(bufTX1 + u8LenP1T2 - 1) = 0xA9;
      *(bufTX1 + u8LenP1T2 - 2) = 202;
      *(bufTX1 + u8LenP1T2 - 3) = 2;
    }
    *(bufTX1 + i1_1) = *(bufTX1 + i1_1) + 1;
    j1_1++;
    if (j1_1 >= 160)
    {
      i1_1++;
      j1_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX1 + u8LenP1T2) = 0xA9;
    *(bufTX1 + u8LenP1T2 + 1) = 0;   //HEADER
    *(bufTX1 + u8LenP1T3 - 1) = 0xA9;
    *(bufTX1 + u8LenP1T3 - 2) = 202;
    *(bufTX1 + u8LenP1T3 - 3) = 3;
   
    if (i1_2 >= (u8LenP1T3 - 2)) 
    {
      i1_2= u8LenP1T2 + 3;
      j1_2= 0;
      //memset(bufTX1, 0, 9);
      *(bufTX1 + u8LenP1T2) = 0xA9;
      for (i = (u8LenP1T2+1); i < (u8LenP1T3-3); i++)
      {
        *(bufTX1 + i) = 0;
      }
      *(bufTX1 + u8LenP1T3 - 1) = 0xA9;
      *(bufTX1 + u8LenP1T3 - 2) = 202;
      *(bufTX1 + u8LenP1T3 - 3) = 3;
    }
    *(bufTX1 + i1_2) = *(bufTX1 + i1_2) + 1;
    j1_2++;
    if (j1_2 >= 160)
    {
      i1_2++;
      j1_2 = 0;
    }
        
    //----------------- WRITE PORT 2 -----------------------
    //Port 2 Tel 1
    *(bufTX2) = 0xA9;
    *(bufTX2 + 1) = 0;        //HEADER
    *(bufTX2 + u8LenP2T1 - 1) = 0xA9;
    *(bufTX2 + u8LenP2T1 - 2) = 203;
    *(bufTX2 + u8LenP2T1 - 3) = 1;
   
    if (i2 >= (u8LenP2T1 - 2)) 
    {
      i2= 3;
      j2= 0;
      //memset(bufTX2, 0, 9);
      *(bufTX2) = 0xA9;
      for (i = 1; i < (u8LenP2T1-3); i++)
      {
        *(bufTX2 + i) = 0;
      }
      *(bufTX2 + u8LenP2T1 - 1) = 0xA9;
      *(bufTX2 + u8LenP2T1 - 2) = 203;
      *(bufTX2 + u8LenP2T1 - 3) = 1;
    }
    *(bufTX2 + i2) = *(bufTX2 + i2) + 1;
    j2++;
    if (j2 >= 160)
    {
      i2++;
      j2 = 0;
    }
    // Port 0 Tel 2
    *(bufTX2 + u8LenP2T1) = 0xA9;
    *(bufTX2 + u8LenP2T1 + 1) = 0;   //HEADER
    *(bufTX2 + u8LenP2T2 - 1) = 0xA9;
    *(bufTX2 + u8LenP2T2 - 2) = 203;
    *(bufTX2 + u8LenP2T2 - 3) = 2;
   
    if (i2_1 >= (u8LenP2T2 - 2)) 
    {
      i2_1= u8LenP2T1 + 3;
      j2_1= 0;
      *(bufTX2 + u8LenP2T1) = 0xA9;
      for (i = (u8LenP2T1+1); i < (u8LenP2T2-3); i++)
      {
        *(bufTX2 + i) = 0;
      }
      *(bufTX2 + u8LenP2T2 - 1) = 0xA9;
      *(bufTX2 + u8LenP2T2 - 2) = 203;
      *(bufTX2 + u8LenP2T2 - 3) = 2;
    }
    *(bufTX2 + i2_1) = *(bufTX2 + i2_1) + 1;
    j2_1++;
    if (j2_1 >= 160)
    {
      i2_1++;
      j2_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX2 + u8LenP2T2) = 0xA9;
    *(bufTX2 + u8LenP2T2 + 1) = 0;   //HEADER
    *(bufTX2 + u8LenP2T3 - 1) = 0xA9;
    *(bufTX2 + u8LenP2T3 - 2) = 203;
    *(bufTX2 + u8LenP2T3 - 3) = 3;
   
    if (i2_2 >= (u8LenP2T3 - 2)) 
    {
      i2_2= u8LenP2T2 + 3;
      j2_2= 0;
      *(bufTX2 + u8LenP2T2) = 0xA9;
      for (i = (u8LenP2T2+1); i < (u8LenP2T3-3); i++)
      {
        *(bufTX2 + i) = 0;
      }
      *(bufTX2 + u8LenP2T3 - 1) = 0xA9;
      *(bufTX2 + u8LenP2T3 - 2) = 203;
      *(bufTX2 + u8LenP2T3 - 3) = 3;
    }
    *(bufTX2 + i2_2) = *(bufTX2 + i2_2) + 1;
    j2_2++;
    if (j2_2 >= 160)
    {
      i2_2++;
      j2_2 = 0;
    }
    
    //----------------- WRITE PORT 3 -----------------------
    //Port 3 Tel 1
    *(bufTX3) = 0xA9;
    *(bufTX3 + 1) = 0;        //HEADER
    *(bufTX3 + u8LenP3T1 - 1) = 0xA9;
    *(bufTX3 + u8LenP3T1 - 2) = 204;
    *(bufTX3 + u8LenP3T1 - 3) = 1;
   
    if (i3 >= (u8LenP3T1 - 2)) 
    {
      i3= 3;
      j3= 0;
      //memset(bufTX3, 0, 9);
      *(bufTX3) = 0xA9;
      for (i = 1; i < (u8LenP3T1-3); i++)
      {
        *(bufTX3 + i) = 0;
      }
      *(bufTX3 + u8LenP3T1 - 1) = 0xA9;
      *(bufTX3 + u8LenP3T1 - 2) = 204;
      *(bufTX3 + u8LenP3T1 - 3) = 1;
    }
    *(bufTX3 + i3) = *(bufTX3 + i3) + 1;
    j3++;
    if (j3 >= 160)
    {
      i3++;
      j3 = 0;
    }
    // Port 0 Tel 2
    *(bufTX3 + u8LenP3T1) = 0xA9;
    *(bufTX3 + u8LenP3T1 + 1) = 0;   //HEADER
    *(bufTX3 + u8LenP3T2 - 1) = 0xA9;
    *(bufTX3 + u8LenP3T2 - 2) = 204;
    *(bufTX3 + u8LenP3T2 - 3) = 2;
   
    if (i3_1 >= (u8LenP3T2 - 2)) 
    {
      i3_1= u8LenP3T1 + 3;
      j3_1= 0;
      *(bufTX3 + u8LenP3T1) = 0xA9;
      for (i = (u8LenP3T1+1); i < (u8LenP3T2-3); i++)
      {
        *(bufTX3 + i) = 0;
      }
      *(bufTX3 + u8LenP3T2 - 1) = 0xA9;
      *(bufTX3 + u8LenP3T2 - 2) = 204;
      *(bufTX3 + u8LenP3T2 - 3) = 2;
    }
    *(bufTX3 + i3_1) = *(bufTX3 + i3_1) + 1;
    j3_1++;
    if (j3_1 >= 160)
    {
      i3_1++;
      j3_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX3 + u8LenP3T2) = 0xA9;
    *(bufTX3 + u8LenP3T2 + 1) = 0;   //HEADER
    *(bufTX3 + u8LenP3T3 - 1) = 0xA9;
    *(bufTX3 + u8LenP3T3 - 2) = 204;
    *(bufTX3 + u8LenP3T3 - 3) = 3;
   
    if (i3_2 >= (u8LenP3T3 - 2)) 
    {
      i3_2= u8LenP3T2 + 3;
      j3_2= 0;
      *(bufTX3 + u8LenP3T2) = 0xA9;
      for (i = (u8LenP3T2+1); i < (u8LenP3T3-3); i++)
      {
        *(bufTX3 + i) = 0;
      }
      *(bufTX3 + u8LenP3T3 - 1) = 0xA9;
      *(bufTX3 + u8LenP3T3 - 2) = 204;
      *(bufTX3 + u8LenP3T3 - 3) = 3;
    }
    *(bufTX3 + i3_2) = *(bufTX3 + i3_2) + 1;
    j3_2++;
    if (j3_2 >= 160)
    {
      i3_2++;
      j3_2 = 0;
    }
    
    //----------------- WRITE PORT 4 -----------------------
    //Port 4 Tel 1
    *(bufTX4) = 0xA9;
    *(bufTX4 + 1) = 0;        //HEADER
    *(bufTX4 + u8LenP4T1 - 1) = 0xA9;
    *(bufTX4 + u8LenP4T1 - 2) = 205;
    *(bufTX4 + u8LenP4T1 - 3) = 1;
   
    if (i4 >= (u8LenP4T1 - 2)) 
    {
      i4= 3;
      j4= 0;
      //memset(bufTX4, 0, 9);
      *(bufTX4) = 0xA9;
      for (i = 1; i < (u8LenP4T1-3); i++)
      {
        *(bufTX4 + i) = 0;
      }
      *(bufTX4 + u8LenP4T1 - 1) = 0xA9;
      *(bufTX4 + u8LenP4T1 - 2) = 205;
      *(bufTX4 + u8LenP4T1 - 3) = 1;
    }
    *(bufTX4 + i4) = *(bufTX4 + i4) + 1;
    j4++;
    if (j4 >= 160)
    {
      i4++;
      j4 = 0;
    }
    // Port 0 Tel 2
    *(bufTX4 + u8LenP4T1) = 0xA9;
    *(bufTX4 + u8LenP4T1 + 1) = 0;   //HEADER
    *(bufTX4 + u8LenP4T2 - 1) = 0xA9;
    *(bufTX4 + u8LenP4T2 - 2) = 205;
    *(bufTX4 + u8LenP4T2 - 3) = 2;
   
    if (i4_1 >= (u8LenP4T2 - 2)) 
    {
      i4_1= u8LenP4T1 + 3;
      j4_1= 0;
      *(bufTX4 + u8LenP4T1) = 0xA9;
      for (i = (u8LenP4T1+1); i < (u8LenP4T2-3); i++)
      {
        *(bufTX4 + i) = 0;
      }
      *(bufTX4 + u8LenP4T2 - 1) = 0xA9;
      *(bufTX4 + u8LenP4T2 - 2) = 205;
      *(bufTX4 + u8LenP4T2 - 3) = 2;
    }
    *(bufTX4 + i4_1) = *(bufTX4 + i4_1) + 1;
    j4_1++;
    if (j4_1 >= 160)
    {
      i4_1++;
      j4_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX4 + u8LenP4T2) = 0xA9;
    *(bufTX4 + u8LenP4T2 + 1) = 0;   //HEADER
    *(bufTX4 + u8LenP4T3 - 1) = 0xA9;
    *(bufTX4 + u8LenP4T3 - 2) = 205;
    *(bufTX4 + u8LenP4T3 - 3) = 3;
   
    if (i4_2 >= (u8LenP4T3 - 2)) 
    {
      i4_2= u8LenP4T2 + 3;
      j4_2= 0;
      *(bufTX4 + u8LenP4T2) = 0xA9;
      for (i = (u8LenP4T2+1); i < (u8LenP4T3-3); i++)
      {
        *(bufTX4 + i) = 0;
      }
      *(bufTX4 + u8LenP4T3 - 1) = 0xA9;
      *(bufTX4 + u8LenP4T3 - 2) = 205;
      *(bufTX4 + u8LenP4T3 - 3) = 3;
    }
    *(bufTX4 + i4_2) = *(bufTX4 + i4_2) + 1;
    j4_2++;
    if (j4_2 >= 160)
    {
      i4_2++;
      j4_2 = 0;
    }
    
    //----------------- WRITE PORT 5 -----------------------
    //Port 5 Tel 1
    *(bufTX5) = 0xA9;
    *(bufTX5 + 1) = 0;        //HEADER
    *(bufTX5 + u8LenP5T1 - 1) = 0xA9;
    *(bufTX5 + u8LenP5T1 - 2) = 206;
    *(bufTX5 + u8LenP5T1 - 3) = 1;
   
    if (i5 >= (u8LenP5T1 - 2)) 
    {
      i5= 3;
      j5= 0;
      //memset(bufTX5, 0, 9);
      *(bufTX5) = 0xA9;
      for (i = 1; i < (u8LenP5T1-3); i++)
      {
        *(bufTX5 + i) = 0;
      }
      *(bufTX5 + u8LenP5T1 - 1) = 0xA9;
      *(bufTX5 + u8LenP5T1 - 2) = 206;
      *(bufTX5 + u8LenP5T1 - 3) = 1;
    }
    *(bufTX5 + i5) = *(bufTX5 + i5) + 1;
    j5++;
    if (j5 >= 160)
    {
      i5++;
      j5 = 0;
    }
    // Port 0 Tel 2
    *(bufTX5 + u8LenP5T1) = 0xA9;
    *(bufTX5 + u8LenP5T1 + 1) = 0;   //HEADER
    *(bufTX5 + u8LenP5T2 - 1) = 0xA9;
    *(bufTX5 + u8LenP5T2 - 2) = 206;
    *(bufTX5 + u8LenP5T2 - 3) = 2;
   
    if (i5_1 >= (u8LenP5T2 - 2)) 
    {
      i5_1= u8LenP5T1 + 3;
      j5_1= 0;
      *(bufTX5 + u8LenP5T1) = 0xA9;
      for (i = (u8LenP5T1+1); i < (u8LenP5T2-3); i++)
      {
        *(bufTX5 + i) = 0;
      }
      *(bufTX5 + u8LenP5T2 - 1) = 0xA9;
      *(bufTX5 + u8LenP5T2 - 2) = 206;
      *(bufTX5 + u8LenP5T2 - 3) = 2;
    }
    *(bufTX5 + i5_1) = *(bufTX5 + i5_1) + 1;
    j5_1++;
    if (j5_1 >= 160)
    {
      i5_1++;
      j5_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX5 + u8LenP5T2) = 0xA9;
    *(bufTX5 + u8LenP5T2 + 1) = 0;   //HEADER
    *(bufTX5 + u8LenP5T3 - 1) = 0xA9;
    *(bufTX5 + u8LenP5T3 - 2) = 206;
    *(bufTX5 + u8LenP5T3 - 3) = 3;
   
    if (i5_2 >= (u8LenP5T3 - 2)) 
    {
      i5_2= u8LenP5T2 + 3;
      j5_2= 0;
      *(bufTX5 + u8LenP5T2) = 0xA9;
      for (i = (u8LenP5T2+1); i < (u8LenP5T3-3); i++)
      {
        *(bufTX5 + i) = 0;
      }
      *(bufTX5 + u8LenP5T3 - 1) = 0xA9;
      *(bufTX5 + u8LenP5T3 - 2) = 206;
      *(bufTX5 + u8LenP5T3 - 3) = 3;
    }
    *(bufTX5 + i5_2) = *(bufTX5 + i5_2) + 1;
    j5_2++;
    if (j5_2 >= 160)
    {
      i5_2++;
      j5_2 = 0;
    }
    
    //----------------- WRITE PORT 6 -----------------------
    //Port 6 Tel 1
    *(bufTX6) = 0xA9;
    *(bufTX6 + 1) = 0;        //HEADER
    *(bufTX6 + u8LenP6T1 - 1) = 0xA9;
    *(bufTX6 + u8LenP6T1 - 2) = 207;
    *(bufTX6 + u8LenP6T1 - 3) = 1;
   
    if (i6 >= (u8LenP6T1 - 2)) 
    {
      i6= 3;
      j6= 0;
      //memset(bufTX6, 0, 9);
      *(bufTX6) = 0xA9;
      for (i = 1; i < (u8LenP6T1-3); i++)
      {
        *(bufTX6 + i) = 0;
      }
      *(bufTX6 + u8LenP6T1 - 1) = 0xA9;
      *(bufTX6 + u8LenP6T1 - 2) = 207;
      *(bufTX6 + u8LenP6T1 - 3) = 1;
    }
    *(bufTX6 + i6) = *(bufTX6 + i6) + 1;
    j6++;
    if (j6 >= 160)
    {
      i6++;
      j6 = 0;
    }
    // Port 0 Tel 2
    *(bufTX6 + u8LenP6T1) = 0xA9;
    *(bufTX6 + u8LenP6T1 + 1) = 0;   //HEADER
    *(bufTX6 + u8LenP6T2 - 1) = 0xA9;
    *(bufTX6 + u8LenP6T2 - 2) = 207;
    *(bufTX6 + u8LenP6T2 - 3) = 2;
   
    if (i6_1 >= (u8LenP6T2 - 2)) 
    {
      i6_1= u8LenP6T1 + 3;
      j6_1= 0;
      *(bufTX6 + u8LenP6T1) = 0xA9;
      for (i = (u8LenP6T1+1); i < (u8LenP6T2-3); i++)
      {
        *(bufTX6 + i) = 0;
      }
      *(bufTX6 + u8LenP6T2 - 1) = 0xA9;
      *(bufTX6 + u8LenP6T2 - 2) = 207;
      *(bufTX6 + u8LenP6T2 - 3) = 2;
    }
    *(bufTX6 + i6_1) = *(bufTX6 + i6_1) + 1;
    j6_1++;
    if (j6_1 >= 160)
    {
      i6_1++;
      j6_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX6 + u8LenP6T2) = 0xA9;
    *(bufTX6 + u8LenP6T2 + 1) = 0;   //HEADER
    *(bufTX6 + u8LenP6T3 - 1) = 0xA9;
    *(bufTX6 + u8LenP6T3 - 2) = 207;
    *(bufTX6 + u8LenP6T3 - 3) = 3;
   
    if (i6_2 >= (u8LenP6T3 - 2)) 
    {
      i6_2= u8LenP6T2 + 3;
      j6_2= 0;
      *(bufTX6 + u8LenP6T2) = 0xA9;
      for (i = (u8LenP6T2+1); i < (u8LenP6T3-3); i++)
      {
        *(bufTX6 + i) = 0;
      }
      *(bufTX6 + u8LenP6T3 - 1) = 0xA9;
      *(bufTX6 + u8LenP6T3 - 2) = 207;
      *(bufTX6 + u8LenP6T3 - 3) = 3;
    }
    *(bufTX6 + i6_2) = *(bufTX6 + i6_2) + 1;
    j6_2++;
    if (j6_2 >= 160)
    {
      i6_2++;
      j6_2 = 0;
    }
    
    //----------------- WRITE PORT 7 -----------------------
    //Port 7 Tel 1
    *(bufTX7) = 0xA9;
    *(bufTX7 + 1) = 0;        //HEADER
    *(bufTX7 + u8LenP7T1 - 1) = 0xA9;
    *(bufTX7 + u8LenP7T1 - 2) = 208;
    *(bufTX7 + u8LenP7T1 - 3) = 1;
   
    if (i7 >= (u8LenP7T1 - 2)) 
    {
      i7= 3;
      j7= 0;
      //memset(bufTX7, 0, 9);
      *(bufTX7) = 0xA9;
      for (i = 1; i < (u8LenP7T1-3); i++)
      {
        *(bufTX7 + i) = 0;
      }
      *(bufTX7 + u8LenP7T1 - 1) = 0xA9;
      *(bufTX7 + u8LenP7T1 - 2) = 208;
      *(bufTX7 + u8LenP7T1 - 3) = 1;
    }
    *(bufTX7 + i7) = *(bufTX7 + i7) + 1;
    j7++;
    if (j7 >= 160)
    {
      i7++;
      j7 = 0;
    }
    // Port 0 Tel 2
    *(bufTX7 + u8LenP7T1) = 0xA9;
    *(bufTX7 + u8LenP7T1 + 1) = 0;   //HEADER
    *(bufTX7 + u8LenP7T2 - 1) = 0xA9;
    *(bufTX7 + u8LenP7T2 - 2) = 208;
    *(bufTX7 + u8LenP7T2 - 3) = 2;
   
    if (i7_1 >= (u8LenP7T2 - 2)) 
    {
      i7_1= u8LenP7T1 + 3;
      j7_1= 0;
      *(bufTX7 + u8LenP7T1) = 0xA9;
      for (i = (u8LenP7T1+1); i < (u8LenP7T2-3); i++)
      {
        *(bufTX7 + i) = 0;
      }
      *(bufTX7 + u8LenP7T2 - 1) = 0xA9;
      *(bufTX7 + u8LenP7T2 - 2) = 208;
      *(bufTX7 + u8LenP7T2 - 3) = 2;
    }
    *(bufTX7 + i7_1) = *(bufTX7 + i7_1) + 1;
    j7_1++;
    if (j7_1 >= 160)
    {
      i7_1++;
      j7_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX7 + u8LenP7T2) = 0xA9;
    *(bufTX7 + u8LenP7T2 + 1) = 0;   //HEADER
    *(bufTX7 + u8LenP7T3 - 1) = 0xA9;
    *(bufTX7 + u8LenP7T3 - 2) = 208;
    *(bufTX7 + u8LenP7T3 - 3) = 3;
   
    if (i7_2 >= (u8LenP7T3 - 2)) 
    {
      i7_2= u8LenP7T2 + 3;
      j7_2= 0;
      *(bufTX7 + u8LenP7T2) = 0xA9;
      for (i = (u8LenP7T2+1); i < (u8LenP7T3-3); i++)
      {
        *(bufTX7 + i) = 0;
      }
      *(bufTX7 + u8LenP7T3 - 1) = 0xA9;
      *(bufTX7 + u8LenP7T3 - 2) = 208;
      *(bufTX7 + u8LenP7T3 - 3) = 3;
    }
    *(bufTX7 + i7_2) = *(bufTX7 + i7_2) + 1;
    j7_2++;
    if (j7_2 >= 160)
    {
      i7_2++;
      j7_2 = 0;
    }
    
    //----------------- WRITE PORT 8 -----------------------
    //Port 8 Tel 1
    *(bufTX8) = 0xA9;
    *(bufTX8 + 1) = 0;        //HEADER
    *(bufTX8 + u8LenP8T1 - 1) = 0xA9;
    *(bufTX8 + u8LenP8T1 - 2) = 209;
    *(bufTX8 + u8LenP8T1 - 3) = 1;
   
    if (i8 >= (u8LenP8T1 - 2)) 
    {
      i8= 3;
      j8= 0;
      //memset(bufTX8, 0, 9);
      *(bufTX8) = 0xA9;
      for (i = 1; i < (u8LenP8T1-3); i++)
      {
        *(bufTX8 + i) = 0;
      }
      *(bufTX8 + u8LenP8T1 - 1) = 0xA9;
      *(bufTX8 + u8LenP8T1 - 2) = 209;
      *(bufTX8 + u8LenP8T1 - 3) = 1;
    }
    *(bufTX8 + i8) = *(bufTX8 + i8) + 1;
    j8++;
    if (j8 >= 160)
    {
      i8++;
      j8 = 0;
    }
    // Port 0 Tel 2
    *(bufTX8 + u8LenP8T1) = 0xA9;
    *(bufTX8 + u8LenP8T1 + 1) = 0;   //HEADER
    *(bufTX8 + u8LenP8T2 - 1) = 0xA9;
    *(bufTX8 + u8LenP8T2 - 2) = 209;
    *(bufTX8 + u8LenP8T2 - 3) = 2;
   
    if (i8_1 >= (u8LenP8T2 - 2)) 
    {
      i8_1= u8LenP8T1 + 3;
      j8_1= 0;
      *(bufTX8 + u8LenP8T1) = 0xA9;
      for (i = (u8LenP8T1+1); i < (u8LenP8T2-3); i++)
      {
        *(bufTX8 + i) = 0;
      }
      *(bufTX8 + u8LenP8T2 - 1) = 0xA9;
      *(bufTX8 + u8LenP8T2 - 2) = 209;
      *(bufTX8 + u8LenP8T2 - 3) = 2;
    }
    *(bufTX8 + i8_1) = *(bufTX8 + i8_1) + 1;
    j8_1++;
    if (j8_1 >= 160)
    {
      i8_1++;
      j8_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX8 + u8LenP8T2) = 0xA9;
    *(bufTX8 + u8LenP8T2 + 1) = 0;   //HEADER
    *(bufTX8 + u8LenP8T3 - 1) = 0xA9;
    *(bufTX8 + u8LenP8T3 - 2) = 209;
    *(bufTX8 + u8LenP8T3 - 3) = 3;
   
    if (i8_2 >= (u8LenP8T3 - 2)) 
    {
      i8_2= u8LenP8T2 + 3;
      j8_2= 0;
      *(bufTX8 + u8LenP8T2) = 0xA9;
      for (i = (u8LenP8T2+1); i < (u8LenP8T3-3); i++)
      {
        *(bufTX8 + i) = 0;
      }
      *(bufTX8 + u8LenP8T3 - 1) = 0xA9;
      *(bufTX8 + u8LenP8T3 - 2) = 209;
      *(bufTX8 + u8LenP8T3 - 3) = 3;
    }
    *(bufTX8 + i8_2) = *(bufTX8 + i8_2) + 1;
    j8_2++;
    if (j8_2 >= 160)
    {
      i8_2++;
      j8_2 = 0;
    }
    
    //----------------- WRITE PORT 9 -----------------------
    //Port 9 Tel 1
    *(bufTX9) = 0xA9;
    *(bufTX9 + 1) = 0;        //HEADER
    *(bufTX9 + u8LenP9T1 - 1) = 0xA9;
    *(bufTX9 + u8LenP9T1 - 2) = 210;
    *(bufTX9 + u8LenP9T1 - 3) = 1;
   
    if (i9 >= (u8LenP9T1 - 2)) 
    {
      i9= 3;
      j9= 0;
      //memset(bufTX9, 0, 9);
      *(bufTX9) = 0xA9;
      for (i = 1; i < (u8LenP9T1-3); i++)
      {
        *(bufTX9 + i) = 0;
      }
      *(bufTX9 + u8LenP9T1 - 1) = 0xA9;
      *(bufTX9 + u8LenP9T1 - 2) = 210;
      *(bufTX9 + u8LenP9T1 - 3) = 1;
    }
    *(bufTX9 + i9) = *(bufTX9 + i9) + 1;
    j9++;
    if (j9 >= 160)
    {
      i9++;
      j9 = 0;
    }
    // Port 0 Tel 2
    *(bufTX9 + u8LenP9T1) = 0xA9;
    *(bufTX9 + u8LenP9T1 + 1) = 0;   //HEADER
    *(bufTX9 + u8LenP9T2 - 1) = 0xA9;
    *(bufTX9 + u8LenP9T2 - 2) = 210;
    *(bufTX9 + u8LenP9T2 - 3) = 2;
   
    if (i9_1 >= (u8LenP9T2 - 2)) 
    {
      i9_1= u8LenP9T1 + 3;
      j9_1= 0;
      *(bufTX9 + u8LenP9T1) = 0xA9;
      for (i = (u8LenP9T1+1); i < (u8LenP9T2-3); i++)
      {
        *(bufTX9 + i) = 0;
      }
      *(bufTX9 + u8LenP9T2 - 1) = 0xA9;
      *(bufTX9 + u8LenP9T2 - 2) = 210;
      *(bufTX9 + u8LenP9T2 - 3) = 2;
    }
    *(bufTX9 + i9_1) = *(bufTX9 + i9_1) + 1;
    j9_1++;
    if (j9_1 >= 160)
    {
      i9_1++;
      j9_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX9 + u8LenP9T2) = 0xA9;
    *(bufTX9 + u8LenP9T2 + 1) = 0;   //HEADER
    *(bufTX9 + u8LenP9T3 - 1) = 0xA9;
    *(bufTX9 + u8LenP9T3 - 2) = 210;
    *(bufTX9 + u8LenP9T3 - 3) = 3;
   
    if (i9_2 >= (u8LenP9T3 - 2)) 
    {
      i9_2= u8LenP9T2 + 3;
      j9_2= 0;
      *(bufTX9 + u8LenP9T2) = 0xA9;
      for (i = (u8LenP9T2+1); i < (u8LenP9T3-3); i++)
      {
        *(bufTX9 + i) = 0;
      }
      *(bufTX9 + u8LenP9T3 - 1) = 0xA9;
      *(bufTX9 + u8LenP9T3 - 2) = 210;
      *(bufTX9 + u8LenP9T3 - 3) = 3;
    }
    *(bufTX9 + i9_2) = *(bufTX9 + i9_2) + 1;
    j9_2++;
    if (j9_2 >= 160)
    {
      i9_2++;
      j9_2 = 0;
    }
    
    //----------------- WRITE PORT 10 -----------------------
    //Port 10 Tel 1
    *(bufTX10) = 0xA9;
    *(bufTX10 + 1) = 0;        //HEADER
    *(bufTX10 + u8LenP10T1 - 1) = 0xA9;
    *(bufTX10 + u8LenP10T1 - 2) = 211;
    *(bufTX10 + u8LenP10T1 - 3) = 1;
   
    if (i10 >= (u8LenP10T1 - 2)) 
    {
      i10= 3;
      j10= 0;
      //memset(bufTX10, 0, 9);
      *(bufTX10) = 0xA9;
      for (i = 1; i < (u8LenP10T1-3); i++)
      {
        *(bufTX10 + i) = 0;
      }
      *(bufTX10 + u8LenP10T1 - 1) = 0xA9;
      *(bufTX10 + u8LenP10T1 - 2) = 211;
      *(bufTX10 + u8LenP10T1 - 3) = 1;
    }
    *(bufTX10 + i10) = *(bufTX10 + i10) + 1;
    j10++;
    if (j10 >= 160)
    {
      i10++;
      j10 = 0;
    }
    // Port 0 Tel 2
    *(bufTX10 + u8LenP10T1) = 0xA9;
    *(bufTX10 + u8LenP10T1 + 1) = 0;   //HEADER
    *(bufTX10 + u8LenP10T2 - 1) = 0xA9;
    *(bufTX10 + u8LenP10T2 - 2) = 211;
    *(bufTX10 + u8LenP10T2 - 3) = 2;
   
    if (i10_1 >= (u8LenP10T2 - 2)) 
    {
      i10_1= u8LenP10T1 + 3;
      j10_1= 0;
      *(bufTX10 + u8LenP10T1) = 0xA9;
      for (i = (u8LenP10T1+1); i < (u8LenP10T2-3); i++)
      {
        *(bufTX10 + i) = 0;
      }
      *(bufTX10 + u8LenP10T2 - 1) = 0xA9;
      *(bufTX10 + u8LenP10T2 - 2) = 211;
      *(bufTX10 + u8LenP10T2 - 3) = 2;
    }
    *(bufTX10 + i10_1) = *(bufTX10 + i10_1) + 1;
    j10_1++;
    if (j10_1 >= 160)
    {
      i10_1++;
      j10_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX10 + u8LenP10T2) = 0xA9;
    *(bufTX10 + u8LenP10T2 + 1) = 0;   //HEADER
    *(bufTX10 + u8LenP10T3 - 1) = 0xA9;
    *(bufTX10 + u8LenP10T3 - 2) = 211;
    *(bufTX10 + u8LenP10T3 - 3) = 3;
   
    if (i10_2 >= (u8LenP10T3 - 2)) 
    {
      i10_2= u8LenP10T2 + 3;
      j10_2= 0;
      *(bufTX10 + u8LenP10T2) = 0xA9;
      for (i = (u8LenP10T2+1); i < (u8LenP10T3-3); i++)
      {
        *(bufTX10 + i) = 0;
      }
      *(bufTX10 + u8LenP10T3 - 1) = 0xA9;
      *(bufTX10 + u8LenP10T3 - 2) = 211;
      *(bufTX10 + u8LenP10T3 - 3) = 3;
    }
    *(bufTX10 + i10_2) = *(bufTX10 + i10_2) + 1;
    j10_2++;
    if (j10_2 >= 160)
    {
      i10_2++;
      j10_2 = 0;
    }
    
    //----------------- WRITE PORT 11 -----------------------
    //Port 11 Tel 1
    *(bufTX11) = 0xA9;
    *(bufTX11 + 1) = 0;        //HEADER
    *(bufTX11 + u8LenP11T1 - 1) = 0xA9;
    *(bufTX11 + u8LenP11T1 - 2) = 212;
    *(bufTX11 + u8LenP11T1 - 3) = 1;
   
    if (i11 >= (u8LenP11T1 - 2)) 
    {
      i11= 3;
      j11= 0;
      //memset(bufTX11, 0, 9);
      *(bufTX11) = 0xA9;
      for (i = 1; i < (u8LenP11T1-3); i++)
      {
        *(bufTX11 + i) = 0;
      }
      *(bufTX11 + u8LenP11T1 - 1) = 0xA9;
      *(bufTX11 + u8LenP11T1 - 2) = 212;
      *(bufTX11 + u8LenP11T1 - 3) = 1;
    }
    *(bufTX11 + i11) = *(bufTX11 + i11) + 1;
    j11++;
    if (j11 >= 160)
    {
      i11++;
      j11 = 0;
    }
    // Port 0 Tel 2
    *(bufTX11 + u8LenP11T1) = 0xA9;
    *(bufTX11 + u8LenP11T1 + 1) = 0;   //HEADER
    *(bufTX11 + u8LenP11T2 - 1) = 0xA9;
    *(bufTX11 + u8LenP11T2 - 2) = 212;
    *(bufTX11 + u8LenP11T2 - 3) = 2;
   
    if (i11_1 >= (u8LenP11T2 - 2)) 
    {
      i11_1= u8LenP11T1 + 3;
      j11_1= 0;
      *(bufTX11 + u8LenP11T1) = 0xA9;
      for (i = (u8LenP11T1+1); i < (u8LenP11T2-3); i++)
      {
        *(bufTX11 + i) = 0;
      }
      *(bufTX11 + u8LenP11T2 - 1) = 0xA9;
      *(bufTX11 + u8LenP11T2 - 2) = 212;
      *(bufTX11 + u8LenP11T2 - 3) = 2;
    }
    *(bufTX11 + i11_1) = *(bufTX11 + i11_1) + 1;
    j11_1++;
    if (j11_1 >= 160)
    {
      i11_1++;
      j11_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX11 + u8LenP11T2) = 0xA9;
    *(bufTX11 + u8LenP11T2 + 1) = 0;   //HEADER
    *(bufTX11 + u8LenP11T3 - 1) = 0xA9;
    *(bufTX11 + u8LenP11T3 - 2) = 212;
    *(bufTX11 + u8LenP11T3 - 3) = 3;
   
    if (i11_2 >= (u8LenP11T3 - 2)) 
    {
      i11_2= u8LenP11T2 + 3;
      j11_2= 0;
      *(bufTX11 + u8LenP11T2) = 0xA9;
      for (i = (u8LenP11T2+1); i < (u8LenP11T3-3); i++)
      {
        *(bufTX11 + i) = 0;
      }
      *(bufTX11 + u8LenP11T3 - 1) = 0xA9;
      *(bufTX11 + u8LenP11T3 - 2) = 212;
      *(bufTX11 + u8LenP11T3 - 3) = 3;
    }
    *(bufTX11 + i11_2) = *(bufTX11 + i11_2) + 1;
    j11_2++;
    if (j11_2 >= 160)
    {
      i11_2++;
      j11_2 = 0;
    }

    //----------------- WRITE PORT 12 -----------------------
    //Port 12 Tel 1
    *(bufTX12) = 0xA9;
    *(bufTX12 + 1) = 0;        //HEADER
    *(bufTX12 + u8LenP12T1 - 1) = 0xA9;
    *(bufTX12 + u8LenP12T1 - 2) = 213;
    *(bufTX12 + u8LenP12T1 - 3) = 1;
   
    if (i12 >= (u8LenP12T1 - 2)) 
    {
      i12= 3;
      j12= 0;
      //memset(bufTX12, 0, 9);
      *(bufTX12) = 0xA9;
      for (i = 1; i < (u8LenP12T1-3); i++)
      {
        *(bufTX12 + i) = 0;
      }
      *(bufTX12 + u8LenP12T1 - 1) = 0xA9;
      *(bufTX12 + u8LenP12T1 - 2) = 213;
      *(bufTX12 + u8LenP12T1 - 3) = 1;
    }
    *(bufTX12 + i12) = *(bufTX12 + i12) + 1;
    j12++;
    if (j12 >= 160)
    {
      i12++;
      j12 = 0;
    }
    // Port 0 Tel 2
    *(bufTX12 + u8LenP12T1) = 0xA9;
    *(bufTX12 + u8LenP12T1 + 1) = 0;   //HEADER
    *(bufTX12 + u8LenP12T2 - 1) = 0xA9;
    *(bufTX12 + u8LenP12T2 - 2) = 213;
    *(bufTX12 + u8LenP12T2 - 3) = 2;
   
    if (i12_1 >= (u8LenP12T2 - 2)) 
    {
      i12_1= u8LenP12T1 + 3;
      j12_1= 0;
      *(bufTX12 + u8LenP12T1) = 0xA9;
      for (i = (u8LenP12T1+1); i < (u8LenP12T2-3); i++)
      {
        *(bufTX12 + i) = 0;
      }
      *(bufTX12 + u8LenP12T2 - 1) = 0xA9;
      *(bufTX12 + u8LenP12T2 - 2) = 213;
      *(bufTX12 + u8LenP12T2 - 3) = 2;
    }
    *(bufTX12 + i12_1) = *(bufTX12 + i12_1) + 1;
    j12_1++;
    if (j12_1 >= 160)
    {
      i12_1++;
      j12_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX12 + u8LenP12T2) = 0xA9;
    *(bufTX12 + u8LenP12T2 + 1) = 0;   //HEADER
    *(bufTX12 + u8LenP12T3 - 1) = 0xA9;
    *(bufTX12 + u8LenP12T3 - 2) = 213;
    *(bufTX12 + u8LenP12T3 - 3) = 3;
   
    if (i12_2 >= (u8LenP12T3 - 2)) 
    {
      i12_2= u8LenP12T2 + 3;
      j12_2= 0;
      *(bufTX12 + u8LenP12T2) = 0xA9;
      for (i = (u8LenP12T2+1); i < (u8LenP12T3-3); i++)
      {
        *(bufTX12 + i) = 0;
      }
      *(bufTX12 + u8LenP12T3 - 1) = 0xA9;
      *(bufTX12 + u8LenP12T3 - 2) = 213;
      *(bufTX12 + u8LenP12T3 - 3) = 3;
    }
    *(bufTX12 + i12_2) = *(bufTX12 + i12_2) + 1;
    j12_2++;
    if (j12_2 >= 160)
    {
      i12_2++;
      j12_2 = 0;
    }

    //----------------- WRITE PORT 13 -----------------------
    //Port 13 Tel 1
    *(bufTX13) = 0xA9;
    *(bufTX13 + 1) = 0;        //HEADER
    *(bufTX13 + u8LenP13T1 - 1) = 0xA9;
    *(bufTX13 + u8LenP13T1 - 2) = 214;
    *(bufTX13 + u8LenP13T1 - 3) = 1;
   
    if (i13 >= (u8LenP13T1 - 2)) 
    {
      i13= 3;
      j13= 0;
      //memset(bufTX13, 0, 9);
      *(bufTX13) = 0xA9;
      for (i = 1; i < (u8LenP13T1-3); i++)
      {
        *(bufTX13 + i) = 0;
      }
      *(bufTX13 + u8LenP13T1 - 1) = 0xA9;
      *(bufTX13 + u8LenP13T1 - 2) = 214;
      *(bufTX13 + u8LenP13T1 - 3) = 1;
    }
    *(bufTX13 + i13) = *(bufTX13 + i13) + 1;
    j13++;
    if (j13 >= 160)
    {
      i13++;
      j13 = 0;
    }
    // Port 0 Tel 2
    *(bufTX13 + u8LenP13T1) = 0xA9;
    *(bufTX13 + u8LenP13T1 + 1) = 0;   //HEADER
    *(bufTX13 + u8LenP13T2 - 1) = 0xA9;
    *(bufTX13 + u8LenP13T2 - 2) = 214;
    *(bufTX13 + u8LenP13T2 - 3) = 2;
   
    if (i13_1 >= (u8LenP13T2 - 2)) 
    {
      i13_1= u8LenP13T1 + 3;
      j13_1= 0;
      *(bufTX13 + u8LenP13T1) = 0xA9;
      for (i = (u8LenP13T1+1); i < (u8LenP13T2-3); i++)
      {
        *(bufTX13 + i) = 0;
      }
      *(bufTX13 + u8LenP13T2 - 1) = 0xA9;
      *(bufTX13 + u8LenP13T2 - 2) = 214;
      *(bufTX13 + u8LenP13T2 - 3) = 2;
    }
    *(bufTX13 + i13_1) = *(bufTX13 + i13_1) + 1;
    j13_1++;
    if (j13_1 >= 160)
    {
      i13_1++;
      j13_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX13 + u8LenP13T2) = 0xA9;
    *(bufTX13 + u8LenP13T2 + 1) = 0;   //HEADER
    *(bufTX13 + u8LenP13T3 - 1) = 0xA9;
    *(bufTX13 + u8LenP13T3 - 2) = 214;
    *(bufTX13 + u8LenP13T3 - 3) = 3;
   
    if (i13_2 >= (u8LenP13T3 - 2)) 
    {
      i13_2= u8LenP13T2 + 3;
      j13_2= 0;
      *(bufTX13 + u8LenP13T2) = 0xA9;
      for (i = (u8LenP13T2+1); i < (u8LenP13T3-3); i++)
      {
        *(bufTX13 + i) = 0;
      }
      *(bufTX13 + u8LenP13T3 - 1) = 0xA9;
      *(bufTX13 + u8LenP13T3 - 2) = 214;
      *(bufTX13 + u8LenP13T3 - 3) = 3;
    }
    *(bufTX13 + i13_2) = *(bufTX13 + i13_2) + 1;
    j13_2++;
    if (j13_2 >= 160)
    {
      i13_2++;
      j13_2 = 0;
    }
    
    //----------------- WRITE PORT 14 -----------------------
    //Port 14 Tel 1
    *(bufTX14) = 0xA9;
    *(bufTX14 + 1) = 0;        //HEADER
    *(bufTX14 + u8LenP14T1 - 1) = 0xA9;
    *(bufTX14 + u8LenP14T1 - 2) = 215;
    *(bufTX14 + u8LenP14T1 - 3) = 1;
   
    if (i14 >= (u8LenP14T1 - 2)) 
    {
      i14= 3;
      j14= 0;
      //memset(bufTX14, 0, 9);
      *(bufTX14) = 0xA9;
      for (i = 1; i < (u8LenP14T1-3); i++)
      {
        *(bufTX14 + i) = 0;
      }
      *(bufTX14 + u8LenP14T1 - 1) = 0xA9;
      *(bufTX14 + u8LenP14T1 - 2) = 215;
      *(bufTX14 + u8LenP14T1 - 3) = 1;
    }
    *(bufTX14 + i14) = *(bufTX14 + i14) + 1;
    j14++;
    if (j14 >= 160)
    {
      i14++;
      j14 = 0;
    }
    // Port 0 Tel 2
    *(bufTX14 + u8LenP14T1) = 0xA9;
    *(bufTX14 + u8LenP14T1 + 1) = 0;   //HEADER
    *(bufTX14 + u8LenP14T2 - 1) = 0xA9;
    *(bufTX14 + u8LenP14T2 - 2) = 215;
    *(bufTX14 + u8LenP14T2 - 3) = 2;
   
    if (i14_1 >= (u8LenP14T2 - 2)) 
    {
      i14_1= u8LenP14T1 + 3;
      j14_1= 0;
      *(bufTX14 + u8LenP14T1) = 0xA9;
      for (i = (u8LenP14T1+1); i < (u8LenP14T2-3); i++)
      {
        *(bufTX14 + i) = 0;
      }
      *(bufTX14 + u8LenP14T2 - 1) = 0xA9;
      *(bufTX14 + u8LenP14T2 - 2) = 215;
      *(bufTX14 + u8LenP14T2 - 3) = 2;
    }
    *(bufTX14 + i14_1) = *(bufTX14 + i14_1) + 1;
    j14_1++;
    if (j14_1 >= 160)
    {
      i14_1++;
      j14_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX14 + u8LenP14T2) = 0xA9;
    *(bufTX14 + u8LenP14T2 + 1) = 0;   //HEADER
    *(bufTX14 + u8LenP14T3 - 1) = 0xA9;
    *(bufTX14 + u8LenP14T3 - 2) = 215;
    *(bufTX14 + u8LenP14T3 - 3) = 3;
   
    if (i14_2 >= (u8LenP14T3 - 2)) 
    {
      i14_2= u8LenP14T2 + 3;
      j14_2= 0;
      *(bufTX14 + u8LenP14T2) = 0xA9;
      for (i = (u8LenP14T2+1); i < (u8LenP14T3-3); i++)
      {
        *(bufTX14 + i) = 0;
      }
      *(bufTX14 + u8LenP14T3 - 1) = 0xA9;
      *(bufTX14 + u8LenP14T3 - 2) = 215;
      *(bufTX14 + u8LenP14T3 - 3) = 3;
    }
    *(bufTX14 + i14_2) = *(bufTX14 + i14_2) + 1;
    j14_2++;
    if (j14_2 >= 160)
    {
      i14_2++;
      j14_2 = 0;
    }
   
    //----------------- WRITE PORT 15 -----------------------
    //Port 15 Tel 1
    *(bufTX15) = 0xA9;
    *(bufTX15 + 1) = 0;        //HEADER
    *(bufTX15 + u8LenP15T1 - 1) = 0xA9;
    *(bufTX15 + u8LenP15T1 - 2) = 216;
    *(bufTX15 + u8LenP15T1 - 3) = 1;
   
    if (i15 >= (u8LenP15T1 - 2)) 
    {
      i15= 3;
      j15= 0;
      //memset(bufTX15, 0, 9);
      *(bufTX15) = 0xA9;
      for (i = 1; i < (u8LenP15T1-3); i++)
      {
        *(bufTX15 + i) = 0;
      }
      *(bufTX15 + u8LenP15T1 - 1) = 0xA9;
      *(bufTX15 + u8LenP15T1 - 2) = 216;
      *(bufTX15 + u8LenP15T1 - 3) = 1;
    }
    *(bufTX15 + i15) = *(bufTX15 + i15) + 1;
    j15++;
    if (j15 >= 160)
    {
      i15++;
      j15 = 0;
    }
    // Port 0 Tel 2
    *(bufTX15 + u8LenP15T1) = 0xA9;
    *(bufTX15 + u8LenP15T1 + 1) = 0;   //HEADER
    *(bufTX15 + u8LenP15T2 - 1) = 0xA9;
    *(bufTX15 + u8LenP15T2 - 2) = 216;
    *(bufTX15 + u8LenP15T2 - 3) = 2;
   
    if (i15_1 >= (u8LenP15T2 - 2)) 
    {
      i15_1= u8LenP15T1 + 3;
      j15_1= 0;
      *(bufTX15 + u8LenP15T1) = 0xA9;
      for (i = (u8LenP15T1+1); i < (u8LenP15T2-3); i++)
      {
        *(bufTX15 + i) = 0;
      }
      *(bufTX15 + u8LenP15T2 - 1) = 0xA9;
      *(bufTX15 + u8LenP15T2 - 2) = 216;
      *(bufTX15 + u8LenP15T2 - 3) = 2;
    }
    *(bufTX15 + i15_1) = *(bufTX15 + i15_1) + 1;
    j15_1++;
    if (j15_1 >= 160)
    {
      i15_1++;
      j15_1 = 0;
    }
    
    // Port 0 Tel 3
    *(bufTX15 + u8LenP15T2) = 0xA9;
    *(bufTX15 + u8LenP15T2 + 1) = 0;   //HEADER
    *(bufTX15 + u8LenP15T3 - 1) = 0xA9;
    *(bufTX15 + u8LenP15T3 - 2) = 216;
    *(bufTX15 + u8LenP15T3 - 3) = 3;
   
    if (i15_2 >= (u8LenP15T3 - 2)) 
    {
      i15_2= u8LenP15T2 + 3;
      j15_2= 0;
      *(bufTX15 + u8LenP15T2) = 0xA9;
      for (i = (u8LenP15T2+1); i < (u8LenP15T3-3); i++)
      {
        *(bufTX15 + i) = 0;
      }
      *(bufTX15 + u8LenP15T3 - 1) = 0xA9;
      *(bufTX15 + u8LenP15T3 - 2) = 216;
      *(bufTX15 + u8LenP15T3 - 3) = 3;
    }
    *(bufTX15 + i15_2) = *(bufTX15 + i15_2) + 1;
    j15_2++;
    if (j15_2 >= 160)
    {
      i15_2++;
      j15_2 = 0;
    }
 
    //---------------------------------------
    
    
    //WRITE:
    struct timespec current;
    struct timespec last_stat;
    struct timespec last_stat1;
    struct timespec stopT;
    tv_nsec = 0;
    tv_nsec1 = 0;
    
    countRead++;
    
    clock_gettime(CLOCK_MONOTONIC, &last_stat);
    //printf("last_stat.tv_sec: %09ld\n", last_stat.tv_nsec);

    t++;
        
    //W1
       
    ret = write(fd, bufTX0, u8LenP0T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      //printf("X ; 1W. TX: %d; [ms]:; %3f;", ret, rate);
      printf("X ; 1W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, 0, tx_rate);
      printf("\n");
      
    }

//goto Lace;
        
    //W2
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd1, bufTX1, u8LenP1T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 2W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W3
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd2, bufTX2, u8LenP2T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 3W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W4
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd3, bufTX3, u8LenP3T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 4W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W5
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd4, bufTX4, u8LenP4T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 5W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W6
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd5, bufTX5, u8LenP5T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 6W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W7
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd6, bufTX6, u8LenP6T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 7W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W8
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd7, bufTX7, u8LenP7T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 8W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W9
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd8, bufTX8, u8LenP8T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 9W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W10
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd9, bufTX9, u8LenP9T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 10W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W11
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd10, bufTX10, u8LenP10T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 11W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W12
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd11, bufTX11, u8LenP11T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 12W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W13
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd12, bufTX12, u8LenP12T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 13W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W14
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd13, bufTX13, u8LenP13T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 14W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W15
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd14, bufTX14, u8LenP14T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 15W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //W16
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    ret = write(fd15, bufTX15, u8LenP15T3);
    if (ret == -1)
    {
      printf("write error: %d\n", ret);
    }
    else if (ret < 0)
    {
      printf("write error: %d\n", ret);
    }
    else
    {
      clock_gettime(CLOCK_MONOTONIC, &current);
      
      
      if (current.tv_nsec > last_stat.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
      }
     
      rate = ((double)tv_nsec/1000000);
      
      
      if (current.tv_nsec > last_stat1.tv_nsec)
      {
        tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
      }
      else
      {
        tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
      }
     
      tx_rate = ((double)tv_nsec/1000000);
      
      if ((tx_rate < tx_rate_min) && (Z > 1000))
      {
        tx_rate_min = tx_rate;
      }
      if ((tx_rate > 1) && (Z > 1000))
      {
        tx_rate_max = ((double)(tx_rate + tx_rate_max)/2);
      }
      
      if ((tx_rate > 1) && (Z > 1000))
      {
        cntErrTx++;
      }
      
      
      printf("X ; 16W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
      printf("\n");
      //clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      
    }
    
    //------------------------------- READ: -----------------------------------------------
    // Polling pause: 1ms
    //usleep(1000);

Lace:
    //usleep(10000);
    clock_gettime(CLOCK_MONOTONIC, &last_stat1);
    cntErrTime0 = 0;
    cntErrRTime0 = 0;
    do
    {
      ret = 0;
      ret = read(fd, bufRX0, 0);
      //printf("RET0: %d\n", ret);
      
      countPollZyc++;
      if (ret < 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      if ((ret & 4096) != 0)
      {
        printf("1. Polling: worklist NOT LEER \n");
      }
      if ((ret & 8192) != 0)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("1. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      if ((ret & 16384) != 0)
      {
        //printf("1. Polling: TX BUF NOT LEER \n");
        cntErrRTime0++;
      }
      if ((ret & 32768) != 0)
      {
        //printf("1. Polling RX BUF Empty\n");
        cntErrTime0++;
      }
      if ((ret & 61440) != 0)
      {
        //break;
      }
      else
      {
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX0[i] == 201)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX0[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX0[i]); 
            printf(ANSI_COLOR_RESET);
          }

       } 
       
       lenTel = ret & 4095;
       lenTelTotal += lenTel;
       if (lenTelTotal >= (u8LenP0T3 - 3))
       {
          break;
       }
       ret &= 61440;
       
       if ((ret != 0) || (lenTel < (u8LenP0T3 - 3)))
       {
          //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
          usleep(100);
       }
     }while((ret != 0) || (lenTel < (u8LenP0T3 - 3)));

          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 1R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP0T1 - 1),  (u8LenP0T2 - u8LenP0T1 - 1),  (u8LenP0T3 - u8LenP0T2 - 1), rate, rx_rate, cntErrRTime0, cntErrTime0);
          printf("\n");
          
     cntErrTime0 = 0;
     cntErrRTime0 = 0;
     lenTel = 0;
     lenTelTotal = 0;

//break;
       
      //2.
      //usleep(1000);
      //ret = ioctl(fd1, TIO_PORT_POLL, &tr);
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime1 = 0;
      cntErrRTime1 = 0;
      do
      {
        ret = read(fd1, bufRX1, 0);
        //printf("RET1: %d\n", ret);
     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("2. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("2. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("2. Polling: TX BUF NOT LEER \n");
          cntErrRTime1++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("2. Polling RX BUF Empty\n");
          cntErrTime1++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {

          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX1[i] == 202)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX1[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX1[i]); 
            printf(ANSI_COLOR_RESET);
          }
                    
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP1T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP1T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP1T3 - 3)));
      clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 2R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP1T1 - 1),  (u8LenP1T2 - u8LenP1T1 - 1),  (u8LenP1T3 - u8LenP1T2 - 1), rate, rx_rate, cntErrRTime1, cntErrTime1);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime1 = 0;
      cntErrRTime1 = 0;
      
      //usleep(1000);
      //R3.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime2 = 0;
      cntErrRTime2 = 0;
      do
      {
        ret = read(fd2, bufRX2, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("3. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("3. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("3. Polling: TX BUF NOT LEER \n");
          cntErrRTime2++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("3. Polling RX BUF Empty\n");
          cntErrTime2++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX2[i] == 203)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX2[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX2[i]); 
            printf(ANSI_COLOR_RESET);
          }
          
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP2T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP2T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP2T3 - 3)));
           
           clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 3R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP2T1 - 1),  (u8LenP2T2 - u8LenP2T1 - 1),  (u8LenP2T3 - u8LenP2T2 - 1), rate, rx_rate, cntErrRTime2, cntErrTime2);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime2 = 0;
      cntErrRTime2 = 0;
       
       //usleep(1000);
      //R4.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime3 = 0;
      cntErrRTime3 = 0;
      do
      {
        ret = read(fd3, bufRX3, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("4. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("4. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("4. Polling: TX BUF NOT LEER \n");
          cntErrRTime3++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("4. Polling RX BUF Empty\n");
          cntErrTime3++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX3[i] == 204)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX3[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX3[i]); 
            printf(ANSI_COLOR_RESET);
          }
          
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP3T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP3T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP3T3 - 3)));
      
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 4R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP3T1 - 1),  (u8LenP3T2 - u8LenP3T1 - 1),  (u8LenP3T3 - u8LenP3T2 - 1), rate, rx_rate, cntErrRTime3, cntErrTime3);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime3 = 0;
      cntErrRTime3 = 0;
      
      //usleep(1000);
      //R5.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime4 = 0;
      cntErrRTime4 = 0;
      do
      {
        ret = read(fd4, bufRX4, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("5. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("5. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("5. Polling: TX BUF NOT LEER \n");
          cntErrRTime4++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("5. Polling RX BUF Empty\n");
          cntErrTime4++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX4[i] == 205)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX4[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX4[i]); 
            printf(ANSI_COLOR_RESET);
          }
          
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP4T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP4T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP4T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 5R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP4T1 - 1),  (u8LenP4T2 - u8LenP4T1 - 1),  (u8LenP4T3 - u8LenP4T2 - 1), rate, rx_rate, cntErrRTime4, cntErrTime4);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime4 = 0;
      cntErrRTime4 = 0;
       
       //usleep(1000);
      //R6.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime5 = 0;
      cntErrRTime5 = 0;
      do
      {
        ret = read(fd5, bufRX5, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("6. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("6. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("6. Polling: TX BUF NOT LEER \n");
          cntErrRTime5++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("6. Polling RX BUF Empty\n");
          cntErrTime5++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX5[i] == 206)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX5[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX5[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP5T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP5T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP5T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 6R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP5T1 - 1),  (u8LenP5T2 - u8LenP5T1 - 1),  (u8LenP5T3 - u8LenP5T2 - 1), rate, rx_rate, cntErrRTime5, cntErrTime5);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime5 = 0;
      cntErrRTime5 = 0;
       
       //usleep(1000);
       //R7.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime6 = 0;
      cntErrRTime6 = 0;
      do
      {
        ret = read(fd6, bufRX6, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("7. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("7. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("7. Polling: TX BUF NOT LEER \n");
          cntErrRTime6++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("7. Polling RX BUF Empty\n");
          cntErrTime6++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX6[i] == 207)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX6[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX6[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP6T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP6T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP6T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 7R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP6T1 - 1),  (u8LenP6T2 - u8LenP6T1 - 1),  (u8LenP6T3 - u8LenP6T2 - 1), rate, rx_rate, cntErrRTime6, cntErrTime6);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime6 = 0;
      cntErrRTime6 = 0;
       
       //usleep(1000);
      //R8.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime7 = 0;
      cntErrRTime7 = 0;
      do
      {
        ret = read(fd7, bufRX7, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("8. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("8. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("8. Polling: TX BUF NOT LEER \n");
          cntErrRTime7++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("8. Polling RX BUF Empty\n");
          cntErrTime7++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX7[i] == 208)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX7[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX7[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP7T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP7T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP7T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 8R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP7T1 - 1),  (u8LenP7T2 - u8LenP7T1 - 1),  (u8LenP7T3 - u8LenP7T2 - 1), rate, rx_rate, cntErrRTime7, cntErrTime7);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime7 = 0;
      cntErrRTime7 = 0;
       
       //usleep(1000);
      //R9.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime8 = 0;
      cntErrRTime8 = 0;
      do
      {
        ret = read(fd8, bufRX8, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("9. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("9. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("9. Polling: TX BUF NOT LEER \n");
          cntErrRTime8++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("9. Polling RX BUF Empty\n");
          cntErrTime8++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX8[i] == 209)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX8[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX8[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP8T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP8T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP8T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 9R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP8T1 - 1),  (u8LenP8T2 - u8LenP8T1 - 1),  (u8LenP8T3 - u8LenP8T2 - 1), rate, rx_rate, cntErrRTime8, cntErrTime8);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime8 = 0;
      cntErrRTime8 = 0;
       
       //usleep(1000);
       //R10.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime9 = 0;
      cntErrRTime9 = 0;
      do
      {
        ret = read(fd9, bufRX9, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("10. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("10. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("10. Polling: TX BUF NOT LEER \n");
          cntErrRTime9++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("10. Polling RX BUF Empty\n");
          cntErrTime9++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX9[i] == 210)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX9[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX9[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP9T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP9T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP9T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 10R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP9T1 - 1),  (u8LenP9T2 - u8LenP9T1 - 1),  (u8LenP9T3 - u8LenP9T2 - 1), rate, rx_rate, cntErrRTime9, cntErrTime9);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime9 = 0;
      cntErrRTime9 = 0;
       
       //usleep(1000);
       //R11.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime10 = 0;
      cntErrRTime10 = 0;
      do
      {
        ret = read(fd10, bufRX10, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("11. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("11. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("11. Polling: TX BUF NOT LEER \n");
          cntErrRTime10++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("11. Polling RX BUF Empty\n");
          cntErrTime10++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX10[i] == 211)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX10[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX10[i]); 
            printf(ANSI_COLOR_RESET);
          }
 
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP10T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP10T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP10T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 11R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP10T1 - 1),  (u8LenP10T2 - u8LenP10T1 - 1),  (u8LenP10T3 - u8LenP10T2 - 1), rate, rx_rate, cntErrRTime10, cntErrTime10);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime10 = 0;
      cntErrRTime10 = 0;
       
       //usleep(1000);
       //R12.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime11 = 0;
      cntErrRTime11 = 0;
      do
      {
        ret = read(fd11, bufRX11, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("12. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("12. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("12. Polling: TX BUF NOT LEER \n");
          cntErrRTime11++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("12. Polling RX BUF Empty\n");
          cntErrTime11++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX11[i] == 212)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX11[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX11[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP11T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP11T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP11T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 12R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP11T1 - 1),  (u8LenP11T2 - u8LenP11T1 - 1),  (u8LenP11T3 - u8LenP11T2 - 1), rate, rx_rate, cntErrRTime11, cntErrTime11);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime11 = 0;
      cntErrRTime11 = 0;
       
       //usleep(1000);
       //R13.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime12 = 0;
      cntErrRTime12 = 0;
      do
      {
        ret = read(fd12, bufRX12, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          //printf("13. Polling: worklist NOT LEER \n");
          cntErrRTime12++;
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("13. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          printf("13. Polling: TX BUF NOT LEER \n");
        }
        if ((ret & 32768) != 0)
        {
          //printf("13. Polling RX BUF Empty\n");
          cntErrTime12++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX12[i] == 213)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX12[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX12[i]); 
            printf(ANSI_COLOR_RESET);
          } 
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP12T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP12T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP12T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 13R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP12T1 - 1),  (u8LenP12T2 - u8LenP12T1 - 1),  (u8LenP12T3 - u8LenP12T2 - 1), rate, rx_rate, cntErrRTime12, cntErrTime12);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime12 = 0;
      cntErrRTime12 = 0;
       
       //usleep(1000);
       //R14.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime13 = 0;
      cntErrRTime13 = 0;
      do
      {
        ret = read(fd13, bufRX13, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("14. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("14. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("14. Polling: TX BUF NOT LEER \n");
          cntErrRTime13++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("14. Polling RX BUF Empty\n");
          cntErrTime13++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX13[i] == 214)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX13[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX13[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP13T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP13T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP13T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 14R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP13T1 - 1),  (u8LenP13T2 - u8LenP13T1 - 1),  (u8LenP13T3 - u8LenP13T2 - 1), rate, rx_rate, cntErrRTime13, cntErrTime13);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime13 = 0;
      cntErrRTime13 = 0;
       
       //usleep(1000);
       //R15.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime14 = 0;
      cntErrRTime14 = 0;
      do
      {
        ret = read(fd14, bufRX14, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("15. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("15. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("15. Polling: TX BUF NOT LEER \n");
          cntErrRTime14++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("15. Polling RX BUF Empty\n");
          cntErrTime14++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX14[i] == 215)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX14[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX14[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP14T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP14T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP14T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 15R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP14T1 - 1),  (u8LenP14T2 - u8LenP14T1 - 1),  (u8LenP14T3 - u8LenP14T2 - 1), rate, rx_rate, cntErrRTime14, cntErrTime14);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime14 = 0;
      cntErrRTime14 = 0;
       
       //usleep(1000);
       //R16.
      clock_gettime(CLOCK_MONOTONIC, &last_stat1);
      cntErrTime15 = 0;
      cntErrRTime15 = 0;
      do
      {
        ret = read(fd15, bufRX15, 0);     
     
        countPollZyc++;
        if (ret < 0)
        {
          printf("ERROR: %d\n", ret);
          break;
        }
        if ((ret & 4096) != 0)
        {
          printf("16. Polling: worklist NOT LEER \n");
        }
        if ((ret & 8192) != 0)
        {
          countPollErr++;
          countPollProz = (countPollErr * 100)/countPollZyc;
          printf("16. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
          //break;
        }
        if ((ret & 16384) != 0)
        {
          //printf("16. Polling: TX BUF NOT LEER \n");
          cntErrRTime15++;
        }
        if ((ret & 32768) != 0)
        {
          //printf("16. Polling RX BUF Empty\n");
          cntErrTime15++;
        }
        if ((ret & 61440) != 0)
        {
          //break;
        }
        else
        {
          
          
          if (ret > 2047)
          {
            printf("BUF ERR\n");
            break;
          }
          for (i = 0; i < ret; i++)
          {
            if (bufRX15[i] == 216)
            {
              //countReadTeleg++;
              printf(ANSI_COLOR_RED);
            }
            else if (bufRX15[i] == 169) 
            {
              printf(ANSI_COLOR_GREEN);
            }
            printf(" %d", bufRX15[i]); 
            printf(ANSI_COLOR_RESET);
          }
        }
        
        lenTel = ret & 4095;
        lenTelTotal += lenTel;
        if (lenTelTotal >= (u8LenP15T3 - 3))
        {
           break;
        }
        ret &= 61440;
        
        if ((ret != 0) || (lenTel < (u8LenP15T3 - 3)))
        {
           //printf("ERR: ret %d, len: %d\n", ret, lenTel); 
           usleep(100);
        }
      }while((ret != 0) || (lenTel < (u8LenP15T3 - 3)));
          clock_gettime(CLOCK_MONOTONIC, &current);
          
          if (current.tv_nsec > last_stat.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
          }

          rate = ((double)tv_nsec/1000000);
          
          if (current.tv_nsec > last_stat1.tv_nsec)
          {
            tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
          }
          else
          {
            tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
          }
         
          rx_rate = ((double)tv_nsec/1000000);
          
          if ((rx_rate < rx_rate_min) && (Z > 1000))
          {
            rx_rate_min = rx_rate;
          }
          if ((rx_rate > 1) && (Z > 1000))
          {
            rx_rate_max = ((double)(rx_rate + rx_rate_max)/2);
          }
          
          if ((rx_rate > 1) && (Z > 1000))
          {
            cntErrRx++;
          }
          
          printf(" ; 16R. RX: %d; Len Tot: %d; Len T1: %d;  Len T2: %d;  Len T3: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d;", ret, lenTelTotal, (u8LenP15T1 - 1),  (u8LenP15T2 - u8LenP15T1 - 1),  (u8LenP15T3 - u8LenP15T2 - 1), rate, rx_rate, cntErrRTime15, cntErrTime15);
          printf("\n");
          
      lenTel = 0;
      lenTelTotal = 0;
      cntErrTime15 = 0;
      cntErrRTime15 = 0;
      

      //-----------------------------------------------------------------------------------------------
      
      Z++;
      persTx = ((double)cntErrTx*100/(Z*16));
      persRx = ((double)cntErrRx*100/(Z*16));
      printf("TEL: %d --------------------------------------; CNT Err:; %d; PERS_TX:; %3f; ; PERS_RX:; %3f; TX min:; %3f; TX max:; %3f; RX min:; %3f; RX max:; %3f;\n", Z, cntErr, persTx, persRx, tx_rate_min, tx_rate_max, rx_rate_min, rx_rate_max);
      
    //break;
    //usleep(5000);
	}
	free(bufTX0);
	free(bufTX1);
	free(bufTX2);
	free(bufTX3);
	free(bufTX4);
	free(bufTX5);
	free(bufTX6);
	free(bufTX7);
	free(bufTX8);
	free(bufTX9);
	free(bufTX10);
	free(bufTX11);
	free(bufTX12);
	free(bufTX13);
	free(bufTX14);
	free(bufTX15);

	
	free(bufRX0);
	free(bufRX1);
	free(bufRX2);
	free(bufRX3);
	free(bufRX4);
	free(bufRX5);
	free(bufRX6);
	free(bufRX7);
	free(bufRX8);
	free(bufRX9);
	free(bufRX10);
	free(bufRX11);
	free(bufRX12);
	free(bufRX13);
	free(bufRX14);
	free(bufRX15);
	
return 0;

}

int random_number(int min_num, int max_num)
{
   int result = 0, low_num = 0, hi_num = 0;
   if (min_num < max_num)
   {
      low_num = min_num;
      hi_num = max_num + 1; // include max_num in output
   } else {
      low_num = max_num + 1; // include max_num in output
      hi_num = min_num;
   }

   //srand(time(NULL));
   result = (rand() % (hi_num - low_num)) + low_num;
   return result;
}