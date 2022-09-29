//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.08.2022
//  File            :  readAsyncLace0.c
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/ttyspi15";
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

uint8_t default_rx[2048] = {0, };
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


static uint64_t _read_count;
static uint64_t _write_count;


int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	uint8_t *tx;
	ssize_t bytes;
	char c;
	uint64_t read_countX = 0;
	uint64_t countRead = 0;
	uint64_t countReadTeleg = 0;
	uint64_t countReadByte = 0;
	uint64_t prev_read_count = 0;
	double rx_rate, tx_rate;
	double rate;
  long tv_nsec = 0;
  int i;
  

	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		printf("ERR");

	}
	else
	{
		printf("OK: %s\n", device);
	}
	
	read_countX = 0;
	rx_rate = 0;
	//c = getchar();
	//printf("read_countX: %lld\n", read_countX);
	//printf("prev_read_count: %lld\n", prev_read_count);
  while (1)
  {
    struct timespec current;
    struct timespec last_stat;
    tv_nsec = 0;
    
    //countRead++;
    
    clock_gettime(CLOCK_MONOTONIC, &last_stat);
    //printf("last_stat.tv_sec: %09ld\n", last_stat.tv_nsec);
    
    ret = read(fd, default_rx, 2048);
    //read_countX += 9;
    if (ret != 128)
    {
      //printf("bytes readed: %d\n", ret);
    }
    else
    {
      //printf("write: ?  %d\n", ret);
    }
    //hex_dump(default_rx, ret, 32, "RX");
    
    
		clock_gettime(CLOCK_MONOTONIC, &current);
		//printf("current.tv_sec: %09ld\n", current.tv_nsec);
		
		if (current.tv_nsec > last_stat.tv_nsec)
		{
      tv_nsec = current.tv_nsec - last_stat.tv_nsec;
		}
		else
		{
      tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
		}
		//printf("current.tv_nsec: %ld\n", current.tv_nsec);
		//printf("last_stat.tv_nsec: %ld\n", last_stat.tv_nsec);
		//printf("tv_nsec:: %ld\n", tv_nsec);


		
		rate = ((double)tv_nsec/1000000);
		//countReadByte += ret;
    printf("%3f", rate);
    for (i = 0; i < ret; i++)
    {
      if (default_rx[i] == 216)
      {
        //countReadTeleg++;
        printf(ANSI_COLOR_RED);
      }
      else if ((default_rx[i] == 250) || (default_rx[i] == 251))
      {
        printf(ANSI_COLOR_GREEN);
      }
      printf(" %d", default_rx[i]); 
      printf(ANSI_COLOR_RESET);
    }
    printf("\n");
      
		//rx_rate = ((double)(read_countX - prev_read_count)*8/(rate));

    //printf("read_countX: %lld, prev_read_count: %lld, rate: rx %.3f\n", read_countX, prev_read_count, rx_rate);

    //prev_read_count = read_countX;
    
    //last_stat = current;
			
	}
	close(fd);
	
  return 0;
}
