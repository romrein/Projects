//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.08.2022
//  File            :  pollingAsyncWSyncAsyncReadStruc.c
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


uint8_t default_tx[2048] = {0, };
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


int main(int argc, char *argv[])
{
	int ret = 0;
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
	unsigned int Z = 0;
	int count = 2048;
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
	double rx_rate, tx_rate;
	double rate;
  long tv_nsec = 0;

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
	while (1)
	{
    struct timespec current;
    struct timespec last_stat;
    tv_nsec = 0;
    
    countRead++;
    
    clock_gettime(CLOCK_MONOTONIC, &last_stat);
    //printf("last_stat.tv_sec: %09ld\n", last_stat.tv_nsec);
    
    j++;
    //ret = ioctl(fd, TIO_PORT_POLL, default_rx);
    ret = ioctl(fd, TIO_PORT_POLL, &tr);
    //printf("RET: %d\n", ret);
 
 
    countPollZyc++;
    if (ret <= 0)
    {
      printf("ERROR: %d\n", ret);
      break;
    }
    else if (ret == 3003)
    {
      countPollErr++;
      countPollProz = (countPollErr * 100)/countPollZyc;
      printf("1. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
      //break;
    }
    else if (ret == 3004)
    {
      printf("1. Polling RX BUF Empty\n");
    }
    else if (ret == 3005)
    {
      printf("1. Polling: worklist NOT LEER \n");
    }
    else
    {
      
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
      countReadByte += ret;
      //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f, RX:", countRead, countReadTeleg, ret, countReadByte, rate);
      
      printf("1. RX: %d; [ms]:; %3f;", ret, rate);
      
      //printf("ioctl OK\n");
      //for (i = 0; i < ret; i++)
      //{
      //  if (default_rx[i] == 201)
      //  {
      //    //countReadTeleg++;
      //    printf(ANSI_COLOR_RED);
      //  }
      //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
      //  {
      //    printf(ANSI_COLOR_GREEN);
      //  }
      //  printf(" %d", default_rx[i]); 
      //  printf(ANSI_COLOR_RESET);
      //}
      
      printf("\n");
      
     }
      
      //2.
      
      ret = ioctl(fd1, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("2. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("2. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("2. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("2. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      
      //3. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd2, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("3. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("3. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("3. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("3. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //4. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd3, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("4. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("4. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("4. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("4. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //5. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd4, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("5. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("5. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("5. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("5. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //6. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd5, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("6. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("6. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("6. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("6. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //7. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd6, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("7. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("7. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("7. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("7. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //8. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd7, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("8. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("8. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("8. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("8. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }
      //-----------------------------------------------------------------------------------------------
      
      //9. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd8, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("9. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("9. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("9. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("9. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //10. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd9, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("10. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("10. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("10. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("10. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //11. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd10, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("11. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("11. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("11. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("11. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //12. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd11, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("12. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("12. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("12. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("12. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //13. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd12, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("13. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("13. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("13. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("13. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //14. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd13, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("14. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("14. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("14. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("14. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //15. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd14, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("15. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("15. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("15. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("15. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      //16. ------------------------------------------------------------------------------------------------------------
      
      ret = ioctl(fd15, TIO_PORT_POLL, &tr);
      //printf("RET: %d\n", ret);
   
   
      countPollZyc++;
      if (ret <= 0)
      {
        printf("ERROR: %d\n", ret);
        break;
      }
      else if (ret == 3003)
      {
        countPollErr++;
        countPollProz = (countPollErr * 100)/countPollZyc;
        printf("16. Polling MUTEX Collision. Busy: %d, Error Rate in \%: %lld\n", ret, countPollProz);
        //break;
      }
      else if (ret == 3004)
      {
        printf("16. Polling RX BUF Empty\n");
      }
      else if (ret == 3005)
      {
        printf("16. Polling: worklist NOT LEER \n");
      }
      else
      {
      
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
         //printf("%lld; TelCNT: %lld;bytes readed: %d; sum bytes: %lld; rate [ms]:; %3f", countRead, countReadTeleg, ret, countReadByte, rate);
        
        printf("16. RX: %d; [ms]:; %3f;", ret, rate);
        
        //printf("ioctl OK\n");
        //for (i = 0; i < ret; i++)
        //{
        //  if (default_rx[i] == 201)
        //  {
        //    //countReadTeleg++;
        //    printf(ANSI_COLOR_RED);
        //  }
        //  else if ((default_rx[i] == 250) || (default_rx[i] == 251))
        //  {
        //    printf(ANSI_COLOR_GREEN);
        //  }
        //  printf(" %d", default_rx[i]); 
        //  printf(ANSI_COLOR_RESET);
        //}
        
        printf("\n");
        
        
        //last_stat = current;
      }

      //-----------------------------------------------------------------------------------------------
      
      Z++;
      printf("TEL: %d --------------------------------------\n", Z);
      
	}
	
return 0;

}
