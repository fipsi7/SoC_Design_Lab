/*****************************************************************************/
/**
* @file i2c_test.c
* This file contains the linux example for i2c.
* Note:	10-bit addressing is not supported in the current linux driver.
* Repeated start also not supported in the current linux driver.
*
*
*
* @note     None.
*
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <fcntl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include "flash.h"
#include <libgpio.h>
#include <sys/ioctl.h>

/************************** Constant Definitions *****************************/

#define I2C_SLAVE_FORCE 0x0706
#define I2C_SLAVE	0x0703	/* Change slave address			*/
#define I2C_FUNCS	0x0705	/* Get the adapter functionality */
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one stop only)*/

#define MUX_ADDR			0x74
#define EEPROM_ADDR			0x54
#define MUX_CHANNEL_KINTEX7		0x04

/*
 * Page size of the EEPROM. This depends on the type of the EEPROM available
 * on board.
 */
#define PAGESIZE							16

/**************************** Type Definitions *******************************/

typedef unsigned char   Xuint8;
typedef char            Xint8;      /**< signed 8-bit */
typedef unsigned short  Xuint16;    /**< unsigned 16-bit */
typedef short           Xint16;     /**< signed 16-bit */
typedef unsigned long   Xuint32;    /**< unsigned 32-bit */
typedef long            Xint32;     /**< signed 32-bit */
typedef float           Xfloat32;   /**< 32-bit floating point */
typedef double          Xfloat64;   /**< 64-bit double precision floating point */
typedef unsigned long   Xboolean;   /**< boolean (XTRUE or XFALSE) */
typedef  Xuint8 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int readBytes(int device, int i2c_addr, uint8_t adr, uint8_t result[], uint8_t count, uint8_t protocol);
int transRecvBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, uint8_t receiveBuf[], uint8_t receiveCount, uint8_t protocol);
void transmitBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, Xuint8 protocol);
int receiveBytes(int device, int i2c_addr, uint8_t receiveBuf[], uint8_t cnt, Xuint8 protocol);
void red(void);
void resetcol(void);
void green(void);

/************************** Variable Definitions *****************************/

/*
 * FD of the IIC device opened.
 */
int Fd;


/**************************** Function Definitions ***************************/


int main()
{
	Xuint8 Buffer[30]; // Send and receive buffer
	int i;
	int protocol = 1; //0: I2C, 1: UART
	int found = 0; // Flag indicating detected protocol
	char bitnames[2][32]={"I2C.bit", "UART9600.bit"}; // Bit-Files for dyn. rec.of PL
	GPIO buttons;
	GPIO leds;

	// LED and Button configuration
	buttons = GPIO_init(0, 0);
	leds = GPIO_init(1, 0);
	setPinMode(leds,1,1,0);
	setPinMode(leds,1,2,0);
	setPinMode(leds,1,3,0);
	setPinMode(leds,1,4,0);

	// UART driver configuration
	system("stty -F /dev/ttyUL1 -echo raw");
	usleep(10000);


	while(!digitalRead(buttons, 2, 1)) // Please press BTNC to exit
	{
		usleep(1000000);

		// Search for protocol until detected
		while(!found)
		{

			// Exit button pressed
			if(digitalRead(buttons, 2, 1))
			{
				GPIO_Close(leds);
				GPIO_Close(buttons);
				close(Fd);
				printf("\nreturn 0\n\n");
				return 0;
			}

			// Switch protocol every try
			protocol = (protocol + 1) % 2;

			printf("\n");

			// DYNAMIC RECONFIGURATION to next protocol
			if(flash_bitfile(bitnames[protocol]))
			{
				GPIO_Close(leds);
				GPIO_Close(buttons);
				close(Fd);
				printf("\nreturn 1\n\n");
				return 1;
			}

			// Open file descriptor
			if(protocol == 1)
			{
				Fd = open("/dev/ttyUL1", O_RDWR);
			}
			else
			{
				Fd = open("/dev/i2c-0", O_RDWR);
			}
			if(Fd < 0)
			{
				printf("Cannot open file descriptor\n");
				{
					GPIO_Close(leds);
					GPIO_Close(buttons);
					close(Fd);
					printf("\nreturn 1\n\n");
					return 1;
				}
			}

			// Detection of id "TUW" (address 0x00), I2C address: 0x39
			if(!readBytes(Fd,0x39, 0x00, Buffer, 3, protocol))
			{
				if((Buffer[0] == 'T') && (Buffer[1] == 'U') && (Buffer[2] == 'W'))
				{
					found = 1;
					green();
					printf("\nProtocol %s found!\n", protocol == 1 ? "UART" : "I2C");
					resetcol();
				}
			}
		}
		found = 0; // Reset flag if not found

		// Set protocol LEDs
		digitalWrite(leds,1,7,protocol);
		digitalWrite(leds,1,8,!protocol);

		usleep(1000000);

		// Continuously receive test data
		while(!readBytes(Fd,0x39, 0x03, Buffer, 1, protocol))
		{

			// Exit button
			if(digitalRead(buttons, 2, 1))
			{
				GPIO_Close(leds);
				GPIO_Close(buttons);
				close(Fd);
				printf("\nreturn 0\n\n");
				return 0;
			}

			// Visualize received value
			Buffer[0]-=0x61;
			printf("\n%d", Buffer[0]);
			digitalWrite(leds,1,1,(Buffer[0]>>0x00)&0x01);
			digitalWrite(leds,1,2,(Buffer[0]>>0x01)&0x01);
			digitalWrite(leds,1,3,(Buffer[0]>>0x02)&0x01);
			digitalWrite(leds,1,4,(Buffer[0]>>0x03)&0x01);

			usleep(100000);
		}

		close(Fd);
	}
	GPIO_Close(leds);
	GPIO_Close(buttons);
	printf("\nreturn 0\n\n");
	return 0;
}

// protocol: 0=I2C, 1=UART
int readBytes(int device,int i2c_addr, uint8_t adr, uint8_t result[], uint8_t count, uint8_t protocol)
{
	uint8_t frameBuffer[30];
	uint8_t readBuffer[30];

	//Command Write Bytes
	frameBuffer[0] = 0x7E;
	frameBuffer[1] = adr + 0x30;
	frameBuffer[2] = count + 0x30;

	// Transmit address and receive values
	if(transRecvBytes(device, i2c_addr, frameBuffer, 3, readBuffer, count, protocol) == 0)
	{
		for (int i = 0; i < count; i++)
		{
			result[i] = readBuffer[i];
		}
		return 0;
	}
	return 1;
}

// Transmit address and receive values
int transRecvBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, uint8_t receiveBuf[], uint8_t receiveCount, uint8_t protocol)
{
	transmitBytes(device, i2c_addr, sendBuf, sendCnt, protocol);
	return receiveBytes(device, i2c_addr, receiveBuf, receiveCount, protocol);
}

// Transmit
void transmitBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, Xuint8 protocol)
{
	int Status;

	// I2C
	if(!protocol)
	{
		Status = ioctl(device, I2C_SLAVE_FORCE, i2c_addr); // configure I2C address
		if(Status < 0)
		{
			red();
			printf("\n NACK\n");
			resetcol();
			return;
		}
		Status = write(device, sendBuf, sendCnt); // send
		if(Status < 0)
		{
			red();
			printf("\n I2C Write failed\n");
			resetcol();
			return;
		}
	}

	// UART
	else
	{
		Status = write(device, sendBuf, sendCnt); // send
		if(Status < 0)
		{
			red();
			printf("\n UART Write failed\n");
			resetcol();
			return;
		}
	}
}

// Receive
int receiveBytes(int device, int i2c_addr, uint8_t receiveBuf[], uint8_t cnt, Xuint8 protocol)
{
	int Status;

	//I2C
	if(!protocol)
	{
		Status = ioctl(device, I2C_SLAVE_FORCE, i2c_addr); // configure address
		if(Status < 0)
		{
			red();
			printf("\n NACK\n");
			resetcol();
			return 1;
		}

		Status = read(device, receiveBuf, cnt); // receive
		if(Status < 0)
		{
			red();
			printf("\n I2C Read failed\n");
			resetcol();
			return 1;
		}
	}
	else
	{

		int timeout_ms = 200; // Timeout: 200ms
		int msCount = 0;
		int bytes_available;

		// UART-Timeout
		while(msCount < timeout_ms)
		{
			ioctl(device, FIONREAD, &bytes_available); // detect how many bytes are available in UART

			// If enough bytes are received
			if(bytes_available >= (cnt + 1))
			{
				char trashBuf[1];
				//Read bytes
				Status = read(device, receiveBuf, cnt); // read
				if(Status < 0)
				{
					red();
					printf("\n UART Read failed\n");
					resetcol();
					return 1;
				}
				Status = read(device, trashBuf, 1);  // Dump received CR
				return 0;
			}

			usleep(10000);
			msCount += 10; // Increase time

		}

		// Timeout
		red();
		printf("\nUART Timeout\n");
		resetcol();
		return 1;

	}
	return 0;
}


// Helper functions for visualization
void red(void)
{
	printf("\033[1;31m");
}

void resetcol(void)
{
	printf("\033[0m");
}

void green(void)
{
	printf("\033[1;32m");
}




