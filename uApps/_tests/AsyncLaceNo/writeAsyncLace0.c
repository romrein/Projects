//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.08.2022
//  File            :  writeAsyncLace0.c
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
#include <errno.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SPI_BUS_IDLE_IRQ	0x01	/* Bit 0: Bus IDLE IRQ (triggers if the BUS idle timeout is reached) */
#define SPI_TX_FIFO_EMPTY_IRQ	0x02	/* Bit 1: TX FIFO Empty IRQ (triggers if the TX Fifo runs dry; transfer is done) */
#define SPI_TX_FIFO_THR_IRQ	0x04	/* Bit 2: TX FIFO Threshold IRQ (triggers if the TX Fifo level is below the threshold) */
#define SPI_TX_FIFO_OVFLW_IRQ	0x08	/* Bit 3: TX FIFO Overflow IRQ (triggers if the TX Fifo overflows and data is lost) */
#define SPI_RX_FIFO_OVFLW_IRQ	0x10	/* Bit 4: RX FIFO Overflow IRQ (triggers if the RX Fifo overflows and data is lost) */
#define SPI_RX_FIFO_THR_IRQ	0x20	/* Bit 5: RX FIFO Threshold IRQ (triggers if the RX Fifo level is above the threshold) */
#define SPI_COLL_DET_IRQ	0x40	/* Bit 6: Collision Detection IRQ (triggers if there was a bus collision when DUS = 1) */
#define SPI_PRTY_ERR_IRQ	0x80	/* Bit 7: RX Parity Error IRQ (triggers if the received parity is incorrect) */

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
uint16_t u16L;

//uint8_t default_tx[] = {
//	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
//	0x07, 0x08, 0x09, 0x0A, 0x00, 0x95,
//	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//	0xF0, 0x0D,
//};

uint8_t default_tx[2048] = {0x00, };
uint8_t default_rx[2048] = {0x00, };
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

		c = getopt_long(argc, argv, "D:s:d:b:i:o:lHOL:C3NR24p:vS:I:",
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
			u16L = (uint16_t)atoi(optarg);
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
	int i0, i1, i2, i3, i4, i5, i6, i7;
	int fd;
	int intReturn;
	uint8_t *tx;
	ssize_t err, bytes;
  int i = 0;
  int j = 0;
  int z = 0;
  uint64_t read_countX = 0;
	uint64_t prev_read_count = 0;
	uint64_t countWrite = 0;
	uint64_t countBytesWrite = 0;
	double rx_rate, tx_rate;
	double rate;
  long tv_nsec;
  unsigned char *bufTX;
	unsigned char *bufRX;
	struct file* filp;
	
	parse_opts(argc, argv);
	
	if (u16L == 0)
	{
    u16L = 2048;
	}
	printf("u16L: %d\n", u16L);
	
  bufTX = malloc(u16L);
  i = 1;
  j = 0; 
  
  
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
    err = 0;
    i0 = 0;
    i1 = 0;
    i2 = 0;
    i3 = 0;
    i4 = 0;
    i5 = 0;
    i6 = 0;
    i7 = 0;
    *(bufTX) = 216;
    *(bufTX + u16L - 1) = 251;
    *(bufTX + u16L - 2) = 250;
    countWrite++;
    struct timespec current;
    struct timespec last_stat;
    
    if (i > (u16L - 1)) 
    {
      i= 1;
      j= 0;
      memset(bufTX, 0, u16L);
      *(bufTX) = 216;
      *(bufTX + u16L - 1) = 251;
      *(bufTX + u16L - 2) = 250;
    }
    *(bufTX + i) = *(bufTX + i) + 1;
    j++;
    if (j >= 200)
    {
      i++;
      j = 0;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &last_stat);
    //printf("last_stat.tv_sec: %09ld\n", last_stat.tv_nsec);
    err = write(fd, bufTX, u16L);
    //i++;
    if (err == -1)
    {
      printf("write error: %d\n", err);
      //printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
      printf("errorno: %d\n", errno);
      ret = errno >> 8;
      
      //ERROR
      if (ret & 0xD8)
      {
        
        if (ret & SPI_TX_FIFO_OVFLW_IRQ)
        {
          printf("status IRQ: SPI_TX_FIFO_OVFLW_IRQ\n");
          i3 = 1;
        }
        if (ret & SPI_RX_FIFO_OVFLW_IRQ)
        {
          printf("status IRQ: SPI_RX_FIFO_OVFLW_IRQ\n");
          i4 = 1;
        }
        if (ret & SPI_COLL_DET_IRQ)
        {
          printf("status IRQ: SPI_COLL_DET_IRQ\n");
          i6 = 1;
        }
        if (ret & SPI_PRTY_ERR_IRQ)
        {
          printf("status IRQ: SPI_PRTY_ERR_IRQ\n");
          i7 = 1;
        }
        break;
      }
      //WARNUNG
      if (ret & SPI_TX_FIFO_THR_IRQ)
      {
        //printf("status IRQ: SPI_TX_FIFO_THR_IRQ\n");
        i2 = 1;
      }
      if (ret & SPI_RX_FIFO_THR_IRQ)
      {
        //printf("status IRQ: SPI_RX_FIFO_THR_IRQ\n");
        i5 = 1;
      }
      if (ret & SPI_BUS_IDLE_IRQ)
      {
        //printf("status IRQ: SPI_BUS_IDLE_IRQ\n");
        i0 = 1;
      }
      if (ret & SPI_TX_FIFO_EMPTY_IRQ)
      {
        //printf("status IRQ: SPI_TX_FIFO_EMPTY_IRQ\n");
        i1 = 1;
      }
    }
    else if (err < 0)
    {
      printf("write error: %d\n", err);
      //printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
      printf("errorno: %d\n", errno);
      err *= (-1);
      ret = err >> 8;
      
      //ERROR
      if (ret & 0xD8)
      {
        
        if (ret & SPI_TX_FIFO_OVFLW_IRQ)
        {
          printf("status IRQ: SPI_TX_FIFO_OVFLW_IRQ\n");
          i3 = 1;
        }
        if (ret & SPI_RX_FIFO_OVFLW_IRQ)
        {
          printf("status IRQ: SPI_RX_FIFO_OVFLW_IRQ\n");
          i4 = 1;
        }
        if (ret & SPI_COLL_DET_IRQ)
        {
          printf("status IRQ: SPI_COLL_DET_IRQ\n");
          i6 = 1;
        }
        if (ret & SPI_PRTY_ERR_IRQ)
        {
          printf("status IRQ: SPI_PRTY_ERR_IRQ\n");
          i7 = 1;
        }
        break;
      }
      //WARNUNG
      if (ret & SPI_TX_FIFO_THR_IRQ)
      {
        //printf("status IRQ: SPI_TX_FIFO_THR_IRQ\n");
        i2 = 1;
      }
      if (ret & SPI_RX_FIFO_THR_IRQ)
      {
        //printf("status IRQ: SPI_RX_FIFO_THR_IRQ\n");
        i5 = 1;
      }
      if (ret & SPI_BUS_IDLE_IRQ)
      {
        //printf("status IRQ: SPI_BUS_IDLE_IRQ\n");
        i0 = 1;
      }
      if (ret & SPI_TX_FIFO_EMPTY_IRQ)
      {
        //printf("status IRQ: SPI_TX_FIFO_EMPTY_IRQ\n");
        i1 = 1;
      }
    }
    else
    {
      if (err < 0)
      {
        //printf("write error < 0: %d\n", err);
      }
      //printf("write: OVERFLOW TX\n");
      //usleep(100000);
    }
    clock_gettime(CLOCK_MONOTONIC, &current);
		//printf("current.tv_sec: %09ld\n", current.tv_nsec);
		
		if (current.tv_nsec > last_stat.tv_nsec)
		{
      tv_nsec = current.tv_nsec - last_stat.tv_nsec;
		}
		else
		{
      //tv_nsec = last_stat.tv_nsec - current.tv_nsec;
      //tv_nsec = current.tv_nsec - last_stat.tv_nsec;
      tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
		}
		
		//printf("current.tv_nsec: %lld\n", current.tv_nsec);
		//printf("last_stat.tv_nsec: %lld\n", last_stat.tv_nsec);
		//printf("tv_nsec: %lld\n", tv_nsec);
		
		//countBytesWrite += u16L;
		
		rate = ((double)tv_nsec/1000000);
		//printf("%lld; rate [ms]:; %9f\n", countWrite, countBytesWrite, rate);
		printf("%9f", rate);

		//rx_rate = ((double)(read_countX - prev_read_count)*8/(rate));

    //printf("read_countX: %lld, prev_read_count: %lld, rate: rx %.3f\n", read_countX, prev_read_count, rx_rate);
    
    //for (z = 0; z < u16L; z++)
    //{
    //  if (*(bufTX + z) == 216)
    //  {
    //    printf(ANSI_COLOR_RED);
    //  }
    //  printf(" %d", *(bufTX + z)); 
    //  printf(ANSI_COLOR_RESET);
    //}
    printf("\n");
    
    //prev_read_count = read_countX;
    
    //last_stat = current;
    
	}//while end
	
	free(bufTX);
	
	//close(fd);
	
return 0;

}
