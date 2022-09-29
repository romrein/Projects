//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.08.2022
//  File            :  pollingAsyncWAsyncRead.c
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

//SPI Addressraum
#define SPI_ADDR_STATUS_OFFSET		8
#define SPI_ADDR_RX_BUF_OFFSET		2048
#define SPI_ADDR_CONFIG_OFFSET		16
#define SPI_ADDR_TX_BUF_OFFSET		2048

#define SPI_ADDR_IRQ_STATUS		512
//#define SPI_ADDR_TX_FIFO_L		513
//#define SPI_ADDR_RX_FIFO_L		515

#define SPI_ADDR_SB_BAUDRATE		33408
#define SPI_ADDR_CD_DS_PM		33409
#define SPI_ADDR_RX_BUS_TOUT		33410
//#define SPI_ADDR_TX_IRQ_TS		33412
//#define SPI_ADDR_RX_IRQ_TS		33414
//#define SPI_ADDR_IRQ_MASK		33416


//UART 0 TX Buffer (128 adresses)
#define SPI_ADDR_TX_BUF			33664

//UART 0 RX Buffer (128 adresses)
#define SPI_ADDR_RX_BUF			640


static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/ttyspi0";
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

uint8_t default_tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0x0D,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };
char *input_tx;


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
	int fd;
	uint8_t *tx;
	ssize_t bytes;
	struct serial_struct sInfo;
	uint16_t uart;
	uint64_t countPollZyc = 0;
	uint64_t countPollErr = 0;
	uint64_t countPollProz = 0;
	int i = 0;
	unsigned int count = 0;
	uint64_t read_countX = 0;
	uint64_t countRead = 0;
	uint64_t countReadTeleg = 0;
	uint64_t countReadByte = 0;
	uint64_t prev_read_count = 0;
	double rx_rate, tx_rate;
	double rate;
  long tv_nsec = 0;
	
	uart = 1;

  memset(&sInfo, 0, sizeof(sInfo));
  
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", device);
	}
	
	while (1)
	{
	
    struct timespec current;
    struct timespec last_stat;
    tv_nsec = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &last_stat);
    
    ret = ioctl(fd, TIOREAD_INFO, &uart);
    if (ret == 3)
    {
      countPollErr++;
      countPollProz = (countPollErr * 100)/countPollZyc;
      printf("Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
      //break;
    }
    else if (ret == 4)
    {
      printf("Polling RX BUF Empty\n");
    }
    else if (ret == 5)
    {
      printf("Polling: worklist NOT LEER\n");
    }
    else
    {
      //printf("ioctl OK\n");
    }
    
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
		count++;
    printf("N: %d; %3f", count, rate);
    printf("\n");
    
    usleep(1000);
	}
	
return 0;

}
