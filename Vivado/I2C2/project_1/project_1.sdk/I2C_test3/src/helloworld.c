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
int I2C_WriteByte(int device, Xuint8 addr, Xuint8 data);
int I2C_ReadByte(int device, Xuint8 addr, Xuint8 *data);

/************************** Variable Definitions *****************************/

/*
 * FD of the IIC device opened.
 */
int Fdiic;
int Fduart;


/**************************** Function Definitions ***************************/

/*****************************************************************************/
/**
*
* Entry point for integration tests on IIC.
*
* @param    .
*
* @return   0 if successful else -1.
*
* @note     None.
*
******************************************************************************/
int main()
{
	int Status;
	Xuint8 Buffer[30];
	Xuint8 BytesWritten;
	int i;
	char i2c_name[]="I2C.bit";
	char uart9600_name[]="UART9600.bit";

	flash_bitfile(i2c_name);

	/*
	 * Open the device.
	 */
	Fdiic = open("/dev/i2c-0", O_RDWR);
	if(Fdiic < 0)
	{
		printf("Cannot open the IIC device\n");
		return -1;
	}

	readBytes(Fdiic,0x39, 0x00, Buffer, 3, 0);
	close(Fdiic);
	for(i = 0; i<3; i++)
	{
		printf("%x", Buffer[i]);
	}

	/*sleep(1);
	flash_bitfile(uart9600_name);
	sleep(1);

	Fduart = open("/dev/ttyUL1", O_RDWR);
	if(Fduart < 0)
	{
		printf("Cannot open the UART device\n");
		return -1;
	}

	WriteBuffer[0]=0xa;
	WriteBuffer[1]='\n';
	write(Fduart, WriteBuffer, 2);
	close(Fduart);*/
	printf("Done");

	return 0;
}

// protocol: 0=I2C
int readBytes(int device,int i2c_addr, uint8_t adr, uint8_t result[], uint8_t count, uint8_t protocol)
{
	uint8_t frameBuffer[30];
	uint8_t readBuffer[30];
	//Command Write Bytes
	frameBuffer[0] = 0x0F;
	frameBuffer[1] = adr;
	frameBuffer[2] = count;
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

int transRecvBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, uint8_t receiveBuf[], uint8_t receiveCount, uint8_t protocol)
{
	transmitBytes(device, i2c_addr, sendBuf, sendCnt, protocol);
	return receiveBytes(device, i2c_addr, receiveBuf, receiveCount, protocol);
}

void transmitBytes(int device, int i2c_addr, uint8_t sendBuf[], uint8_t sendCnt, Xuint8 protocol)
{
	int Status;

	// I2C
	if(!protocol)
	{
		Status = ioctl(device, I2C_SLAVE_FORCE, i2c_addr);
		if(Status < 0)
		{
			printf("\n NACK\n");
			return;
		}
		Status = write(device, sendBuf, sendCnt);
		if(Status < 0)
		{
			printf("\n Write failed\n");
			return;
		}
	}

	// UART
	else
	{

	}
}

int receiveBytes(int device, int i2c_addr, uint8_t receiveBuf[], uint8_t cnt, Xuint8 protocol)
{
	int Status;

	//I2C
	if(!protocol)
	{
		Status = ioctl(device, I2C_SLAVE_FORCE, i2c_addr);
		if(Status < 0)
		{
			printf("\n NACK\n");
			return 1;
		}

		Status = read(device, receiveBuf, cnt);
		if(Status < 0)
		{
			printf("\n Read failed\n");
			return 1;
		}
	}
	else
	{ int timeout_s;int msCount;
		while(msCount < timeout_s * 1000)
		{
			//if(readBytesAvailable() > = cnt)
			{
				//Fill with I2C or UART
				//Read bytes
				return 0;
			}
			//else
			{
				delay_ms(10);
				msCount += 10;
			}
		}
	}
	return 0;
}


int I2C_WriteByte(int device, Xuint8 addr, Xuint8 data)
{
	int Status;
	Xuint8 WriteBuf[1];
	WriteBuf[0] = data;

	Status = ioctl(device, I2C_SLAVE_FORCE, addr);
	if(Status < 0)
	{
		printf("\n NACK\n");
		return 1;
	}

	Status = write(device, WriteBuf, 1);
	if(Status < 0)
	{
		printf("\n Write failed\n");
		return 1;
	}
	return 0;
}

int I2C_ReadByte(int device, Xuint8 addr, Xuint8 *data)
{
	int Status;

	Status = ioctl(device, I2C_SLAVE_FORCE, addr);
	if(Status < 0)
	{
		printf("\n NACK\n");
		return 1;
	}

	Status = read(device, data, 1);
	if(Status < 0)
	{
		printf("\n Read failed\n");
		return 1;
	}
	return 0;
}


