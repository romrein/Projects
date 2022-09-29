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



#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

#include <linux/types.h>
#include <linux/serial.h>

 /*
  * SPI testing utility (using spidev driver)
  *
  * Copyright (c) 2007  MontaVista Software, Inc.
  * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License.
  *
  * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
  */

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

#define TIO_KERNEL_POLL			0x6628

#define TIO_PORT_TYP_RSYNC		0x6629
#define TIO_PORT_TYP_RASYNC		0x6630
#define TIO_PORT_TYP_WSYNC		0x6631
#define TIO_PORT_TYP_WASYNC		0x6632
#define TIO_PORTS_STATUS		  0x6633

#define PORT1		0x0001
#define PORT2		0x0002
#define PORT3		0x0004
#define PORT4		0x0008
#define PORT5		0x0010
#define PORT6		0x0020
#define PORT7		0x0040
#define PORT8		0x0080
#define PORT9		0x0100
#define PORT10	0x0200
#define PORT11	0x0400
#define PORT12	0x0800
#define PORT13	0x1000
#define PORT14	0x2000
#define PORT15	0x4000
#define PORT16	0x8000


static void pabort(const char* s)
{
    perror(s);
    abort();
}

static const char* device = "/dev/ttyspi0";
static const char* device1 = "/dev/ttyspi1";
static const char* device2 = "/dev/ttyspi2";
static const char* device3 = "/dev/ttyspi3";
static const char* device4 = "/dev/ttyspi4";
static const char* device5 = "/dev/ttyspi5";
static const char* device6 = "/dev/ttyspi6";
static const char* device7 = "/dev/ttyspi7";
static const char* device8 = "/dev/ttyspi8";
static const char* device9 = "/dev/ttyspi9";
static const char* device10 = "/dev/ttyspi10";
static const char* device11 = "/dev/ttyspi11";
static const char* device12 = "/dev/ttyspi12";
static const char* device13 = "/dev/ttyspi13";
static const char* device14 = "/dev/ttyspi14";
static const char* device15 = "/dev/ttyspi15";

static uint32_t mode;
static uint8_t bits = 8;
static char* input_file;
static char* output_file;
static uint32_t speed = 500000;
static uint16_t delay;
static int verbose;
static int transfer_size;
static int iterations;
static int interval = 5; /* interval in seconds for showing transfer rate */

int PortTypW = 1;
int PortTypR = 1;
int DriverTyp = 2;
int PortTypRsync = 1;
int PortTypRasync = 1;
int PortTypWsync = 1;
int PortTypWasync = 1;

uint16_t u16L;
uint16_t u16Status = 0;
__u8		statusPorts[2048];

struct spi_tty_status {
    __u16 u16irqStatus;
    __u16 u16TxFIFOLev;
    __u16 u16RxFIFOLev;
    __u8 u8res1;
    __u8 u8res2;
};

struct spi_ioc_kernel {
    __u8		tx_buf0[2048];
    __u8		rx_buf0[2048];

    __u8		tx_buf1[2048];
    __u8		rx_buf1[2048];

    __u8		tx_buf2[2048];
    __u8		rx_buf2[2048];

    __u8		tx_buf3[2048];
    __u8		rx_buf3[2048];

    __u8		tx_buf4[2048];
    __u8		rx_buf4[2048];

    __u8		tx_buf5[2048];
    __u8		rx_buf5[2048];

    __u8		tx_buf6[2048];
    __u8		rx_buf6[2048];

    __u8		tx_buf7[2048];
    __u8		rx_buf7[2048];

    __u8		tx_buf8[2048];
    __u8		rx_buf8[2048];

    __u8		tx_buf9[2048];
    __u8		rx_buf9[2048];

    __u8		tx_buf10[2048];
    __u8		rx_buf10[2048];

    __u8		tx_buf11[2048];
    __u8		rx_buf11[2048];

    __u8		tx_buf12[2048];
    __u8		rx_buf12[2048];

    __u8		tx_buf13[2048];
    __u8		rx_buf13[2048];

    __u8		tx_buf14[2048];
    __u8		rx_buf14[2048];

    __u8		tx_buf15[2048];
    __u8		rx_buf15[2048];

    __u64		tx_bufX0;
    __u64		rx_bufX0;

    __u64		tx_bufX1;
    __u64		rx_bufX1;

    __u64		tx_bufX2;
    __u64		rx_bufX2;

    __u64		tx_bufX3;
    __u64		rx_bufX3;

    __u64		tx_bufX4;
    __u64		rx_bufX4;

    __u64		tx_bufX5;
    __u64		rx_bufX5;

    __u64		tx_bufX6;
    __u64		rx_bufX6;

    __u64		tx_bufX7;
    __u64		rx_bufX7;

    __u64		tx_bufX8;
    __u64		rx_bufX8;

    __u64		tx_bufX9;
    __u64		rx_bufX9;

    __u64		tx_bufX10;
    __u64		rx_bufX10;

    __u64		tx_bufX11;
    __u64		rx_bufX11;

    __u64		tx_bufX12;
    __u64		rx_bufX12;

    __u64		tx_bufX13;
    __u64		rx_bufX13;

    __u64		tx_bufX14;
    __u64		rx_bufX14;

    __u64		tx_bufX15;
    __u64		rx_bufX15;

    __u64		rx_status;

    __u32		len[16];
    __u32		lenTX[16];
    __u32		lenRX[16];
    __u32		speed_hz;

    __u32		lenRX0;
    __u32		lenRX1;
    __u32		lenRX2;
    __u32		lenRX3;
    __u32		lenRX4;
    __u32		lenRX5;
    __u32		lenRX6;
    __u32		lenRX7;
    __u32		lenRX8;
    __u32		lenRX9;
    __u32		lenRX10;
    __u32		lenRX11;
    __u32		lenRX12;
    __u32		lenRX13;
    __u32		lenRX14;
    __u32		lenRX15;

    __u16		statusTX;
    __u16		statusRX;
    __u16		statusRead;
    __u16		delay_usecs;
    __u8		bits_per_word;
    __u8		cs_change;
    __u8		tx_nbits;
    __u8		rx_nbits;
    __u16		pad;

    /* If the contents of 'struct spi_ioc_transfer' ever change
     * incompatibly, then the ioctl number (currently 0) must change;
     * ioctls with constant size fields get a bit more in the way of
     * error checking than ones (like this) where that field varies.
     *
     * NOTE: struct layout is the same in 64bit and 32bit userspace.
     */
};



uint8_t default_tx[2048] = { 0xAF, 0xBF, };
uint8_t default_tx1[2048] = { 0xAE, };
uint8_t default_tx2[2048] = { 0, };
uint8_t default_tx3[2048] = { 0, };
uint8_t default_tx4[2048] = { 0, };
uint8_t default_tx5[2048] = { 0, };
uint8_t default_tx6[2048] = { 0, };
uint8_t default_tx7[2048] = { 0, };
uint8_t default_tx8[2048] = { 0, };
uint8_t default_tx9[2048] = { 0, };
uint8_t default_tx10[2048] = { 0, };
uint8_t default_tx11[2048] = { 0, };
uint8_t default_tx12[2048] = { 0, };
uint8_t default_tx13[2048] = { 0, };
uint8_t default_tx14[2048] = { 0, };
uint8_t default_tx15[2048] = { 0, };

uint8_t default_rx[2048] = { 0xEE, };
uint8_t default_rx1[2048] = { 0, };
uint8_t default_rx2[2048] = { 0, };
uint8_t default_rx3[2048] = { 0, };
uint8_t default_rx4[2048] = { 0, };
uint8_t default_rx5[2048] = { 0, };
uint8_t default_rx6[2048] = { 0, };
uint8_t default_rx7[2048] = { 0, };
uint8_t default_rx8[2048] = { 0, };
uint8_t default_rx9[2048] = { 0, };
uint8_t default_rx10[2048] = { 0, };
uint8_t default_rx11[2048] = { 0, };
uint8_t default_rx12[2048] = { 0, };
uint8_t default_rx13[2048] = { 0, };
uint8_t default_rx14[2048] = { 0, };
uint8_t default_rx15[2048] = { 0, };


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

static void parse_opts(int argc, char* argv[])
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


int main(int argc, char* argv[])
{
    uint8_t idx0[16];
    uint8_t idx1[16];
    uint8_t idx2[16];
    uint8_t jdx0[16];
    uint8_t jdx1[16];
    uint8_t jdx2[16];

    int ret = 0;
    int lenTel = 0;
    int lenTelTotal = 0;
    int lenTel1 = 0;
    int lenTelTotal1 = 0;
    int lenTel2 = 0;
    int lenTelTotal2 = 0;
    int lenTel3 = 0;
    int lenTelTotal3 = 0;
    int lenTel4 = 0;
    int lenTelTotal4 = 0;
    int lenTel5 = 0;
    int lenTelTotal5 = 0;
    int lenTel6 = 0;
    int lenTelTotal6 = 0;
    int lenTel7 = 0;
    int lenTelTotal7 = 0;
    int lenTel8 = 0;
    int lenTelTotal8 = 0;
    int lenTel9 = 0;
    int lenTelTotal9 = 0;
    int lenTel10 = 0;
    int lenTelTotal10 = 0;
    int lenTel11 = 0;
    int lenTelTotal11 = 0;
    int lenTel12 = 0;
    int lenTelTotal12 = 0;
    int lenTel13 = 0;
    int lenTelTotal13 = 0;
    int lenTel14 = 0;
    int lenTelTotal14 = 0;
    int lenTel15 = 0;
    int lenTelTotal15 = 0;

    int fd, fd1, fd2, fd3, fd4, fd5, fd6, fd7, fd8, fd9, fd10, fd11, fd12, fd13, fd14, fd15;
    uint8_t* tx;
    uint16_t statusRX[2];
    uint16_t statusRead;;
    uint16_t statusRX16;
    uint16_t lenRX32[1];
    uint16_t lenRX32_1[1];
    uint16_t lenRX32_2[1];
    uint16_t lenRX32_3[1];
    uint16_t lenRX32_4[1];
    uint16_t lenRX32_5[1];
    uint16_t lenRX32_6[1];
    uint16_t lenRX32_7[1];
    uint16_t lenRX32_8[1];
    uint16_t lenRX32_9[1];
    uint16_t lenRX32_10[1];
    uint16_t lenRX32_11[1];
    uint16_t lenRX32_12[1];
    uint16_t lenRX32_13[1];
    uint16_t lenRX32_14[1];
    uint16_t lenRX32_15[1];

    struct spi_tty_status* ttyStatusPorts;
    static struct t_SpiDebug* ptGenBuffer;
    static struct t_BufTXgen tTXGenBuffer[16];

    ssize_t bytes;
    struct serial_struct sInfo;
    uint16_t uart;
    uint64_t countPollZyc = 0;
    uint64_t countPollErr = 0;
    uint64_t countPollProz = 0;
    int i = 0;
    int j = 0;
    int t = 0;

    memset(idx0, 0, sizeof(idx0));
    memset(idx1, 0, sizeof(idx1));
    memset(idx2, 0, sizeof(idx2));
    memset(jdx0, 0, sizeof(jdx0));
    memset(jdx1, 0, sizeof(jdx1));
    memset(jdx2, 0, sizeof(jdx2));

    for (i = 0; i < 16; i++)
    {
        idx0[i] = 3;
        idx1[i] = 13;
        idx2[i] = 23;
        jdx0[i] = 0;
        jdx1[i] = 0;
        jdx2[i] = 0;
    }
    i = 0;

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

    int cntErrZykl0 = 0;
    int cntErrZykl1 = 0;
    int cntErrZykl2 = 0;
    int cntErrZykl3 = 0;
    int cntErrZykl4 = 0;
    int cntErrZykl5 = 0;
    int cntErrZykl6 = 0;
    int cntErrZykl7 = 0;
    int cntErrZykl8 = 0;
    int cntErrZykl9 = 0;
    int cntErrZykl10 = 0;
    int cntErrZykl11 = 0;
    int cntErrZykl12 = 0;
    int cntErrZykl13 = 0;
    int cntErrZykl14 = 0;
    int cntErrZykl15 = 0;

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

    struct spi_ioc_kernel trk;

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
    unsigned char* bufTX0;
    unsigned char* bufTX1;
    unsigned char* bufTX2;
    unsigned char* bufTX3;
    unsigned char* bufTX4;
    unsigned char* bufTX5;
    unsigned char* bufTX6;
    unsigned char* bufTX7;
    unsigned char* bufTX8;
    unsigned char* bufTX9;
    unsigned char* bufTX10;
    unsigned char* bufTX11;
    unsigned char* bufTX12;
    unsigned char* bufTX13;
    unsigned char* bufTX14;
    unsigned char* bufTX15;

    unsigned char* bufRX0;
    unsigned char* bufRX1;
    unsigned char* bufRX2;
    unsigned char* bufRX3;
    unsigned char* bufRX4;
    unsigned char* bufRX5;
    unsigned char* bufRX6;
    unsigned char* bufRX7;
    unsigned char* bufRX8;
    unsigned char* bufRX9;
    unsigned char* bufRX10;
    unsigned char* bufRX11;
    unsigned char* bufRX12;
    unsigned char* bufRX13;
    unsigned char* bufRX14;
    unsigned char* bufRX15;

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

    memset(&trk, 0, sizeof(trk));

 
    default_tx[5] = 253;

    trk.tx_bufX0 = (unsigned long)default_tx;
    trk.rx_bufX0 = (unsigned long)default_rx;

    trk.tx_bufX1 = (unsigned long)default_tx1;
    trk.rx_bufX1 = (unsigned long)default_rx1;

    trk.tx_bufX2 = (unsigned long)default_tx2;
    trk.rx_bufX2 = (unsigned long)default_rx2;

    trk.tx_bufX3 = (unsigned long)default_tx3;
    trk.rx_bufX3 = (unsigned long)default_rx3;

    trk.tx_bufX4 = (unsigned long)default_tx4;
    trk.rx_bufX4 = (unsigned long)default_rx4;

    trk.tx_bufX5 = (unsigned long)default_tx5;
    trk.rx_bufX5 = (unsigned long)default_rx5;

    trk.tx_bufX6 = (unsigned long)default_tx6;
    trk.rx_bufX6 = (unsigned long)default_rx6;

    trk.tx_bufX7 = (unsigned long)default_tx7;
    trk.rx_bufX7 = (unsigned long)default_rx7;

    trk.tx_bufX8 = (unsigned long)default_tx8;
    trk.rx_bufX8 = (unsigned long)default_rx8;

    trk.tx_bufX9 = (unsigned long)default_tx9;
    trk.rx_bufX9 = (unsigned long)default_rx9;

    trk.tx_bufX10 = (unsigned long)default_tx10;
    trk.rx_bufX10 = (unsigned long)default_rx10;

    trk.tx_bufX11 = (unsigned long)default_tx11;
    trk.rx_bufX11 = (unsigned long)default_rx11;

    trk.tx_bufX12 = (unsigned long)default_tx12;
    trk.rx_bufX12 = (unsigned long)default_rx12;

    trk.tx_bufX13 = (unsigned long)default_tx13;
    trk.rx_bufX13 = (unsigned long)default_rx13;

    trk.tx_bufX14 = (unsigned long)default_tx14;
    trk.rx_bufX14 = (unsigned long)default_rx14;

    trk.tx_bufX15 = (unsigned long)default_tx15;
    trk.rx_bufX15 = (unsigned long)default_rx15;

    trk.rx_status = (unsigned long)statusRX;

    trk.statusRX = (unsigned int)statusRX16;

    trk.lenRX0 = (unsigned int)lenRX32;

    trk.lenRX1 = (unsigned int)lenRX32_1;
    trk.lenRX2 = (unsigned int)lenRX32_2;
    trk.lenRX3 = (unsigned int)lenRX32_3;
    trk.lenRX4 = (unsigned int)lenRX32_4;
    trk.lenRX5 = (unsigned int)lenRX32_5;
    trk.lenRX6 = (unsigned int)lenRX32_6;
    trk.lenRX7 = (unsigned int)lenRX32_7;
    trk.lenRX8 = (unsigned int)lenRX32_8;
    trk.lenRX9 = (unsigned int)lenRX32_9;
    trk.lenRX10 = (unsigned int)lenRX32_10;
    trk.lenRX11 = (unsigned int)lenRX32_11;
    trk.lenRX12 = (unsigned int)lenRX32_12;
    trk.lenRX13 = (unsigned int)lenRX32_13;
    trk.lenRX14 = (unsigned int)lenRX32_14;
    trk.lenRX15 = (unsigned int)lenRX32_15;


    //trk.lenRX[1] = (unsigned long)lenRX32_1;

    //trk.tx_buf0 = (unsigned long)default_tx;
    //trk.tx_buf1 = (unsigned long)default_tx1;
    //trk.tx_buf2 = (unsigned long)default_tx2;
    //trk.tx_buf3 = (unsigned long)default_tx3;
    //trk.tx_buf4 = (unsigned long)default_tx4;
    //trk.tx_buf5 = (unsigned long)default_tx5;
    //trk.tx_buf6 = (unsigned long)default_tx6;
    //trk.tx_buf7 = (unsigned long)default_tx7;
    //trk.tx_buf8 = (unsigned long)default_tx8;
    //trk.tx_buf9 = (unsigned long)default_tx9;
    //trk.tx_buf10 = (unsigned long)default_tx10;
    //trk.tx_buf11 = (unsigned long)default_tx11;
    //trk.tx_buf12 = (unsigned long)default_tx12;
    //trk.tx_buf13 = (unsigned long)default_tx13;
    //trk.tx_buf14 = (unsigned long)default_tx14;
    //trk.tx_buf15 = (unsigned long)default_tx15;

    //trk.rx_buf0 = (unsigned long)default_rx;
    //trk.rx_buf1 = (unsigned long)default_rx1;
    //trk.rx_buf2 = (unsigned long)default_rx2;
    //trk.rx_buf3 = (unsigned long)default_rx3;
    //trk.rx_buf4 = (unsigned long)default_rx4;
    //trk.rx_buf5 = (unsigned long)default_rx5;
    //trk.rx_buf6 = (unsigned long)default_rx6;
    //trk.rx_buf7 = (unsigned long)default_rx7;
    //trk.rx_buf8 = (unsigned long)default_rx8;
    //trk.rx_buf9 = (unsigned long)default_rx9;
    //trk.rx_buf10 = (unsigned long)default_rx10;
    //trk.rx_buf11 = (unsigned long)default_rx11;
    //trk.rx_buf12 = (unsigned long)default_rx12;
    //trk.rx_buf13 = (unsigned long)default_rx13;
    //trk.rx_buf14 = (unsigned long)default_rx14;
    //trk.rx_buf15 = (unsigned long)default_rx15;

    //trk.tx_buf1 = (unsigned long)default_tx1;
    //trk.rx_buf1 = (unsigned long)default_rx1;

    trk.tx_buf0[0] = 0xCC;
    trk.tx_buf0[1] = 0xAE;
    trk.tx_buf0[2] = 0xFA;

    trk.tx_buf1[0] = 0xEA;
    trk.tx_buf1[1] = 0xEF;
    trk.tx_buf1[2] = 0xEE;

    trk.len[0] = 16;
    trk.len[1] = 13;
    trk.len[2] = 14;
    trk.len[3] = 14;
    trk.len[4] = 14;
    trk.len[5] = 14;
    trk.len[6] = 14;
    trk.len[7] = 14;
    trk.len[8] = 14;
    trk.len[9] = 14;
    trk.len[10] = 14;
    trk.len[11] = 14;
    trk.len[12] = 14;
    trk.len[13] = 14;
    trk.len[14] = 14;
    trk.len[15] = 14;

    trk.lenTX[0] = 200;

    ptGenBuffer = malloc(sizeof(struct t_SpiDebug));


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

    //WR TEST IRQ !!!!!!!!!!!
    //PortTypR = 2;
    //ret = ioctl(fd, TIO_PORT_TYP_R, &PortTypR);
    //ret = ioctl(fd, TIO_DRIVER_TYP, &DriverTyp);

    ret = ioctl(fd, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd1, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd2, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd3, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd4, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd5, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd6, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd7, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd8, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd9, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd10, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd11, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd12, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd13, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd14, TIO_PORT_TYP_RSYNC, &PortTypRsync);
    ret = ioctl(fd15, TIO_PORT_TYP_RSYNC, &PortTypRsync);



    ///////////////////////////////////////////////////////////////////////////////

    u16Status |= (PORT1);
    u16Status |= (PORT2);
    u16Status |= (PORT3);
    u16Status |= (PORT4);
    u16Status |= (PORT5);
    u16Status |= (PORT6);
    u16Status |= (PORT7);
    u16Status |= (PORT8);
    u16Status |= (PORT9);
    u16Status |= (PORT10);
    u16Status |= (PORT11);
    u16Status |= (PORT12);
    u16Status |= (PORT13);
    u16Status |= (PORT14);
    u16Status |= (PORT15);
    u16Status |= (PORT16);
    printf("STATUS: %d\n", u16Status);
    trk.statusRead = u16Status;

    struct timespec startT;
    clock_gettime(1, &startT);    //1
    srand(time(NULL));

    //-----0
    u8LenP0T1 = (uint8_t)random_number(9, 64);
    printf("LEN: %d\n", u8LenP0T1);

    u8LenP0T2 = u8LenP0T1 + (uint8_t)random_number(9, 64);
    printf("LEN1: %d, PUNKT1: %d\n", u8LenP0T2 - u8LenP0T1, u8LenP0T2);
    idx1[0] = u8LenP0T1 + 3;

    u8LenP0T3 = u8LenP0T2 + (uint8_t)random_number(9, 64);
    printf("LEN2: %d, PUNKT2: %d\n", u8LenP0T3 - u8LenP0T2, u8LenP0T3);
    idx2[0] = u8LenP0T2 + 3;

    tTXGenBuffer[0].pu8DataOut = bufTX0;
    tTXGenBuffer[0].pu8DataIn = bufTX0;
    tTXGenBuffer[0].u8Header = 0xA9;
    tTXGenBuffer[0].u8PortID = 201;
    tTXGenBuffer[0].pui0 = &idx0[0];
    tTXGenBuffer[0].pui1 = &idx1[0];
    tTXGenBuffer[0].pui2 = &idx2[0];
    tTXGenBuffer[0].puj0 = &jdx0[0];
    tTXGenBuffer[0].puj1 = &jdx1[0];
    tTXGenBuffer[0].puj2 = &jdx2[0];
    tTXGenBuffer[0].u8LenPT1 = u8LenP0T1;
    tTXGenBuffer[0].u8LenPT2 = u8LenP0T2;
    tTXGenBuffer[0].u8LenPT3 = u8LenP0T3;

    //-----1

    u8LenP1T1 = (uint8_t)random_number(9, 64);

    u8LenP1T2 = u8LenP1T1 + (uint8_t)random_number(9, 64);
    idx1[1] = u8LenP1T1 + 3;

    u8LenP1T3 = u8LenP1T2 + (uint8_t)random_number(9, 64);
    idx2[1] = u8LenP1T2 + 3;

    tTXGenBuffer[1].pu8DataOut = bufTX1;
    tTXGenBuffer[1].pu8DataIn = bufTX1;
    tTXGenBuffer[1].u8Header = 0xA9;
    tTXGenBuffer[1].u8PortID = 202;
    tTXGenBuffer[1].pui0 = &idx0[1];
    tTXGenBuffer[1].pui1 = &idx1[1];
    tTXGenBuffer[1].pui2 = &idx2[1];
    tTXGenBuffer[1].puj0 = &jdx0[1];
    tTXGenBuffer[1].puj1 = &jdx1[1];
    tTXGenBuffer[1].puj2 = &jdx2[1];
    tTXGenBuffer[1].u8LenPT1 = u8LenP1T1;
    tTXGenBuffer[1].u8LenPT2 = u8LenP1T2;
    tTXGenBuffer[1].u8LenPT3 = u8LenP1T3;

    //-----2
    u8LenP2T1 = (uint8_t)random_number(9, 64);

    u8LenP2T2 = u8LenP2T1 + (uint8_t)random_number(9, 64);
    idx1[2] = u8LenP2T1 + 3;

    u8LenP2T3 = u8LenP2T2 + (uint8_t)random_number(9, 64);
    idx2[2] = u8LenP2T2 + 3;

    tTXGenBuffer[2].pu8DataOut = bufTX2;
    tTXGenBuffer[2].pu8DataIn = bufTX2;
    tTXGenBuffer[2].u8Header = 0xA9;
    tTXGenBuffer[2].u8PortID = 203;
    tTXGenBuffer[2].pui0 = &idx0[2];
    tTXGenBuffer[2].pui1 = &idx1[2];
    tTXGenBuffer[2].pui2 = &idx2[2];
    tTXGenBuffer[2].puj0 = &jdx0[2];
    tTXGenBuffer[2].puj1 = &jdx1[2];
    tTXGenBuffer[2].puj2 = &jdx2[2];
    tTXGenBuffer[2].u8LenPT1 = u8LenP2T1;
    tTXGenBuffer[2].u8LenPT2 = u8LenP2T2;
    tTXGenBuffer[2].u8LenPT3 = u8LenP2T3;

    //-----3
    u8LenP3T1 = (uint8_t)random_number(9, 64);

    u8LenP3T2 = u8LenP3T1 + (uint8_t)random_number(9, 64);
    idx1[3] = u8LenP3T1 + 3;

    u8LenP3T3 = u8LenP3T2 + (uint8_t)random_number(9, 64);
    idx2[3] = u8LenP3T2 + 3;

    tTXGenBuffer[3].pu8DataOut = bufTX3;
    tTXGenBuffer[3].pu8DataIn = bufTX3;
    tTXGenBuffer[3].u8Header = 0xA9;
    tTXGenBuffer[3].u8PortID = 204;
    tTXGenBuffer[3].pui0 = &idx0[3];
    tTXGenBuffer[3].pui1 = &idx1[3];
    tTXGenBuffer[3].pui2 = &idx2[3];
    tTXGenBuffer[3].puj0 = &jdx0[3];
    tTXGenBuffer[3].puj1 = &jdx1[3];
    tTXGenBuffer[3].puj2 = &jdx2[3];
    tTXGenBuffer[3].u8LenPT1 = u8LenP3T1;
    tTXGenBuffer[3].u8LenPT2 = u8LenP3T2;
    tTXGenBuffer[3].u8LenPT3 = u8LenP3T3;

    //-----4
    u8LenP4T1 = (uint8_t)random_number(9, 64);

    u8LenP4T2 = u8LenP4T1 + (uint8_t)random_number(9, 64);
    idx1[4] = u8LenP4T1 + 3;

    u8LenP4T3 = u8LenP4T2 + (uint8_t)random_number(9, 64);
    idx2[4] = u8LenP4T2 + 3;

    tTXGenBuffer[4].pu8DataOut = bufTX4;
    tTXGenBuffer[4].pu8DataIn = bufTX4;
    tTXGenBuffer[4].u8Header = 0xA9;
    tTXGenBuffer[4].u8PortID = 205;
    tTXGenBuffer[4].pui0 = &idx0[4];
    tTXGenBuffer[4].pui1 = &idx1[4];
    tTXGenBuffer[4].pui2 = &idx2[4];
    tTXGenBuffer[4].puj0 = &jdx0[4];
    tTXGenBuffer[4].puj1 = &jdx1[4];
    tTXGenBuffer[4].puj2 = &jdx2[4];
    tTXGenBuffer[4].u8LenPT1 = u8LenP4T1;
    tTXGenBuffer[4].u8LenPT2 = u8LenP4T2;
    tTXGenBuffer[4].u8LenPT3 = u8LenP4T3;

    //-----5
    u8LenP5T1 = (uint8_t)random_number(9, 64);

    u8LenP5T2 = u8LenP5T1 + (uint8_t)random_number(9, 64);
    idx1[5] = u8LenP5T1 + 3;

    u8LenP5T3 = u8LenP5T2 + (uint8_t)random_number(9, 64);
    idx2[5] = u8LenP5T2 + 3;

    tTXGenBuffer[5].pu8DataOut = bufTX5;
    tTXGenBuffer[5].pu8DataIn = bufTX5;
    tTXGenBuffer[5].u8Header = 0xA9;
    tTXGenBuffer[5].u8PortID = 206;
    tTXGenBuffer[5].pui0 = &idx0[5];
    tTXGenBuffer[5].pui1 = &idx1[5];
    tTXGenBuffer[5].pui2 = &idx2[5];
    tTXGenBuffer[5].puj0 = &jdx0[5];
    tTXGenBuffer[5].puj1 = &jdx1[5];
    tTXGenBuffer[5].puj2 = &jdx2[5];
    tTXGenBuffer[5].u8LenPT1 = u8LenP5T1;
    tTXGenBuffer[5].u8LenPT2 = u8LenP5T2;
    tTXGenBuffer[5].u8LenPT3 = u8LenP5T3;

    //-----6
    u8LenP6T1 = (uint8_t)random_number(9, 64);

    u8LenP6T2 = u8LenP6T1 + (uint8_t)random_number(9, 64);
    idx1[6] = u8LenP6T1 + 3;

    u8LenP6T3 = u8LenP6T2 + (uint8_t)random_number(9, 64);
    idx2[6] = u8LenP6T2 + 3;

    tTXGenBuffer[6].pu8DataOut = bufTX6;
    tTXGenBuffer[6].pu8DataIn = bufTX6;
    tTXGenBuffer[6].u8Header = 0xA9;
    tTXGenBuffer[6].u8PortID = 207;
    tTXGenBuffer[6].pui0 = &idx0[6];
    tTXGenBuffer[6].pui1 = &idx1[6];
    tTXGenBuffer[6].pui2 = &idx2[6];
    tTXGenBuffer[6].puj0 = &jdx0[6];
    tTXGenBuffer[6].puj1 = &jdx1[6];
    tTXGenBuffer[6].puj2 = &jdx2[6];
    tTXGenBuffer[6].u8LenPT1 = u8LenP6T1;
    tTXGenBuffer[6].u8LenPT2 = u8LenP6T2;
    tTXGenBuffer[6].u8LenPT3 = u8LenP6T3;

    //-----7
    u8LenP7T1 = (uint8_t)random_number(9, 64);

    u8LenP7T2 = u8LenP7T1 + (uint8_t)random_number(9, 64);
    idx1[7] = u8LenP7T1 + 3;

    u8LenP7T3 = u8LenP7T2 + (uint8_t)random_number(9, 64);
    idx2[7] = u8LenP7T2 + 3;

    tTXGenBuffer[7].pu8DataOut = bufTX7;
    tTXGenBuffer[7].pu8DataIn = bufTX7;
    tTXGenBuffer[7].u8Header = 0xA9;
    tTXGenBuffer[7].u8PortID = 208;
    tTXGenBuffer[7].pui0 = &idx0[7];
    tTXGenBuffer[7].pui1 = &idx1[7];
    tTXGenBuffer[7].pui2 = &idx2[7];
    tTXGenBuffer[7].puj0 = &jdx0[7];
    tTXGenBuffer[7].puj1 = &jdx1[7];
    tTXGenBuffer[7].puj2 = &jdx2[7];
    tTXGenBuffer[7].u8LenPT1 = u8LenP7T1;
    tTXGenBuffer[7].u8LenPT2 = u8LenP7T2;
    tTXGenBuffer[7].u8LenPT3 = u8LenP7T3;

    //-----8
    u8LenP8T1 = (uint8_t)random_number(9, 64);

    u8LenP8T2 = u8LenP8T1 + (uint8_t)random_number(9, 64);
    idx1[8] = u8LenP8T1 + 3;

    u8LenP8T3 = u8LenP8T2 + (uint8_t)random_number(9, 64);
    idx2[8] = u8LenP8T2 + 3;

    tTXGenBuffer[8].pu8DataOut = bufTX8;
    tTXGenBuffer[8].pu8DataIn = bufTX8;
    tTXGenBuffer[8].u8Header = 0xA9;
    tTXGenBuffer[8].u8PortID = 209;
    tTXGenBuffer[8].pui0 = &idx0[8];
    tTXGenBuffer[8].pui1 = &idx1[8];
    tTXGenBuffer[8].pui2 = &idx2[8];
    tTXGenBuffer[8].puj0 = &jdx0[8];
    tTXGenBuffer[8].puj1 = &jdx1[8];
    tTXGenBuffer[8].puj2 = &jdx2[8];
    tTXGenBuffer[8].u8LenPT1 = u8LenP8T1;
    tTXGenBuffer[8].u8LenPT2 = u8LenP8T2;
    tTXGenBuffer[8].u8LenPT3 = u8LenP8T3;

    //-----9
    u8LenP9T1 = (uint8_t)random_number(9, 64);

    u8LenP9T2 = u8LenP9T1 + (uint8_t)random_number(9, 64);
    idx1[9] = u8LenP9T1 + 3;

    u8LenP9T3 = u8LenP9T2 + (uint8_t)random_number(9, 64);
    idx2[9] = u8LenP9T2 + 3;

    tTXGenBuffer[9].pu8DataOut = bufTX9;
    tTXGenBuffer[9].pu8DataIn = bufTX9;
    tTXGenBuffer[9].u8Header = 0xA9;
    tTXGenBuffer[9].u8PortID = 210;
    tTXGenBuffer[9].pui0 = &idx0[9];
    tTXGenBuffer[9].pui1 = &idx1[9];
    tTXGenBuffer[9].pui2 = &idx2[9];
    tTXGenBuffer[9].puj0 = &jdx0[9];
    tTXGenBuffer[9].puj1 = &jdx1[9];
    tTXGenBuffer[9].puj2 = &jdx2[9];
    tTXGenBuffer[9].u8LenPT1 = u8LenP9T1;
    tTXGenBuffer[9].u8LenPT2 = u8LenP9T2;
    tTXGenBuffer[9].u8LenPT3 = u8LenP9T3;

    //-----10
    u8LenP10T1 = (uint8_t)random_number(9, 64);

    u8LenP10T2 = u8LenP10T1 + (uint8_t)random_number(9, 64);
    idx1[10] = u8LenP10T1 + 3;

    u8LenP10T3 = u8LenP10T2 + (uint8_t)random_number(9, 64);
    idx2[10] = u8LenP10T2 + 3;

    tTXGenBuffer[10].pu8DataOut = bufTX10;
    tTXGenBuffer[10].pu8DataIn = bufTX10;
    tTXGenBuffer[10].u8Header = 0xA9;
    tTXGenBuffer[10].u8PortID = 211;
    tTXGenBuffer[10].pui0 = &idx0[10];
    tTXGenBuffer[10].pui1 = &idx1[10];
    tTXGenBuffer[10].pui2 = &idx2[10];
    tTXGenBuffer[10].puj0 = &jdx0[10];
    tTXGenBuffer[10].puj1 = &jdx1[10];
    tTXGenBuffer[10].puj2 = &jdx2[10];
    tTXGenBuffer[10].u8LenPT1 = u8LenP10T1;
    tTXGenBuffer[10].u8LenPT2 = u8LenP10T2;
    tTXGenBuffer[10].u8LenPT3 = u8LenP10T3;

    //-----11
    u8LenP11T1 = (uint8_t)random_number(9, 64);

    u8LenP11T2 = u8LenP11T1 + (uint8_t)random_number(9, 64);
    idx1[11] = u8LenP11T1 + 3;

    u8LenP11T3 = u8LenP11T2 + (uint8_t)random_number(9, 64);
    idx2[11] = u8LenP11T2 + 3;

    tTXGenBuffer[11].pu8DataOut = bufTX11;
    tTXGenBuffer[11].pu8DataIn = bufTX11;
    tTXGenBuffer[11].u8Header = 0xA9;
    tTXGenBuffer[11].u8PortID = 212;
    tTXGenBuffer[11].pui0 = &idx0[11];
    tTXGenBuffer[11].pui1 = &idx1[11];
    tTXGenBuffer[11].pui2 = &idx2[11];
    tTXGenBuffer[11].puj0 = &jdx0[11];
    tTXGenBuffer[11].puj1 = &jdx1[11];
    tTXGenBuffer[11].puj2 = &jdx2[11];
    tTXGenBuffer[11].u8LenPT1 = u8LenP11T1;
    tTXGenBuffer[11].u8LenPT2 = u8LenP11T2;
    tTXGenBuffer[11].u8LenPT3 = u8LenP11T3;

    //-----12
    u8LenP12T1 = (uint8_t)random_number(9, 64);

    u8LenP12T2 = u8LenP12T1 + (uint8_t)random_number(9, 64);
    idx1[12] = u8LenP12T1 + 3;

    u8LenP12T3 = u8LenP12T2 + (uint8_t)random_number(9, 64);
    idx2[12] = u8LenP12T2 + 3;

    tTXGenBuffer[12].pu8DataOut = bufTX12;
    tTXGenBuffer[12].pu8DataIn = bufTX12;
    tTXGenBuffer[12].u8Header = 0xA9;
    tTXGenBuffer[12].u8PortID = 213;
    tTXGenBuffer[12].pui0 = &idx0[12];
    tTXGenBuffer[12].pui1 = &idx1[12];
    tTXGenBuffer[12].pui2 = &idx2[12];
    tTXGenBuffer[12].puj0 = &jdx0[12];
    tTXGenBuffer[12].puj1 = &jdx1[12];
    tTXGenBuffer[12].puj2 = &jdx2[12];
    tTXGenBuffer[12].u8LenPT1 = u8LenP12T1;
    tTXGenBuffer[12].u8LenPT2 = u8LenP12T2;
    tTXGenBuffer[12].u8LenPT3 = u8LenP12T3;

    //-----13
    u8LenP13T1 = (uint8_t)random_number(9, 64);

    u8LenP13T2 = u8LenP13T1 + (uint8_t)random_number(9, 64);
    idx1[13] = u8LenP13T1 + 3;

    u8LenP13T3 = u8LenP13T2 + (uint8_t)random_number(9, 64);
    idx2[13] = u8LenP13T2 + 3;

    tTXGenBuffer[13].pu8DataOut = bufTX13;
    tTXGenBuffer[13].pu8DataIn = bufTX13;
    tTXGenBuffer[13].u8Header = 0xA9;
    tTXGenBuffer[13].u8PortID = 214;
    tTXGenBuffer[13].pui0 = &idx0[13];
    tTXGenBuffer[13].pui1 = &idx1[13];
    tTXGenBuffer[13].pui2 = &idx2[13];
    tTXGenBuffer[13].puj0 = &jdx0[13];
    tTXGenBuffer[13].puj1 = &jdx1[13];
    tTXGenBuffer[13].puj2 = &jdx2[13];
    tTXGenBuffer[13].u8LenPT1 = u8LenP13T1;
    tTXGenBuffer[13].u8LenPT2 = u8LenP13T2;
    tTXGenBuffer[13].u8LenPT3 = u8LenP13T3;

    //-----14
    u8LenP14T1 = (uint8_t)random_number(9, 64);

    u8LenP14T2 = u8LenP14T1 + (uint8_t)random_number(9, 64);
    idx1[14] = u8LenP14T1 + 3;

    u8LenP14T3 = u8LenP14T2 + (uint8_t)random_number(9, 64);
    idx2[14] = u8LenP14T2 + 3;

    tTXGenBuffer[14].pu8DataOut = bufTX14;
    tTXGenBuffer[14].pu8DataIn = bufTX14;
    tTXGenBuffer[14].u8Header = 0xA9;
    tTXGenBuffer[14].u8PortID = 215;
    tTXGenBuffer[14].pui0 = &idx0[14];
    tTXGenBuffer[14].pui1 = &idx1[14];
    tTXGenBuffer[14].pui2 = &idx2[14];
    tTXGenBuffer[14].puj0 = &jdx0[14];
    tTXGenBuffer[14].puj1 = &jdx1[14];
    tTXGenBuffer[14].puj2 = &jdx2[14];
    tTXGenBuffer[14].u8LenPT1 = u8LenP14T1;
    tTXGenBuffer[14].u8LenPT2 = u8LenP14T2;
    tTXGenBuffer[14].u8LenPT3 = u8LenP14T3;

    //-----15
    u8LenP15T1 = (uint8_t)random_number(9, 64);

    u8LenP15T2 = u8LenP15T1 + (uint8_t)random_number(9, 64);
    idx1[15] = u8LenP15T1 + 3;

    u8LenP15T3 = u8LenP15T2 + (uint8_t)random_number(9, 64);
    idx2[15] = u8LenP15T2 + 3;

    tTXGenBuffer[15].pu8DataOut = bufTX15;
    tTXGenBuffer[15].pu8DataIn = bufTX15;
    tTXGenBuffer[15].u8Header = 0xA9;
    tTXGenBuffer[15].u8PortID = 216;
    tTXGenBuffer[15].pui0 = &idx0[15];
    tTXGenBuffer[15].pui1 = &idx1[15];
    tTXGenBuffer[15].pui2 = &idx2[15];
    tTXGenBuffer[15].puj0 = &jdx0[15];
    tTXGenBuffer[15].puj1 = &jdx1[15];
    tTXGenBuffer[15].puj2 = &jdx2[15];
    tTXGenBuffer[15].u8LenPT1 = u8LenP15T1;
    tTXGenBuffer[15].u8LenPT2 = u8LenP15T2;
    tTXGenBuffer[15].u8LenPT3 = u8LenP15T3;

    ///////////////////////////////////////////////////////////////////////////////
    while (1)   //t < 2
    {

        struct timespec current;
        struct timespec last_stat;
        struct timespec last_stat1;
        struct timespec last_stat2;
        struct timespec last_stat3;
        struct timespec last_stat4;
        struct timespec last_stat5;
        struct timespec last_stat6;
        struct timespec last_stat7;
        struct timespec last_stat8;
        struct timespec last_stat9;
        struct timespec last_stat10;
        struct timespec last_stat11;
        struct timespec last_stat12;
        struct timespec last_stat13;
        struct timespec last_stat14;
        struct timespec last_stat15;
        struct timespec last_stat16;
        struct timespec stopT;

        clock_gettime(1, &last_stat);     //1

    zykl:

        //ptGenBuffer = ptTXgenerate(&tTXGenBuffer[0]);

        //for (int i = 0; i < u8LenP0T3; i++)
        //{
        //    printf(" %02X", *(bufTX0 + i));
        //}

        //printf("\n");
        //printf("idx 0: %d, %d, %d, %d, %d, %d, \n", idx0[0], idx1[0], idx2[0], jdx0[0], jdx1[0], jdx2[0]);

        //printf("\n");
        //printf("\n");

        //ptGenBuffer = ptTXgenerate(&tTXGenBuffer[0]);

        //for (int i = 0; i < u8LenP0T3; i++)
        //{
        //    printf(" %02X", *(bufTX0 + i));
        //}

        //printf("\n");
        //printf("idx 2: %d, %d, %d, %d, %d, %d, \n", idx0[2], idx1[2], idx2[2], jdx0[2], jdx1[2], jdx2[2]);
        //printf("\n");
          
        //ptGenBuffer = ptTXgenerate(&tTXGenBuffer[2]);

        //for (int i = 0; i < u8LenP0T3; i++)
        //{
        //    printf(" %02X", *(bufTX0 + i));
        //}

        //printf("\n");
        //printf("idx 2: %d, %d, %d, %d, %d, %d, \n", idx0[2], idx1[2], idx2[2], jdx0[2], jdx1[2], jdx2[2]);
        //printf("\n");

        //printf("----------2---------\n");

        //ptGenBuffer = ptTXgenerate(&tTXGenBuffer[2]);

        //for (int i = 0; i < u8LenP0T3; i++)
        //{
        //    printf(" %02X", *(bufTX0 + i));
        //}

        //printf("\n");
        //printf("idx 2: %d, %d, %d, %d, %d, %d, \n", idx0[2], idx1[2], idx2[2], jdx0[2], jdx1[2], jdx2[2]);
        //printf("\n");


        //----------------- WRITE PORT 0 -----------------------
        if ((u16Status & PORT1) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[0]);
        }
        //----------------- WRITE PORT 1 -----------------------
        //Port 1 Tel 1
        if ((u16Status & PORT2) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[1]);
        }
        //----------------- WRITE PORT 2 -----------------------
        //Port 2 Tel 1
        if ((u16Status & PORT3) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[2]);
        }

        //----------------- WRITE PORT 3 -----------------------
        //Port 3 Tel 1
        if ((u16Status & PORT4) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[3]);
        }

        //----------------- WRITE PORT 4 -----------------------
        //Port 4 Tel 1
        if ((u16Status & PORT5) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[4]);
        }

        //----------------- WRITE PORT 5 -----------------------
        //Port 5 Tel 1
        if ((u16Status & PORT6) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[5]);
        }

        //----------------- WRITE PORT 6 -----------------------
        //Port 6 Tel 1
        if ((u16Status & PORT7) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[6]);
        }

        //----------------- WRITE PORT 7 -----------------------
        //Port 7 Tel 1
        if ((u16Status & PORT8) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[7]);
        }

        //----------------- WRITE PORT 8 -----------------------
        //Port 8 Tel 1
        if ((u16Status & PORT9) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[8]);
        }

        //----------------- WRITE PORT 9 -----------------------
        //Port 9 Tel 1
        if ((u16Status & PORT10) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[9]);
        }

        //----------------- WRITE PORT 10 -----------------------
        //Port 10 Tel 1
        if ((u16Status & PORT11) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[10]);
        }

        //----------------- WRITE PORT 11 -----------------------
        //Port 11 Tel 1
        if ((u16Status & PORT12) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[11]);
        }

        //----------------- WRITE PORT 12 -----------------------
        //Port 12 Tel 1
        if ((u16Status & PORT13) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[12]);
        }

        //----------------- WRITE PORT 13 -----------------------
        //Port 13 Tel 1
        if ((u16Status & PORT14) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[13]);
        }

        //----------------- WRITE PORT 14 -----------------------
        //Port 14 Tel 1
        if ((u16Status & PORT15) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[14]);
        }

        //----------------- WRITE PORT 15 -----------------------
        //Port 15 Tel 1
        if ((u16Status & PORT16) != 0)
        {
            ptGenBuffer = ptTXgenerate(&tTXGenBuffer[15]);
        }

        //---------------------------------------


        //WRITE:

        tv_nsec = 0;
        tv_nsec1 = 0;

        countRead++;


        //printf("last_stat.tv_sec: %09ld\n", last_stat.tv_nsec);

        t++;

        //----------------------------------- PORT 1 WRITE / READ START -------------------------------------------------------------- 
        //WRITE 1

    //zykl:
        

        clock_gettime(1, &last_stat);
        if ((u16Status & PORT1) != 0)
        {
            u16Status &= ~(PORT1);
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
                clock_gettime(1, &current);

                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }


                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }

                //printf("X ; 1W. TX: %d; [ms]:; %3f;", ret, rate);
                printf("X ; 1W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, 0, tx_rate);
                printf("\n");

            }
            clock_gettime(1, &last_stat1);
            cntErrTime0 = 0;
            cntErrRTime0 = 0;
            cntErrZykl0 = 0;
        }

        if ((u16Status & PORT2) != 0)
        {
            u16Status &= ~(PORT2);
            clock_gettime(1, &last_stat2);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat2.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat2.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat2.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 2W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");

            }
            clock_gettime(1, &last_stat2);
            cntErrTime1 = 0;
            cntErrRTime1 = 0;
            cntErrZykl1 = 0;
        }
        //WRITE 3
        if ((u16Status & PORT3) != 0)
        {
            u16Status &= ~(PORT3);
            clock_gettime(1, &last_stat3);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat3.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat3.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat3.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 3W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat3);
            cntErrTime2 = 0;
            cntErrRTime2 = 0;
            cntErrZykl2 = 0;
        }
        //WRITE 4
        if ((u16Status & PORT4) != 0)
        {
            u16Status &= ~(PORT4);
            clock_gettime(1, &last_stat4);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat4.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat4.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat4.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 4W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat4);
            cntErrTime3 = 0;
            cntErrRTime3 = 0;
            cntErrZykl3 = 0;
        }
        //WRITE 5
        if ((u16Status & PORT5) != 0)
        {
            u16Status &= ~(PORT5);
            clock_gettime(1, &last_stat5);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat5.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat5.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat5.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 5W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");

            }
            clock_gettime(1, &last_stat5);
            cntErrZykl4 = 0;
            cntErrTime4 = 0;
            cntErrRTime4 = 0;
        }
        //WRITE 6
        if ((u16Status & PORT6) != 0)
        {
            u16Status &= ~(PORT6);
            clock_gettime(1, &last_stat6);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat6.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat6.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat6.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 6W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat6);
            cntErrTime5 = 0;
            cntErrRTime5 = 0;
            cntErrZykl5 = 0;
        }
        //WRITE 7
        if ((u16Status & PORT7) != 0)
        {
            u16Status &= ~(PORT7);
            clock_gettime(1, &last_stat7);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat7.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat7.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat7.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 7W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat7);
            cntErrTime6 = 0;
            cntErrRTime6 = 0;
            cntErrZykl6 = 0;
        }
        //WRITE 8
        if ((u16Status & PORT8) != 0)
        {
            u16Status &= ~(PORT8);
            clock_gettime(1, &last_stat8);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat8.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat8.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat8.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 8W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat8);
            cntErrTime7 = 0;
            cntErrRTime7 = 0;
            cntErrZykl7 = 0;
        }
        //WRITE 9
        if ((u16Status & PORT9) != 0)
        {
            u16Status &= ~(PORT9);
            clock_gettime(1, &last_stat9);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat9.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat9.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat9.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 9W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat9);
            cntErrTime8 = 0;
            cntErrRTime8 = 0;
            cntErrZykl8 = 0;
        }
        //WRITE 10
        if ((u16Status & PORT10) != 0)
        {
            u16Status &= ~(PORT10);
            clock_gettime(1, &last_stat10);
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
                clock_gettime(1, &current);

                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat10.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat10.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat10.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 10W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat10);
            cntErrTime9 = 0;
            cntErrRTime9 = 0;
            cntErrZykl9 = 0;
        }
        //WRITE 11
        if ((u16Status & PORT11) != 0)
        {
            u16Status &= ~(PORT11);
            clock_gettime(1, &last_stat11);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat11.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat11.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat11.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 11W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat11);
            cntErrTime10 = 0;
            cntErrRTime10 = 0;
            cntErrZykl10 = 0;
        }
        //WRITE 12
        if ((u16Status & PORT12) != 0)
        {
            u16Status &= ~(PORT12);
            clock_gettime(1, &last_stat12);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat12.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat12.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat12.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 12W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat12);
            cntErrTime11 = 0;
            cntErrRTime11 = 0;
            cntErrZykl11 = 0;
        }
        //WRITE 13
        if ((u16Status & PORT13) != 0)
        {
            u16Status &= ~(PORT13);
            clock_gettime(1, &last_stat13);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat13.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat13.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat13.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 13W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat13);
            cntErrTime12 = 0;
            cntErrRTime12 = 0;
            cntErrZykl12 = 0;
        }
        //W14
        if ((u16Status & PORT14) != 0)
        {
            u16Status &= ~(PORT14);
            clock_gettime(1, &last_stat14);
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
                clock_gettime(1, &current);

                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat14.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat14.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat14.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 14W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat14);
            cntErrTime13 = 0;
            cntErrRTime13 = 0;
            cntErrZykl13 = 0;
        }
        //WRITE 15
        if ((u16Status & PORT15) != 0)
        {
            u16Status &= ~(PORT15);
            clock_gettime(1, &last_stat15);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat15.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat15.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat15.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 15W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat15);
            cntErrTime14 = 0;
            cntErrRTime14 = 0;
            cntErrZykl14 = 0;
        }
        //WRITE 16
        if ((u16Status & PORT16) != 0)
        {
            u16Status &= ~(PORT16);
            clock_gettime(1, &last_stat16);
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
                clock_gettime(1, &current);


                if (current.tv_nsec > last_stat.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
                }

                rate = ((double)tv_nsec / 1000000);


                if (current.tv_nsec > last_stat16.tv_nsec)
                {
                    tv_nsec = current.tv_nsec - last_stat16.tv_nsec;
                }
                else
                {
                    tv_nsec = (999999999 - last_stat16.tv_nsec) + current.tv_nsec;
                }

                tx_rate = ((double)tv_nsec / 1000000);

                if ((tx_rate < tx_rate_min) && (Z > 1000))
                {
                    tx_rate_min = tx_rate;
                }
                if ((tx_rate > 1) && (Z > 1000))
                {
                    tx_rate_max = ((double)(tx_rate + tx_rate_max) / 2);
                }

                if ((tx_rate > 1) && (Z > 1000))
                {
                    cntErrTx++;
                }


                printf("X ; 16W. TX: %d; Last[ms]:; %3f; Current[ms]:; %3f;", ret, rate, tx_rate);
                printf("\n");
                //clock_gettime(1, &last_stat1);

            }
            clock_gettime(1, &last_stat16);
            cntErrTime15 = 0;
            cntErrRTime15 = 0;
            cntErrZykl15 = 0;
        }

        j++;
        t++;
        trk.statusTX = 15;
        //ret = ioctl(fd, TIO_PORT_POLL, default_rx);
        clock_gettime(1, &last_stat1);
        clock_gettime(1, &last_stat2);
        clock_gettime(1, &last_stat3);
        clock_gettime(1, &last_stat4);
        clock_gettime(1, &last_stat5);
        clock_gettime(1, &last_stat6);
        clock_gettime(1, &last_stat7);
        clock_gettime(1, &last_stat8);
        clock_gettime(1, &last_stat9);
        clock_gettime(1, &last_stat10);
        clock_gettime(1, &last_stat11);
        clock_gettime(1, &last_stat12);
        clock_gettime(1, &last_stat13);
        clock_gettime(1, &last_stat14);
        clock_gettime(1, &last_stat15);

        ret = ioctl(fd, TIO_KERNEL_POLL, &trk);
        //printf("RET: %d\n", ret);

        printf("trk.statusTX: %d\n", trk.statusTX);
        printf("trk.statusRX: %d\n", trk.statusRX);


        printf("statusRX_0: %d\n", statusRX[0]);
        printf("statusRX_1: %d\n", statusRX[1]);

        printf("LENx_0: %d\n", lenRX32[0]);

        if (lenRX32[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat1.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat1.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat1.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32[0]; i++)
            {
                if (default_rx[i] == 201)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 1R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime0, cntErrTime0, cntErrZykl0);
            printf(":\n");
        }

        if (lenRX32_1[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat2.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat2.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat2.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_1[0]; i++)
            {
                if (default_rx1[i] == 202)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx1[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx1[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 2R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime1, cntErrTime1, cntErrZykl1);
            printf(":\n");
        }
        if (lenRX32_2[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat3.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat3.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat3.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_2[0]; i++)
            {
                if (default_rx2[i] == 203)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx2[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx2[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 3R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime2, cntErrTime2, cntErrZykl2);
            printf(":\n");
        }
        if (lenRX32_3[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat4.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat4.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat4.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_3[0]; i++)
            {
                if (default_rx3[i] == 204)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx3[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx3[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 4R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime3, cntErrTime3, cntErrZykl3);
            printf(":\n");
        }
        if (lenRX32_4[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat5.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat5.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat5.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_4[0]; i++)
            {
                if (default_rx4[i] == 205)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx4[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx4[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 5R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime4, cntErrTime4, cntErrZykl4);
            printf(":\n");
        }
        if (lenRX32_5[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat6.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat6.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat6.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_5[0]; i++)
            {
                if (default_rx5[i] == 206)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx5[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx5[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 6R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime5, cntErrTime5, cntErrZykl5);
            printf(":\n");
        }
        if (lenRX32_6[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat7.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat7.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat7.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_6[0]; i++)
            {
                if (default_rx6[i] == 207)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx6[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx6[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 7R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime6, cntErrTime6, cntErrZykl6);
            printf(":\n");
        }
        if (lenRX32_7[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat8.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat8.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat8.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_7[0]; i++)
            {
                if (default_rx7[i] == 208)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx7[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx7[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 8R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime7, cntErrTime7, cntErrZykl7);
            printf(":\n");
        }
        if (lenRX32_8[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat9.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat9.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat9.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_8[0]; i++)
            {
                if (default_rx8[i] == 209)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx8[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx8[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 9R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime8, cntErrTime8, cntErrZykl8);
            printf(":\n");
        }
        if (lenRX32_9[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat10.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat10.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat10.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_9[0]; i++)
            {
                if (default_rx9[i] == 210)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx9[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx9[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 10R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime9, cntErrTime9, cntErrZykl9);
            printf(":\n");
        }
        if (lenRX32_10[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat11.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat11.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat11.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_10[0]; i++)
            {
                if (default_rx10[i] == 211)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx10[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx10[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 11R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime10, cntErrTime10, cntErrZykl10);
            printf(":\n");
        }
        if (lenRX32_11[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat12.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat12.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat12.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_11[0]; i++)
            {
                if (default_rx11[i] == 212)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx11[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx11[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 12R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime11, cntErrTime11, cntErrZykl11);
            printf(":\n");
        }
        if (lenRX32_12[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat13.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat13.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat13.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_12[0]; i++)
            {
                if (default_rx12[i] == 213)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx12[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx12[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 13R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime12, cntErrTime12, cntErrZykl12);
            printf(":\n");
        }
        if (lenRX32_13[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat14.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat14.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat14.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_13[0]; i++)
            {
                if (default_rx13[i] == 214)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx13[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx13[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 14R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime13, cntErrTime13, cntErrZykl13);
            printf(":\n");
        }
        if (lenRX32_14[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat15.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat15.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat15.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_14[0]; i++)
            {
                if (default_rx14[i] == 215)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx14[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx14[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 15R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime14, cntErrTime14, cntErrZykl14);
            printf(":\n");
        }
        if (lenRX32_15[0] != 0)
        {
            clock_gettime(1, &current);

            if (current.tv_nsec > last_stat.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat.tv_nsec) + current.tv_nsec;
            }

            rate = ((double)tv_nsec / 1000000);

            if (current.tv_nsec > last_stat16.tv_nsec)
            {
                tv_nsec = current.tv_nsec - last_stat16.tv_nsec;
            }
            else
            {
                tv_nsec = (999999999 - last_stat16.tv_nsec) + current.tv_nsec;
            }

            rx_rate = ((double)tv_nsec / 1000000);

            if ((rx_rate < rx_rate_min) && (Z > 1000))
            {
                rx_rate_min = rx_rate;
            }
            if ((rx_rate > 1) && (Z > 1000))
            {
                rx_rate_max = ((double)(rx_rate + rx_rate_max) / 2);
            }

            if ((rx_rate > 1) && (Z > 1000))
            {
                cntErrRx++;
            }

            printf(":\n");
            for (i = 0; i < lenRX32_15[0]; i++)
            {
                if (default_rx15[i] == 216)
                {
                    //countReadTeleg++;
                    printf(ANSI_COLOR_RED);
                }
                else if (default_rx15[i] == 169)
                {
                    printf(ANSI_COLOR_GREEN);
                }
                printf(" %d", default_rx15[i]);
                printf(ANSI_COLOR_RESET);
            }

            printf(" ; 16R. RX: %d; Last[ms]:; %3f; Current[ms]:; %3f; TX NOT Empty:; %d; RX Empty:; %d; RX ErrZykl:; %d;", ret, rate, rx_rate, cntErrRTime15, cntErrTime15, cntErrZykl15);
            printf(":\n");
        }

        printf("LENx_1: %d\n", lenRX32_1[0]);
        printf("LENx_2: %d\n", lenRX32_2[0]);
        printf("LENx_3: %d\n", lenRX32_3[0]);
        printf("LENx_4: %d\n", lenRX32_4[0]);
        printf("LENx_5: %d\n", lenRX32_5[0]);
        printf("LENx_6: %d\n", lenRX32_6[0]);
        printf("LENx_7: %d\n", lenRX32_7[0]);
        printf("LENx_8: %d\n", lenRX32_8[0]);
        printf("LENx_9: %d\n", lenRX32_9[0]);
        printf("LENx_10: %d\n", lenRX32_10[0]);
        printf("LENx_11: %d\n", lenRX32_11[0]);
        printf("LENx_12: %d\n", lenRX32_12[0]);
        printf("LENx_13: %d\n", lenRX32_13[0]);
        printf("LENx_14: %d\n", lenRX32_14[0]);
        printf("LENx_15: %d\n", lenRX32_15[0]);


        //statusRX = (uint16_t)((default_rx9[0] << 8) | (default_rx9[1]));
        //printf("statusRX_4: %d\n", statusRX);

        ret = ioctl(fd, TIO_PORTS_STATUS, &statusPorts);

        for (i = 0; i < 128; i++)
        {
            printf("statusPorts: %d\n", statusPorts[i]);
        }

        ttyStatusPorts = (struct spi_tty_status*)statusPorts;

        printf("ttyStatusPorts: %d\n", ttyStatusPorts->u16RxFIFOLev);

        printf("ttyStatusPorts3: %d\n", ttyStatusPorts[3].u16RxFIFOLev);

        break;

        countPollZyc++;
        if (ret <= 0)
        {
            printf("ERROR: %d\n", ret);
            break;
        }
        else if (ret == 3003)
        {
            countPollErr++;
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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

        break;

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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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
            countPollProz = (countPollErr * 100) / countPollZyc;
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

            clock_gettime(1, &current);
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



            rate = ((double)tv_nsec / 1000000);
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


        usleep(1000);
    }

end:

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

    free(ptGenBuffer);


    return 0;

}
