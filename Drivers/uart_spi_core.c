//UART-SPI ------------------------------------------------------------------------------------->
#include <linux/spi/spidev.h>
#include <uapi/linux/gpio.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/list.h>

static const char *device = "/dev/spidev3.0";

struct file* test;

int32_t iSpeed = 10000000;

#define TRUE              1
#define FALSE             0
#define SPI_UART_HAEDER   4
#define BYTE              1
#define TWO_BYTE          2
#define SPI_ADDR          512

#define RX_FIFO_THRESHOLD_IRQ         0x20

#define PORT1		0x0001
#define PORT2		0x0002
#define PORT3		0x0004
#define PORT4		0x0008
#define PORT5		0x0010
#define PORT6		0x0020
#define PORT7		0x0040
#define PORT8		0x0080
#define PORT9		0x0100
#define PORT10		0x0200
#define PORT11		0x0400
#define PORT12		0x0800
#define PORT13		0x1000
#define PORT14		0x2000
#define PORT15		0x4000
#define PORT16		0x8000

struct spi_tty_data {
	/* producer-published */
	size_t read_head;
	size_t commit_head;
	size_t canon_head;
	size_t echo_head;
	size_t echo_commit;
	size_t echo_mark;
	DECLARE_BITMAP(char_map, 256);

	/* private to n_tty_receive_overrun (single-threaded) */
	unsigned long overrun_time;
	int num_overrun;

	/* non-atomic */
	bool no_room;

	/* must hold exclusive termios_rwsem to reset these */
	unsigned char lnext:1, erasing:1, raw:1, real_raw:1, icanon:1;
	unsigned char push:1;

	/* shared by producer and consumer */
	char read_buf[N_TTY_BUF_SIZE];
	DECLARE_BITMAP(read_flags, N_TTY_BUF_SIZE);
	unsigned char echo_buf[N_TTY_BUF_SIZE];

	/* consumer-published */
	size_t read_tail;
	size_t line_start;

	/* protected by output lock */
	unsigned int column;
	unsigned int canon_column;
	size_t echo_tail;

	struct mutex atomic_read_lock;
	struct mutex output_lock;
};

struct spi_tty_status {
	u16 u16irqStatus;
	u16 u16TxFIFOLev;
	u16 u16RxFIFOLev;
	u8 u8res1;
	u8 u8res2;
};

extern int spidev_open(struct inode *inode, struct file *filp);
static int spi_uart_readwrite(struct tty_struct *tty, const unsigned char *bufIn, unsigned char *bufOut, uint32_t addr, int count, bool write);
static int spi_uart_read_status(struct tty_struct *tty, const unsigned char *bufIn, unsigned char *bufOut, uint32_t addr, int count, bool write, unsigned long speed);
static int spi_uart_read_rx(struct tty_struct *tty, unsigned char *bufOut, uint32_t addr, int count, unsigned long speed);
static int spi_uart_read_allstatus(struct tty_struct *tty, unsigned char *bufOut, uint32_t addr, int count, unsigned long speed);
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
struct spi_work_struct {
    struct work_struct work;    // "base class" members, do not touch
    int spi_w_counter;         // spi data: number 
	int spi_r_counter;         // my data: number of irq
	char idxIn;
	char idxOut;
    unsigned long spi_jiffies_in;   // spi data: time 
	unsigned long spi_jiffies_out;   // spi data: time
	struct tty_port *port;
	struct tty_struct *tty;
	const unsigned char *buf; 
	int count;
};

// the workqueue
static struct workqueue_struct *spi_workqueue;

// called by the kernel to dequeue my work item
static void spi_workqueue_function( struct work_struct *work )
{		
	struct spi_work_struct *spi_work = (struct spi_work_struct *)work; // downcasting to struct my_work_struct
	int ret = 0;
	int retMutex = 0;
	uint32_t u32AdrSwap;
	int i;
	ssize_t err;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX;
	unsigned char *bufRX;
	struct tty_struct *tty = spi_work->tty;
	struct tty_driver *driver;

	driver = tty->driver;

	bufTX = kmalloc((spi_work->count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX) {
			goto end;
	}
	bufRX = kmalloc((spi_work->count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX) {
			kfree(bufTX);
			goto end;
	}
	
	for (i = 0; i < (spi_work->count + SPI_UART_HAEDER); i++)
  	{
    	*(bufRX + i) = 0x0;
    	*(bufTX + i) = 0x0;
  	}
  	memset(&tTransfer, 0, sizeof(tTransfer));
    
	u32AdrSwap = (SPI_ADDR_TX_BUF + (SPI_ADDR_TX_BUF_OFFSET * spi_work->tty->index));
   
	u32AdrSwap = u32AdrSwap << 1;

	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX, &u32AdrSwap, 3);
	
	memcpy((bufTX + 4), spi_work->buf, spi_work->count);

	tTransfer.tx_buf = (unsigned long)bufTX;
	tTransfer.rx_buf = (unsigned long)bufRX;
	tTransfer.len = (spi_work->count + SPI_UART_HAEDER);
	if (spi_work->tty->port->spiSpeed == 0)
	{
		tTransfer.speed_hz = iSpeed;
	}
	else
	{
		tTransfer.speed_hz = spi_work->tty->port->spiSpeed;
	}
	tTransfer.bits_per_word = 8;

	//TX Lock while RX active
	while(READ_ONCE(driver->irqEvent))
	{
		spi_work->port->err_cnt_queue++;
	}

	retMutex = 0;
	do{
		retMutex = mutex_trylock(&driver->spi_read_lock);
		if (retMutex) 
		{
			WRITE_ONCE(driver->TxSpi, 1);
			if ( (spi_work->count + SPI_UART_HAEDER + (READ_ONCE(spi_work->port->tx_fifolvl_last)) + (READ_ONCE(spi_work->port->tx_fifolvl_neu))) >= 2048 )
			{
				//printk("serial_core.c OVERFLOWNEU to SPI !!!: Port Nr: %d, spi_work->count: %zu spi_work->port->tx_fifolvl_last: %zu, spi_work->port->tx_fifolvl_neu: %zu, spi_work->port->rx_fifolvl: %zu\n", spi_work->tty->index, spi_work->count, spi_work->port->tx_fifolvl_last, spi_work->port->tx_fifolvl_neu, spi_work->port->rx_fifolvl);
			}
			else if ( (spi_work->count + SPI_UART_HAEDER + (READ_ONCE(spi_work->port->tx_fifolvl_last)) ) >= 2048 )
			{
				//printk("serial_core.c OVERFLOWOK to SPI !!!: Port Nr: %d, spi_work->count: %zu spi_work->port->tx_fifolvl_last: %zu, spi_work->port->tx_fifolvl_neu: %zu, spi_work->port->rx_fifolvl: %zu\n", spi_work->tty->index, spi_work->count, spi_work->port->tx_fifolvl_last, spi_work->port->tx_fifolvl_neu, spi_work->port->rx_fifolvl);
			}
			else
			{
				//printk("serial_core.c OVERFLOWNOT to SPI: Port Nr: %d, spi_work->count: %zu spi_work->port->tx_fifolvl_last: %zu, spi_work->port->tx_fifolvl_neu: %zu, spi_work->port->rx_fifolvl: %zu\n", spi_work->tty->index, spi_work->count, spi_work->port->tx_fifolvl_last, spi_work->port->tx_fifolvl_neu, spi_work->port->rx_fifolvl);
			}

			spi_work->port->tx_fifolvl_neu = spi_work->count;

			if (IS_ERR(driver->filp)) {
					ret = -ENOENT;
					goto endKfree;
				} 
			else
			{
				if ( (driver->filp == NULL) || (driver->filp->f_op == NULL))
				{
					ret = -ENOENT;
					goto endKfree;
				}
				else
				{
					err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
				}
				if (err < 0)
				{
					ret = -EINVAL;
				}
				else
				{
					ret = spi_work->count;
				}
			}

			mutex_unlock(&driver->spi_read_lock);
			WRITE_ONCE(driver->TxSpi, 0);
		}
		else
		{
			spi_work->port->err_cnt_queue++;
		}
	}
	while(!retMutex);

	if (retMutex < 0)
	{
		//my_work->port->irq_done = 0;	
	}
	else
	{
		spi_work->port->idxOut = (spi_work->port->idxOut + 1) & 0x0F;
		spi_work->idxOut = spi_work->port->idxOut;
		spi_work->spi_jiffies_out = jiffies;
		spi_work->port->spi_jiffies_out = jiffies;
		if ( (waitqueue_active(&spi_work->tty->write_wait)) && (spi_work->port->PortTypW) )
		{
			spi_work->port->tx_trigger = 1;
			wake_up_interruptible(&spi_work->port->tty->write_wait);
		}
	}

	if ( waitqueue_active(&driver->spi_waitTX) )
	{
		wake_up_interruptible(&driver->spi_waitTX);
	}

endKfree:
  
  	kfree(bufTX);
	kfree(bufRX);
	
end:
	
	if (work == NULL)
	{
		//
	}
	else
	{
		kfree(work);
	}

    return;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
void spi_uart_write_wakeup(struct uart_port *port)
{
	struct uart_state *state = port->state;
	
	/*
	 * This means you called this function _after_ the port was
	 * closed.  No cookie for you.
	 */
	BUG_ON(!state);
	tty_port_tty_wakeup(&state->port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...


//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_stop(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;
	
	port = uart_port_lock(state, flags);
	if (port)
		port->ops->stop_tx(port);
	uart_port_unlock(port, flags);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
int spi_int_message(struct tty_port *port)
{
	int ret, i, j, z, retMutex;
	int val = 99;
	unsigned int queueEmpty = 0;
	int p = 0;
	u8 *bufSpiRX;
	u8 *bufSpiStatus;
	u16 u16irqStatus = 0;
	long timeoutRX = 5000;
	unsigned long ulFlagsTemp = 0;
	unsigned char *portTrigger[16];
	
	struct tty_struct *tty = port->tty;
	struct tty_driver *driver;
	struct tty_port *portIn;
	struct tty_struct *ttyIn;
	struct spi_tty_status *ttyStatus;
	struct spi_tty_status *ttyStatusTmp;
	struct spi_tty_data *ldata;

	DEFINE_WAIT_FUNC(waitRX, woken_wake_function);

	struct gpio_desc *gpiod;
	ret = 0;

	tty = READ_ONCE(port->itty);
	if (!tty)
	{
    	printk(KERN_ALERT "serial_core.c: spi_int_message: port->itty ERROR\n");
  	}
  	tty = READ_ONCE(port->tty);
  	if (!tty)
	{
    	printk(KERN_ALERT "serial_core.c: spi_int_message: port->tty ERROR\n");
		return 0;
  	}

	driver = tty->driver;
	//Device Tree PORT (Master) muss laufen, damit die anderen verlinkt werden konnten!

	if (!gpio_is_valid(25)) {
		//
	}

	gpiod = gpio_to_desc(25);
	if (!gpiod)
	{
		//
	}
	if(gpiod_get_value_cansleep(gpiod) == 0)
	{
		ret = 3;
		goto end;
	}	

	//WR TxSPI START
	WRITE_ONCE(driver->irqEvent, 1);
	
	down_read(&tty->termios_rwsemSpi);
	add_wait_queue(&driver->spi_waitTX, &waitRX);

	//TX Lock while RX active
	while(1)
	{
		if ( (!timeoutRX) )
		{
			break;
		}
		if (!(READ_ONCE(driver->TxSpi)))
		{
			break;
		}
		up_read(&tty->termios_rwsemSpi);
	
		timeoutRX = wait_woken(&waitRX, TASK_INTERRUPTIBLE, (msecs_to_jiffies(5000)));
		down_read(&tty->termios_rwsemSpi);
	}

	remove_wait_queue(&driver->spi_waitTX, &waitRX);
	up_read(&tty->termios_rwsemSpi);

	ldata = tty->disc_data;
	do
	{
		if (READ_ONCE(driver->TxSpi))
		{
			if (!ret)
			{
				port->err_cnt_spiread++;
			}
			ret = 3;
		}
		else
		{
			retMutex = mutex_trylock(&driver->spi_read_lock);
			if (retMutex == 0)
			{
				if (!ret)
				{
					port->err_cnt_spiread++;
				}
				ret = 3;
			}
			else
			{
				do
				{
					WRITE_ONCE(driver->irqEvent, 1);

					p = 0;
					bufSpiStatus = kmalloc(128, GFP_KERNEL);
					if (!bufSpiStatus) {
						//
					}
					ttyStatus = kcalloc(16, sizeof(*ttyStatus), GFP_KERNEL);
					ttyStatusTmp = ttyStatus;
					memset(bufSpiStatus, 0, 128);
					ret = spi_uart_read_allstatus(tty, bufSpiStatus, SPI_ADDR_IRQ_STATUS, 128, port->spiSpeed);
					if (ret < 0)
					{
						//
					}
					else
					{
						for (i = 0; i < 16; i++, p++, ttyStatus++) {
							ttyStatus->u16irqStatus = (uint16_t)((bufSpiStatus[0 + i * 8] << 8) | (bufSpiStatus[1 + i * 8]));
							ttyStatus->u16TxFIFOLev = (uint16_t)((bufSpiStatus[2 + i * 8] << 8) | (bufSpiStatus[3 + i * 8]));
							ttyStatus->u16RxFIFOLev = (uint16_t)((bufSpiStatus[4 + i * 8] << 8) | (bufSpiStatus[5 + i * 8]));
							ttyStatus->u8res1 = bufSpiStatus[6 + i * 8];
							ttyStatus->u8res2 = bufSpiStatus[7 + i * 8];

							if (driver->ports[p] == 0)
							{ 
								//
							}
							else
							{
								if (driver->ports[p]->tty == 0)
								{
									//
								}
								else
								{
									WRITE_ONCE(driver->ports[p]->tx_fifolvl_last, (int)ttyStatus->u16TxFIFOLev);
									WRITE_ONCE(driver->ports[p]->tx_fifolvl_neu, 0);
								}
							}
						}
					}

					port->idxOutIrq = port->idxInIrq;
					
					ttyStatus = ttyStatusTmp;
					for (i = 0; i < driver->num; i++, ttyStatus++)
					{
						if (driver->ports[i] == 0)
						{
							//
						}
						else
						{
							portIn = driver->ports[i];
							if (driver->ports[i]->tty == 0)
							{
								//
							}
							else
							{
								portTrigger[i] = &driver->ports[i]->tx_trigger;
								ttyIn = driver->ports[i]->tty;
									
								portIn->threard_count++;
								
								u16irqStatus = ttyStatus->u16irqStatus;

								ulFlagsTemp = (unsigned long)u16irqStatus;
								
								if ( (((ulFlagsTemp & SPI_TX_FIFO_EMPTY_IRQ) != 0) && ((portIn->spiflags & SPI_TX_FIFO_EMPTY_IRQ) != 0))
									|| (((ulFlagsTemp & SPI_TX_FIFO_THR_IRQ) != 0) && ((portIn->spiflags & SPI_TX_FIFO_THR_IRQ) != 0)) ) 
								{
									//ret = -ECOMM;
									//goto end_kfree;
								}
								
								if ( ((ulFlagsTemp & SPI_TX_FIFO_OVFLW_IRQ) != 0) || ((ulFlagsTemp & SPI_RX_FIFO_OVFLW_IRQ) != 0)
									|| ((ulFlagsTemp & SPI_COLL_DET_IRQ) != 0) || ((ulFlagsTemp & SPI_PRTY_ERR_IRQ) != 0) ) 
								{
									portIn->spi_message = 1;
								}
								
								//IRQs
								if ((ulFlagsTemp & SPI_BUS_IDLE_IRQ) != 0)
								{
									portIn->spiflags |= SPI_BUS_IDLE_IRQ;
								} 
								else
								{
									//portIn->spiflags &= ~(SPI_BUS_IDLE_IRQ);
								}
								if ((ulFlagsTemp & SPI_TX_FIFO_EMPTY_IRQ) != 0)
								{
									portIn->spiflags |= SPI_TX_FIFO_EMPTY_IRQ;
								} 
								if ((ulFlagsTemp & SPI_TX_FIFO_THR_IRQ) != 0)
								{
									portIn->spiflags |= SPI_TX_FIFO_THR_IRQ;
								}
								if ((ulFlagsTemp & SPI_TX_FIFO_OVFLW_IRQ) != 0)
								{
									portIn->spiflags |= SPI_TX_FIFO_OVFLW_IRQ;
								}
								if ((ulFlagsTemp & SPI_RX_FIFO_OVFLW_IRQ) != 0)
								{
									portIn->spiflags |= SPI_RX_FIFO_OVFLW_IRQ;
									portIn->db7_count = 10000;
								}
								if ((ulFlagsTemp & SPI_RX_FIFO_THR_IRQ) != 0)
								{
									portIn->spiflags |= SPI_RX_FIFO_THR_IRQ;
								}
								else
								{
									//portIn->spiflags &= ~(SPI_RX_FIFO_THR_IRQ);
								}
								if ((ulFlagsTemp & SPI_COLL_DET_IRQ) != 0)
								{
									portIn->spiflags |= SPI_COLL_DET_IRQ;
								}
								if ((ulFlagsTemp & SPI_PRTY_ERR_IRQ) != 0)
								{
									portIn->spiflags |= SPI_PRTY_ERR_IRQ;
								}
								
								WRITE_ONCE(portIn->tx_fifolvl_last, (int)ttyStatus->u16TxFIFOLev);
								WRITE_ONCE(portIn->tx_fifolvl_neu, 0);
								WRITE_ONCE(portIn->rx_fifolvl, (int)ttyStatus->u16RxFIFOLev);

								if ( (driver->DrTyp != 2) && (ttyStatus->u16RxFIFOLev != 0) 
									&& ( 
										((ttyStatus->u16irqStatus & SPI_RX_FIFO_THR_IRQ) != 0) || ((ttyStatus->u16irqStatus & SPI_BUS_IDLE_IRQ) != 0) ) 
									)
								{ 
									bufSpiRX = kmalloc(ttyStatus->u16RxFIFOLev, GFP_KERNEL);
									if (!bufSpiRX) {
										//
									}
									else
									{
										memset(bufSpiRX, 0, ttyStatus->u16RxFIFOLev);
										ret = spi_uart_read_rx(ttyIn, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * ttyIn->index)), ttyStatus->u16RxFIFOLev, portIn->spiSpeed);
										if (ret < 0)
										{
											kfree(bufSpiRX);
											//goto end_kfree;
										}
										for (j = 0; j < ttyStatus->u16RxFIFOLev; j++)
										{
											ret = tty_insert_flip_char(portIn, *(bufSpiRX + j), 0);
											if ((ret < 0) || (!ret))
											{
												portIn->err_cnt_read++;
											}
										}
										kfree(bufSpiRX);
									}
									tty_flip_buffer_push(portIn);
								}
								portIn->irq_jiffies_out[0] = jiffies;
							}
						}
					} 

					kfree(ttyStatusTmp);
					kfree(bufSpiStatus);

					port->irq_done = 0;

					WRITE_ONCE(driver->irqEvent, 0);
					mutex_unlock(&driver->spi_read_lock);

					if (driver->DrTyp == 2)
					{
						goto waitStatus;
					}

					flush_workqueue(spi_workqueue);
					queueEmpty = 0;
					for (z = 0; z < driver->num; z++)
					{
						if (driver->ports[z] == 0)
						{
							//
						}
						else
						{
							portIn = driver->ports[z];
							if (READ_ONCE(portIn->idxIn) != READ_ONCE(portIn->idxOut))
							{
								//ret = 5;
								//goto end;
								queueEmpty |= (1 << z); 
							}
							else
							{
								//
							}
						}
					}

					if (queueEmpty != 0)
					{
						flush_workqueue(spi_workqueue);
					}			
					
					WRITE_ONCE(driver->irqEvent, 1);

					val = gpiod_get_value_cansleep(gpiod);
				}
				while(gpiod_get_value_cansleep(gpiod) == 1);
			}
		}
	}while(!retMutex);

	flush_workqueue(system_unbound_wq);

waitStatus:
	for (i = 0; i < driver->num; i++)
	{
		if (driver->ports[i] == 0)
		{
			//
		}
		else
		{
			portIn = driver->ports[i];
			ttyIn = driver->ports[i]->tty;
			if (driver->ports[i]->tty == 0)
			{
				//
			}
			else
			{	
				if (driver->DrTyp != 2)
				{
					if ( ( waitqueue_active(&ttyIn->write_wait) 
							&& ( ((portIn->spiflags & SPI_TX_FIFO_EMPTY_IRQ) != 0) || ((portIn->spiflags & SPI_TX_FIFO_THR_IRQ) != 0)  ))  
						|| ( portIn->spi_message) ) 
					{
						portIn->spi_message = 0;
						portIn->db1_count++;
						portIn->tx_trigger = 1;
						wake_up_interruptible(&ttyIn->write_wait);
					}
					else
					{
						//
					}
					port->irqq_count = 0;
				}
				else
				{
					if ( ( waitqueue_active(&ttyIn->read_wait)) && ((portIn->spiflags & SPI_BUS_IDLE_IRQ) != 0) )  
					{
						//
						wake_up_interruptible(&ttyIn->read_wait);
					}
					else
					{
						//
					}
				}
			}
		}
	}

end:
	WRITE_ONCE(driver->irqEvent, 0);

	return ret;
}
EXPORT_SYMBOL_GPL(spi_int_message);
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
int spi_poll_all(struct tty_port *port)
{
	int ret = 0;
	int txEmpty = 0;
	int queNotEmpty = 0;
	int i, j, retMutex;
	u8 *bufSpiRX;
	u8 *bufSpiStatus;
	u16 u16irqStatus = 0;
	unsigned char *portTrigger[16];
	
	struct tty_struct *tty = port->tty;
	struct tty_driver *driver;
	struct tty_port *portIn;
	struct tty_struct *ttyIn;
	struct spi_tty_status *ttyStatus;
	struct spi_tty_status *ttyStatusTmp;
	struct spi_tty_data *ldata;
	long timeoutRX = 5000;

	DEFINE_WAIT_FUNC(waitRX, woken_wake_function);

	tty = READ_ONCE(port->itty);
	if (!tty)
	{
    	printk(KERN_ALERT "serial_core.c: spi_int_message: port->itty ERROR\n");
  	}
  	tty = READ_ONCE(port->tty);
  	if (!tty)
	{
    	printk(KERN_ALERT "serial_core.c: spi_int_message: port->tty ERROR\n");
		return 0;
  	}

	driver = tty->driver;

	down_read(&tty->termios_rwsemSpi);
	add_wait_queue(&driver->spi_waitTX, &waitRX);

	//TX Lock while RX active
	while(1)
	{
		queNotEmpty = 0;
		for (i = 0; i < driver->num; i++)
		{
			if (driver->ports[i] == 0)
			{
				//
			}
			else
			{
				portIn = driver->ports[i];
				if (portIn->idxIn != portIn->idxOut)
				{
					//ret = 5;
					//goto end;
					queNotEmpty = 1;
					break;
				}
				else
				{
					//
				}
			}
		}
		if ( (!timeoutRX) )
		{
			break;
		}
		if (!(READ_ONCE(driver->TxSpi)) && (!queNotEmpty))
		{
			break;
		}
		if (queNotEmpty)
		{
			flush_workqueue(spi_workqueue);
		}

		up_read(&tty->termios_rwsemSpi);
	
		timeoutRX = wait_woken(&waitRX, TASK_INTERRUPTIBLE, (msecs_to_jiffies(5000)));
		down_read(&tty->termios_rwsemSpi);
	}

	remove_wait_queue(&driver->spi_waitTX, &waitRX);
	up_read(&tty->termios_rwsemSpi);

	ldata = tty->disc_data;

	do
	{
		retMutex = mutex_trylock(&driver->spi_read_lock);
		if (retMutex == 0)
		{
			port->err_cnt_spiread++;
			ret = 3;
			//goto end;
		}
		else
		{
			driver = tty->driver;
	
			bufSpiStatus = kmalloc(128, GFP_KERNEL);
			if (!bufSpiStatus) {
				//
			}
			ttyStatus = kcalloc(16, sizeof(*ttyStatus), GFP_KERNEL);
			ttyStatusTmp = ttyStatus;
			
			do
			{
				memset(bufSpiStatus, 0, 128);
				ret = spi_uart_read_allstatus(tty, bufSpiStatus, SPI_ADDR_IRQ_STATUS, 128, port->spiSpeed);
				if (ret < 0)
				{
					//
				}
				else
				{
					txEmpty = 0;
					ttyStatus = ttyStatusTmp;
					for (i = 0; i < 16; i++, ttyStatus++) {
						ttyStatus->u16irqStatus = (uint16_t)((bufSpiStatus[0 + i * 8] << 8) | (bufSpiStatus[1 + i * 8]));
						ttyStatus->u16TxFIFOLev = (uint16_t)((bufSpiStatus[2 + i * 8] << 8) | (bufSpiStatus[3 + i * 8]));
						if (ttyStatus->u16TxFIFOLev != 0)
						{
							txEmpty = 1;
							break;
						}
						ttyStatus->u16RxFIFOLev = (uint16_t)((bufSpiStatus[4 + i * 8] << 8) | (bufSpiStatus[5 + i * 8]));
						ttyStatus->u8res1 = bufSpiStatus[6 + i * 8];
						ttyStatus->u8res2 = bufSpiStatus[7 + i * 8];
					}
				}
			}
			while(txEmpty);

			ttyStatus = ttyStatusTmp;
		
			for (i = 0; i < driver->num; i++, ttyStatus++)
			{
				if (driver->ports[i] == 0)
				{
					//
				}
				else
				{
					portIn = driver->ports[i];
					if (driver->ports[i]->tty == 0)
					{
						//
					}
					else
					{
						portTrigger[i] = &driver->ports[i]->tx_trigger;
						ttyIn = driver->ports[i]->tty;
							
						u16irqStatus = ttyStatus->u16irqStatus;
						
						portIn->tx_fifolvl_last = (int)ttyStatus->u16TxFIFOLev;
						
						portIn->rx_fifolvl = (int)ttyStatus->u16RxFIFOLev;
						if ( (ttyStatus->u16RxFIFOLev != 0) )
						{ 
							bufSpiRX = kmalloc(ttyStatus->u16RxFIFOLev, GFP_KERNEL);
							if (!bufSpiRX) {
								//
							}
							else
							{
								memset(bufSpiRX, 0, ttyStatus->u16RxFIFOLev);
								ret = spi_uart_read_rx(ttyIn, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * ttyIn->index)), ttyStatus->u16RxFIFOLev, portIn->spiSpeed);
								if (ret < 0)
								{
									kfree(bufSpiRX);
								}
								for (j = 0; j < ttyStatus->u16RxFIFOLev; j++)
								{
									ret = tty_insert_flip_char(portIn, *(bufSpiRX + j), 0);
								}
								if (ret < 0)
								{
									//
								}
								kfree(bufSpiRX);
							}
							tty_flip_buffer_push(portIn);
						}
					}
				}
			} 

			kfree(ttyStatusTmp);
			kfree(bufSpiStatus);
			mutex_unlock(&driver->spi_read_lock);
		}
	} 
	while(!retMutex);
  	

	for (i = 0; i < driver->num; i++)
	{
		if (driver->ports[i] == 0)
		{
			//
		}
		else
		{
			//
			portIn = driver->ports[i];
			ttyIn = driver->ports[i]->tty;
			if (driver->ports[i]->tty == 0)
			{
				//
			}
			else
			{
				if ( (waitqueue_active(&ttyIn->write_wait)) || ( portIn->spi_message) ) 
				{
					portIn->spi_message = 0;
					portIn->db1_count++;
					portIn->tx_trigger = 1;
					wake_up_interruptible(&ttyIn->write_wait);
				}
				port->irqq_count = 0;
			}
		}
	}

	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
int spi_poll_message(struct tty_struct *tty, unsigned long arg, struct tty_port *port, int readonly)
{
	struct tty_driver *driver; 
	int ret, i, retMutex, sizeFlip;
	struct spi_tty_status *ttyStatus;
	long timeoutRX = 5000;
	int txEmpty = 0;
	struct spi_ioc_transfer *ptTransfer;
	struct spi_ioc_transfer __user *u_ioc = (struct spi_ioc_transfer __user *)arg;

	DEFINE_WAIT_FUNC(waitRX, woken_wake_function);

	u8 *bufSpiStatus;
	u8 *bufSpiRX;
	u16 u16irqStatus = 0;
	ret = 0;
	sizeFlip = 0;
	
	ptTransfer = memdup_user(u_ioc, sizeof(struct spi_ioc_transfer));
	if (IS_ERR(ptTransfer))
		return -1;

	driver = tty->driver;

	down_read(&tty->termios_rwsemSpi);
	add_wait_queue(&driver->spi_waitTX, &waitRX);

	//TX Lock while RX active
	while(1)
	{
		if ( (!timeoutRX) )
		{
			break;
		}
		if ( (!(READ_ONCE(driver->TxSpi))) && (port->idxIn == port->idxOut) )
		{
			break;
		}
		if (port->idxIn != port->idxOut)
		{
			flush_workqueue(spi_workqueue);
		}
		
		up_read(&tty->termios_rwsemSpi);

		timeoutRX = wait_woken(&waitRX, TASK_INTERRUPTIBLE, (msecs_to_jiffies(5000)));
		down_read(&tty->termios_rwsemSpi);
	}

	remove_wait_queue(&driver->spi_waitTX, &waitRX);
	up_read(&tty->termios_rwsemSpi);

	if (port->idxIn != port->idxOut)
	{
		ret = 3005;
		goto end;
	}
	
	retMutex = 0;
	do{
		retMutex = mutex_trylock(&driver->spi_read_lock);
		if (retMutex == 0)
		{
			ret = 3003;
			//goto end;
		}
		else
		{
			bufSpiStatus = kmalloc(8, GFP_KERNEL);
			if (!bufSpiStatus) {
				//
			}
			ttyStatus = kcalloc(1, sizeof(*ttyStatus), GFP_KERNEL);
			do
			{
				memset(bufSpiStatus, 0, 8);
				ret = spi_uart_read_allstatus(tty, bufSpiStatus, (SPI_ADDR_IRQ_STATUS + (SPI_ADDR_STATUS_OFFSET * tty->index)), 8, port->spiSpeed);
				if (ret < 0)
				{
					//
				}
				else
				{
					txEmpty = 0;
					ttyStatus->u16irqStatus = (uint16_t)((bufSpiStatus[0] << 8) | (bufSpiStatus[1]));
					ttyStatus->u16TxFIFOLev = (uint16_t)((bufSpiStatus[2] << 8) | (bufSpiStatus[3]));
					if (ttyStatus->u16TxFIFOLev != 0)
					{
						txEmpty = 1;
						break;
					}
					ttyStatus->u16RxFIFOLev = (uint16_t)((bufSpiStatus[4] << 8) | (bufSpiStatus[5]));
					ttyStatus->u8res1 = bufSpiStatus[6];
					ttyStatus->u8res2 = bufSpiStatus[7];
				}
			}
			while(txEmpty);
			
			u16irqStatus = ttyStatus->u16irqStatus;
			port->spiflags = (unsigned long)u16irqStatus;
			port->tx_fifolvl_last = (int)ttyStatus->u16TxFIFOLev;
			port->rx_fifolvl = (int)ttyStatus->u16RxFIFOLev;

			if ( (ttyStatus->u16RxFIFOLev != 0) )
			{ 
				bufSpiRX = kmalloc(ttyStatus->u16RxFIFOLev, GFP_KERNEL);
				if (!bufSpiRX) {
					ret = 3002;
				}
				else
				{
					memset(bufSpiRX, 0, ttyStatus->u16RxFIFOLev);
					ret = spi_uart_read_rx(tty, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * tty->index)), ttyStatus->u16RxFIFOLev, port->spiSpeed);
					if (ret < 0)
					{
						kfree(bufSpiRX);
						goto end_kfree;
					}
					sizeFlip = 0;

					ptTransfer->len = ttyStatus->u16RxFIFOLev;
					ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_buf, bufSpiRX, ptTransfer->len);

					for (i = 0; i < ttyStatus->u16RxFIFOLev; i++)
					{
						ret = tty_insert_flip_char(port, *(bufSpiRX + i), 0);
						sizeFlip += ret;
					}
					kfree(bufSpiRX);
					ret = (int)ttyStatus->u16RxFIFOLev;
				}
				tty_flip_buffer_push(port);
			}
			else
			{
				ret = 3004;
			}
			kfree(ttyStatus);
			kfree(bufSpiStatus);
			mutex_unlock(&driver->spi_read_lock);
		}
	} 
	while(!retMutex);

	if (waitqueue_active(&tty->write_wait)) 
	{
		port->tx_trigger = 1;
		wake_up_interruptible(&tty->write_wait);
	}
	
end_kfree:

end:
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
int spi_kern_poll(struct tty_struct *tty, unsigned long arg, struct tty_port *port, int readonly)
{

	struct tty_driver *driver; 
	int err, ret, i, j, retMutex, sizeFlip, idx;
	struct spi_tty_status *ttyStatus;
	long timeoutRX = 5000;
	int txEmpty = 0;
	struct spi_ioc_transfer tTransfer;
	struct spi_ioc_ktransfer *ptTransfer;
	struct spi_ioc_ktransfer __user *u_ioc = (struct spi_ioc_ktransfer __user *)arg;
	unsigned char *bufTX;
	unsigned char *bufRX;
	int queNotEmpty = 0;
	u8 *bufSpiRX;
	u8 *bufSpiStatus;
	uint32_t u32AdrSwap;
	u16 u16irqStatus = 0;
	u16 statusTX = 0;
	u16 statusRX = 0;
	u16 statusRead = 0;
	//u16 *ptStatusTX;
	u16 *ptStatusRX;
	u16 *ptLen0;

	unsigned char *portTrigger[16];
	struct tty_port *portIn;
	struct tty_struct *ttyIn;
	struct spi_tty_status *ttyStatusTmp;

	DEFINE_WAIT_FUNC(waitRX, woken_wake_function);

	ret = 0;
	sizeFlip = 0;
	tty = READ_ONCE(port->itty);
	if (!tty)
	{
    	//
  	}
  	tty = READ_ONCE(port->tty);
  	if (!tty)
	{
		return 0;
  	}

	ptTransfer = memdup_user(u_ioc, sizeof(struct spi_ioc_ktransfer));
	if (IS_ERR(ptTransfer))
		return -1;

	bufTX = kmalloc(2048, GFP_KERNEL);
	if (!bufTX) {
			//
	}
	bufRX = kmalloc(2048, GFP_KERNEL);
	if (!bufRX) {
			//
	}
	
	driver = tty->driver;

	if (ptTransfer->statusTX != 0)
	{
		do{
			retMutex = mutex_trylock(&driver->spi_read_lock);
			if (retMutex)
			{
				for (i = 0; i < driver->num; i++)
				{
					statusTX = (1 << i);
					if ( (ptTransfer->statusTX & statusTX) != 0 ) 
					{
						if (driver->ports[i] == 0)
						{
							//
						}
						else
						{
							portIn = driver->ports[i];
							if (portIn->PortTypWsync != 1)
							{
								continue;
							}
							memset(bufRX, 0, 2048);
							memset(bufTX, 0, 2048);
							memset(&tTransfer, 0, sizeof(tTransfer));
							
							u32AdrSwap = (SPI_ADDR_TX_BUF + (SPI_ADDR_TX_BUF_OFFSET * i));
						
							u32AdrSwap = u32AdrSwap << 1;

							u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
						
							memcpy(bufTX, &u32AdrSwap, 3);
														
							switch (i)
							{
								case 0:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[0]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf0[idx]; }
								}
								break;
								case 1:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[1]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf1[idx]; }
								}
								break;
								case 2:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[2]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf2[idx]; }
								}
								break;
								case 3:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[3]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf3[idx]; }
								}
								break;
								case 4:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[4]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf4[idx]; }
								}
								break;
								case 5:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[5]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf5[idx]; }
								}
								break;
								case 6:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[6]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf6[idx]; }
								}									
								break;
								case 7:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[7]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf7[idx]; }
								}
								break;
								case 8:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[8]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf8[idx]; }
								}
								break;
								case 9:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[9]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf9[idx]; }
								}
								break;
								case 10:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[10]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf10[idx]; }
								}
								break;
								case 11:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[11]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf11[idx]; } 
								}
								break;
								case 12:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[12]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf12[idx]; } 
								}
								break;
								case 13:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[13]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf13[idx]; } 
								}
								break;
								case 14:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[14]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf14[idx]; } 
								}
								break;
								case 15:
								{ 
									for (idx = 0; idx < ptTransfer->lenTX[15]; idx++)
									{ *(bufTX + 4 + idx) = ptTransfer->tx_buf15[idx]; }
								}
								break;
								default:
								{
									//error
								}
								break;
							}

							tTransfer.tx_buf = (unsigned long)bufTX;
							tTransfer.rx_buf = (unsigned long)bufRX;
							tTransfer.len = (ptTransfer->lenTX[i] + SPI_UART_HAEDER);
							if (portIn->spiSpeed == 0)
							{
								tTransfer.speed_hz = iSpeed;
							}
							else
							{
								tTransfer.speed_hz = portIn->spiSpeed;
							}
							tTransfer.bits_per_word = 8;

							if (IS_ERR(driver->filp)) 
							{
								ret = -ENOENT;
								goto endKfree;
							} 
							else
							{
								if ( (driver->filp == NULL) || (driver->filp->f_op == NULL))
								{
									ret = -ENOENT;
									goto endKfree;
								}
								else
								{
									err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
								}
								if (err < 0)
								{
									ret = -EINVAL;
								}
								else
								{
									//
								}
							}
						}
					}
				}

				mutex_unlock(&driver->spi_read_lock);
			}
			else
			{

			}
		}while(!retMutex);
	}

	down_read(&tty->termios_rwsemSpi);
	add_wait_queue(&driver->spi_waitTX, &waitRX);

	//TX Lock while RX active
	while(1)
	{
		queNotEmpty = 0;
		for (i = 0; i < driver->num; i++)
		{
			if (driver->ports[i] == 0)
			{
				//
			}
			else
			{
				portIn = driver->ports[i];
				if (portIn->idxIn != portIn->idxOut)
				{
					//ret = 5;
					//goto end;
					queNotEmpty = 1;
					break;
				}
			}
		}
		if ( (!timeoutRX) )
		{
			break;
		}
		if (!(READ_ONCE(driver->TxSpi)) && (!queNotEmpty))
		{
			break;
		}
		if (queNotEmpty)
		{
			flush_workqueue(spi_workqueue);
		}

		up_read(&tty->termios_rwsemSpi);
	
		timeoutRX = wait_woken(&waitRX, TASK_INTERRUPTIBLE, (msecs_to_jiffies(5000)));
		down_read(&tty->termios_rwsemSpi);
	}

	remove_wait_queue(&driver->spi_waitTX, &waitRX);
	up_read(&tty->termios_rwsemSpi);

	if (ptTransfer->statusRead != 0)
	{
		do
		{
			retMutex = mutex_trylock(&driver->spi_read_lock);
			if (retMutex == 0)
			{
				port->err_cnt_spiread++;
				ret = 3;
				//goto end;
			}
			else
			{
				driver = tty->driver;
		
				bufSpiStatus = kmalloc(128, GFP_KERNEL);
				if (!bufSpiStatus) {
					//
				}
				ttyStatus = kcalloc(16, sizeof(*ttyStatus), GFP_KERNEL);
				ttyStatusTmp = ttyStatus;
				
				do
				{
					memset(bufSpiStatus, 0, 128);
					ret = spi_uart_read_allstatus(tty, bufSpiStatus, SPI_ADDR_IRQ_STATUS, 128, port->spiSpeed);
					if (ret < 0)
					{
						//
					}
					else
					{
						txEmpty = 0;
						ttyStatus = ttyStatusTmp;
						for (i = 0; i < 16; i++, ttyStatus++) {
							ttyStatus->u16irqStatus = (uint16_t)((bufSpiStatus[0 + i * 8] << 8) | (bufSpiStatus[1 + i * 8]));
							ttyStatus->u16TxFIFOLev = (uint16_t)((bufSpiStatus[2 + i * 8] << 8) | (bufSpiStatus[3 + i * 8]));
							if (ttyStatus->u16TxFIFOLev != 0)
							{
								txEmpty = 1;
								break;
							}
							ttyStatus->u16RxFIFOLev = (uint16_t)((bufSpiStatus[4 + i * 8] << 8) | (bufSpiStatus[5 + i * 8]));
							ttyStatus->u8res1 = bufSpiStatus[6 + i * 8];
							ttyStatus->u8res2 = bufSpiStatus[7 + i * 8];
						}
					}
				}
				while(txEmpty);

				ttyStatus = ttyStatusTmp;
			
				for (i = 0; i < driver->num; i++, ttyStatus++)
				{
					statusRead = (1 << i);
					if ((ptTransfer->statusRead & statusRead) != 0)		
					{
						if (driver->ports[i] == 0)
						{
							//
						}
						else
						{
							portIn = driver->ports[i];
							if (driver->ports[i]->tty == 0)
							{
								//
							}
							else
							{
								if ( (portIn->PortTypRsync != 1) && (portIn->PortTypRasync != 1) )
								{
									continue;
								}
								
								portTrigger[i] = &driver->ports[i]->tx_trigger;
								ttyIn = driver->ports[i]->tty;
									
								u16irqStatus = ttyStatus->u16irqStatus;
								
								portIn->tx_fifolvl_last = (int)ttyStatus->u16TxFIFOLev;
								
								portIn->rx_fifolvl = (int)ttyStatus->u16RxFIFOLev;
								ptLen0 = &(ttyStatus->u16RxFIFOLev); 
								if ( (ttyStatus->u16RxFIFOLev != 0) )
								{ 
									bufSpiRX = kmalloc(ttyStatus->u16RxFIFOLev, GFP_KERNEL);
									if (!bufSpiRX) {
										//
									}
									else
									{
										memset(bufSpiRX, 0, ttyStatus->u16RxFIFOLev);
										ret = spi_uart_read_rx(ttyIn, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * ttyIn->index)), ttyStatus->u16RxFIFOLev, portIn->spiSpeed);
										if (ret < 0)
										{
											kfree(bufSpiRX);
										}
										if (portIn->PortTypRasync)
										{
											for (j = 0; j < ttyStatus->u16RxFIFOLev; j++)
											{
												ret = tty_insert_flip_char(portIn, *(bufSpiRX + j), 0);
											}
											if (ret < 0)
											{
												//
											}
											statusRX |= (1 << ttyIn->index);
										}
										if (portIn->PortTypRsync)
										{
											switch (ttyIn->index)
											{
												case 0:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX0, bufSpiRX, ttyStatus->u16RxFIFOLev);
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX0, ptLen0, TWO_BYTE);
												}
												break;
												case 1:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX1, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX1, ptLen0, TWO_BYTE);
												}
												break;
												case 2:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX2, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX2, ptLen0, TWO_BYTE);
												}
												break;
												case 3:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX3, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX3, ptLen0, TWO_BYTE);
												}
												break;
												case 4:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX4, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX4, ptLen0, TWO_BYTE);
												}
												break;
												case 5:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX5, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX5, ptLen0, TWO_BYTE);
												}
												break;
												case 6:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX6, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX6, ptLen0, TWO_BYTE);
												}									break;
												case 7:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX7, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX7, ptLen0, TWO_BYTE);
												}
												break;
												case 8:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX8, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX8, ptLen0, TWO_BYTE);
												}
												break;
												case 9:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX9, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX9, ptLen0, TWO_BYTE);
												}
												break;
												case 10:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX10, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX10, ptLen0, TWO_BYTE);
												}
												break;
												case 11:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX11, bufSpiRX, ttyStatus->u16RxFIFOLev);
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX11, ptLen0, TWO_BYTE); 
												}
												break;
												case 12:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX12, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX12, ptLen0, TWO_BYTE); 
												}
												break;
												case 13:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX13, bufSpiRX, ttyStatus->u16RxFIFOLev);
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX13, ptLen0, TWO_BYTE);  
												}
												break;
												case 14:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX14, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX14, ptLen0, TWO_BYTE);
												}
												break;
												case 15:
												{ 
													ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_bufX15, bufSpiRX, ttyStatus->u16RxFIFOLev); 
													ret = copy_to_user((u32 __user *)(uintptr_t)ptTransfer->lenRX15, ptLen0, TWO_BYTE);
												}
												break;
												default:
												{
													//error
												}
												break;
											}
											statusRX |= (1 << ttyIn->index);
										}

										kfree(bufSpiRX);								
									}
									if (portIn->PortTypRasync)
									{
										tty_flip_buffer_push(portIn);
									}
								}
							}
						}
					}
				} 

				kfree(ttyStatusTmp);
				kfree(bufSpiStatus);
				mutex_unlock(&driver->spi_read_lock);
			}
		} 
		while(!retMutex);
	}

endKfree:

	kfree(bufTX);
	kfree(bufRX);

	ptStatusRX = &statusRX;

	ret = copy_to_user((u16 __user *)(uintptr_t)ptTransfer->rx_status, ptStatusRX, TWO_BYTE);

	ret = copy_to_user((u16 __user *)(uintptr_t)ptTransfer->statusRX, ptStatusRX, TWO_BYTE);

	return 0;
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_start(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;

	port = uart_port_lock(state, flags);
	__uart_start(tty);
	uart_port_unlock(port, flags);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_put_char(struct tty_struct *tty, unsigned char c)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	struct circ_buf *circ;
	unsigned long flags;
	int ret = 0;

	circ = &state->xmit;
	if (!circ->buf)
		return 0;

	port = uart_port_lock(state, flags);
	if (port && uart_circ_chars_free(circ) != 0) {
		circ->buf[circ->head] = c;
		circ->head = (circ->head + 1) & (UART_XMIT_SIZE - 1);
		ret = 1;
	}
	uart_port_unlock(port, flags);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_flush_chars(struct tty_struct *tty)
{
	uart_start(tty);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_write(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	int ret = 0;
	bool bWork = false;
	struct spi_work_struct *spi_work;

	tty->port->spi_w_counter++;

	spi_work = (struct spi_work_struct *)kmalloc(sizeof(struct spi_work_struct), GFP_KERNEL);

    if (!spi_work) {
        pr_err("failed to kmalloc spi_work"); 
    }
    else {
        INIT_WORK( (struct work_struct *)spi_work, &spi_workqueue_function );
        spi_work->spi_w_counter = tty->port->spi_w_counter;
        spi_work->spi_jiffies_in = jiffies;
		tty->port->spi_jiffies_in = jiffies;
		spi_work->port = tty->port;
		spi_work->tty = tty;
		spi_work->buf = buf; 
		spi_work->count = count;
		tty->port->idxIn = (tty->port->idxIn + 1) & 0x0F;
		spi_work->idxIn = tty->port->idxIn;
		
        bWork = queue_work( spi_workqueue, (struct work_struct *)spi_work ); // enqueue my work item
    }
	
	ret = count;

return ret;
	
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_rws_write(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	uint32_t u32AdrSwap;
	int ret = 0;
	int i;
	ssize_t err;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX;
	unsigned char *bufRX;
	u8 *bufTX_poll_m;
	u8 *bufRX_poll_m;
	u8 *bufSpiRX;
	u16 u16irqStatus = 0;
	u16 u16RxFIFOLev = 0;
	struct tty_driver *driver;

	driver = tty->driver;
	
	bufTX_poll_m = kmalloc(SPI_UART_HAEDER + 2, GFP_KERNEL);
	if (!bufTX_poll_m) {
			printk("error tx_buf \n");
	}
	bufRX_poll_m = kmalloc(SPI_UART_HAEDER + 2, GFP_KERNEL);
	if (!bufRX_poll_m) {
			printk("error tx_buf \n");
	}

	bufTX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX) {
			printk("error tx_buf \n");
			goto end;
	}
	bufRX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX) {
			printk("error rx_buf \n");
			kfree(bufTX);
			goto end;
	}
	
	for (i = 0; i < (count + SPI_UART_HAEDER); i++)
  	{
    	*(bufRX + i) = 0x0;
    	*(bufTX + i) = 0x0;
  	}
  	memset(&tTransfer, 0, sizeof(tTransfer));
    
	u32AdrSwap = (SPI_ADDR_TX_BUF + (SPI_ADDR_TX_BUF_OFFSET * tty->index));
	u32AdrSwap = u32AdrSwap << 1;
	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX, &u32AdrSwap, 3);
	
	memcpy((bufTX + 4), buf, count);
	

	tTransfer.tx_buf = (unsigned long)bufTX;
	tTransfer.rx_buf = (unsigned long)bufRX;
	tTransfer.len = (count + SPI_UART_HAEDER);
	if (tty->port->spiSpeed == 0)
	{
		tTransfer.speed_hz = iSpeed;
	}
	else
	{
		tTransfer.speed_hz = tty->port->spiSpeed;
	}
	tTransfer.bits_per_word = 8;
	
	if (IS_ERR(driver->filp)) {
		ret = -ENOENT;
		goto endKfree;
	} 
	else
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
		if (err < 0)
		{
		ret = -EINVAL;
		}
		else
		{
		ret = count;
		}
		
		memset(bufTX_poll_m, 0, (SPI_UART_HAEDER + 2));
		memset(bufRX_poll_m, 0, (SPI_UART_HAEDER + 2));
		
		ret = spi_uart_read_status(tty, bufTX_poll_m, bufRX_poll_m, (SPI_ADDR_IRQ_STATUS + (SPI_ADDR_STATUS_OFFSET * tty->index)), TWO_BYTE, FALSE, tty->port->spiSpeed);
		if (ret < 0)
		{
			goto end_kfree;
		}
		memcpy(&u16irqStatus, (bufRX_poll_m + SPI_UART_HAEDER), TWO_BYTE);
		tty->port->spiflags = (unsigned long)u16irqStatus;

		memset(bufTX_poll_m, 0, (SPI_UART_HAEDER + 2));
		memset(bufRX_poll_m, 0, (SPI_UART_HAEDER + 2));
		memset(bufTX_poll_m, 0, (SPI_UART_HAEDER + 2));
		memset(bufRX_poll_m, 0, (SPI_UART_HAEDER + 2));
		
		ret = spi_uart_read_status(tty, bufTX_poll_m, bufRX_poll_m, (SPI_ADDR_RX_FIFO_L + (SPI_ADDR_STATUS_OFFSET * tty->index)), TWO_BYTE, FALSE, tty->port->spiSpeed);
		if (ret < 0)
		{
			goto end_kfree;
		}
		memcpy(&u16RxFIFOLev, (bufRX_poll_m + SPI_UART_HAEDER), TWO_BYTE);
		tty->port->rx_fifolvl = (int)u16RxFIFOLev;

		if ( (u16RxFIFOLev != 0) )
		{ 
			bufSpiRX = kmalloc(u16RxFIFOLev, GFP_KERNEL);
			if (!bufSpiRX) {
				printk("error tx_buf !!!!!!!!!!!\n");
				ret = -1;
			}
			else
			{
			memset(bufSpiRX, 0, u16RxFIFOLev);
			ret = spi_uart_read_rx(tty, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * tty->index)), u16RxFIFOLev, tty->port->spiSpeed);
			if (ret < 0)
			{
				kfree(bufSpiRX);
				goto end_kfree;
			}
			for (i = 0; i < u16RxFIFOLev; i++)
			{
				ret = tty_insert_flip_char(tty->port, *(bufSpiRX + i), 0);
			}
			kfree(bufSpiRX);
			ret = (int)u16RxFIFOLev;
			}
			tty_flip_buffer_push(tty->port);
		}
		else
		{
			ret = 0;
		}

	}

endKfree:
end_kfree:
  
 	kfree(bufTX);
	kfree(bufRX);
	if (bufTX_poll_m)
	{
		kfree(bufTX_poll_m);
	}
	if (bufRX_poll_m)
	{
		kfree(bufRX_poll_m);
	}
	
end:

	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_readwrite(struct tty_struct *tty,
					const unsigned char *bufIn, unsigned char *bufOut, uint32_t addr, int count, bool write)
{
	int ret = 0;
	int i;
	ssize_t err;
	uint32_t u32AdrSwap;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX;
	unsigned char *bufRX;
	struct tty_driver *driver;

	if (!tty)
	{
		return ret;
	}
	else
	{
		if (!tty->driver)
		{
			return ret;
		}
		else
		{
			if (!tty->driver->filp)
			{
				return ret;
			}
			else
			{
				driver = tty->driver;
			}
		}
	}
		
	bufTX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX) {
			//
	}
	bufRX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX) {
			//
	}
	memset(&tTransfer, 0, sizeof(tTransfer));
	for (i = 0; i < (count + SPI_UART_HAEDER); i++)
	{
		*(bufRX + i) = 0x0;
		*(bufTX + i) = 0x0;
	}

  	u32AdrSwap = addr;
	u32AdrSwap = u32AdrSwap << 1;
	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX, &u32AdrSwap, 3);
	   
	if (write != TRUE)
	{
		*(bufTX + 2) |= 0x01;
		*(bufTX + 2 + count) = 0x01;
		*(bufTX + 3 + count) = 0x01;
	}
	else
	{
		if (count == 1)
		{
			*(bufTX + SPI_UART_HAEDER) = *bufIn;
		}
		else
		{
			*(bufTX + SPI_UART_HAEDER) = *(bufIn +1);
			*(bufTX + SPI_UART_HAEDER + 1) = *bufIn;
		}
	}

	tTransfer.tx_buf = (unsigned long)bufTX;
	tTransfer.rx_buf = (unsigned long)bufRX;
	tTransfer.len = (count + SPI_UART_HAEDER);
	if (tty->port->spiSpeed == 0)
	{
		tTransfer.speed_hz = iSpeed;
	}
	else
	{
		tTransfer.speed_hz = tty->port->spiSpeed;
	}
	tTransfer.bits_per_word = 8;
  
	if (IS_ERR(driver->filp)) 
	{
		ret = -ENOENT;
		goto end;
	} 
	else
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
	}
		
	if (count == 1)
	{
		*(bufOut + SPI_UART_HAEDER) = *(bufRX + SPI_UART_HAEDER);
	}
	else
	{
		*(bufOut + SPI_UART_HAEDER) = *(bufRX + SPI_UART_HAEDER + 1);
		*(bufOut + SPI_UART_HAEDER + 1) = *(bufRX + SPI_UART_HAEDER);
	}

end:

  	kfree(bufTX);
	kfree(bufRX);
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_read_status(struct tty_struct *tty, const unsigned char *bufIn, unsigned char *bufOut, uint32_t addr, int count, bool write, unsigned long speed)
{
	int ret = 0;
	int i;
	ssize_t err;
	uint32_t u32AdrSwap;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX_read_status;
	unsigned char *bufRX_read_status;
	struct tty_driver *driver;

	driver = tty->driver;
		
	bufTX_read_status = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX_read_status) {
			//
	}
	bufRX_read_status = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX_read_status) {
			//
	}
	memset(&tTransfer, 0, sizeof(tTransfer));
	for (i = 0; i < (count + SPI_UART_HAEDER); i++)
	{
		*(bufRX_read_status + i) = 0x0;
		*(bufTX_read_status + i) = 0x0;
	}

	u32AdrSwap = addr;
	u32AdrSwap = u32AdrSwap << 1;
	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX_read_status, &u32AdrSwap, 3);
   
	if (write != TRUE)
	{
		*(bufTX_read_status + 2) |= 0x01;
		*(bufTX_read_status + 2 + count) = 0x01;
		*(bufTX_read_status + 3 + count) = 0x01;
	}
	else
	{
		if (count == 1)
		{
			*(bufTX_read_status + SPI_UART_HAEDER) = *bufIn;
		}
		else
		{
			*(bufTX_read_status + SPI_UART_HAEDER) = *(bufIn +1);
			*(bufTX_read_status + SPI_UART_HAEDER + 1) = *bufIn;
		}
	}

	tTransfer.tx_buf = (unsigned long)bufTX_read_status;
	tTransfer.rx_buf = (unsigned long)bufRX_read_status;
	tTransfer.len = (count + SPI_UART_HAEDER);
	tTransfer.speed_hz = iSpeed;
	tTransfer.bits_per_word = 8;
  
	if (IS_ERR(driver->filp)) 
	{
		ret = -ENOENT;
		goto end;
	} 
	else 
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
	}
  
	if (count == 1)
	{
		*(bufOut + SPI_UART_HAEDER) = *(bufRX_read_status + SPI_UART_HAEDER);
	}
	else
	{
		*(bufOut + SPI_UART_HAEDER) = *(bufRX_read_status + SPI_UART_HAEDER + 1);
		*(bufOut + SPI_UART_HAEDER + 1) = *(bufRX_read_status + SPI_UART_HAEDER);
	}
	
end:

	if (bufTX_read_status)
	{
		kfree(bufTX_read_status);
	}
	if (bufRX_read_status)
	{
		kfree(bufRX_read_status);
	}
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_read_rx(struct tty_struct *tty, unsigned char *bufOut, uint32_t addr, int count, unsigned long speed)
{
	int ret = 0;
	int i;
	ssize_t err = 0;
	uint32_t u32AdrSwap;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX_read_rx;
	unsigned char *bufRX_read_rx;
	struct tty_driver *driver;

	driver = tty->driver;

	bufTX_read_rx = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX_read_rx) {
			//
	}
	bufRX_read_rx = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX_read_rx) {
			//
	}
	memset(&tTransfer, 0, sizeof(tTransfer));
	for (i = 0; i < (count + SPI_UART_HAEDER); i++)
	{
		*(bufRX_read_rx + i) = 0x0;
		*(bufTX_read_rx + i) = 0x0;
	}

	u32AdrSwap = addr;
	u32AdrSwap = u32AdrSwap << 1;
	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX_read_rx, &u32AdrSwap, 3);
   
	*(bufTX_read_rx + 2) |= 0x01; 
	*(bufTX_read_rx + 2 + count) = 0x01;
	*(bufTX_read_rx + 3 + count) = 0x01;
  
	tTransfer.tx_buf = (unsigned long)bufTX_read_rx;
	tTransfer.rx_buf = (unsigned long)bufRX_read_rx;
	tTransfer.len = (count + SPI_UART_HAEDER);
	tTransfer.speed_hz = iSpeed;
	tTransfer.bits_per_word = 8;
  
	if (IS_ERR(driver->filp)) 
	{
		ret = -ENOENT;
		goto end;
	} 
	else 
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
	}
	
	for (i = 0; i < count; i++)
	{
		*(bufOut + i) = *(bufRX_read_rx + SPI_UART_HAEDER + i);
	}
	
end:

	if (bufTX_read_rx)
	{
		kfree(bufTX_read_rx);
	}
	if (bufRX_read_rx)
	{
		kfree(bufRX_read_rx);
	}
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_read_allstatus(struct tty_struct *tty, unsigned char *bufOut, uint32_t addr, int count, unsigned long speed)
{
	int ret = 0;
	int i;
	ssize_t err;
	uint32_t u32AdrSwap;
	struct spi_ioc_transfer tTransfer;
	unsigned char *bufTX;
	unsigned char *bufRX;
	struct tty_driver *driver;

	driver = tty->driver;
	
	bufTX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufTX) {
			//
	}
	bufRX = kmalloc((count + SPI_UART_HAEDER), GFP_KERNEL);
	if (!bufRX) {
			//
	}
	memset(&tTransfer, 0, sizeof(tTransfer));
	for (i = 0; i < (count + SPI_UART_HAEDER); i++)
	{
		*(bufRX + i) = 0x0;
		*(bufTX + i) = 0x0;
	}

	u32AdrSwap = addr;
	u32AdrSwap = u32AdrSwap << 1;
	u32AdrSwap = (uint32_t)( ((u32AdrSwap & 0xFF0000) >> 16) | ((u32AdrSwap & 0xFF) << 16) | (u32AdrSwap & 0xFF00) );
  
	memcpy(bufTX, &u32AdrSwap, 3);
   
	*(bufTX + 2) |= 0x01; 
	*(bufTX + 2 + count) = 0x01;
	*(bufTX + 3 + count) = 0x01;
  
	tTransfer.tx_buf = (unsigned long)bufTX;
	tTransfer.rx_buf = (unsigned long)bufRX;
	tTransfer.len = (count + SPI_UART_HAEDER);
	tTransfer.speed_hz = iSpeed;
	tTransfer.bits_per_word = 8;

  
	if (IS_ERR(driver->filp)) 
	{
		ret = -ENOENT;
		goto end;
	} 
	else 
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &tTransfer);
	}
	
	for (i = 0; i < count; i++)
	{
		*(bufOut + i) = *(bufRX + SPI_UART_HAEDER + i);
	}
	
end:

	kfree(bufTX);
	kfree(bufRX);
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_message(struct tty_struct *tty, unsigned long arg)
{
	int ret = 0;
	ssize_t err;
	struct spi_ioc_transfer spiTransfer;
	struct spi_ioc_transfer *ptTransfer;
	unsigned char *bufTX;
	unsigned char *bufRX;
	struct tty_driver *driver;
	struct spi_ioc_transfer __user *u_ioc = (struct spi_ioc_transfer __user *)arg;
	driver = tty->driver;
	
	memset(&spiTransfer, 0, sizeof(spiTransfer));
	
	ptTransfer = memdup_user(u_ioc, sizeof(struct spi_ioc_transfer));
	if (IS_ERR(ptTransfer))
		return -1;
		
	bufTX = kmalloc(ptTransfer->len, GFP_KERNEL);
	if (!bufTX) {
			//
	}
	bufRX = kmalloc(ptTransfer->len, GFP_KERNEL);
	if (!bufRX) {
			//
	}
	
	ret = copy_from_user(bufTX, (const u8 __user *)(uintptr_t) ptTransfer->tx_buf, 	ptTransfer->len);
	 
	spiTransfer.tx_buf = (unsigned long)bufTX;
	spiTransfer.rx_buf = (unsigned long)bufRX;
	spiTransfer.len = ptTransfer->len;
	spiTransfer.speed_hz = ptTransfer->speed_hz;
	spiTransfer.bits_per_word = ptTransfer->bits_per_word;
  
	if (IS_ERR(driver->filp)) 
	{
		ret = -ENOENT;
		goto end;
	} 
	else
	{
		err = driver->filp->f_op->unlocked_ioctl(driver->filp, SPI_IOC_UMESSAGE(1), &spiTransfer);
	}
		
	ret = copy_to_user((u8 __user *)(uintptr_t) ptTransfer->rx_buf, bufRX, ptTransfer->len);
  
end:
   
	kfree(bufRX);
  	kfree(bufTX);
	
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_read(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct tty_driver *driver; 
	int ret, retMutex, sizeFlip;
	struct spi_tty_status *ttyStatus;
	long timeoutRX = 5000;
	int txEmpty = 0;

	DEFINE_WAIT_FUNC(waitRX, woken_wake_function);

	u8 *bufSpiStatus;
	u8 *bufSpiRX;
	u16 u16irqStatus = 0;
	ret = 0;
	
	driver = tty->driver;

	down_read(&tty->termios_rwsemSpi);
	add_wait_queue(&driver->spi_waitTX, &waitRX);

	//TX Lock while RX active
	while(1)
	{
		if ( (!timeoutRX) )
		{
			break;
		}
		if ( (!(READ_ONCE(driver->TxSpi))) && (port->idxIn == port->idxOut) )
		{
			break;
		}
		if (port->idxIn != port->idxOut)
		{
			flush_workqueue(spi_workqueue);
		}
		
		up_read(&tty->termios_rwsemSpi);
	
		timeoutRX = wait_woken(&waitRX, TASK_INTERRUPTIBLE, (msecs_to_jiffies(5000)));
		down_read(&tty->termios_rwsemSpi);
	}

	remove_wait_queue(&driver->spi_waitTX, &waitRX);
	up_read(&tty->termios_rwsemSpi);

	if (port->idxIn != port->idxOut)
	{
		ret |= (1 << 12);
		goto end;
	}
	
	retMutex = 0;
	do{
		retMutex = mutex_trylock(&driver->spi_read_lock);
		if (retMutex == 0)
		{
			ret |= (1 << 13);
			//goto end;
		}
		else
		{
				if (!count)
				{
					bufSpiStatus = kmalloc(8, GFP_KERNEL);
					if (!bufSpiStatus) {
						//
					}
					ttyStatus = kcalloc(1, sizeof(*ttyStatus), GFP_KERNEL);
					do
					{
						memset(bufSpiStatus, 0, 8);
						ret = spi_uart_read_allstatus(tty, bufSpiStatus, (SPI_ADDR_IRQ_STATUS + (SPI_ADDR_STATUS_OFFSET * tty->index)), 8, port->spiSpeed);
						if (ret < 0)
						{
							//
						}
						else
						{
							txEmpty = 0;
							ttyStatus->u16irqStatus = (uint16_t)((bufSpiStatus[0] << 8) | (bufSpiStatus[1]));
							ttyStatus->u16TxFIFOLev = (uint16_t)((bufSpiStatus[2] << 8) | (bufSpiStatus[3]));
							if (ttyStatus->u16TxFIFOLev != 0)
							{
								txEmpty = 1;
								ret |= (1 << 14);
								break;
							}
							ttyStatus->u16RxFIFOLev = (uint16_t)((bufSpiStatus[4] << 8) | (bufSpiStatus[5]));
							ttyStatus->u8res1 = bufSpiStatus[6];
							ttyStatus->u8res2 = bufSpiStatus[7];
						}
					}
					while(txEmpty);
					
					u16irqStatus = ttyStatus->u16irqStatus;
					port->spiflags = (unsigned long)u16irqStatus;

					port->tx_fifolvl_last = (int)ttyStatus->u16TxFIFOLev;
					port->rx_fifolvl = (int)ttyStatus->u16RxFIFOLev;

					if ( (ttyStatus->u16RxFIFOLev != 0) )
					{ 
						bufSpiRX = kmalloc(ttyStatus->u16RxFIFOLev, GFP_KERNEL);
						if (!bufSpiRX) {
							//ret |= (1 << 15);
						}
						else
						{
							memset(bufSpiRX, 0, ttyStatus->u16RxFIFOLev);
							ret = spi_uart_read_rx(tty, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * tty->index)), ttyStatus->u16RxFIFOLev, port->spiSpeed);
							if (ret < 0)
							{
								kfree(bufSpiRX);
								goto end;
							}
							sizeFlip = 0;
							ret = copy_to_user((char __user *)buf, bufSpiRX, ttyStatus->u16RxFIFOLev);

							kfree(bufSpiRX);
							ret |= (int)ttyStatus->u16RxFIFOLev;
						}
					}
					else
					{
						ret |= (1 << 15);
					}
					kfree(ttyStatus);
					kfree(bufSpiStatus);
				}
				else
				{
					if ( (count < 2048) )
					{ 
						bufSpiRX = kmalloc(count, GFP_KERNEL);
						if (!bufSpiRX) {
							//ret |= (1 << 11);
						}
						else
						{
							memset(bufSpiRX, 0, count);
							ret = spi_uart_read_rx(tty, bufSpiRX, (SPI_ADDR_RX_BUF + (SPI_ADDR_RX_BUF_OFFSET * tty->index)), count, port->spiSpeed);
							if (ret < 0)
							{
								kfree(bufSpiRX);
								goto end;
							}
							ret = copy_to_user((char __user *)buf, bufSpiRX, count);
							ret |= count;
						}
						kfree(bufSpiRX);
					}
					else
					{
						ret |= (1 << 15);
					}
				}

			mutex_unlock(&driver->spi_read_lock);
		}
	} 
	while(!retMutex);

end:
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_write_room(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;
	int ret;

	port = uart_port_lock(state, flags);
	ret = uart_circ_chars_free(&state->xmit);
	uart_port_unlock(port, flags);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_chars_in_buffer(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;
	int ret;

	port = uart_port_lock(state, flags);
	ret = uart_circ_chars_pending(&state->xmit);
	uart_port_unlock(port, flags);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_flush_buffer(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;

	/*
	 * This means you called this function _after_ the port was
	 * closed.  No cookie for you.
	 */
	if (!state) {
		WARN_ON(1);
		return;
	}

	pr_debug("uart_flush_buffer(%d) called\n", tty->index);

	port = uart_port_lock(state, flags);
	if (!port)
		return;
	uart_circ_clear(&state->xmit);
	if (port->ops->flush_buffer)
		port->ops->flush_buffer(port);
	uart_port_unlock(port, flags);
	tty_port_tty_wakeup(&state->port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_send_xchar(struct tty_struct *tty, char ch)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long flags;

	port = uart_port_ref(state);
	if (!port)
		return;

	if (port->ops->send_xchar)
		port->ops->send_xchar(port, ch);
	else {
		spin_lock_irqsave(&port->lock, flags);
		port->x_char = ch;
		if (ch)
			port->ops->start_tx(port);
		spin_unlock_irqrestore(&port->lock, flags);
	}
	uart_port_deref(port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_throttle(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	upstat_t mask = 0;

	port = uart_port_ref(state);
	if (!port)
		return;

	if (I_IXOFF(tty))
		mask |= UPSTAT_AUTOXOFF;
	if (C_CRTSCTS(tty))
		mask |= UPSTAT_AUTORTS;

	if (port->status & mask) {
		port->ops->throttle(port);
		mask &= ~port->status;
	}

	if (mask & UPSTAT_AUTORTS)
		uart_clear_mctrl(port, TIOCM_RTS);

	if (mask & UPSTAT_AUTOXOFF)
		uart_send_xchar(tty, STOP_CHAR(tty));

	uart_port_deref(port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_unthrottle(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	upstat_t mask = 0;

	port = uart_port_ref(state);
	if (!port)
		return;

	if (I_IXOFF(tty))
		mask |= UPSTAT_AUTOXOFF;
	if (C_CRTSCTS(tty))
		mask |= UPSTAT_AUTORTS;

	if (port->status & mask) {
		port->ops->unthrottle(port);
		mask &= ~port->status;
	}

	if (mask & UPSTAT_AUTORTS)
		uart_set_mctrl(port, TIOCM_RTS);

	if (mask & UPSTAT_AUTOXOFF)
		uart_send_xchar(tty, START_CHAR(tty));

	uart_port_deref(port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_tiocmget(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport;
	int result = -EIO;

	mutex_lock(&port->mutex);
	uport = uart_port_check(state);
	if (!uport)
		goto out;

	if (!tty_io_error(tty)) {
		result = uport->mctrl;
		spin_lock_irq(&uport->lock);
		result |= uport->ops->get_mctrl(uport);
		spin_unlock_irq(&uport->lock);
	}
out:
	mutex_unlock(&port->mutex);
	return result;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int
spi_uart_tiocmset(struct tty_struct *tty, unsigned int set, unsigned int clear)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport;
	int ret = -EIO;

	mutex_lock(&port->mutex);
	uport = uart_port_check(state);
	if (!uport)
		goto out;

	if (!tty_io_error(tty)) {
		uart_update_mctrl(uport, set, clear);
		ret = 0;
	}
out:
	mutex_unlock(&port->mutex);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_break_ctl(struct tty_struct *tty, int break_state)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport;
	int ret = -EIO;

	mutex_lock(&port->mutex);
	uport = uart_port_check(state);
	if (!uport)
		goto out;

	if (uport->type != PORT_UNKNOWN)
		uport->ops->break_ctl(uport, break_state);
	ret = 0;
out:
	mutex_unlock(&port->mutex);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_get_icount(struct tty_struct *tty,
			  struct serial_icounter_struct *icount)
{
	struct uart_state *state = tty->driver_data;
	struct uart_icount cnow;
	struct uart_port *uport;

	uport = uart_port_ref(state);
	if (!uport)
		return -EIO;
	spin_lock_irq(&uport->lock);
	memcpy(&cnow, &uport->icount, sizeof(struct uart_icount));
	spin_unlock_irq(&uport->lock);
	uart_port_deref(uport);

	icount->cts         = cnow.cts;
	icount->dsr         = cnow.dsr;
	icount->rng         = cnow.rng;
	icount->dcd         = cnow.dcd;
	icount->rx          = cnow.rx;
	icount->tx          = cnow.tx;
	icount->frame       = cnow.frame;
	icount->overrun     = cnow.overrun;
	icount->parity      = cnow.parity;
	icount->brk         = cnow.brk;
	icount->buf_overrun = cnow.buf_overrun;

	return 0;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int
spi_uart_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport;
	void __user *uarg = (void __user *)arg;
	int ret = -ENOIOCTLCMD;
	int readFlag = 0;
	int i;
	u8 *bufTX;
	u8 *bufRX;
	u8 *bufTXspi;
	u8 *bufRXspi;
	struct tty_bufhead *buf = &port->buf;
	struct tty_buffer *b;
	struct tty_driver *driver;
	struct spi_tty_status *ttyStatusPorts;
	struct spi_tty_status *ttyStatusTmpPorts;
	u8 *bufSpiStatusPorts;

	b = buf->tail;
	
	driver = tty->driver;

	bufTX = kmalloc(SPI_UART_HAEDER + 2, GFP_KERNEL);
	if (!bufTX) {
			//
	}
	bufRX = kmalloc(SPI_UART_HAEDER + 2, GFP_KERNEL);
	if (!bufRX) {
			//
	}
	bufTXspi = kmalloc(128, GFP_KERNEL);
	if (!bufTXspi) {
			//
	}
	bufRXspi = kmalloc(2048, GFP_KERNEL);
	if (!bufTXspi) {
			//
	}
	memset(bufTX, 0, (SPI_UART_HAEDER + 2));
	memset(bufRX, 0, (SPI_UART_HAEDER + 2));
	memset(bufRXspi, 0, 2048);
	/*
	 * These ioctls don't rely on the hardware to be present.
	 */
	switch (cmd) {
	case TIOCGSERIAL:

		//ret = uart_get_info_user(port, uarg); uart_insert_char
	break;
	case TIOREAD_INFO:
    
    	ret = copy_from_user(&readFlag, (const u8 __user *)uarg, sizeof(int));
		ret = spi_poll_all(port);
	break;
	case TIOWRITE:
    	ret = copy_from_user(&iSpeed, (const u8 __user *)uarg, sizeof(int32_t));
	break;
	//Baudrate | Stop Bit Count. WRITE	
	case TIOWRITE_SB_BAUDRATE:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_SB_BAUDRATE + (SPI_ADDR_CONFIG_OFFSET * tty->index)), BYTE, TRUE);

	break;
	//Baudrate | Stop Bit Count. READ	
	case TIOREAD_SB_BAUDRATE:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_SB_BAUDRATE + (SPI_ADDR_CONFIG_OFFSET * tty->index)), BYTE, FALSE);
	    ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), BYTE);
	break;
	//Collision Detection | Duplex Select | Parity Mode Select. WRITE	
	case TIOWRITE_CD_DS_PM:
	    ret = copy_from_user(bufTX, (const u8 __user *)uarg, BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_CD_DS_PM + (SPI_ADDR_CONFIG_OFFSET * tty->index)), BYTE, TRUE);

	break;
	//Collision Detection | Duplex Select | Parity Mode Select. READ	
	case TIOREAD_CD_DS_PM:
        ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_CD_DS_PM + (SPI_ADDR_CONFIG_OFFSET * tty->index)), BYTE, FALSE);
	    ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), BYTE);
	break;
	//RX Bus IDLE Timeout. WRITE	
	case TIOWRITE_RX_BUS_TOUT:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
        memcpy(&port->spi_timeout, bufTX, 2);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_RX_BUS_TOUT + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, TRUE);
	break;
	//RX Bus IDLE Timeout. READ	
	case TIOREAD_RX_BUS_TOUT:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_RX_BUS_TOUT + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	//LACE Bus IDLE Timeout. WRITE	
	case TIOWRITE_LACE_BUS_TOUT:
	    ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
    	ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_LACE_BUS_TOUT + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, TRUE);
	break;
	//LACE Bus IDLE Timeout. READ	
	case TIOREAD_LACE_BUS_TOUT:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_LACE_BUS_TOUT + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	//TX IRQ Threshold. WRITE	
	case TIOWRITE_TX_IRQ_TS:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_TX_IRQ_TS + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, TRUE);
	break;
	//TX IRQ Threshold. READ	
	case TIOREAD_TX_IRQ_TS:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_TX_IRQ_TS + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	//RX IRQ Threshold. WRITE	
	case TIOWRITE_RX_IRQ_TS:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_RX_IRQ_TS + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, TRUE);
	break;
	//RX IRQ Threshold. READ	
	case TIOREAD_RX_IRQ_TS:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_RX_IRQ_TS + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;		
	//IRQ Mask Config. WRITE	
	case TIOWRITE_IRQ_MASK:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_IRQ_MASK + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, TRUE);
	break;
	//IRQ Mask Config. READ	
	case TIOREAD_IRQ_MASK:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_IRQ_MASK + (SPI_ADDR_CONFIG_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	//UART Status
	case TIOREAD_IRQ_STATUS:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_IRQ_STATUS + (SPI_ADDR_STATUS_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), BYTE);
	break;
	case TIOREAD_TX_FIFO_L:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_TX_FIFO_L + (SPI_ADDR_STATUS_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	case TIOREAD_RX_FIFO_L:
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_ADDR_RX_FIFO_L + (SPI_ADDR_STATUS_OFFSET * tty->index)), TWO_BYTE, FALSE);
		ret = copy_to_user((u8 __user *)uarg, (bufRX + SPI_UART_HAEDER), TWO_BYTE);
	break;
	case TIOWRITE_LOOPBACK_MODE:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_MAIN_CONF_LB_MODE), TWO_BYTE, TRUE);
	break;
	case TIOWRITE_LOOPBACK_UART:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, TWO_BYTE);
		ret = spi_uart_readwrite(tty, bufTX, bufRX, (SPI_MAIN_CONF_LB_UART + (2 * tty->index)), TWO_BYTE, TRUE);
	break;	
	//UART to SPI Read/Write Message	
	case TIOMESSAGE:
		ret = spi_uart_message(tty, arg);
	break;
	case TIOSPI:
		ret = copy_from_user(bufTXspi, (const u8 __user *)uarg, 128);
		ret = spi_rws_write(tty, bufTXspi, 128);
	break;
	case TIO_SPI_SPEED:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 4);
    	memcpy(&port->spiSpeed, bufTX, 4);
	break;
	case TIO_PORT_POLL:
    	ret = spi_poll_message(tty, arg, port, 0);
	break;
	case TIO_KERNEL_POLL:
    	ret = spi_kern_poll(tty, arg, port, 0);
	break;
	case TIO_DRIVER_TYP:
		ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
       	memcpy(&driver->DrTyp, bufTX, 1);
	break;
	case TIO_PORT_TYP_W:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
    	memcpy(&port->PortTypW, bufTX, 1);
		break;
	case TIO_PORT_TYP_R:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
    	memcpy(&port->PortTypR, bufTX, 1);
	break;
	case TIO_PORT_TYP_RSYNC:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
    	memcpy(&port->PortTypRsync, bufTX, 1);
	break;
	case TIO_PORT_TYP_RASYNC:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
    	memcpy(&port->PortTypRasync, bufTX, 1);
	break;
	case TIO_PORT_TYP_WSYNC:
    	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
    	memcpy(&port->PortTypWsync, bufTX, 1);
	break;
	case TIO_PORT_TYP_WASYNC:
     	ret = copy_from_user(bufTX, (const u8 __user *)uarg, 1);
	  	memcpy(&port->PortTypWasync, bufTX, 1);
	break;
	case TIO_PORTS_STATUS:
		bufSpiStatusPorts = kmalloc(128, GFP_KERNEL);
		if (!bufSpiStatusPorts) {
			//
		}
		ttyStatusPorts = kcalloc(16, sizeof(*ttyStatusPorts), GFP_KERNEL);
		ttyStatusTmpPorts = ttyStatusPorts;
		
		memset(bufSpiStatusPorts, 0, 128);
		ret = spi_uart_read_allstatus(tty, bufSpiStatusPorts, SPI_ADDR_IRQ_STATUS, 128, port->spiSpeed);
		if (ret < 0)
		{
			//
		}
		else
		{
			ttyStatusPorts = ttyStatusTmpPorts;
			for (i = 0; i < 16; i++, ttyStatusPorts++) {
				ttyStatusPorts->u16irqStatus = (uint16_t)((bufSpiStatusPorts[0 + i * 8] << 8) | (bufSpiStatusPorts[1 + i * 8]));
				ttyStatusPorts->u16TxFIFOLev = (uint16_t)((bufSpiStatusPorts[2 + i * 8] << 8) | (bufSpiStatusPorts[3 + i * 8]));
				ttyStatusPorts->u16RxFIFOLev = (uint16_t)((bufSpiStatusPorts[4 + i * 8] << 8) | (bufSpiStatusPorts[5 + i * 8]));
				ttyStatusPorts->u8res1 = bufSpiStatusPorts[6 + i * 8];
				ttyStatusPorts->u8res2 = bufSpiStatusPorts[7 + i * 8];
			}
		}
		ttyStatusPorts = ttyStatusTmpPorts;

		ret = copy_to_user((u8 __user *)uarg, ttyStatusPorts, 128);

		kfree(ttyStatusTmpPorts);
		kfree(bufSpiStatusPorts);

	break;
	case TIOCSSERIAL:
		down_write(&tty->termios_rwsem);
		ret = uart_set_info_user(tty, state, uarg);
		up_write(&tty->termios_rwsem);
		break;

	case TIOCSERCONFIG:
		down_write(&tty->termios_rwsem);
		ret = uart_do_autoconfig(tty, state);
		up_write(&tty->termios_rwsem);
		break;

	case TIOCSERGWILD: /* obsolete */
	case TIOCSERSWILD: /* obsolete */
		ret = 0;
		break;
	}

	if (ret != -ENOIOCTLCMD)
		goto out;

	if (tty_io_error(tty)) {
		ret = -EIO;
		goto out;
	}

	/*
	 * The following should only be used when hardware is present.
	 */
	switch (cmd) {
	case TIOCMIWAIT:
		ret = uart_wait_modem_status(state, arg);
		break;
	}

	if (ret != -ENOIOCTLCMD)
		goto out;

	mutex_lock(&port->mutex);
	uport = uart_port_check(state);

	if (!uport || tty_io_error(tty)) {
		ret = -EIO;
		goto out_up;
	}

	/*
	 * All these rely on hardware being present and need to be
	 * protected against the tty being hung up.
	 */

	switch (cmd) {
	case TIOCSERGETLSR: /* Get line status register */
		ret = uart_get_lsr_info(tty, state, uarg);
		break;

	case TIOCGRS485:
		ret = uart_get_rs485_config(uport, uarg);
		break;

	case TIOCSRS485:
		ret = uart_set_rs485_config(uport, uarg);
		break;
	default:
		if (uport->ops->ioctl)
			ret = uport->ops->ioctl(uport, cmd, arg);
		break;
	}
out_up:
	mutex_unlock(&port->mutex);
out:
  	kfree(bufTX);
 	kfree(bufRX);
  	kfree(bufTXspi);
  	kfree(bufRXspi);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_set_ldisc(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *uport;
	
	mutex_lock(&state->port.mutex);
	uport = uart_port_check(state);
	if (uport && uport->ops->set_ldisc)
		uport->ops->set_ldisc(uport, &tty->termios);
	mutex_unlock(&state->port.mutex);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_set_termios(struct tty_struct *tty,
						struct ktermios *old_termios)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *uport;
	unsigned int cflag = tty->termios.c_cflag;
	unsigned int iflag_mask = IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK;
	bool sw_changed = false;

	mutex_lock(&state->port.mutex);
	uport = uart_port_check(state);
	if (!uport)
		goto out;

	/*
	 * Drivers doing software flow control also need to know
	 * about changes to these input settings.
	 */
	if (uport->flags & UPF_SOFT_FLOW) {
		iflag_mask |= IXANY|IXON|IXOFF;
		sw_changed =
		   tty->termios.c_cc[VSTART] != old_termios->c_cc[VSTART] ||
		   tty->termios.c_cc[VSTOP] != old_termios->c_cc[VSTOP];
	}

	/*
	 * These are the bits that are used to setup various
	 * flags in the low level driver. We can ignore the Bfoo
	 * bits in c_cflag; c_[io]speed will always be set
	 * appropriately by set_termios() in tty_ioctl.c
	 */
	if ((cflag ^ old_termios->c_cflag) == 0 &&
	    tty->termios.c_ospeed == old_termios->c_ospeed &&
	    tty->termios.c_ispeed == old_termios->c_ispeed &&
	    ((tty->termios.c_iflag ^ old_termios->c_iflag) & iflag_mask) == 0 &&
	    !sw_changed) {
		goto out;
	}

	uart_change_speed(tty, state, old_termios);
	/* reload cflag from termios; port driver may have overriden flags */
	cflag = tty->termios.c_cflag;

	/* Handle transition to B0 status */
	if ((old_termios->c_cflag & CBAUD) && !(cflag & CBAUD))
		uart_clear_mctrl(uport, TIOCM_RTS | TIOCM_DTR);
	/* Handle transition away from B0 status */
	else if (!(old_termios->c_cflag & CBAUD) && (cflag & CBAUD)) {
		unsigned int mask = TIOCM_DTR;
		if (!(cflag & CRTSCTS) || !tty_throttled(tty))
			mask |= TIOCM_RTS;
		uart_set_mctrl(uport, mask);
	}
out:
	mutex_unlock(&state->port.mutex);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_close(struct tty_struct *tty, struct file *filp)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port;

	if (!state) {
		struct uart_driver *drv = tty->driver->driver_state;

		state = drv->state + tty->index;
		port = &state->port;
		spin_lock_irq(&port->lock);
		--port->count;
		spin_unlock_irq(&port->lock);
		return;
	}

	port = &state->port;
	pr_debug("uart_close(%d) called\n", tty->index);

	spi_port_close(tty->port, tty, filp);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_wait_until_sent(struct tty_struct *tty, int timeout)
{
	struct uart_state *state = tty->driver_data;
	struct uart_port *port;
	unsigned long char_time, expire;

	port = uart_port_ref(state);
	if (!port)
		return;

	if (port->type == PORT_UNKNOWN || port->fifosize == 0) {
		uart_port_deref(port);
		return;
	}

	/*
	 * Set the check interval to be 1/5 of the estimated time to
	 * send a single character, and make it at least 1.  The check
	 * interval should also be less than the timeout.
	 *
	 * Note: we have to use pretty tight timings here to satisfy
	 * the NIST-PCTS.
	 */
	char_time = (port->timeout - HZ/50) / port->fifosize;
	char_time = char_time / 5;
	if (char_time == 0)
		char_time = 1;
	if (timeout && timeout < char_time)
		char_time = timeout;

	/*
	 * If the transmitter hasn't cleared in twice the approximate
	 * amount of time to send the entire FIFO, it probably won't
	 * ever clear.  This assumes the UART isn't doing flow
	 * control, which is currently the case.  Hence, if it ever
	 * takes longer than port->timeout, this is probably due to a
	 * UART bug of some kind.  So, we clamp the timeout parameter at
	 * 2*port->timeout.
	 */
	if (timeout == 0 || timeout > 2 * port->timeout)
		timeout = 2 * port->timeout;

	expire = jiffies + timeout;

	pr_debug("uart_wait_until_sent(%d), jiffies=%lu, expire=%lu...\n",
		port->line, jiffies, expire);

	/*
	 * Check whether the transmitter is empty every 'char_time'.
	 * 'timeout' / 'expire' give us the maximum amount of time
	 * we wait.
	 */
	while (!port->ops->tx_empty(port)) {
		msleep_interruptible(jiffies_to_msecs(char_time));
		if (signal_pending(current))
			break;
		if (time_after(jiffies, expire))
			break;
	}
	uart_port_deref(port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_hangup(struct tty_struct *tty)
{
	struct uart_state *state = tty->driver_data;
	struct tty_port *port = &state->port;
	struct uart_port *uport;
	unsigned long flags;

	pr_debug("uart_hangup(%d)\n", tty->index);

	mutex_lock(&port->mutex);
	uport = uart_port_check(state);
	WARN(!uport, "hangup of detached port!\n");

	if (tty_port_active(port)) {
		uart_flush_buffer(tty);
		uart_shutdown(tty, state);
		spin_lock_irqsave(&port->lock, flags);
		port->count = 0;
		spin_unlock_irqrestore(&port->lock, flags);
		tty_port_set_active(port, 0);
		tty_port_tty_set(port, NULL);
		if (uport && !uart_console(uport))
			uart_change_pm(state, UART_PM_STATE_OFF);
		wake_up_interruptible(&port->open_wait);
		wake_up_interruptible(&port->delta_msr_wait);
	}
	mutex_unlock(&port->mutex);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_open(struct tty_struct *tty, struct file *filp)
{
	struct uart_driver *drv = tty->driver->driver_state;
	int retval, line = tty->index;
	struct uart_state *state = drv->state + line;

	tty->driver_data = state;

	retval = spi_port_open(&state->port, tty, filp);
	if (retval > 0)
		retval = 0;

	return retval;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_poll_init(struct tty_driver *driver, int line, char *options)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct tty_port *tport;
	struct uart_port *port;
	int baud = 9600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	int ret = 0;

	tport = &state->port;
	mutex_lock(&tport->mutex);

	port = uart_port_check(state);
	if (!port || !(port->ops->poll_get_char && port->ops->poll_put_char)) {
		ret = -1;
		goto out;
	}

	if (port->ops->poll_init) {
		/*
		 * We don't set initialized as we only initialized the hw,
		 * e.g. state->xmit is still uninitialized.
		 */
		if (!tty_port_initialized(tport))
			ret = port->ops->poll_init(port);
	}

	if (!ret && options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
		ret = uart_set_options(port, NULL, baud, parity, bits, flow);
	}
out:
	mutex_unlock(&tport->mutex);
	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static int spi_uart_poll_get_char(struct tty_driver *driver, int line)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct uart_port *port;
	int ret = -1;
  
	port = uart_port_ref(state);
	if (port) {
		ret = port->ops->poll_get_char(port);
		uart_port_deref(port);
	}

	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static void spi_uart_poll_put_char(struct tty_driver *driver, int line, char ch)
{
	struct uart_driver *drv = driver->driver_state;
	struct uart_state *state = drv->state + line;
	struct uart_port *port;

	port = uart_port_ref(state);
	if (!port)
		return;

	if (ch == '\n')
		port->ops->poll_put_char(port, '\r');
	port->ops->poll_put_char(port, ch);
	uart_port_deref(port);
}
//UART-SPI -------------------------------------------------------------------------------------<

...

//UART-SPI ------------------------------------------------------------------------------------->
static const struct tty_operations spi_uart_ops = {
	.open		= spi_uart_open,
	.close		= spi_uart_close,
	.read		= spi_uart_read,
	.write		= spi_uart_write,
	.put_char	= spi_uart_put_char,
	.flush_chars	= spi_uart_flush_chars,
	.write_room	= spi_uart_write_room,
	.chars_in_buffer= spi_uart_chars_in_buffer,
	.flush_buffer	= spi_uart_flush_buffer,
	.ioctl		= spi_uart_ioctl,
	.throttle	= spi_uart_throttle,
	.unthrottle	= spi_uart_unthrottle,
	.send_xchar	= spi_uart_send_xchar,
	.set_termios	= spi_uart_set_termios,
	.set_ldisc	= spi_uart_set_ldisc,
	.stop		= spi_uart_stop,
	.start		= spi_uart_start,
	.hangup		= spi_uart_hangup,
	.break_ctl	= spi_uart_break_ctl,
	.wait_until_sent= spi_uart_wait_until_sent,
#ifdef CONFIG_PROC_FS
	.proc_fops	= &uart_proc_fops,
#endif
	.tiocmget	= spi_uart_tiocmget,
	.tiocmset	= spi_uart_tiocmset,
	.get_icount	= spi_uart_get_icount,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init	= spi_uart_poll_init,
	.poll_get_char	= spi_uart_poll_get_char,
	.poll_put_char	= spi_uart_poll_put_char,
#endif
};

static const struct tty_port_operations uart_port_ops = {
	.carrier_raised = uart_carrier_raised,
	.dtr_rts	= uart_dtr_rts,
	.activate	= uart_port_activate,
	.shutdown	= uart_tty_port_shutdown,
};
//UART-SPI -------------------------------------------------------------------------------------<

...

/**
 *	spi_uart_register_driver - register a driver with the uart core layer
 *	@drv: low level driver structure
 *
 *	Register a uart driver with the core driver.  We in turn register
 *	with the tty layer, and initialise the core driver per-port state.
 *
 *	We have a proc file in /proc/tty/driver which is named after the
 *	normal driver.
 *
 *	drv->port should be NULL, and the per-port structures should be
 *	registered using uart_add_one_port after this call has succeeded.
 */

 //UART-SPI ------------------------------------------------------------------------------------->
int spi_uart_register_driver(struct uart_driver *drv)
{
	struct tty_driver *normal;
	int i, retval;

	BUG_ON(drv->state);

	/*
	 * Maybe we should be using a slab cache for this, especially if
	 * we have a large number of ports to handle.
	 */
	drv->state = kzalloc(sizeof(struct uart_state) * drv->nr, GFP_KERNEL);
	if (!drv->state)
	{
		goto out;
  	}
	normal = alloc_tty_driver(drv->nr);
	if (!normal)
	{
		goto out_kfree;
	}

	drv->tty_driver = normal;

	normal->driver_name	= drv->driver_name;
	normal->name		= drv->dev_name;
	normal->major		= drv->major;
	normal->minor_start	= drv->minor;
	normal->type		= TTY_DRIVER_TYPE_SERIAL;
	normal->subtype		= SERIAL_TYPE_NORMAL;
	normal->init_termios	= spi_std_termios;
	normal->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	normal->init_termios.c_ispeed = normal->init_termios.c_ospeed = 9600;
	normal->flags		= TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	normal->driver_state    = drv;

	normal->filp = filp_open(device,O_RDWR,0);
  
	if (IS_ERR(normal->filp)) {
		//
	} 

	mutex_init(&normal->spi_read_lock);
	mutex_init(&normal->spi_write_lock);
	init_waitqueue_head(&normal->spi_waitTX);
	
	tty_set_operations(normal, &spi_uart_ops);

	/*
	 * Initialise the UART state(s).
	 */
	for (i = 0; i < drv->nr; i++) {
		struct uart_state *state = drv->state + i;
		struct tty_port *port = &state->port;

		tty_port_init(port);
		port->ops = &spi_uart_port_ops;
	}

	retval = spi_tty_register_driver(normal);
	if (retval >= 0)
	{
		/*Creating workqueue */
		//own_workqueue = create_workqueue("own_wq");
		spi_workqueue = create_workqueue("spi_queue");
		return retval;
  	}

	for (i = 0; i < drv->nr; i++)
		tty_port_destroy(&drv->state[i].port);
	put_tty_driver(normal);
out_kfree:
	kfree(drv->state);
out:
	return -ENOMEM;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

/**
 *	spi_uart_add_one_port - attach a driver-defined port structure
 *	@drv: pointer to the uart low level driver structure for this port
 *	@uport: uart port structure to use for this port.
 *
 *	This allows the driver to register its own uart_port structure
 *	with the core driver.  The main purpose is to allow the low
 *	level uart drivers to expand uart_port, rather than having yet
 *	more levels of structures.
 */

 //UART-SPI ------------------------------------------------------------------------------------->
int spi_uart_add_one_port(struct uart_driver *drv, struct uart_port *uport)
{
	struct uart_state *state;
	struct tty_port *port;
	int ret = 0;
	struct device *tty_dev;
	int num_groups;

	BUG_ON(in_interrupt());

	if (uport->line >= drv->nr)
		return -EINVAL;

	state = drv->state + uport->line;
	port = &state->port; 

	mutex_lock(&port_mutex);
	mutex_lock(&port->mutex);
	if (state->uart_port) {
		ret = -EINVAL;
		goto out;
	}

	/* Link the port to the driver state table and vice versa */
	atomic_set(&state->refcount, 1);
	init_waitqueue_head(&state->remove_wait);
	state->uart_port = uport;
	uport->state = state;

	state->pm_state = UART_PM_STATE_UNDEFINED;
	uport->cons = drv->cons;
	uport->minor = drv->tty_driver->minor_start + uport->line;
	uport->name = kasprintf(GFP_KERNEL, "%s%d", drv->dev_name,
				drv->tty_driver->name_base + uport->line);
	if (!uport->name) {
		ret = -ENOMEM;
		goto out;
	}
  
	/*
	 * If this port is a console, then the spinlock is already
	 * initialised.
	 */
	if (!(uart_console(uport) && (uport->cons->flags & CON_ENABLED))) {
		spin_lock_init(&uport->lock);
		lockdep_set_class(&uport->lock, &port_lock_key);
	}
	if (uport->cons && uport->dev)
		of_console_check(uport->dev->of_node, uport->cons->name, uport->line);

	uart_configure_port(drv, state, uport);

	port->console = uart_console(uport);

	num_groups = 2;
	if (uport->attr_group)
		num_groups++;

	uport->tty_groups = kcalloc(num_groups, sizeof(*uport->tty_groups),
				    GFP_KERNEL);
	if (!uport->tty_groups) {
		ret = -ENOMEM;
		goto out;
	}
	uport->tty_groups[0] = &tty_dev_attr_group;
	if (uport->attr_group)
		uport->tty_groups[1] = uport->attr_group;

	/*
	 * Register the port whether it's detected or not.  This allows
	 * setserial to be used to alter this port's parameters.
	 */
	tty_dev = spi_tty_port_register_device_attr_serdev(port, drv->tty_driver,
			uport->line, uport->dev, port, uport->tty_groups);
	if (likely(!IS_ERR(tty_dev))) {
		device_set_wakeup_capable(tty_dev, 1);
	} else {
		dev_err(uport->dev, "Cannot register tty device on line %d\n",
		       uport->line);
	}

	/*
	 * Ensure UPF_DEAD is not set.
	 */
	uport->flags &= ~UPF_DEAD;

 out:
	mutex_unlock(&port->mutex);
	mutex_unlock(&port_mutex);

	return ret;
}
//UART-SPI -------------------------------------------------------------------------------------<

...

