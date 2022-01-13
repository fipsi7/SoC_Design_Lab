/*
 * libuart.h
 *
 *  Created on: 13.01.2022
 *      Author: andi
 */

#ifndef SRC_LIBUART_H_
#define SRC_LIBUART_H_



#include <stdint.h>
typedef uint8_t byte;
typedef void * GPIO;

/**********************HARDWARE REGISTER OFFSETS************************/
#define CHANNEL_1_DATA		0x00
#define CHANNEL_1_DIRECTION	0x04
#define CHANNEL_2_DATA		0x08
#define CHANNEL_2_DIRECTION	0x0C

/************************OPERATIONAL MASKS****************************/
#define INPUT				1
#define OUTPUT				0
#define HIGH				1
#define LOW					0

#define CHANNEL_MAX_SIZE		32
#define CHANNEL_MIN_SIZE		1

/**************************HELPFUL MACROS****************************/



/**************************FUNC DEFITIONS****************************/
GPIO GPIO_init(uint8_t uioNum, uint8_t mapNum);
int8_t setPinMode(GPIO vm, uint8_t channel, int8_t pinNum, int8_t direction);
int8_t digitalWrite(GPIO vm, uint8_t channel, int8_t pinNum, int8_t value);
int8_t digitalRead(GPIO vm, uint8_t channel, int8_t pinNum);
int8_t setChannelValue(GPIO vm, uint32_t valMask, int8_t channel);
uint32_t readChannelValue(GPIO vm, int8_t channel);
int8_t setChannelDirection(GPIO vm, uint32_t dirMask, int8_t channel);
uint8_t GPIO_Close(GPIO vm);



#endif /* SRC_LIBUART_H_ */
