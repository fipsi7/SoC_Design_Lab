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

int I2C_WriteByte(int device, Xuint8 addr, Xuint8 data);
int I2C_ReadByte(int device, Xuint8 addr, Xuint8 *data);

/************************** Variable Definitions *****************************/

/*
 * FD of the IIC device opened.
 */
int Fdiic;


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
	Xuint8 WriteBuffer[2];
	Xuint8 BytesWritten;

	/*
	 * Open the device.
	 */
	Fdiic = open("/dev/i2c-0", O_RDWR);
	if(Fdiic < 0)
	{
		printf("Cannot open the IIC device\n");
		return -1;
	}

	I2C_WriteByte(Fdiic, 0x39, 0x25);
	I2C_ReadByte(Fdiic, 0x39, WriteBuffer);
	printf("%d", WriteBuffer[0]);


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



