/*
  ******************************************************************************
	In diesem Projekt gilt:
	*=============================================================================
  *        SYSCLK(Hz)                             | 64000000
  *-----------------------------------------------------------------------------
  *        AHB Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB2 Prescaler                         | 1
  *-----------------------------------------------------------------------------
  *        APB1 Prescaler                         | 2
  *=============================================================================
  ******************************************************************************
*/

#include "stm32f30x_conf.h"

/******************************************************************************
************************** globale Variablen **********************************
******************************************************************************/
int IoT_select = 1; // 0 if UART, 1 if I2C

// Signal the main routine to transmit UART data
// only used in UART, in I2C it is triggered by Zedb.
int UART_sendRequest = 0; 

static int UART_addr = 0; // address of memory
static int UART_count = 0; // Number of memory bytes to transmit

uint8_t memory[7]; // Test memory of the sensor


/******************************************************************************
**************************** Deklarationen ************************************
******************************************************************************/
void initMem();
void button_init(void); 
void LED_init(void); 
void uart1_init(void); // IoT-Geraet, PB6 und PB7
void uart2_init(void); // PC-Interface
void timer17_init(void); // Sekuendlich
void i2c1_init(void); // Iot-Geraet
void main_init(void);
void IoT_reconfigure(int mode);	// Switcht zwischen UART (0) und I2C (1)

/******************************************************************************
*************************** Unterprogramme ************************************
******************************************************************************/

// Memory
void initMem()
{
	//ID
	memory[0] = 0x54; // id TUW
	memory[1] = 0x55;
	memory[2] = 0x57;
	//EP
	memory[3] = 0x61; // Test counter from a to p (0-15)
	memory[4] = 0xFF;
	memory[5] = 0xFF;
	memory[6] = 0xFF;
}

// Button
void button_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;	
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;     
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;   
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;       
	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// LED
void LED_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;        
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}

// Interface UART (PB6 and PB7)
void uart1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef  USART_InitStructure;	
	NVIC_InitTypeDef   NVIC_InitStructure;
	
		// Takt
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// Pins: see IoT_reconfigure

	USART_InitStructure.USART_BaudRate=9600;                                       // Baudrate = 9600
  USART_InitStructure.USART_WordLength=USART_WordLength_8b;                      // Datenbits = 8
  USART_InitStructure.USART_StopBits=USART_StopBits_1;                           // Stopbits = 1
  USART_InitStructure.USART_Parity=USART_Parity_No;                              // kein Paritybit
  USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;  // keine Hardware Flow Control
  USART_InitStructure.USART_Mode=USART_Mode_Tx | USART_Mode_Rx;                  // Einstellungen gelten für RX und TX
	USART_Init(USART1, &USART_InitStructure);
	//USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;            
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;       // Gruppenpriorität    (0=höchste)
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;              // Subgruppenpriorität (0=höchste)
	NVIC_Init(&NVIC_InitStructure);
}

// USART2: Debug-UART
void uart2_init(void)
{
	// Takt
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef  USART_InitStructure;	
	NVIC_InitTypeDef   NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	USART_InitStructure.USART_BaudRate=9600;                                       // Baudrate = 9600
  USART_InitStructure.USART_WordLength=USART_WordLength_8b;                      // Datenbits = 8
  USART_InitStructure.USART_StopBits=USART_StopBits_1;                           // Stopbits = 1
  USART_InitStructure.USART_Parity=USART_Parity_No;                              // kein Paritybit
  USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;  // keine Hardware Flow Control
  USART_InitStructure.USART_Mode=USART_Mode_Tx | USART_Mode_Rx;                  // Einstellungen gelten für RX und TX
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;        
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;       // Gruppenpriorität    (0=höchste)
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;              // Subgruppenpriorität (0=höchste)
	NVIC_Init(&NVIC_InitStructure);
}

// Interface I2C (PB6 and PB7)
void i2c1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// Clocks
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	// Pins: see IoT_reconfigure
	
	// I2C
	I2C_DeInit(I2C1);
	I2C_InitStructure.I2C_Timing=0;
	I2C_InitStructure.I2C_AnalogFilter=I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter=0;
	I2C_InitStructure.I2C_Mode=I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1=0x73;
	I2C_InitStructure.I2C_Ack=I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStructure);
	
	// Interrupt	
	I2C_ITConfig(I2C1, I2C_IT_RXNE, ENABLE);
	I2C_ITConfig(I2C1, I2C_IT_ADDR, ENABLE);
	I2C_ITConfig(I2C1, I2C_IT_TXIS, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel=I2C1_EV_IRQn;        
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;       // Gruppenpriorität    (0=höchste)
  NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;              // Subgruppenpriorität (0=höchste)
	NVIC_Init(&NVIC_InitStructure);
	
	// Enable
	//I2C_Cmd(I2C1, ENABLE);
}

// 1s-Timer
void timer17_init(void)
{
	NVIC_InitTypeDef NVIC_NVICInitStructure; 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_Prescaler=7999; // 7999, 7999: sekündlich auslösen
	TIM_TimeBaseInitStructure.TIM_Period= 7999;
	TIM_TimeBaseInit(TIM17,&TIM_TimeBaseInitStructure);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_NVICInitStructure.NVIC_IRQChannel=TIM1_TRG_COM_TIM17_IRQn;           	 // TIM2 Channel
  NVIC_NVICInitStructure.NVIC_IRQChannelCmd=ENABLE;           
  NVIC_NVICInitStructure.NVIC_IRQChannelPreemptionPriority=0x02; // Gruppenpriorität    (0=höchste)
  NVIC_NVICInitStructure.NVIC_IRQChannelSubPriority=0x02;        // Subgruppenpriorität (0=höchste)
	NVIC_Init(&NVIC_NVICInitStructure);
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM17, ENABLE); 
}

// Initialisations
void main_init(void)
{
	button_init();
	LED_init();
	uart1_init();
	uart2_init();
	i2c1_init();
	timer17_init();
	initMem();
}

// Chooses between UART and I2C being active
void IoT_reconfigure(int mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// I2C
	if (mode == 1)
	{
		USART_Cmd(USART1, DISABLE);
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP; // Sicher?!
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_4);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_4);
		I2C_Cmd(I2C1, ENABLE);
	}
	
	// UART
	else
	{
		I2C_Cmd(I2C1, DISABLE);
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_7);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_7);
		USART_Cmd(USART1, ENABLE);
	}
}

/******************************************************************************
************************* Interrupt-Handler ***********************************
******************************************************************************/

// I2C1-Handler for interface
void I2C1_EV_IRQHandler()
{
	static uint8_t i = 0;
	static uint8_t status = 0;
	static int addr = 0;
	static int count = 0;
	
	// ACK received flag
	if(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_ADDR);
	}
	
	// Send Buffer needs to be filled
	if(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS))
	{
		static int trans_count = 0;
		
		// Transmit count-number of bytes
		if((trans_count < count))
		{
			I2C_SendData(I2C1, memory[addr]);
			addr++;
			trans_count++;
		}
		
		// Reset after transmission
		if(trans_count >= count)
		{
			trans_count = 0;
			count = 0;
		}
	}
	
	// Read byte
	if(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE))
	{
		int recvd;
		static int receiveBuf[30];
		recvd = I2C_ReceiveData(I2C1);	// Received byte
		
		/* First three bytes:
		*		identifier 0x7E
		*   memory address
		*   burst size
		*/
		if(status < 3)
		{
			receiveBuf[status] = recvd;
			status++;
		}
		if(status >= 3)
		{
			if(receiveBuf[0] == 0x7E) // Only react if first byte is identifier
			{
				addr = receiveBuf[1] - 0x30; // 0x30 to convert to numbers
				count = receiveBuf[2] - 0x30;
				status = 0;
			}
			else // if not -> retry
				status = 0;
		}
	}
}

// USART1-Handler for interface
void USART1_IRQHandler()
{
	int recvd;
	static int receiveBuf[30];
	static uint8_t i = 0;
	static uint8_t status = 0;
	recvd=USART_ReceiveData(USART1);	// Received byte
	
	if(status < 3)
	{
		
				/* First three bytes:
				*		identifier 0x7E
				*   memory address
				*   burst size
				*/
			if((recvd == 0x7E) || (status > 0)) // Only react if first byte is identifier
			{
				receiveBuf[status] = recvd;
				status++;
			}
	}

	if(status >= 3) // Start sending after received identifier, address and count
	{
		UART_addr = receiveBuf[1] - 0x30;
		UART_count = receiveBuf[2] - 0x30;
		status = 0;
		UART_sendRequest = 1;
	}
}

// USART2-Handler for debugging
void USART2_IRQHandler()
{
	int recvd;
	recvd=USART_ReceiveData(USART2);	// Received byte
}

// Button: Mode Switch I2C and UART
void EXTI15_10_IRQHandler()
{	
		if(EXTI_GetFlagStatus(EXTI_Line12)==SET || EXTI_GetFlagStatus(EXTI_Line13)==SET)
		{
			EXTI_ClearITPendingBit(EXTI_Line13);
			
			// Change mode
			IoT_select = (IoT_select + 1) % 2;
			IoT_reconfigure(IoT_select);
		}	

}

// Timer17-Handler, every second
void TIM1_TRG_COM_TIM17_IRQHandler(void)
{	
	if(memory[3] >= 0x70)
		memory[3] = 0x61;
	else
		memory[3]++; // Test sensor: counts every second from 0-15
	TIM_ClearITPendingBit(TIM17,TIM_IT_Update);  
}

/******************************************************************************
**************************** Hauptprogramm ************************************
******************************************************************************/

int main(void)
{
	//Initialisierungen
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	main_init();
	IoT_reconfigure(1); // Start with I2C
	while (1)
	{
		// Visualization of used protocol
		if(IoT_select == 0)
			GPIO_ResetBits(GPIOA, GPIO_Pin_5);
		else
			GPIO_SetBits(GPIOA, GPIO_Pin_5);
		
		// Transmit bytes if UART mode
		if(UART_sendRequest)
		{
			if(UART_count > 0)
			{
				while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)!=SET){};
        USART_SendData(USART1, memory[UART_addr]);
				UART_addr++;
				UART_count--;
			}
			else
			{
				while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)!=SET){};
        USART_SendData(USART1, 0xa);
				UART_sendRequest = 0;
			}
		}
	};
}


